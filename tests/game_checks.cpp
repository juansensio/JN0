#include "janus/replay.hpp"
#include "rng.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <locale>
#include <set>
#include <stdexcept>

using namespace janus;
namespace {
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> void invalid(F f) {
    bool threw=false; try { f(); } catch (const std::invalid_argument&) { threw=true; }
    check(threw,"Expected invalid_argument");
}
PlayerId other(PlayerId p) { return p==PlayerId::first ? PlayerId::second : PlayerId::first; }
std::size_t idx(PlayerId p) { return static_cast<std::size_t>(p); }
std::vector<unsigned> ids(const std::vector<Card>& cards) {
    std::vector<unsigned> result; for (const auto& c:cards) result.push_back(c.id.value); return result;
}
void reject(Game& g, Action a, ActionError e) {
    const auto before=g.snapshot(); const auto result=g.step(a);
    check(result.error==e,"Wrong action error precedence");
    check(result.result==before.result && g.snapshot()==before,"Rejected action mutated state");
}
void accept(Game& g, Action a) { check(g.step(a).error==ActionError::none,"Legal action rejected"); }
Card card(const std::vector<Card>& zone, CardId id) {
    for (auto c:zone) if (c.id==id) return c;
    throw std::runtime_error("Missing test card");
}
void remove(std::vector<Card>& zone, CardId id) { std::erase_if(zone,[&](auto c){return c.id==id;}); }
// Independently check each accepted transition against its pre-action state.
void transition(const GameState& b, const Action& a, const GameState& after) {
    auto expected=b; ++expected.action_count;
    auto& own=expected.players[idx(a.actor)]; auto& enemy=expected.players[idx(other(a.actor))];
    if (auto p=std::get_if<Play>(&a.payload)) {
        expected.consecutive_passes=0;
        own.board.push_back(card(own.hand,p->card)); remove(own.hand,p->card);
        if (!own.deck.empty()) { own.hand.push_back(own.deck.front()); own.deck.erase(own.deck.begin()); }
        expected.active_player=other(a.actor);
    } else if (auto p=std::get_if<Attack>(&a.payload)) {
        expected.consecutive_passes=0;
        if (!enemy.board.empty()) { expected.phase=Phase::awaiting_defense; expected.pending_attack=PendingAttack{a.actor,p->card}; }
        else if (--enemy.lives==0) { expected.phase=Phase::terminal; expected.result={Outcome::win,a.actor,ResultReason::zero_lives}; }
        else expected.active_player=other(a.actor);
    } else if (auto p=std::get_if<Defend>(&a.payload)) {
        expected.consecutive_passes=0;
        const auto attacking=card(enemy.board,b.pending_attack->card); const auto defending=card(own.board,p->card);
        if (attacking.value<=defending.value) { enemy.discard.push_back(attacking); remove(enemy.board,attacking.id); }
        if (defending.value<=attacking.value) { own.discard.push_back(defending); remove(own.board,defending.id); }
        expected.pending_attack.reset(); expected.phase=Phase::main; expected.active_player=a.actor;
    } else {
        if (++expected.consecutive_passes==2) { expected.phase=Phase::terminal; expected.result={Outcome::draw,std::nullopt,ResultReason::both_players_stuck}; }
        else expected.active_player=other(a.actor);
    }
    check(after==expected,"Transition has unexpected full-state effects");
}
void invariants(Game& g) {
    const auto s=g.snapshot(); std::set<unsigned> all;
    for (std::size_t i=0;i<2;++i) {
        const auto& p=s.players[i];
        check(p.lives<=3 && p.board.size()<=3,"Lives/board invariant");
        std::size_t count=0;
        for (const auto* zone:{&p.deck,&p.hand,&p.board,&p.discard}) for (auto c:*zone) {
            check(c.id.value/12==i && c.value==s.config.deck_values[c.id.value%12],"Card identity invariant");
            check(all.insert(c.id.value).second,"Duplicate card"); ++count;
        }
        check(count==12,"Lost card");
        auto o=g.observe(static_cast<PlayerId>(i));
        check(o.own_hand==p.hand && o.config==s.config && o.viewer==static_cast<PlayerId>(i),"Observation own data");
        check(o.active_player==s.active_player && o.phase==s.phase && o.pending_attack==s.pending_attack && o.result==s.result && o.action_count==s.action_count && o.consecutive_passes==s.consecutive_passes,"Observation lifecycle");
        for (std::size_t j=0;j<2;++j) {
            const auto& pub=o.players[j]; const auto& full=s.players[j];
            check(pub.lives==full.lives && pub.board==full.board && pub.discard==full.discard && pub.deck_count==full.deck.size() && pub.hand_count==full.hand.size(),"Observation public data");
        }
        o.own_hand.clear(); o.players[0].board.clear(); check(g.snapshot()==s,"Observation aliases game");
    }
    check((s.phase==Phase::awaiting_defense)==s.pending_attack.has_value(),"Pending attack invariant");
    if (s.pending_attack) {
        check(s.pending_attack->attacker==s.active_player,"Pending owner");
        (void)card(s.players[idx(s.active_player)].board,s.pending_attack->card);
    }
    const auto o=g.observe(PlayerId::first);
    if (s.phase==Phase::terminal) {
        check(!o.acting_player && g.legal_actions(PlayerId::first).empty() && g.legal_actions(PlayerId::second).empty(),"Terminal actor/actions");
        check(s.result.outcome!=Outcome::ongoing,"Terminal result");
    } else {
        const auto acting=s.phase==Phase::main?s.active_player:other(s.active_player);
        check(o.acting_player==acting && g.legal_actions(other(acting)).empty(),"Wrong actor/actions");
        const auto actions=g.legal_actions(acting); check(!actions.empty(),"No explicit action");
        unsigned previous=0; bool first=true; std::size_t payload_index=0;
        for (const auto& a:actions) {
            check(a.actor==acting,"Legal actor");
            unsigned id=0; std::visit([&](auto p){if constexpr(requires{p.card;}) id=p.card.value;},a.payload);
            check(first || a.payload.index()>payload_index || (a.payload.index()==payload_index && id>previous),"Legal ordering");
            previous=id; payload_index=a.payload.index(); first=false;
        }
    }
}
void setup_and_errors() {
    detail::SplitMix64 rng(0);
    check(rng.next()==0xe220a8397b1dcdafULL && rng.next()==0x6e789e6aa1b965f4ULL && rng.next()==0x06c45d188009454fULL,"RNG golden samples");
    // Large bound forces rejection for seed 3; verify stream consumption independently.
    detail::SplitMix64 bounded_rng(3), raw_rng(3);
    const auto n=(std::uint64_t{1}<<63)+1; const auto threshold=(std::uint64_t{0}-n)%n;
    auto sample=raw_rng.next(); unsigned rejected=0;
    while(sample<threshold) { ++rejected; sample=raw_rng.next(); }
    check(rejected>0 && bounded_rng.bounded(n)==sample%n && bounded_rng.next()==raw_rng.next(),"Bounded rejection/consumption");
    Game g; auto s=g.snapshot();
    check(s.seed==0 && s.active_player==PlayerId::first && s.action_count==0 && s.consecutive_passes==0 && s.phase==Phase::main,"Initial lifecycle");
    check(ids(s.players[0].hand)==std::vector<unsigned>{4,1,6,8} && ids(s.players[0].deck)==std::vector<unsigned>{0,5,2,3,11,9,10,7},"Player 0 golden shuffle");
    check(ids(s.players[1].hand)==std::vector<unsigned>{18,23,19,12} && ids(s.players[1].deck)==std::vector<unsigned>{17,20,16,15,14,13,21,22},"Player 1 golden shuffle");
    for (auto p:s.players) check(p.lives==3 && p.board.empty() && p.discard.empty() && p.hand.size()==4 && p.deck.size()==8,"Initial setup");
    const auto initial=s; s.players[0].hand.clear(); check(g.snapshot()==initial,"Snapshot aliases game");
    auto config=GameConfig{}; config.rules_version=2; invalid([&]{Game bad(config);});
    config=GameConfig{}; config.starting_lives=2; invalid([&]{Game bad(config);});
    const auto invalid_id=static_cast<PlayerId>(2);
    invalid([&]{(void)g.observe(invalid_id);}); invalid([&]{(void)g.legal_actions(invalid_id);});
    reject(g,{invalid_id,Defend{{99}}},ActionError::invalid_actor);
    reject(g,{PlayerId::second,Defend{{99}}},ActionError::wrong_actor);
    reject(g,{PlayerId::first,Defend{{99}}},ActionError::wrong_phase);
    reject(g,{PlayerId::first,Play{{24}}},ActionError::unknown_card);
    reject(g,{PlayerId::first,Play{{23}}},ActionError::wrong_zone);
    reject(g,{PlayerId::first,Play{{0}}},ActionError::wrong_zone);
    reject(g,{PlayerId::first,Attack{{4}}},ActionError::wrong_zone);
    reject(g,{PlayerId::first,Pass{}},ActionError::pass_not_allowed);
    accept(g,{PlayerId::first,Play{{6}}});
    check(ids(g.snapshot().players[0].hand)==std::vector<unsigned>{4,1,8,0} && ids(g.snapshot().players[0].board)==std::vector<unsigned>{6},"Play/draw ordering");
    accept(g,{PlayerId::second,Play{{23}}}); accept(g,{PlayerId::first,Attack{{6}}});
    check(g.snapshot().active_player==PlayerId::first && g.observe(PlayerId::first).acting_player==PlayerId::second,"Pending defense owner");
    reject(g,{PlayerId::first,Defend{{6}}},ActionError::wrong_actor);
    reject(g,{PlayerId::second,Play{{99}}},ActionError::wrong_phase);
    reject(g,{PlayerId::second,Attack{{23}}},ActionError::wrong_phase);
    reject(g,{PlayerId::second,Pass{}},ActionError::wrong_phase);
    reject(g,{PlayerId::second,Defend{{99}}},ActionError::unknown_card);
    reject(g,{PlayerId::second,Defend{{18}}},ActionError::wrong_zone);
    accept(g,{PlayerId::second,Defend{{23}}});
    check(g.snapshot().players[0].board.empty() && ids(g.snapshot().players[0].discard)==std::vector<unsigned>{6},"Lower attacker survives");
    accept(g,{PlayerId::second,Attack{{23}}});
    check(g.snapshot().players[0].lives==2 && ids(g.snapshot().players[1].board)==std::vector<unsigned>{23},"Direct damage or attacker reuse");
    g.reset(0); check(g.snapshot()==initial,"Reset failed to replace match");
    for (auto a:std::vector<Action>{{PlayerId::first,Play{{6}}},{PlayerId::second,Play{{23}}},{PlayerId::first,Play{{4}}},{PlayerId::second,Play{{18}}},{PlayerId::first,Play{{1}}},{PlayerId::second,Play{{19}}}}) accept(g,a);
    reject(g,{PlayerId::first,Play{{8}}},ActionError::board_full);
    reject(g,{PlayerId::first,Play{{24}}},ActionError::unknown_card);
    reject(g,{PlayerId::first,Play{{23}}},ActionError::wrong_zone);
    reject(g,{PlayerId::first,Pass{}},ActionError::pass_not_allowed);
    check(g.legal_actions(PlayerId::first)==std::vector<Action>{{PlayerId::first,Attack{{1}}},{PlayerId::first,Attack{{4}}},{PlayerId::first,Attack{{6}}}},"Full board legal actions");
    g.reset(std::numeric_limits<Seed>::max()); Game max_seed; max_seed.reset(std::numeric_limits<Seed>::max());
    check(g.snapshot()==max_seed.snapshot() && g.snapshot()!=initial,"Extreme seed reset/determinism");
    invariants(g);
    for (unsigned defender:{12u,18u}) {
        g.reset(0); accept(g,{PlayerId::first,Play{{6}}}); accept(g,{PlayerId::second,Play{{static_cast<std::uint16_t>(defender)}}});
        accept(g,{PlayerId::first,Attack{{6}}}); accept(g,{PlayerId::second,Defend{{static_cast<std::uint16_t>(defender)}}});
        const auto state=g.snapshot();
        check(state.players[0].lives==3 && state.players[1].lives==3,"Blocked life damage");
        check(state.players[1].board.empty() && state.players[0].board.size()==(defender==12?1u:0u),"Higher attacker/tie combat");
        check(state.active_player==PlayerId::second && state.phase==Phase::main && !state.pending_attack,"Defense turn handoff");
    }
}
void fixture_and_replay() {
    std::ifstream file(FIXTURE_PATH); check(file.good(),"Fixture missing");
    const std::string text{std::istreambuf_iterator<char>(file),{}};
    const auto replay=parse_replay(text); check(replay.actions.size()==62,"Fixture action count");
    const auto final=execute_replay(replay);
    check(final==execute_replay(parse_replay(encode_replay(replay))),"Replay roundtrip/determinism");
    check(final.result==GameResult{Outcome::win,PlayerId::first,ResultReason::zero_lives} && final.phase==Phase::terminal && final.action_count==62 && final.consecutive_passes==0 && final.active_player==PlayerId::first && !final.pending_attack,"Fixture final lifecycle");
    check(final.players[0].lives==2 && final.players[1].lives==0 && ids(final.players[0].hand)==std::vector<unsigned>{1,0,2} && final.players[1].hand.empty(),"Fixture lives/hands");
    check(ids(final.players[0].board)==std::vector<unsigned>{7} && final.players[1].board.empty() && final.players[0].deck.empty() && final.players[1].deck.empty(),"Fixture boards/decks");
    check(ids(final.players[0].discard)==std::vector<unsigned>{6,8,4,5,3,11,9,10} && ids(final.players[1].discard)==std::vector<unsigned>{23,18,19,20,15,16,17,21,22,12,13,14},"Fixture discards");
    Game g; for(const auto& a:replay.actions) { const auto before=g.snapshot(); accept(g,a); transition(before,a,g.snapshot()); invariants(g); }
    reject(g,{static_cast<PlayerId>(2),Defend{{99}}},ActionError::terminal);
    g.reset(0); check(g.snapshot()==Game{}.snapshot(),"Terminal reset");
    auto r=replay; r.actions[1]={PlayerId::first,Play{{1}}};
    try { (void)execute_replay(r); check(false,"Illegal replay accepted"); }
    catch(const ReplayExecutionError& e) { check(e.action_index==1 && e.action_error==ActionError::wrong_actor,"Replay failure index/error"); }
    r=replay; r.actions.push_back({PlayerId::first,Pass{}}); invalid([&]{(void)execute_replay(r);});
    r=replay; r.actions.pop_back(); r.expected_result.reset(); invalid([&]{(void)execute_replay(r);}); check(execute_replay(r,false).result.outcome==Outcome::ongoing,"Diagnostic replay rejected");
    r=replay; r.expected_result->winner=PlayerId::second; invalid([&]{(void)execute_replay(r);});
    r=replay; r.format_version=2; invalid([&]{(void)execute_replay(r);});
    r=replay; r.config.board_capacity=4; invalid([&]{(void)execute_replay(r);});
    Replay empty; empty.seed=std::numeric_limits<Seed>::max(); check(parse_replay(encode_replay(empty)).seed==empty.seed,"Max seed roundtrip");
    const auto base=encode_replay(empty);
    struct Grouped : std::numpunct<char> {
        char do_thousands_sep() const override { return ','; }
        std::string do_grouping() const override { return "\3"; }
    };
    const auto original_locale=std::locale();
    std::locale::global(std::locale(original_locale,new Grouped));
    const auto localized=encode_replay(empty);
    std::locale::global(original_locale);
    check(localized==base,"Replay encoding depends on locale");
    auto replace=[&](std::string from,std::string to) { auto modified=base; const auto p=modified.find(from); check(p!=std::string::npos,"Bad test replacement"); modified.replace(p,from.size(),to); invalid([&]{(void)parse_replay(modified);}); };
    replace("\"format_version\":1","\"format_version\":1,\"format_version\":1");
    replace("\"format_version\":1","\"format_version\":1,\"extra\":0");
    replace("\"format_version\":1","\"format_version\":2"); replace("\"format_version\":1","\"format_version\":1.0");
    replace("\"rules_version\":1","\"rules_version\":0"); replace("\"starting_lives\":3","\"starting_lives\":256");
    replace("18446744073709551615","18446744073709551616"); replace("18446744073709551615","01");
    replace("18446744073709551615","+1"); replace("18446744073709551615"," 1"); replace("18446744073709551615","");
    replace("\"actions\":[]","\"actions\":[{\"actor\":2,\"type\":\"pass\"}]");
    replace("\"actions\":[]","\"actions\":[{\"actor\":0,\"type\":\"play\"}]");
    replace("\"actions\":[]","\"actions\":[{\"actor\":0,\"type\":\"play\",\"card\":24}]");
    replace("\"actions\":[]","\"actions\":[{\"actor\":0,\"type\":\"pass\",\"card\":0}]");
    replace("\"actions\":[]","\"actions\":[{\"actor\":0,\"type\":\"unknown\",\"card\":0}]");
    replace("\"actions\":[]","\"actions\":null"); replace("\"seed\":\"18446744073709551615\"","\"seed\":1");
    for (const auto& input:std::vector<std::string>{"", "{}", "[]", base+"x", base.substr(0,base.size()-1)}) invalid([&]{(void)parse_replay(input);});
    replace("\"actions\":[]","\"actions\":[{\"actor\":0,\"type\":\"pass\",\"extra\":0}]");
    replace("\"actions\":[]","\"actions\":[],\"expected_result\":{\"outcome\":\"win\",\"winner\":null,\"reason\":\"zero_lives\"}");
    replace("\"actions\":[]","\"actions\":[],\"expected_result\":{\"outcome\":\"draw\",\"winner\":0,\"reason\":\"both_players_stuck\"}");
    replace("\"rules_version\":1","\"rules_version\":1,\"rules_version\":1");
    auto duplicate_escaped=base; duplicate_escaped.insert(1,"\"s\\u0065ed\":\"0\",");
    invalid([&]{(void)parse_replay(duplicate_escaped);});
    auto escaped=base; escaped.replace(escaped.find("seed"),4,"s\\u0065ed"); check(parse_replay(escaped).seed==empty.seed,"JSON escaped key");
}
void full_games() {
    unsigned draws=0,wins=0; std::uint64_t steps=0; bool no_draw=false, pass_reset=false; std::set<int> combats;
    for (Seed seed=0;seed<1024;++seed) {
        Game g, repeat; g.reset(seed); repeat.reset(seed); detail::SplitMix64 choices(seed+1234);
        Replay replay; replay.seed=seed;
        while (g.result().outcome==Outcome::ongoing) {
            invariants(g); const auto before=g.snapshot();
            const auto acting=*g.observe(PlayerId::first).acting_player; const auto actions=g.legal_actions(acting);
            // Probe all payload/card combinations on reachable states, including other actors.
            if (seed<16) for (unsigned actor=0;actor<3;++actor) for (unsigned type=0;type<4;++type) for (unsigned id=0;id<25;++id) {
                Action a{static_cast<PlayerId>(actor),Pass{}};
                if (type==0) a.payload=Play{{static_cast<std::uint16_t>(id)}};
                if (type==1) a.payload=Attack{{static_cast<std::uint16_t>(id)}};
                if (type==2) a.payload=Defend{{static_cast<std::uint16_t>(id)}};
                Game probe=g; const auto result=probe.step(a);
                const bool legal=std::find(actions.begin(),actions.end(),a)!=actions.end();
                check((result.error==ActionError::none)==legal,"Legal list/step mismatch");
                if (!legal) check(probe.snapshot()==before,"Illegal candidate mutated state");
            }
            const auto a=actions[choices.bounded(actions.size())];
            if (auto p=std::get_if<Play>(&a.payload)) { (void)p; no_draw |= before.players[idx(acting)].deck.empty(); }
            if (auto p=std::get_if<Defend>(&a.payload)) {
                const auto av=card(before.players[idx(other(acting))].board,before.pending_attack->card).value;
                const auto dv=card(before.players[idx(acting)].board,p->card).value; combats.insert(av<dv?-1:av==dv?0:1);
            }
            accept(g,a); accept(repeat,a); replay.actions.push_back(a); ++steps;
            transition(before,a,g.snapshot()); check(g.snapshot()==repeat.snapshot(),"Determinism regression");
            if (!std::holds_alternative<Pass>(a.payload) && before.consecutive_passes) { pass_reset=true; check(g.snapshot().consecutive_passes==0,"Pass reset"); }
            // Resource argument gives a conservative 160-action bound; this is a test guard, not a game rule.
            check(replay.actions.size()<=160,"Game failed to terminate");
        }
        invariants(g); replay.expected_result=g.result();
        check(execute_replay(parse_replay(encode_replay(replay)))==g.snapshot(),"Full-game replay parity");
        if (g.result().outcome==Outcome::draw) {
            ++draws; check(g.snapshot().consecutive_passes==2 && !g.result().winner && g.result().reason==ResultReason::both_players_stuck,"Stuck draw");
            for (const auto& p:g.snapshot().players) check(p.hand.empty() && p.board.empty(),"Draw with available action");
        } else ++wins;
        reject(g,{PlayerId::first,Pass{}},ActionError::terminal);
    }
    check(draws>0 && wins>0 && no_draw && pass_reset && combats.size()==3,"Missing full-game rule coverage");
    std::cout << "1024 complete games; " << wins << " wins, " << draws << " draws; " << steps << " actions; zero determinism regressions.\n";
}
}
int main() {
    try { setup_and_errors(); fixture_and_replay(); full_games(); }
    catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
    std::cout<<"M1 rules, illegal actions, observations, fixture and replay checks passed.\n";
}
