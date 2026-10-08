#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace rofi_hud {

struct ActiveWindowInfo {
    std::string service_name; // D-Bus service name (e.g. :1.xxx or org.example.app)
    std::string object_path;  // D-Bus object path (e.g. /MenuBar/2)
    std::string app_title;    // Active window title
    std::string app_class;    // Application class name
    uint32_t pid{0};          // Process ID
};

class WindowTracker {
public:
    virtual ~WindowTracker() = default;

    // Recupera informazioni sulla finestra attiva e il relativo menu D-Bus
    virtual std::optional<ActiveWindowInfo> get_active_window_info() = 0;
};

} // namespace rofi_hud
