#pragma once

#include "window_tracker.hpp"
#include <memory>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

class SwayTracker : public WindowTracker {
public:
    SwayTracker();
    ~SwayTracker() override = default;

    std::optional<ActiveWindowInfo> get_active_window_info() override;

private:
    std::unique_ptr<sdbus::IConnection> connection_;
};

} // namespace rofi_hud
