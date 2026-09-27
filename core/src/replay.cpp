#include "janus/replay.hpp"
#include <charconv>
#include <map>
#include <locale>
#include <type_traits>
#include <sstream>

namespace janus {
namespace {
[[noreturn]] void bad(const std::string& message) { throw std::invalid_argument("Replay: " + message); }
struct Json {
    enum Kind { object, array, string, number, null } kind = null;
    std::map<std::string, Json> fields;
    std::vector<Json> items;
    std::string text;
    std::uint64_t integer{};
};
class Parser {
public:
    explicit Parser(std::string_view s) : source(s) {}
    Json parse() { auto j = value(0); whitespace(); if (pos != source.size()) bad("trailing input"); return j; }
private:
    std::string_view source;
    std::size_t pos{};
    void whitespace() { while (pos < source.size() && (source[pos]==' ' || source[pos]=='\n' || source[pos]=='\r' || source[pos]=='\t')) ++pos; }
    bool take(char c) { whitespace(); if (pos < source.size() && source[pos] == c) { ++pos; return true; } return false; }
    void need(char c) { if (!take(c)) bad("unexpected token at " + std::to_string(pos)); }
    unsigned hex4() {
        unsigned x = 0;
        for (int i=0; i<4; ++i) {
            if (pos == source.size()) bad("truncated Unicode escape");
            const char c = source[pos++]; unsigned d;
            if (c >= '0' && c <= '9') d=c-'0'; else if (c >= 'a' && c <= 'f') d=c-'a'+10;
            else if (c >= 'A' && c <= 'F') d=c-'A'+10; else bad("invalid Unicode escape");
            x = x*16+d;
        }
        return x;
    }
    std::string str() {
        need('"'); std::string s;
        while (pos < source.size()) {
            const auto c = static_cast<unsigned char>(source[pos++]);
            if (c == '"') return s;
            if (c < 32) bad("control character in string");
            if (c != '\\') { s += static_cast<char>(c); continue; }
            if (pos == source.size()) bad("truncated escape");
            const char e = source[pos++];
            switch (e) {
            case '"': case '\\': case '/': s += e; break;
            case 'b': s += '\b'; break; case 'f': s += '\f'; break;
            case 'n': s += '\n'; break; case 'r': s += '\r'; break; case 't': s += '\t'; break;
            case 'u': {
                auto x=hex4();
                if (x>=0xd800 && x<=0xdbff) {
                    if (pos+2>source.size() || source.substr(pos,2)!="\\u") bad("missing low surrogate");
                    pos+=2; const auto low=hex4(); if (low<0xdc00 || low>0xdfff) bad("invalid low surrogate");
                    x=0x10000+((x-0xd800)<<10)+(low-0xdc00);
                } else if (x>=0xdc00 && x<=0xdfff) bad("unpaired surrogate");
                if (x<0x80) s+=static_cast<char>(x);
                else if (x<0x800) { s+=static_cast<char>(0xc0|(x>>6)); s+=static_cast<char>(0x80|(x&63)); }
                else if (x<0x10000) { s+=static_cast<char>(0xe0|(x>>12)); s+=static_cast<char>(0x80|((x>>6)&63)); s+=static_cast<char>(0x80|(x&63)); }
                else { s+=static_cast<char>(0xf0|(x>>18)); s+=static_cast<char>(0x80|((x>>12)&63)); s+=static_cast<char>(0x80|((x>>6)&63)); s+=static_cast<char>(0x80|(x&63)); }
                break;
            }
            default: bad("invalid escape");
            }
        }
        bad("unterminated string");
    }
    Json value(unsigned depth) {
        if (depth>32) bad("nesting too deep");
        whitespace(); Json j;
        if (pos == source.size()) bad("missing value");
        if (take('{')) {
            j.kind=Json::object;
            if (take('}')) return j;
            do { auto key=str(); need(':'); auto item=value(depth+1);
                if (!j.fields.emplace(key,std::move(item)).second) bad("duplicate key " + key);
            } while (take(','));
            need('}'); return j;
        }
        if (take('[')) {
            j.kind=Json::array;
            if (take(']')) return j;
            do { j.items.push_back(value(depth+1)); } while (take(','));
            need(']'); return j;
        }
        if (source[pos]=='"') { j.kind=Json::string; j.text=str(); return j; }
        if (source.substr(pos,4)=="null") { pos+=4; return j; }
        const auto start=pos;
        while (pos<source.size() && source[pos]>='0' && source[pos]<='9') ++pos;
        if (pos==start || (pos-start>1 && source[start]=='0')) bad("expected unsigned integer");
        j.kind=Json::number;
        const auto parsed=std::from_chars(source.data()+start,source.data()+pos,j.integer);
        if (parsed.ec!=std::errc{}) bad("integer overflow");
        return j;
    }
};
void fields(const Json& j, std::initializer_list<std::string_view> required, std::initializer_list<std::string_view> optional = {}) {
    if (j.kind!=Json::object) bad("expected object");
    for (auto key:required) if (!j.fields.contains(std::string(key))) bad("missing " + std::string(key));
    for (const auto& [key,v]:j.fields) {
        bool known=false; for (auto r:required) known |= key==r; for (auto o:optional) known |= key==o;
        if (!known) bad("unknown field " + key);
    }
}
const Json& at(const Json& j, const char* key) { return j.fields.at(key); }
std::uint64_t integer(const Json& j, std::uint64_t max) {
    if (j.kind!=Json::number || j.integer>max) bad("integer out of range or wrong type");
    return j.integer;
}
const std::string& string(const Json& j) { if (j.kind!=Json::string) bad("expected string"); return j.text; }
void validate(const Replay& r) {
    if (r.format_version!=1 || validate_config(r.config)!=ConfigError::none) bad("unsupported format/config");
    for (const auto& a:r.actions) {
        if (static_cast<unsigned>(a.actor)>1) bad("invalid actor");
        std::visit([](const auto& p) { if constexpr (requires { p.card; }) if (p.card.value>=24) bad("invalid card"); }, a.payload);
    }
    if (r.expected_result) {
        const auto& e=*r.expected_result;
        const bool win=e.outcome==Outcome::win && e.winner && static_cast<unsigned>(*e.winner)<2 && e.reason==ResultReason::zero_lives;
        const bool draw=e.outcome==Outcome::draw && !e.winner && e.reason==ResultReason::both_players_stuck;
        if (!win && !draw) bad("invalid expected result");
    }
}
}
Replay parse_replay(std::string_view input) {
    const auto j=Parser(input).parse();
    fields(j,{"format_version","config","seed","actions"},{"expected_result"});
    Replay r; r.format_version=static_cast<std::uint32_t>(integer(at(j,"format_version"),1));
    const auto& c=at(j,"config");
    fields(c,{"rules_version","starting_lives","initial_hand_size","board_capacity","deck_values"});
    r.config.rules_version=static_cast<std::uint32_t>(integer(at(c,"rules_version"),1));
    r.config.starting_lives=static_cast<std::uint8_t>(integer(at(c,"starting_lives"),255));
    r.config.initial_hand_size=static_cast<std::uint8_t>(integer(at(c,"initial_hand_size"),255));
    r.config.board_capacity=static_cast<std::uint8_t>(integer(at(c,"board_capacity"),255));
    const auto& deck=at(c,"deck_values");
    if (deck.kind!=Json::array || deck.items.size()!=12) bad("invalid deck");
    for (std::size_t i=0;i<12;++i) r.config.deck_values[i]=static_cast<std::uint8_t>(integer(deck.items[i],255));
    const auto& seed=string(at(j,"seed"));
    if (seed.empty() || (seed.size()>1 && seed[0]=='0')) bad("invalid seed");
    for (auto ch:seed) if (ch<'0' || ch>'9') bad("invalid seed");
    const auto converted=std::from_chars(seed.data(),seed.data()+seed.size(),r.seed);
    if (converted.ec!=std::errc{} || converted.ptr!=seed.data()+seed.size()) bad("seed overflow");
    const auto& actions=at(j,"actions"); if (actions.kind!=Json::array) bad("expected actions array");
    for (const auto& a:actions.items) {
        fields(a,{"actor","type"},{"card"});
        const auto actor=static_cast<PlayerId>(integer(at(a,"actor"),1)); const auto& type=string(at(a,"type"));
        if (type=="pass") { fields(a,{"actor","type"}); r.actions.push_back({actor,Pass{}}); }
        else {
            fields(a,{"actor","type","card"}); const CardId id{static_cast<std::uint16_t>(integer(at(a,"card"),23))};
            if (type=="play") r.actions.push_back({actor,Play{id}});
            else if (type=="attack") r.actions.push_back({actor,Attack{id}});
            else if (type=="defend") r.actions.push_back({actor,Defend{id}});
            else bad("unknown action type");
        }
    }
    if (j.fields.contains("expected_result")) {
        const auto& e=at(j,"expected_result"); fields(e,{"outcome","winner","reason"});
        GameResult result; const auto& outcome=string(at(e,"outcome")); const auto& reason=string(at(e,"reason"));
        if (outcome=="win") result.outcome=Outcome::win; else if (outcome=="draw") result.outcome=Outcome::draw; else bad("invalid outcome");
        if (reason=="zero_lives") result.reason=ResultReason::zero_lives; else if (reason=="both_players_stuck") result.reason=ResultReason::both_players_stuck; else bad("invalid reason");
        const auto& winner=at(e,"winner"); if (winner.kind!=Json::null) result.winner=static_cast<PlayerId>(integer(winner,1));
        r.expected_result=result;
    }
    validate(r); return r;
}
std::string encode_replay(const Replay& r) {
    validate(r); std::ostringstream out; out.imbue(std::locale::classic());
    out << "{\"format_version\":1,\"config\":{\"rules_version\":1,\"starting_lives\":3,\"initial_hand_size\":4,\"board_capacity\":3,\"deck_values\":[1,1,1,2,2,2,3,3,3,4,4,4]},\"seed\":\"" << r.seed << "\",\"actions\":[";
    bool first=true;
    for (const auto& a:r.actions) {
        if (!first) out << ','; first=false;
        out << "{\"actor\":" << static_cast<unsigned>(a.actor) << ",\"type\":\"";
        std::visit([&](const auto& p) {
            using T=std::decay_t<decltype(p)>;
            if constexpr (std::is_same_v<T,Play>) out << "play";
            else if constexpr (std::is_same_v<T,Attack>) out << "attack";
            else if constexpr (std::is_same_v<T,Defend>) out << "defend";
            else out << "pass";
            out << '"'; if constexpr (requires { p.card; }) out << ",\"card\":" << p.card.value;
        }, a.payload);
        out << '}';
    }
    out << ']';
    if (r.expected_result) {
        const auto& e=*r.expected_result;
        out << ",\"expected_result\":{\"outcome\":\"" << (e.outcome==Outcome::win?"win":"draw") << "\",\"winner\":";
        if (e.winner) out << static_cast<unsigned>(*e.winner); else out << "null";
        out << ",\"reason\":\"" << (e.reason==ResultReason::zero_lives?"zero_lives":"both_players_stuck") << "\"}";
    }
    out << '}'; return out.str();
}
ReplayExecutionError::ReplayExecutionError(std::size_t i, ActionError e)
    : std::invalid_argument("Replay: illegal action at index " + std::to_string(i) + ", core error " + std::to_string(static_cast<unsigned>(e))), action_index(i), action_error(e) {}
GameState execute_replay(const Replay& r, bool require_terminal) {
    validate(r); Game game(r.config); game.reset(r.seed);
    for (std::size_t i=0;i<r.actions.size();++i) {
        const auto step=game.step(r.actions[i]); if (step.error!=ActionError::none) throw ReplayExecutionError(i,step.error);
    }
    const auto s=game.snapshot();
    if (require_terminal && s.phase!=Phase::terminal) bad("incomplete match");
    if (r.expected_result && *r.expected_result!=s.result) bad("expected result mismatch");
    return s;
}
} // namespace janus
