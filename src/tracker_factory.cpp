#include "rofi_hud/tracker_factory.hpp"
#include "rofi_hud/kwin_tracker.hpp"
#include "rofi_hud/hyprland_tracker.hpp"
#include "rofi_hud/sway_tracker.hpp"
#include <cstdlib>
#include <string>
#include <algorithm>

namespace rofi_hud {

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::unique_ptr<WindowTracker> TrackerFactory::create_tracker() {
    // 1. Controllo variabili d'ambiente specifiche del compositore
    if (std::getenv("HYPRLAND_INSTANCE_SIGNATURE") != nullptr) {
        return std::make_unique<HyprlandTracker>();
    }

    if (std::getenv("SWAYSOCK") != nullptr) {
        return std::make_unique<SwayTracker>();
    }

    // 2. Controllo XDG_CURRENT_DESKTOP
    const char* desktop_env = std::getenv("XDG_CURRENT_DESKTOP");
    if (desktop_env) {
        std::string desktop = to_lower(desktop_env);
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

    // 3. Fallback di default: KWinTracker
    return std::make_unique<KWinTracker>();
}

} // namespace rofi_hud
