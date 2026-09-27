#include "janus/simulator.hpp"
#include "janus/replay.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
using namespace janus;
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> void rejects(F f) {
    try { f(); } catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("expected invalid_argument");
}
void policy_checks() {
    Game game; game.reset(0);
    auto o = game.observe(*game.observe(PlayerId::first).acting_player);
    const auto legal = game.legal_actions(o.viewer);
    RandomBot a(123), b(123);
    for (int i = 0; i < 100; ++i) {
        const auto chosen = a.choose(o, legal);
        check(chosen == b.choose(o, legal), "random seed reproducibility");
        check(std::find(legal.begin(), legal.end(), chosen) != legal.end(), "random legality");
    }
    HeuristicBot heuristic;
    rejects([&] { (void)heuristic.choose(o, {}); });
    rejects([&] { (void)a.choose(o, {}); });
    o.viewer = PlayerId::second;
    o.phase = Phase::awaiting_defense;
    o.pending_attack = PendingAttack{PlayerId::first, CardId{3}};
    o.players[0].board = {{CardId{3}, 2}};
    o.players[1].board = {{CardId{12}, 1}, {CardId{18}, 3}, {CardId{21}, 4}};
    std::vector<Action> defenses = {{o.viewer, Defend{CardId{12}}}, {o.viewer, Defend{CardId{18}}}, {o.viewer, Defend{CardId{21}}}};
    check(heuristic.choose(o, defenses) == defenses[1], "cheapest winning defense");
    o.players[0].board[0].value = 4;
    check(heuristic.choose(o, defenses) == defenses[2], "tie preferred to loss");
    o.players[1].board.pop_back(); defenses.pop_back();
    check(heuristic.choose(o, defenses) == defenses[0], "weakest losing defense");
}
}
int main() {
    try {
        using namespace janus;
        policy_checks();
        for (Seed seed = 0; seed < 256; ++seed) {
            for (const auto first : {BotKind::random, BotKind::heuristic}) {
                const auto r = run_match(seed, first, BotKind::random);
                check(r.final_state.result.outcome != Outcome::ongoing, "complete match");
                check(execute_replay(parse_replay(encode_replay(r.replay))) == r.final_state, "replay exact parity");
                check(run_match(seed, first, BotKind::random).replay.actions == r.replay.actions, "bot determinism");
            }
        }
        BatchConfig config; config.games = 2000;
        const auto one = run_batch(config);
        config.workers = 4;
        const auto many = run_batch(config);
        check(one.games == config.games && many.games == config.games, "batch complete");
        check(one.checksum == many.checksum && one.actions == many.actions
              && one.first_wins == many.first_wins && one.second_wins == many.second_wins
              && one.draws == many.draws, "worker determinism");
        config.swap_seats = true; config.first = BotKind::heuristic;
        config.games = 20000;
        const auto evaluation = run_batch(config);
        check(paired_score_lower_bound(evaluation.score(), config.games / 2) > 0.5, "heuristic superiority");
        config.workers = 1;
        const auto sequential = run_batch(config);
        check(evaluation.checksum == sequential.checksum && evaluation.first_wins == sequential.first_wins
              && evaluation.second_wins == sequential.second_wins && evaluation.draws == sequential.draws,
              "paired evaluation worker determinism");
        config.games = 1; rejects([&] { (void)run_batch(config); });
        config.swap_seats = false; config.games = 2; config.first_seed = UINT64_MAX;
        rejects([&] { (void)run_batch(config); });
        config.first_seed = 0; config.workers = 0; rejects([&] { (void)run_batch(config); });
        config.workers = 1; config.games = 0; rejects([&] { (void)run_batch(config); });
        std::cout << "Simulator checks passed; heuristic score=" << evaluation.score() << '\n';
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
