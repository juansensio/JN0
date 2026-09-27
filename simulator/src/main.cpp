#include "janus/simulator.hpp"
#include "janus/replay.hpp"
#include <algorithm>
#include <type_traits>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
namespace {
using namespace janus;
std::uint64_t number(const std::string& s) {
    std::uint64_t n{};
    const auto [end, error] = std::from_chars(s.data(), s.data() + s.size(), n);
    if (error != std::errc{} || end != s.data() + s.size()) throw std::invalid_argument("expected unsigned integer: " + s);
    return n;
}
BotKind bot(const std::string& s) {
    if (s == "random") return BotKind::random;
    if (s == "heuristic") return BotKind::heuristic;
    throw std::invalid_argument("bot must be random or heuristic");
}
std::string action_text(const Action& a) {
    return std::visit([](const auto& p) -> std::string {
        using T = std::decay_t<decltype(p)>;
        if constexpr (std::is_same_v<T, Pass>) return "Pass";
        else {
            std::string verb;
            if constexpr (std::is_same_v<T, Play>) verb = "Play";
            if constexpr (std::is_same_v<T, Attack>) verb = "Attack";
            if constexpr (std::is_same_v<T, Defend>) verb = "Defend";
            return verb + " card #" + std::to_string(p.card.value);
        }
    }, a.payload);
}
void cards(const std::vector<Card>& zone) {
    if (zone.empty()) std::cout << "(empty)";
    for (const auto& c : zone) std::cout << "#" << c.id.value << "[" << static_cast<int>(c.value) << "] ";
    std::cout << '\n';
}
void display(const Observation& o) {
    std::cout << "\nAction " << o.action_count << " | ";
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& p = o.players[i];
        std::cout << "P" << i << " lives=" << static_cast<int>(p.lives)
                  << " hand=" << static_cast<int>(p.hand_count) << " deck=" << static_cast<int>(p.deck_count) << "  ";
    }
    std::cout << '\n';
    for (std::size_t i = 0; i < 2; ++i) { std::cout << "P" << i << " board: "; cards(o.players[i].board); }
    std::cout << "Your hand: "; cards(o.own_hand);
    if (o.pending_attack) std::cout << "Pending attack: #" << o.pending_attack->card.value << '\n';
}
void result_text(const GameResult& r) {
    if (r.outcome == Outcome::draw) std::cout << "Draw: both players are stuck.\n";
    else if (r.winner) std::cout << "Player " << static_cast<int>(*r.winner) << " wins.\n";
}
void save(const Replay& replay, const std::string& path) {
    if (path.empty()) return;
    std::ofstream out(path);
    if (!out || !(out << encode_replay(replay) << '\n')) throw std::runtime_error("cannot write replay: " + path);
    std::cout << "Replay saved: " << path << '\n';
}
void report(const BatchResult& r, unsigned workers) {
    std::cout << "workers=" << workers << " games=" << r.games << " first_wins=" << r.first_wins
              << " second_wins=" << r.second_wins << " draws=" << r.draws << " actions=" << r.actions
              << " score=" << std::setprecision(6) << r.score() << " seconds=" << r.seconds
              << " games/s=" << static_cast<double>(r.games) / r.seconds << " checksum=" << r.checksum << '\n';
}
void help() {
    std::cout << "Janus Noise headless CLI\n"
              << "  janus_cli play [--seed N] [--bot random|heuristic] [--human 0|1] [--save FILE]\n"
              << "  janus_cli batch [--games N] [--seed N] [--workers N] [--first BOT] [--second BOT]\n"
              << "  janus_cli evaluate [--games EVEN_N] [--seed N] [--workers N]\n"
              << "  janus_cli benchmark [--games N] [--seed N] [--workers MAX]\n"
              << "  janus_cli match [--seed N] [--first BOT] [--second BOT] [--save FILE]\n"
              << "  janus_cli replay --file FILE\n"
              << "Evaluation defaults: 20,000 games / 10,000 seat-swapped seed pairs; heuristic vs random.\n"
              << "Benchmark defaults: 100,000 random vs random games per worker count.\n";
}
}
int main(int argc, char** argv) {
    try {
        using namespace janus;
        if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "help") { help(); return 0; }
        const std::string mode = argv[1];
        if (mode != "play" && mode != "batch" && mode != "evaluate" && mode != "benchmark" && mode != "match" && mode != "replay")
            throw std::invalid_argument("unknown command: " + mode);
        BatchConfig config;
        if (mode == "evaluate") { config.games = 20000; config.first = BotKind::heuristic; config.swap_seats = true; }
        if (mode == "benchmark") { config.games = 100000; config.workers = std::max(1u, std::thread::hardware_concurrency()); }
        BotKind opponent = BotKind::heuristic;
        PlayerId human = PlayerId::first;
        std::string output, input;
        for (int i = 2; i < argc; ++i) {
            const std::string key = argv[i];
            if (key == "--help") { help(); return 0; }
            if (i + 1 >= argc) throw std::invalid_argument("missing value for " + key);
            const std::string value = argv[++i];
            if (key == "--seed" && mode != "replay") config.first_seed = number(value);
            else if (key == "--games" && (mode == "batch" || mode == "evaluate" || mode == "benchmark")) config.games = number(value);
            else if (key == "--workers" && (mode == "batch" || mode == "evaluate" || mode == "benchmark")) {
                const auto n = number(value);
                if (n == 0 || n > std::max(1u, std::thread::hardware_concurrency())) throw std::invalid_argument("workers must be between 1 and hardware concurrency");
                config.workers = static_cast<unsigned>(n);
            } else if (key == "--first" && (mode == "batch" || mode == "match")) config.first = bot(value);
            else if (key == "--second" && (mode == "batch" || mode == "match")) config.second = bot(value);
            else if (key == "--bot" && mode == "play") opponent = bot(value);
            else if (key == "--human" && mode == "play") {
                const auto n = number(value); if (n > 1) throw std::invalid_argument("human must be 0 or 1");
                human = static_cast<PlayerId>(n);
            } else if (key == "--save" && (mode == "play" || mode == "match")) output = value;
            else if (key == "--file" && mode == "replay") input = value;
            else throw std::invalid_argument("unsupported option: " + key);
        }
        if (mode == "replay") {
            if (input.empty()) throw std::invalid_argument("--file is required");
            std::ifstream file(input);
            if (!file) throw std::runtime_error("cannot read replay: " + input);
            std::ostringstream content; content << file.rdbuf();
            const auto s = execute_replay(parse_replay(content.str()));
            std::cout << "Replay verified: " << s.action_count << " actions.\n"; result_text(s.result);
        } else if (mode == "match") {
            const auto r = run_match(config.first_seed, config.first, config.second);
            std::cout << "Seed " << config.first_seed << ", " << r.final_state.action_count << " actions.\n";
            result_text(r.final_state.result); save(r.replay, output);
        } else if (mode == "play") {
            Game game; game.reset(config.first_seed);
            Replay replay; replay.seed = config.first_seed;
            RandomBot random(config.first_seed ^ 0x68756d616e626f74ULL); HeuristicBot heuristic;
            std::cout << "You are player " << static_cast<int>(human) << ". Seed " << config.first_seed
                      << ". Cards show #id[value]. Choose a numbered legal action; q quits.\n";
            while (game.result().outcome == Outcome::ongoing) {
                const auto o = game.observe(human);
                const auto actor = *o.acting_player;
                const auto legal = game.legal_actions(actor);
                Action a = legal.front();
                if (actor == human) {
                    display(o);
                    for (std::size_t i = 0; i < legal.size(); ++i) std::cout << i + 1 << ". " << action_text(legal[i]) << '\n';
                    for (;;) {
                        std::cout << "Choose> " << std::flush;
                        std::string line;
                        if (!std::getline(std::cin, line) || line == "q" || line == "quit") {
                            std::cout << "Game ended early; no complete replay saved.\n"; return 0;
                        }
                        try {
                            const auto n = number(line);
                            if (n > 0 && n <= legal.size()) { a = legal[static_cast<std::size_t>(n - 1)]; break; }
                        } catch (const std::invalid_argument&) {}
                        std::cout << "Enter a listed action number or q.\n";
                    }
                } else {
                    const auto visible = game.observe(actor);
                    a = opponent == BotKind::random ? random.choose(visible, legal) : heuristic.choose(visible, legal);
                    std::cout << "Bot: " << action_text(a) << '\n';
                }
                if (game.step(a).error != ActionError::none) throw std::runtime_error("selected action was rejected");
                replay.actions.push_back(a);
            }
            display(game.observe(human)); result_text(game.result());
            replay.expected_result = game.result(); save(replay, output);
        } else if (mode == "benchmark") {
            const auto max = config.workers;
            double baseline = 0;
            std::uint64_t checksum = 0;
            std::cout << "Random vs random, seed=" << config.first_seed << ", hardware threads="
                      << std::max(1u, std::thread::hardware_concurrency()) << '\n';
            for (unsigned w = 1;; w = std::min(max, w * 2)) {
                config.workers = w;
                const auto r = run_batch(config);
                if (w == 1) { baseline = r.seconds; checksum = r.checksum; }
                else if (r.checksum != checksum) throw std::runtime_error("worker determinism mismatch");
                report(r, w); std::cout << "speedup=" << baseline / r.seconds << '\n';
                if (w == max) break;
            }
        } else {
            std::cout << "seed=" << config.first_seed << " seat_pairs=" << config.swap_seats << '\n';
            const auto r = run_batch(config); report(r, config.workers);
            if (mode == "evaluate") {
                const auto lower = paired_score_lower_bound(r.score(), r.games / 2);
                std::cout << "heuristic_score_95pct_lower_bound=" << lower << " gate=" << (lower > 0.5 ? "PASS" : "FAIL") << '\n';
                return lower > 0.5 ? 0 : 2;
            }
        }
        return 0;
    } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << '\n'; return 1; }
}
