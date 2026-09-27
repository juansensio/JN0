#pragma once
#include "janus/game.hpp"
#include <stdexcept>
#include <string>
#include <string_view>

namespace janus {
// Parsing failures throw invalid_argument; execution identifies the first bad action.
class ReplayExecutionError : public std::invalid_argument {
public:
    ReplayExecutionError(std::size_t index, ActionError error);
    const std::size_t action_index;
    const ActionError action_error;
};
[[nodiscard]] Replay parse_replay(std::string_view json);
[[nodiscard]] std::string encode_replay(const Replay& replay);
// Complete-match validation is the default. No partial state is returned on failure.
[[nodiscard]] GameState execute_replay(const Replay& replay, bool require_terminal = true);
}
