#include "rofi_hud/compositors/tracker_factory.hpp"
#include "rofi_hud/compositors/hyprland_tracker.hpp"
#include "rofi_hud/compositors/kwin_tracker.hpp"
#include "rofi_hud/compositors/sway_tracker.hpp"
#include <algorithm>
#include <string>
#include <cctype>
#include <cstdlib>

namespace rofi_hud {

namespace {
std::string to_lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}
} // namespace

std::unique_ptr<WindowTracker> TrackerFactory::create_tracker() {
    // 1. Compositor-specific environment variables (most reliable signal).
    if (std::getenv("HYPRLAND_INSTANCE_SIGNATURE") != nullptr) {
        return std::make_unique<HyprlandTracker>();
    }
    if (std::getenv("SWAYSOCK") != nullptr) {
        return std::make_unique<SwayTracker>();
    }

    // 2. Desktop name reported by the session.
    if (const char* desktop_env = std::getenv("XDG_CURRENT_DESKTOP")) {
        const std::string desktop = to_lower(desktop_env);
        if (desktop.find("hyprland") != std::string::npos) {
            return std::make_unique<HyprlandTracker>();
        }
        if (desktop.find("sway") != std::string::npos) {
            return std::make_unique<SwayTracker>();
        }
        if (desktop.find("kde") != std::string::npos || desktop.find("plasma") != std::string::npos) {
            return std::make_unique<KWinTracker>();
        }
    }

    // 3. Default fallback.
    return std::make_unique<KWinTracker>();
}

} // namespace rofi_hud
