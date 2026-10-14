#pragma once

#include "rofi_hud/core/window_tracker.hpp"
#include <memory>

namespace rofi_hud {

// Creates the WindowTracker matching the compositor of the current session,
// detected from environment variables:
//   HYPRLAND_INSTANCE_SIGNATURE -> HyprlandTracker
//   SWAYSOCK                    -> SwayTracker
//   XDG_CURRENT_DESKTOP         -> compositor name substring match
// Fallback (no match)           -> KWinTracker
class TrackerFactory {
public:
    static std::unique_ptr<WindowTracker> create_tracker();
};

} // namespace rofi_hud
