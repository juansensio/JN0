#include "janus/types.hpp"

namespace janus {
ConfigError validate_config(const GameConfig& config) noexcept {
    if (config.rules_version != 1) return ConfigError::unsupported_rules_version;
    if (config != GameConfig{}) return ConfigError::unsupported_setup;
    return ConfigError::none;
}
} // namespace janus
