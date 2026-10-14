#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <utility>

namespace rofi_hud::log {

enum class Level : int {
    Error = 0,
    Warning = 1,
    Debug = 2,
};

// Logging goes to stderr only. WARNING: in Rofi script-mode any stderr output is
// rendered by Rofi as message lines in the UI, so error messages must stay
// concise and stdout must never be used for logging.
void set_min_level(Level level);
Level min_level();
bool enabled(Level level);

namespace detail {
template <typename... Args>
void write(Level level, Args&&... args) {
    if (!enabled(level)) return;
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args));
    std::cerr << stream.str() << '\n';
}
} // namespace detail

template <typename... Args>
void error(Args&&... args) {
    detail::write(Level::Error, std::forward<Args>(args)...);
}

template <typename... Args>
void warning(Args&&... args) {
    detail::write(Level::Warning, std::forward<Args>(args)...);
}

template <typename... Args>
void debug(Args&&... args) {
    detail::write(Level::Debug, std::forward<Args>(args)...);
}

} // namespace rofi_hud::log
