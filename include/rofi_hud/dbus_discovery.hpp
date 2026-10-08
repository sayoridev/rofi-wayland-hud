#pragma once

#include <string>
#include <cstdint>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

class DBusDiscovery {
public:
    // Trova il bus univoco (:1.xxx) per un processo dato il suo PID
    static std::string resolve_owner_bus_name(sdbus::IConnection& connection, uint32_t pid);

    // Risolve servizio e path D-Bus per un determinato PID, testando candidate paths e introspezione
    static bool resolve_dbus_menu_for_pid(sdbus::IConnection& connection,
                                          uint32_t pid,
                                          std::string& out_service,
                                          std::string& out_path);

    // Legge il nome del processo da /proc/<pid>/comm
    static std::string get_process_name(uint32_t pid);

    // Legge il PPID da /proc/<pid>/status
    static uint32_t get_parent_pid(uint32_t pid);

private:
    static bool check_path_implements_dbusmenu(sdbus::IConnection& connection,
                                               const std::string& bus_name,
                                               const std::string& path);

    static bool check_registrar_for_window(sdbus::IConnection& connection,
                                           uint32_t id,
                                           std::string& out_service,
                                           std::string& out_path);
};

} // namespace rofi_hud
