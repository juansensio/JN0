#pragma once
#include "janus/game.hpp"
#include "janus/bots.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {
class JanusGame : public Node {
    GDCLASS(JanusGame, Node)
    janus::Game game_;
    janus::Replay replay_;
    janus::RandomBot random_{42};
    Dictionary accept(const janus::Action &action);
protected:
    static void _bind_methods();
public:
    JanusGame();
    Dictionary start(const String &seed);
    Dictionary observe(int64_t viewer) const;
    Array legal_actions(int64_t actor) const;
    Dictionary submit(int64_t actor, const String &kind, int64_t card);
    Dictionary bot_step(int64_t actor, const String &kind);
    String export_replay() const;
    Dictionary load_replay(const String &json);
};
}
