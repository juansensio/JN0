#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace janus {
enum class PlayerId : std::uint8_t { first = 0, second = 1 };
struct CardId {
    std::uint16_t value{};
    bool operator==(const CardId&) const = default;
};
struct Card {
    CardId id;
    std::uint8_t value;
    bool operator==(const Card&) const = default;
};
using Seed = std::uint64_t;

// Rules v1 accepts only these defaults. Future variants require a rules version.
struct GameConfig {
    std::uint32_t rules_version = 1;
    std::uint8_t starting_lives = 3;
    std::uint8_t initial_hand_size = 4;
    std::uint8_t board_capacity = 3;
    std::array<std::uint8_t, 12> deck_values{1,1,1,2,2,2,3,3,3,4,4,4};
    bool operator==(const GameConfig&) const = default;
};
enum class ConfigError { none, unsupported_rules_version, unsupported_setup };
[[nodiscard]] ConfigError validate_config(const GameConfig& config) noexcept;

enum class Phase { main, awaiting_defense, terminal };
enum class Outcome { ongoing, win, draw };
enum class ResultReason { none, zero_lives, both_players_stuck };
struct GameResult {
    Outcome outcome = Outcome::ongoing;
    std::optional<PlayerId> winner;
    ResultReason reason = ResultReason::none;
    bool operator==(const GameResult&) const = default;
};
struct PendingAttack {
    PlayerId attacker;
    CardId card;
    bool operator==(const PendingAttack&) const = default;
};
struct PlayerState {
    std::uint8_t lives = 3;
    std::vector<Card> deck; // Next draw is front; private order.
    std::vector<Card> hand;
    std::vector<Card> board;
    std::vector<Card> discard;
    bool operator==(const PlayerState&) const = default;
};
struct GameState {
    GameConfig config;
    Seed seed{};
    std::array<PlayerState, 2> players;
    PlayerId active_player = PlayerId::first; // Main-turn owner, even during defense.
    Phase phase = Phase::main;
    std::optional<PendingAttack> pending_attack;
    std::uint8_t consecutive_passes{};
    std::uint64_t action_count{};
    GameResult result;
    bool operator==(const GameState&) const = default;
};
struct PublicPlayer {
    std::uint8_t lives{};
    std::uint8_t deck_count{};
    std::uint8_t hand_count{};
    std::vector<Card> board;
    std::vector<Card> discard;
    bool operator==(const PublicPlayer&) const = default;
};
// Owned values only: no reference or pointer into complete private state.
struct Observation {
    GameConfig config;
    PlayerId viewer;
    std::array<PublicPlayer, 2> players;
    std::vector<Card> own_hand;
    PlayerId active_player;
    std::optional<PlayerId> acting_player; // Absent after termination.
    Phase phase;
    std::optional<PendingAttack> pending_attack;
    std::uint8_t consecutive_passes{};
    std::uint64_t action_count{};
    GameResult result;
    bool operator==(const Observation&) const = default;
};
} // namespace janus
