#include "janus/bots.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace janus {
Action RandomBot::choose(const Observation&, const std::vector<Action>& legal) {
    if (legal.empty()) throw std::invalid_argument("bot requires legal actions");
    const auto n = static_cast<std::uint64_t>(legal.size());
    const auto threshold = (std::uint64_t{0} - n) % n;
    for (;;) {
        auto z = (state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        z ^= z >> 31;
        if (z >= threshold) return legal[static_cast<std::size_t>(z % n)];
    }
}
Action HeuristicBot::choose(const Observation& o, const std::vector<Action>& legal) const {
    if (legal.empty()) throw std::invalid_argument("bot requires legal actions");
    const auto self = static_cast<std::size_t>(o.viewer);
    const auto enemy = 1 - self;
    auto value = [](const std::vector<Card>& cards, CardId id) {
        const auto it = std::find_if(cards.begin(), cards.end(), [id](const Card& c) { return c.id == id; });
        if (it == cards.end()) throw std::invalid_argument("legal action absent from observation");
        return static_cast<int>(it->value);
    };
    int best = std::numeric_limits<int>::min();
    Action chosen = legal.front();
    for (const auto& a : legal) {
        int score = 0;
        if (const auto* p = std::get_if<Play>(&a.payload)) {
            const int v = value(o.own_hand, p->card);
            // Establish a strong board; extra weak cards offer little protection.
            score = 20 + v * 10;
        } else if (const auto* p = std::get_if<Attack>(&a.payload)) {
            const int v = value(o.players[self].board, p->card);
            if (o.players[enemy].board.empty()) score = 200 + v;
            else {
                int strongest = 0;
                for (const auto& c : o.players[enemy].board) strongest = std::max(strongest, static_cast<int>(c.value));
                // Avoid offering a stronger defender a free capture; prefer winning trades.
                score = v > strongest ? 100 + v : (v == strongest ? 55 + v : -10 + v);
            }
        } else if (const auto* p = std::get_if<Defend>(&a.payload)) {
            if (!o.pending_attack) throw std::invalid_argument("defense requires pending attack");
            const int attack = value(o.players[enemy].board, o.pending_attack->card);
            const int v = value(o.players[self].board, p->card);
            // Use the cheapest winning defender, then a tie, otherwise sacrifice the weakest.
            score = v > attack ? 200 - v : (v == attack ? 100 - v : -v);
        }
        if (score > best) { best = score; chosen = a; }
    }
    return chosen;
}
}
