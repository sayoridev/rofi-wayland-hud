#include "rofi_hud/dbus/dbus_discovery.hpp"
#include "rofi_hud/dbus/timeouts.hpp"
#include "rofi_hud/platform/log.hpp"
#include "rofi_hud/platform/string_utils.hpp"
#include <chrono>
#include <map>
#include <cstdint>
#include <fstream>
#include <regex>
#include <string_view>
#include <vector>

namespace rofi_hud {

namespace {
// Object paths probed in order when looking for an exported dbusmenu.
// Qt/KDE applications create menu bars with dynamic indices (/MenuBar/1..8).
constexpr std::string_view kKnownMenuPaths[] = {
    "/MenuBar/1",
    "/MenuBar/2",
    "/MenuBar/3",
    "/MenuBar/4",
    "/MenuBar/5",
    "/MenuBar/6",
    "/MenuBar/7",
    "/MenuBar/8",
    "/com/canonical/dbusmenu",
    "/org/ayatana/NotificationItem/MenuBar",
    "/org/gtk/menus",
    "/MenuBar",
    "/AppMenu",
};

// sdbus-c++ takes timeouts as integer microseconds.
uint64_t microseconds(std::chrono::microseconds duration) {
    return static_cast<uint64_t>(duration.count());
}
} // namespace

std::string DBusDiscovery::get_process_name(uint32_t pid) {
    if (pid == 0) return {};
    std::ifstream comm_file("/proc/" + std::to_string(pid) + "/comm");
    std::string comm;
    if (std::getline(comm_file, comm)) {
        return trim(comm);
    }
    return {};
}

uint32_t DBusDiscovery::get_parent_pid(uint32_t pid) {
    if (pid == 0) return 0;
    std::ifstream status_file("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.rfind("PPid:", 0) == 0) {
            uint32_t ppid = 0;
            if (parse_uint32(line.substr(5), ppid)) {
                return ppid;
            }
            return 0;
        }
    }
    return 0;
}

std::string DBusDiscovery::resolve_owner_bus_name(sdbus::IConnection& connection, uint32_t pid) {
    if (pid == 0) return {};

    try {
        auto dbus_proxy = sdbus::createProxy(
            connection,
            sdbus::ServiceName("org.freedesktop.DBus"),
            sdbus::ObjectPath("/org/freedesktop/DBus"));

        std::vector<std::string> names;
        dbus_proxy->callMethod("ListNames")
                  .onInterface("org.freedesktop.DBus")
                  .withTimeout(microseconds(dbus_timeouts::kListNames))
                  .storeResultsTo(names);

        // Linear scan over unique names (":1.x") until the owner of `pid` is
        // found. This is O(unique connections); kept for behavior compatibility.
        for (const auto& name : names) {
            if (!name.empty() && name[0] == ':') {
                try {
                    uint32_t owner_pid = 0;
                    dbus_proxy->callMethod("GetConnectionUnixProcessID")
                              .onInterface("org.freedesktop.DBus")
                              .withArguments(name)
                              .withTimeout(microseconds(dbus_timeouts::kGetConnectionPid))
                              .storeResultsTo(owner_pid);
                    if (owner_pid == pid) {
                        return name;
                    }
                } catch (...) {
                    // The name may vanish mid-scan; skip it.
                }
            }
        }
    } catch (const sdbus::Error& e) {
        log::error("[DBusDiscovery] ListNames failed: ", e.what());
    }
    return {};
}

bool DBusDiscovery::path_implements_dbusmenu(sdbus::IConnection& connection,
                                             const std::string& bus_name,
                                             const std::string& path) {
    try {
        auto proxy = sdbus::createProxy(connection,
                                        sdbus::ServiceName(bus_name),
                                        sdbus::ObjectPath(path));
        uint32_t revision = 0;
        sdbus::Struct<int32_t, std::map<std::string, sdbus::Variant>, std::vector<sdbus::Variant>> dummy;

        // Shallow GetLayout probe (depth 1): if the object answers, it is a
        // dbusmenu exporter.
        proxy->callMethod("GetLayout")
             .onInterface("com.canonical.dbusmenu")
             .withArguments(0, 1, std::vector<std::string>{})
             .withTimeout(microseconds(dbus_timeouts::kDiscoveryCall))
             .storeResultsTo(revision, dummy);
        return true;
    } catch (...) {
        return false;
    }
}

bool DBusDiscovery::query_appmenu_registrar(sdbus::IConnection& connection,
                                            uint32_t id,
                                            std::string& out_service,
                                            std::string& out_path) {
    try {
        auto registrar_proxy = sdbus::createProxy(
            connection,
            sdbus::ServiceName("com.canonical.AppMenu.Registrar"),
            sdbus::ObjectPath("/com/canonical/AppMenu/Registrar"));

        sdbus::ObjectPath path_result;
        std::string service_result;
        registrar_proxy->callMethod("GetMenuForWindow")
                       .onInterface("com.canonical.AppMenu.Registrar")
                       .withArguments(id)
                       .withTimeout(microseconds(dbus_timeouts::kRegistrarCall))
                       .storeResultsTo(service_result, path_result);

        if (service_result.empty() || path_result.empty()) return false;

        // The registrar is designed for X11 window ids and often misbehaves on
        // Wayland, so the returned service name is validated: it must be a
        // well-known name (contains '.') or a unique name (starts with ':').
        const bool plausible = service_result.find('.') != std::string::npos
                            || service_result[0] == ':';
        if (plausible) {
            out_service = service_result;
            out_path = path_result;
            return true;
        }

        const std::string valid_bus = resolve_owner_bus_name(connection, id);
        if (!valid_bus.empty()) {
            out_service = valid_bus;
            out_path = path_result;
            return true;
        }
    } catch (...) {
        // Registrar absent or unresponsive: the remaining strategies follow.
    }
    return false;
}

bool DBusDiscovery::try_known_menu_paths(sdbus::IConnection& connection,
                                         const std::string& bus_name,
                                         std::string& out_path) {
    for (const std::string_view path : kKnownMenuPaths) {
        if (path_implements_dbusmenu(connection, bus_name, std::string(path))) {
            out_path = std::string(path);
            return true;
        }
    }
    return false;
}

bool DBusDiscovery::try_menubar_introspection(sdbus::IConnection& connection,
                                              const std::string& bus_name,
                                              std::string& out_path) {
    try {
        auto proxy = sdbus::createProxy(connection,
                                        sdbus::ServiceName(bus_name),
                                        sdbus::ObjectPath("/MenuBar"));
        std::string xml;
        proxy->callMethod("Introspect")
             .onInterface("org.freedesktop.DBus.Introspectable")
             .withTimeout(microseconds(dbus_timeouts::kDiscoveryCall))
             .storeResultsTo(xml);

        // Child nodes are advertised as <node name="N"/>; probe each /MenuBar/N.
        const std::regex node_regex{R"raw(<node\s+name="([^"]+)"/>)raw"};
        for (auto it = std::sregex_iterator(xml.begin(), xml.end(), node_regex);
             it != std::sregex_iterator(); ++it) {
            const std::string subpath = "/MenuBar/" + it->str(1);
            if (path_implements_dbusmenu(connection, bus_name, subpath)) {
                out_path = subpath;
                return true;
            }
        }
    } catch (...) {
        // No introspectable /MenuBar node on this bus name.
    }
    return false;
}

bool DBusDiscovery::resolve_dbus_menu_for_pid(sdbus::IConnection& connection,
                                              uint32_t pid,
                                              std::string& out_service,
                                              std::string& out_path) {
    if (pid == 0) return false;

    // Electron/Chromium apps export their menu from the browser process, so the
    // parent pid is probed as well.
    const uint32_t parent_pid = get_parent_pid(pid);
    std::vector<uint32_t> candidate_pids{pid};
    if (parent_pid > 1 && parent_pid != pid) {
        candidate_pids.push_back(parent_pid);
    }

    for (const uint32_t target_pid : candidate_pids) {
        // 1. Best source of truth when the registrar is available.
        if (query_appmenu_registrar(connection, target_pid, out_service, out_path)) {
            return true;
        }

        // 2. Unique bus name owned by the target pid.
        const std::string bus_name = resolve_owner_bus_name(connection, target_pid);
        if (bus_name.empty()) continue;

        // 3. Well-known menu object paths.
        if (try_known_menu_paths(connection, bus_name, out_path)) {
            out_service = bus_name;
            return true;
        }

        // 4. Dynamic discovery of /MenuBar/<N> children.
        if (try_menubar_introspection(connection, bus_name, out_path)) {
            out_service = bus_name;
            return true;
        }
    }
    return false;
}

} // namespace rofi_hud
