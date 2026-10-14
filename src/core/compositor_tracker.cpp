#include "rofi_hud/core/compositor_tracker.hpp"
#include "rofi_hud/dbus/dbus_discovery.hpp"
#include "rofi_hud/platform/log.hpp"
#include "rofi_hud/platform/string_utils.hpp"
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

CompositorTracker::CompositorTracker(const char* tracker_name) {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        // Same message format as the original per-tracker constructors, so the
        // Rofi-visible stderr output is unchanged.
        log::error("[", tracker_name, " Error] Impossible to connect to Session Bus: ", e.what());
    }
}

CompositorTracker::~CompositorTracker() = default;

std::optional<ActiveWindowInfo> CompositorTracker::get_active_window_info() {
    if (!connection_) return std::nullopt;

    auto info = detect_active_window();
    if (!info || info->pid == 0) return std::nullopt;

    if (info->app_title.empty()) {
        info->app_title = DBusDiscovery::get_process_name(info->pid);
    }

    if (DBusDiscovery::resolve_dbus_menu_for_pid(*connection_,
                                                 info->pid,
                                                 info->service_name,
                                                 info->object_path)) {
        return info;
    }
    return std::nullopt;
}

} // namespace rofi_hud
