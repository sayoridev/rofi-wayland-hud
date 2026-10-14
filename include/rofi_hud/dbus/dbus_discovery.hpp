#pragma once

#include <cstdint>
#include <string>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

// Resolves the D-Bus coordinates (service name + object path) of the
// application menu exported by a process, trying several strategies in order:
//   1. com.canonical.AppMenu.Registrar
//   2. a static list of well-known menu object paths (/MenuBar/1..8, ...)
//   3. dynamic introspection of /MenuBar (Qt/KDE dynamic menu indices)
// The parent process is probed as well, because Electron/Chromium apps export
// their menu from the browser process rather than the renderer.
class DBusDiscovery {
public:
    // Returns the unique bus name (":1.x") owned by `pid`, or an empty string.
    // Note: scans all unique names linearly (O(unique connections)); kept for
    // behavior compatibility.
    static std::string resolve_owner_bus_name(sdbus::IConnection& connection, uint32_t pid);

    // Resolves service and object path of the menu for `pid`. Returns false
    // when no exporting object can be found.
    static bool resolve_dbus_menu_for_pid(sdbus::IConnection& connection,
                                          uint32_t pid,
                                          std::string& out_service,
                                          std::string& out_path);

    // Reads the process name from /proc/<pid>/comm ("" on failure).
    static std::string get_process_name(uint32_t pid);

    // Reads the parent pid from /proc/<pid>/status (0 on failure).
    static uint32_t get_parent_pid(uint32_t pid);

private:
    // True when `path` answers a shallow GetLayout on com.canonical.dbusmenu.
    static bool path_implements_dbusmenu(sdbus::IConnection& connection,
                                         const std::string& bus_name,
                                         const std::string& path);

    // Queries the AppMenu registrar; validates the returned service name.
    static bool query_appmenu_registrar(sdbus::IConnection& connection,
                                        uint32_t id,
                                        std::string& out_service,
                                        std::string& out_path);

    // Probes the well-known menu object paths against `bus_name`.
    static bool try_known_menu_paths(sdbus::IConnection& connection,
                                     const std::string& bus_name,
                                     std::string& out_path);

    // Discovers /MenuBar/<N> children via introspection and probes each.
    static bool try_menubar_introspection(sdbus::IConnection& connection,
                                          const std::string& bus_name,
                                          std::string& out_path);
};

} // namespace rofi_hud
