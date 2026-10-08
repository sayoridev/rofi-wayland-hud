#include "rofi_hud/kwin_tracker.hpp"
#include "rofi_hud/dbus_discovery.hpp"
#include <iostream>
#include <array>
#include <vector>

namespace rofi_hud {

KWinTracker::KWinTracker() {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        std::cerr << "[KWinTracker Error] Impossible to connect to Session Bus: " << e.what() << std::endl;
    }
}

std::optional<ActiveWindowInfo> KWinTracker::get_active_window_info() {
    if (!connection_) return std::nullopt;

    ActiveWindowInfo info;

    // Recupera PID, titolo e classe della finestra attiva in una singola invocazione di kdotool
    try {
        std::array<char, 256> buffer;
        FILE* pipe = popen("kdotool getactivewindow getwindowpid %1 getwindowname %1 getwindowclassname %1 2>/dev/null", "r");
        if (pipe) {
            std::vector<std::string> lines;
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                std::string line = buffer.data();
                line.erase(line.find_last_not_of(" \n\r\t") + 1);
                lines.push_back(line);
            }
            pclose(pipe);

            if (!lines.empty() && !lines[0].empty()) {
                try {
                    info.pid = static_cast<uint32_t>(std::stoul(lines[0]));
                } catch (...) {}
            }
            if (lines.size() > 1) {
                info.app_title = lines[1];
            }
            if (lines.size() > 2) {
                info.app_class = lines[2];
            }
        }
    } catch (...) {}

    // Fallback: se kdotool con argomenti multipli fallisce nel recuperare il PID, prova il comando base
    if (info.pid == 0) {
        try {
            std::array<char, 128> buffer;
            FILE* pipe_pid = popen("kdotool getactivewindow getwindowpid 2>/dev/null", "r");
            if (pipe_pid) {
                if (fgets(buffer.data(), buffer.size(), pipe_pid) != nullptr) {
                    std::string pid_str = buffer.data();
                    pid_str.erase(pid_str.find_last_not_of(" \n\r\t") + 1);
                    if (!pid_str.empty()) info.pid = static_cast<uint32_t>(std::stoul(pid_str));
                }
                pclose(pipe_pid);
            }
        } catch (...) {}
    }

    if (info.pid == 0) return std::nullopt;

    // Se il titolo è ancora vuoto, usa il nome del processo
    if (info.app_title.empty()) {
        info.app_title = DBusDiscovery::get_process_name(info.pid);
    }

    if (DBusDiscovery::resolve_dbus_menu_for_pid(*connection_, info.pid, info.service_name, info.object_path)) {
        return info;
    }

    return std::nullopt;
}

} // namespace rofi_hud