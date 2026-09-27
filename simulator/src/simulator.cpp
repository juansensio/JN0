#include "janus/simulator.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
namespace janus {
namespace {
std::uint64_t mix(std::uint64_t z) {
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
}
MatchRecord run_match(Seed seed, BotKind first, BotKind second) {
    Game game;
    game.reset(seed);
    RandomBot random0(mix(seed ^ 0x72616e646f6d3030ULL));
    RandomBot random1(mix(seed ^ 0x72616e646f6d3131ULL));
    HeuristicBot heuristic;
    MatchRecord record;
    record.replay.seed = seed;
    while (game.result().outcome == Outcome::ongoing) {
        const auto initial = game.observe(PlayerId::first);
        const auto actor = *initial.acting_player;
        const auto o = game.observe(actor);
        const auto legal = game.legal_actions(actor);
        const auto kind = actor == PlayerId::first ? first : second;
        auto& random = actor == PlayerId::first ? random0 : random1;
        const auto a = kind == BotKind::random ? random.choose(o, legal) : heuristic.choose(o, legal);
        if (game.step(a).error != ActionError::none) throw std::runtime_error("bot produced illegal action");
        record.replay.actions.push_back(a);
    }
    record.replay.expected_result = game.result();
    record.final_state = game.snapshot();
    return record;
}
double BatchResult::score() const {
    return games == 0 ? 0.0 : (static_cast<double>(first_wins) + 0.5 * static_cast<double>(draws)) / static_cast<double>(games);
}
double paired_score_lower_bound(double score, std::uint64_t pairs) {
    if (pairs == 0) throw std::invalid_argument("evaluation requires pairs");
    return score - std::sqrt(std::log(20.0) / (2.0 * static_cast<double>(pairs)));
}
BatchResult run_batch(const BatchConfig& c) {
    if (c.games == 0 || c.workers == 0 || (c.swap_seats && c.games % 2 != 0))
        throw std::invalid_argument("positive games/workers and complete seat pairs required");
    const auto seeds = c.swap_seats ? c.games / 2 : c.games;
    if (seeds - 1 > UINT64_MAX - c.first_seed) throw std::invalid_argument("seed range overflows");
    const auto workers = static_cast<unsigned>(std::min<std::uint64_t>(c.workers, c.games));
    std::vector<BatchResult> totals(workers);
    std::atomic<std::uint64_t> next{0};
    std::exception_ptr failure;
    std::mutex mutex;
    const auto start = std::chrono::steady_clock::now();
    {
        std::vector<std::jthread> threads;
        for (unsigned w = 0; w < workers; ++w) threads.emplace_back([&, w] {
            try {
                auto& r = totals[w];
                for (;;) {
                    const auto i = next.fetch_add(1);
                    if (i >= c.games) break;
                    const bool swapped = c.swap_seats && i % 2 == 1;
                    const auto seed = c.first_seed + (c.swap_seats ? i / 2 : i);
                    const auto record = run_match(seed, swapped ? c.second : c.first, swapped ? c.first : c.second);
                    const auto& s = record.final_state;
                    ++r.games;
                    r.actions += s.action_count;
                    if (s.result.outcome == Outcome::draw) ++r.draws;
                    else if ((*s.result.winner == PlayerId::first) != swapped) ++r.first_wins;
                    else ++r.second_wins;
                    std::uint64_t digest = mix(i) ^ mix(seed) ^ mix(s.action_count);
                    for (const auto& a : record.replay.actions) {
                        const auto id = std::visit([](const auto& p) -> std::uint64_t {
                            if constexpr (requires { p.card; }) return p.card.value;
                            else return 0;
                        }, a.payload);
                        digest = mix(digest ^ (id + 32 * a.payload.index() + 128 * static_cast<unsigned>(a.actor)));
                    }
                    r.checksum ^= mix(digest ^ static_cast<unsigned>(s.result.outcome)
                        ^ (s.result.winner ? 256 + static_cast<unsigned>(*s.result.winner) : 512));
                }
            } catch (...) { std::lock_guard lock(mutex); if (!failure) failure = std::current_exception(); }
        });
    }
    if (failure) std::rethrow_exception(failure);
    BatchResult result;
    for (const auto& r : totals) {
        result.games += r.games; result.first_wins += r.first_wins;
        result.second_wins += r.second_wins; result.draws += r.draws;
        result.actions += r.actions; result.checksum ^= r.checksum;
    }
    result.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return result;
}
}
