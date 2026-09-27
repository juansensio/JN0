#include "janus/game.hpp"
#include "rng.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace janus {
namespace {
std::size_t index(PlayerId p) { return static_cast<std::size_t>(p); }
bool valid(PlayerId p) { return index(p) < 2; }
PlayerId other(PlayerId p) { return p == PlayerId::first ? PlayerId::second : PlayerId::first; }
PlayerId actor(const GameState& s) {
    return s.phase == Phase::awaiting_defense ? other(s.active_player) : s.active_player;
}
auto find_card(std::vector<Card>& zone, CardId id) {
    return std::find_if(zone.begin(), zone.end(), [id](const Card& c) { return c.id == id; });
}
void draw(PlayerState& p) {
    if (!p.deck.empty()) { p.hand.push_back(p.deck.front()); p.deck.erase(p.deck.begin()); }
}
void destroy(PlayerState& p, CardId id) {
    const auto it = find_card(p.board, id);
    p.discard.push_back(*it); p.board.erase(it);
}
}
Game::Game(GameConfig config) {
    if (validate_config(config) != ConfigError::none) throw std::invalid_argument("Unsupported game config");
    state_.config = config; reset(0);
}
void Game::reset(Seed seed) {
    GameState fresh;
    fresh.config = state_.config; fresh.seed = seed;
    detail::SplitMix64 rng(seed);
    for (std::size_t p = 0; p < 2; ++p) {
        auto& player = fresh.players[p]; player.lives = fresh.config.starting_lives;
        for (std::size_t i = 0; i < fresh.config.deck_values.size(); ++i)
            player.deck.push_back({{static_cast<std::uint16_t>(12*p+i)}, fresh.config.deck_values[i]});
        for (std::size_t i = player.deck.size()-1; i > 0; --i)
            std::swap(player.deck[i], player.deck[rng.bounded(i+1)]);
    }
    for (auto& p : fresh.players) for (int i = 0; i < fresh.config.initial_hand_size; ++i) draw(p);
    fresh.active_player = static_cast<PlayerId>(rng.bounded(2));
    state_ = std::move(fresh);
}
Observation Game::observe(PlayerId viewer) const {
    if (!valid(viewer)) throw std::invalid_argument("Invalid player");
    Observation o{};
    o.config = state_.config; o.viewer = viewer; o.own_hand = state_.players[index(viewer)].hand;
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& p = state_.players[i];
        o.players[i] = {p.lives, static_cast<std::uint8_t>(p.deck.size()),
                       static_cast<std::uint8_t>(p.hand.size()), p.board, p.discard};
    }
    o.active_player = state_.active_player; o.phase = state_.phase;
    if (state_.phase != Phase::terminal) o.acting_player = actor(state_);
    o.pending_attack = state_.pending_attack; o.consecutive_passes = state_.consecutive_passes;
    o.action_count = state_.action_count; o.result = state_.result;
    return o;
}
std::vector<Action> Game::legal_actions(PlayerId player) const {
    if (!valid(player)) throw std::invalid_argument("Invalid player");
    std::vector<Action> actions;
    if (state_.phase == Phase::terminal || player != actor(state_)) return actions;
    const auto& p = state_.players[index(player)];
    auto add = [&](std::vector<Card> cards, auto payload) {
        std::sort(cards.begin(), cards.end(), [](const Card& a, const Card& b) { return a.id.value < b.id.value; });
        for (const auto& c : cards) actions.push_back({player, payload(c.id)});
    };
    if (state_.phase == Phase::awaiting_defense) add(p.board, [](CardId c) { return Defend{c}; });
    else {
        if (p.board.size() < state_.config.board_capacity) add(p.hand, [](CardId c) { return Play{c}; });
        add(p.board, [](CardId c) { return Attack{c}; });
        if (actions.empty()) actions.push_back({player, Pass{}});
    }
    return actions;
}
StepResult Game::step(const Action& action) {
    const auto fail = [&](ActionError e) { return StepResult{e, state_.result}; };
    if (state_.phase == Phase::terminal) return fail(ActionError::terminal);
    if (!valid(action.actor)) return fail(ActionError::invalid_actor);
    if (action.actor != actor(state_)) return fail(ActionError::wrong_actor);
    const bool defense = std::holds_alternative<Defend>(action.payload);
    if (defense != (state_.phase == Phase::awaiting_defense)) return fail(ActionError::wrong_phase);
    auto& own = state_.players[index(action.actor)];
    auto& enemy = state_.players[index(other(action.actor))];
    const bool play = std::holds_alternative<Play>(action.payload);
    const bool attack = std::holds_alternative<Attack>(action.payload);
    const bool pass = std::holds_alternative<Pass>(action.payload);
    CardId id{};
    if (!pass) {
        id = std::visit([](const auto& p) -> CardId {
            if constexpr (requires { p.card; }) return p.card; else return {};
        }, action.payload);
        if (id.value >= 24) return fail(ActionError::unknown_card);
        auto& zone = play ? own.hand : own.board;
        if (find_card(zone, id) == zone.end()) return fail(ActionError::wrong_zone);
        if (play && own.board.size() >= state_.config.board_capacity) return fail(ActionError::board_full);
    } else if (!own.board.empty() || (!own.hand.empty() && own.board.size() < state_.config.board_capacity)) {
        return fail(ActionError::pass_not_allowed);
    }
    ++state_.action_count;
    if (!pass) state_.consecutive_passes = 0;
    if (play) {
        const auto it = find_card(own.hand, id); own.board.push_back(*it); own.hand.erase(it); draw(own);
        state_.active_player = other(action.actor);
    } else if (attack) {
        if (!enemy.board.empty()) {
            state_.pending_attack = PendingAttack{action.actor, id}; state_.phase = Phase::awaiting_defense;
        } else {
            --enemy.lives;
            if (enemy.lives == 0) {
                state_.result = {Outcome::win, action.actor, ResultReason::zero_lives}; state_.phase = Phase::terminal;
            } else state_.active_player = other(action.actor);
        }
    } else if (defense) {
        const auto attacking_id = state_.pending_attack->card;
        const auto a = find_card(enemy.board, attacking_id)->value;
        const auto d = find_card(own.board, id)->value;
        if (a <= d) destroy(enemy, attacking_id);
        if (d <= a) destroy(own, id);
        state_.pending_attack.reset(); state_.phase = Phase::main; state_.active_player = action.actor;
    } else {
        if (++state_.consecutive_passes == 2) {
            state_.result = {Outcome::draw, std::nullopt, ResultReason::both_players_stuck}; state_.phase = Phase::terminal;
        } else state_.active_player = other(action.actor);
    }
    return {ActionError::none, state_.result};
}
GameResult Game::result() const { return state_.result; }
GameState Game::snapshot() const { return state_; }
} // namespace janus
