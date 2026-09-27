#pragma once
#include "janus/types.hpp"
#include <variant>

namespace janus {
struct Play { CardId card; bool operator==(const Play&) const = default; };
struct Attack { CardId card; bool operator==(const Attack&) const = default; };
struct Defend { CardId card; bool operator==(const Defend&) const = default; };
struct Pass { bool operator==(const Pass&) const = default; };
using ActionPayload = std::variant<Play, Attack, Defend, Pass>;
struct Action {
    PlayerId actor;
    ActionPayload payload;
    bool operator==(const Action&) const = default;
};
enum class ActionError {
    none, terminal, invalid_actor, wrong_actor, wrong_phase,
    unknown_card, wrong_zone, board_full, pass_not_allowed
};
struct StepResult {
    ActionError error = ActionError::none;
    GameResult result;
};

// Deterministic rules v1; snapshots are for trusted consumers only.
class Game {
public:
    explicit Game(GameConfig config = {}); // Invalid config: std::invalid_argument.
    void reset(Seed seed);
    [[nodiscard]] Observation observe(PlayerId player) const;
    [[nodiscard]] std::vector<Action> legal_actions(PlayerId player) const;
    [[nodiscard]] StepResult step(const Action& action);
    [[nodiscard]] GameResult result() const;
    [[nodiscard]] GameState snapshot() const; // Trusted debugging/simulation only.
private:
    GameState state_;
};
struct Replay {
    std::uint32_t format_version = 1;
    GameConfig config;
    Seed seed{};
    std::vector<Action> actions;
    std::optional<GameResult> expected_result;
};
} // namespace janus
