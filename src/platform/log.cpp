#include "rofi_hud/platform/log.hpp"

namespace rofi_hud::log {

namespace {
// Errors and warnings are always emitted; debug logging is opt-in.
Level g_min_level = Level::Warning;
}

void set_min_level(Level level) {
    g_min_level = level;
}

Level min_level() {
    return g_min_level;
}

bool enabled(Level level) {
    return static_cast<int>(level) >= static_cast<int>(g_min_level);
}

} // namespace rofi_hud::log
