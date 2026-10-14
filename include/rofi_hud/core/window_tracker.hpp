#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace rofi_hud {

// Snapshot of the currently focused window together with the D-Bus coordinates
// of the menu exported by its application.
struct ActiveWindowInfo {
    std::string service_name; // D-Bus service name (e.g. ":1.42" or "org.example.app")
    std::string object_path;  // D-Bus object path (e.g. "/MenuBar/2")
    std::string app_title;    // Active window title
    std::string app_class;    // Application class name
    uint32_t pid{0};          // Process ID
};

// Compositor-specific strategy for detecting the active window.
// Implementations only detect the window (pid/title/class); menu resolution is
// provided by the CompositorTracker base class.
class WindowTracker {
public:
    virtual ~WindowTracker() = default;

    // Returns the active window info, or std::nullopt when the active window
    // cannot be determined or exports no D-Bus menu.
    virtual std::optional<ActiveWindowInfo> get_active_window_info() = 0;
};

} // namespace rofi_hud
