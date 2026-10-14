#pragma once

#include "rofi_hud/core/compositor_tracker.hpp"

namespace rofi_hud {

// Active window tracker for Sway / wlroots via swaymsg.
class SwayTracker final : public CompositorTracker {
public:
    SwayTracker();
    ~SwayTracker() override = default;

protected:
    std::optional<ActiveWindowInfo> detect_active_window() override;
};

} // namespace rofi_hud
