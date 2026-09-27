#pragma once

#include "janus/game.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
class JanusGame : public Node {
    GDCLASS(JanusGame, Node)

    janus::Game game_;

protected:
    static void _bind_methods();

public:
    JanusGame() = default;
    void _ready() override;
    void smoke_test();
};
} // namespace godot
