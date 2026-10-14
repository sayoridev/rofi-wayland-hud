#pragma once

#include "rofi_hud/core/window_tracker.hpp"
#include <memory>

namespace sdbus {
class IConnection;
}

namespace rofi_hud {

// Base class shared by all compositor trackers.
//
// Owns the D-Bus session connection and implements the common tail of
// get_active_window_info(): menu resolution via DBusDiscovery and the
// process-name fallback for the window title. Concrete trackers only implement
// detect_active_window(), which must fill at least the process id.
class CompositorTracker : public WindowTracker {
public:
    ~CompositorTracker() override;

    std::optional<ActiveWindowInfo> get_active_window_info() override;

protected:
    // `tracker_name` is used as a prefix for error reporting.
    explicit CompositorTracker(const char* tracker_name);

    sdbus::IConnection* connection() const { return connection_.get(); }

    // Compositor-specific active window detection. Must fill at least `pid`;
    // `app_title`/`app_class` may be left empty (title falls back to the
    // process name in get_active_window_info()).
    virtual std::optional<ActiveWindowInfo> detect_active_window() = 0;

private:
    std::unique_ptr<sdbus::IConnection> connection_;
};

} // namespace rofi_hud
