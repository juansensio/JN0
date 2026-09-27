#include "janus_game.hpp"
#include "janus/replay.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <charconv>
#include <string>
#include <type_traits>

namespace godot {
namespace {
Dictionary status(bool ok, const String &error = "") {
    Dictionary d; d["ok"] = ok; d["error"] = error; return d;
}
int64_t player(janus::PlayerId p) { return static_cast<int64_t>(p); }
Dictionary result(const janus::GameResult &r) {
    Dictionary d;
    d["outcome"] = r.outcome == janus::Outcome::ongoing ? "ongoing" : r.outcome == janus::Outcome::win ? "win" : "draw";
    d["winner"] = r.winner ? player(*r.winner) : -1;
    d["reason"] = r.reason == janus::ResultReason::none ? "none" : r.reason == janus::ResultReason::zero_lives ? "zero_lives" : "both_players_stuck";
    return d;
}
Array cards(const std::vector<janus::Card> &source) {
    Array out;
    for (const auto &c : source) { Dictionary d; d["id"] = c.id.value; d["value"] = c.value; out.push_back(d); }
    return out;
}
Dictionary action_data(const janus::Action &a) {
    Dictionary d; d["actor"] = player(a.actor); d["card"] = -1;
    std::visit([&](const auto &p) {
        using T = std::decay_t<decltype(p)>;
        if constexpr (std::is_same_v<T, janus::Pass>) d["kind"] = "pass";
        else {
            d["card"] = p.card.value;
            if constexpr (std::is_same_v<T, janus::Play>) d["kind"] = "play";
            if constexpr (std::is_same_v<T, janus::Attack>) d["kind"] = "attack";
            if constexpr (std::is_same_v<T, janus::Defend>) d["kind"] = "defend";
        }
    }, a.payload);
    return d;
}
}
JanusGame::JanusGame() { start("42"); }
void JanusGame::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start", "seed"), &JanusGame::start);
    ClassDB::bind_method(D_METHOD("observe", "viewer"), &JanusGame::observe);
    ClassDB::bind_method(D_METHOD("legal_actions", "actor"), &JanusGame::legal_actions);
    ClassDB::bind_method(D_METHOD("submit", "actor", "kind", "card"), &JanusGame::submit);
    ClassDB::bind_method(D_METHOD("bot_step", "actor", "kind"), &JanusGame::bot_step);
    ClassDB::bind_method(D_METHOD("export_replay"), &JanusGame::export_replay);
    ClassDB::bind_method(D_METHOD("load_replay", "json"), &JanusGame::load_replay);
}
Dictionary JanusGame::start(const String &seed) {
    const std::string s = seed.utf8().get_data();
    janus::Seed value{};
    auto parsed = std::from_chars(s.data(), s.data() + s.size(), value);
    if (s.empty() || parsed.ec != std::errc{} || parsed.ptr != s.data() + s.size()) return status(false, "Seed must be an unsigned 64-bit decimal integer.");
    game_.reset(value);
    replay_ = {}; replay_.seed = value;
    random_ = janus::RandomBot(value);
    return status(true);
}
Dictionary JanusGame::observe(int64_t viewer) const {
    if (viewer < 0 || viewer > 1) return status(false, "Invalid viewer.");
    const auto o = game_.observe(static_cast<janus::PlayerId>(viewer));
    Dictionary d = status(true);
    d["viewer"] = viewer; d["own_hand"] = cards(o.own_hand);
    Array players;
    for (const auto &p : o.players) {
        Dictionary v; v["lives"] = p.lives; v["hand_count"] = p.hand_count; v["deck_count"] = p.deck_count;
        v["board"] = cards(p.board); v["discard"] = cards(p.discard); players.push_back(v);
    }
    d["players"] = players;
    d["active_player"] = player(o.active_player);
    d["acting_player"] = o.acting_player ? player(*o.acting_player) : -1;
    d["phase"] = o.phase == janus::Phase::main ? "main" : o.phase == janus::Phase::awaiting_defense ? "awaiting_defense" : "terminal";
    d["action_count"] = static_cast<int64_t>(o.action_count); d["consecutive_passes"] = o.consecutive_passes;
    d["result"] = result(o.result);
    Dictionary pending;
    if (o.pending_attack) { pending["attacker"] = player(o.pending_attack->attacker); pending["card"] = o.pending_attack->card.value; }
    d["pending_attack"] = pending;
    Dictionary config; config["rules_version"] = o.config.rules_version; config["starting_lives"] = o.config.starting_lives;
    config["initial_hand_size"] = o.config.initial_hand_size; config["board_capacity"] = o.config.board_capacity;
    Array values; for (auto v : o.config.deck_values) values.push_back(v); config["deck_values"] = values; d["config"] = config;
    return d;
}
Array JanusGame::legal_actions(int64_t actor) const {
    Array out;
    if (actor < 0 || actor > 1) return out;
    for (const auto &a : game_.legal_actions(static_cast<janus::PlayerId>(actor))) out.push_back(action_data(a));
    return out;
}
Dictionary JanusGame::accept(const janus::Action &a) {
    const auto step = game_.step(a);
    if (step.error != janus::ActionError::none) {
        Dictionary d = status(false, "Core rejected action."); d["code"] = static_cast<int64_t>(step.error); return d;
    }
    replay_.actions.push_back(a);
    Dictionary d = status(true); d["action"] = action_data(a); return d;
}
Dictionary JanusGame::submit(int64_t actor, const String &kind, int64_t card) {
    if (actor < 0 || actor > 1) return status(false, "Invalid actor.");
    if (kind != "pass" && (card < 0 || card > 65535)) return status(false, "Invalid card ID.");
    janus::Action a{static_cast<janus::PlayerId>(actor), janus::Pass{}};
    const janus::CardId id{static_cast<uint16_t>(kind == "pass" ? 0 : card)};
    if (kind == "play") a.payload = janus::Play{id};
    else if (kind == "attack") a.payload = janus::Attack{id};
    else if (kind == "defend") a.payload = janus::Defend{id};
    else if (kind != "pass") return status(false, "Invalid action kind.");
    return accept(a);
}
Dictionary JanusGame::bot_step(int64_t actor, const String &kind) {
    if (actor < 0 || actor > 1) return status(false, "Invalid actor.");
    if (kind != "random" && kind != "heuristic") return status(false, "Invalid bot.");
    const auto p = static_cast<janus::PlayerId>(actor);
    const auto legal = game_.legal_actions(p);
    if (legal.empty()) return status(false, "Bot has no legal action.");
    const auto o = game_.observe(p);
    return accept(kind == "random" ? random_.choose(o, legal) : janus::HeuristicBot{}.choose(o, legal));
}
String JanusGame::export_replay() const {
    auto replay = replay_;
    if (game_.result().outcome != janus::Outcome::ongoing) replay.expected_result = game_.result();
    return String(janus::encode_replay(replay).c_str());
}
Dictionary JanusGame::load_replay(const String &json) {
    try {
        const auto replay = janus::parse_replay(json.utf8().get_data());
        const auto verified = janus::execute_replay(replay, false);
        janus::Game candidate(replay.config); candidate.reset(replay.seed);
        for (const auto &a : replay.actions) {
            if (candidate.step(a).error != janus::ActionError::none) return status(false, "Replay action rejected.");
        }
        if (candidate.snapshot() != verified) return status(false, "Replay parity failure.");
        game_ = std::move(candidate); replay_ = replay; random_ = janus::RandomBot(replay.seed);
        return status(true);
    } catch (const std::exception &e) { return status(false, String(e.what())); }
}
}
