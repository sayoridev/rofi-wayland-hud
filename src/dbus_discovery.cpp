#include "rofi_hud/dbus_discovery.hpp"
#include <fstream>
#include <regex>
#include <vector>
#include <iostream>

namespace rofi_hud {

std::string DBusDiscovery::get_process_name(uint32_t pid) {
    if (pid == 0) return "";
    std::ifstream comm_file("/proc/" + std::to_string(pid) + "/comm");
    std::string comm;
    if (std::getline(comm_file, comm)) {
        comm.erase(comm.find_last_not_of(" \n\r\t") + 1);
        return comm;
    }
    return "";
}

uint32_t DBusDiscovery::get_parent_pid(uint32_t pid) {
    if (pid == 0) return 0;
    std::ifstream status_file("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.rfind("PPid:", 0) == 0) {
            try {
                std::string ppid_str = line.substr(5);
                ppid_str.erase(0, ppid_str.find_first_not_of(" \t"));
                ppid_str.erase(ppid_str.find_last_not_of(" \n\r\t") + 1);
                return static_cast<uint32_t>(std::stoul(ppid_str));
            } catch (...) {
                return 0;
            }
        }
    }
    return 0;
}

std::string DBusDiscovery::resolve_owner_bus_name(sdbus::IConnection& connection, uint32_t pid) {
    if (pid == 0) return "";

    try {
        auto dbus_proxy = sdbus::createProxy(
            connection,
            "org.freedesktop.DBus",
            "/org/freedesktop/DBus"
        );

        std::vector<std::string> names;
        dbus_proxy->callMethod("ListNames")
                  .onInterface("org.freedesktop.DBus")
                  .withTimeout(100000)
                  .storeResultsTo(names);

        // Prima cerchiamo una corrispondenza diretta tra i bus unici (:1.xxx)
        for (const auto& name : names) {
            if (!name.empty() && name[0] == ':') {
                try {
                    uint32_t owner_pid = 0;
                    dbus_proxy->callMethod("GetConnectionUnixProcessID")
                              .onInterface("org.freedesktop.DBus")
                              .withArguments(name)
                              .withTimeout(30000)
                              .storeResultsTo(owner_pid);

                    if (owner_pid == pid) {
                        return name;
                    }
                } catch (...) {}
            }
        }
    } catch (...) {}

    return "";
}

bool DBusDiscovery::check_path_implements_dbusmenu(sdbus::IConnection& connection,
                                                   const std::string& bus_name,
                                                   const std::string& path) {
    try {
        auto proxy = sdbus::createProxy(connection, bus_name, path);
        uint32_t revision = 0;
        sdbus::Struct<int32_t, std::map<std::string, sdbus::Variant>, std::vector<sdbus::Variant>> dummy;

        proxy->callMethod("GetLayout")
             .onInterface("com.canonical.dbusmenu")
             .withArguments(0, 1, std::vector<std::string>{})
             .withTimeout(50000)
             .storeResultsTo(revision, dummy);

        return true;
    } catch (...) {
        return false;
    }
}

bool DBusDiscovery::check_registrar_for_window(sdbus::IConnection& connection,
                                               uint32_t id,
                                               std::string& out_service,
                                               std::string& out_path) {
    try {
        auto registrar_proxy = sdbus::createProxy(
            connection,
            "com.canonical.AppMenu.Registrar",
            "/com/canonical/AppMenu/Registrar"
        );

        sdbus::ObjectPath path_result;
        std::string service_result;

        registrar_proxy->callMethod("GetMenuForWindow")
                       .onInterface("com.canonical.AppMenu.Registrar")
                       .withArguments(id)
                       .withTimeout(50000) // Timeout breve per evitare blocchi
                       .storeResultsTo(service_result, path_result);

        if (!service_result.empty() && !path_result.empty()) {
            if (service_result.find('.') == std::string::npos && service_result[0] != ':') {
                std::string valid_bus = resolve_owner_bus_name(connection, id);
                if (!valid_bus.empty()) {
                    out_service = valid_bus;
                    out_path = path_result;
                    return true;
                }
            } else {
                out_service = service_result;
                out_path = path_result;
                return true;
            }
        }
    } catch (...) {}

    return false;
}

bool DBusDiscovery::resolve_dbus_menu_for_pid(sdbus::IConnection& connection,
                                              uint32_t pid,
                                              std::string& out_service,
                                              std::string& out_path) {
    if (pid == 0) return false;

    // Tentativo con il PID corrente ed eventualmente con il genitore (es. per Electron/Chromium)
    std::vector<uint32_t> candidate_pids = {pid};
    uint32_t parent_pid = get_parent_pid(pid);
    if (parent_pid > 1 && parent_pid != pid) {
        candidate_pids.push_back(parent_pid);
    }

    const std::vector<std::string> candidate_paths = {
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
        "/AppMenu"
    };

    for (uint32_t target_pid : candidate_pids) {
        // 1. Prova registrar se disponibile
        if (check_registrar_for_window(connection, target_pid, out_service, out_path)) {
            return true;
        }

        // 2. Risolvi bus name per il PID
        std::string bus_name = resolve_owner_bus_name(connection, target_pid);
        if (bus_name.empty()) continue;

        // 3. Prova candidate paths comuni
        for (const auto& path : candidate_paths) {
            if (check_path_implements_dbusmenu(connection, bus_name, path)) {
                out_service = bus_name;
                out_path = path;
                return true;
            }
        }

        // 4. Se nessun candidate path ha risposto, prova introspezione di /MenuBar
        try {
            auto proxy = sdbus::createProxy(connection, bus_name, "/MenuBar");
            std::string xml;
            proxy->callMethod("Introspect")
                 .onInterface("org.freedesktop.DBus.Introspectable")
                 .withTimeout(50000)
                 .storeResultsTo(xml);

            // Cerca nodi figli come <node name="X"/>
            std::regex node_regex(R"raw(<node\s+name="([^"]+)"/>)raw");
            auto words_begin = std::sregex_iterator(xml.begin(), xml.end(), node_regex);
            auto words_end = std::sregex_iterator();

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                std::string subpath = "/MenuBar/" + match[1].str();
                if (check_path_implements_dbusmenu(connection, bus_name, subpath)) {
                    out_service = bus_name;
                    out_path = subpath;
                    return true;
                }
            }
        } catch (...) {}
    }

    return false;
}

} // namespace rofi_hud
