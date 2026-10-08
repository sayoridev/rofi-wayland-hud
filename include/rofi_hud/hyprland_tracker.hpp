#pragma once

#include "window_tracker.hpp"
#include <memory>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

class HyprlandTracker : public WindowTracker {
public:
    HyprlandTracker();
    ~HyprlandTracker() override = default;

    std::optional<ActiveWindowInfo> get_active_window_info() override;

private:
    std::unique_ptr<sdbus::IConnection> connection_;
};

} // namespace rofi_hud
