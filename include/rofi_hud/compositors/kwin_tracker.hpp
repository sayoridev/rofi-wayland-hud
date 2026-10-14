#pragma once

#include "rofi_hud/core/compositor_tracker.hpp"

namespace rofi_hud {

// Active window tracker for KDE Plasma 6 via kdotool.
class KWinTracker final : public CompositorTracker {
public:
    KWinTracker();
    ~KWinTracker() override = default;

protected:
    std::optional<ActiveWindowInfo> detect_active_window() override;
};

} // namespace rofi_hud
