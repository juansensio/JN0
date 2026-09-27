#pragma once
#include "janus/game.hpp"
namespace janus {
enum class BotKind { random, heuristic };
// Policies receive only player-visible information and core-generated legal actions.
class RandomBot {
public:
    explicit RandomBot(Seed seed) : state_(seed) {}
    Action choose(const Observation&, const std::vector<Action>& legal);
private:
    std::uint64_t state_;
};
class HeuristicBot {
public:
    Action choose(const Observation& observation, const std::vector<Action>& legal) const;
};
}
