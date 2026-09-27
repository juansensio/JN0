#include "janus/game.hpp"
#include <iostream>
#include <type_traits>

static_assert(!std::is_same_v<janus::GameState, janus::Observation>);
static_assert(std::variant_size_v<janus::ActionPayload> == 4);
static_assert(std::is_same_v<decltype(janus::Observation::own_hand), std::vector<janus::Card>>);
static_assert(std::is_same_v<decltype(janus::GameState::seed), std::uint64_t>);

int main() {
    int failures = 0;
    auto check = [&](bool condition, const char* message) {
        if (!condition) { std::cerr << message << '\n'; ++failures; }
    };
    janus::GameConfig config;
    check(janus::validate_config(config) == janus::ConfigError::none, "Default config rejected");
    auto changed = config;
    changed.rules_version = 2;
    check(janus::validate_config(changed) == janus::ConfigError::unsupported_rules_version, "Version accepted");
    changed = config;
    changed.deck_values[0] = 4;
    check(janus::validate_config(changed) == janus::ConfigError::unsupported_setup, "Deck change accepted");
    changed = config; changed.starting_lives = 0;
    check(janus::validate_config(changed) == janus::ConfigError::unsupported_setup, "Invalid lives accepted");
    changed = config; changed.initial_hand_size = 5;
    check(janus::validate_config(changed) == janus::ConfigError::unsupported_setup, "Hand change accepted");
    changed = config; changed.board_capacity = 4;
    check(janus::validate_config(changed) == janus::ConfigError::unsupported_setup, "Capacity change accepted");
    janus::GameState state;
    state.players[0].hand.push_back({{0}, 1});
    janus::Observation observation{};
    observation.own_hand = state.players[0].hand;
    observation.own_hand.clear();
    check(state.players[0].hand.size() == 1, "Observation aliases state");
    janus::Action action{janus::PlayerId::first, janus::Defend{{9}}};
    check(std::get<janus::Defend>(action.payload).card.value == 9, "Action payload lost identity");
    check(!janus::GameResult{}.winner.has_value(), "Ongoing result has winner");
    if (failures == 0) std::cout << "Core contract checks passed.\n";
    return failures == 0 ? 0 : 1;
}
