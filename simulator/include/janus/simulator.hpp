#pragma once
#include "janus/bots.hpp"
namespace janus {
struct MatchRecord { Replay replay; GameState final_state; };
struct BatchConfig {
    std::uint64_t games = 10000;
    Seed first_seed = 0;
    unsigned workers = 1;
    BotKind first = BotKind::random;
    BotKind second = BotKind::random;
    bool swap_seats = false; // Consecutive games form a same-seed, opposite-seat pair.
};
struct BatchResult {
    std::uint64_t games{}, first_wins{}, second_wins{}, draws{}, actions{};
    std::uint64_t checksum{}; // Scheduling-independent digest of indexed match results.
    double seconds{};
    double score() const;
};
MatchRecord run_match(Seed seed, BotKind first, BotKind second);
BatchResult run_batch(const BatchConfig& config);
// One-sided 95% distribution-free lower bound for independent paired scores.
double paired_score_lower_bound(double score, std::uint64_t pairs);
}
