#include "janus_game.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
void JanusGame::_bind_methods() {
    ClassDB::bind_method(D_METHOD("smoke_test"), &JanusGame::smoke_test);
}

void JanusGame::_ready() {
    UtilityFunctions::print("[JN0] JanusGame::_ready()");
    smoke_test();
}

void JanusGame::smoke_test() {
    constexpr janus::Seed seed = 42;
    game_.reset(seed);

    // Trusted diagnostic only; a player-facing UI must consume Observation.
    const auto state = game_.snapshot();
    const auto legal = game_.legal_actions(state.active_player);
    const int active_player = state.active_player == janus::PlayerId::first ? 0 : 1;

    UtilityFunctions::print("[JN0] Core game instantiated. seed=", static_cast<int64_t>(seed));
    UtilityFunctions::print("[JN0] active_player=", active_player);
    for (int player = 0; player < 2; ++player) {
        const auto &p = state.players[player];
        UtilityFunctions::print("[JN0] P", player,
            " lives=", static_cast<int64_t>(p.lives),
            " hand=", static_cast<int64_t>(p.hand.size()),
            " deck=", static_cast<int64_t>(p.deck.size()),
            " board=", static_cast<int64_t>(p.board.size()));
    }
    UtilityFunctions::print("[JN0] legal actions for active player=", static_cast<int64_t>(legal.size()));
}
} // namespace godot
