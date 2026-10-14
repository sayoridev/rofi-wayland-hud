#pragma once

#include "rofi_hud/core/compositor_tracker.hpp"

namespace rofi_hud {

// Active window tracker for Hyprland via hyprctl.
class HyprlandTracker final : public CompositorTracker {
public:
    HyprlandTracker();
    ~HyprlandTracker() override = default;

protected:
    std::optional<ActiveWindowInfo> detect_active_window() override;
};

} // namespace rofi_hud
