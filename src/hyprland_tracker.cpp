#include "rofi_hud/hyprland_tracker.hpp"
#include "rofi_hud/dbus_discovery.hpp"
#include <iostream>
#include <array>
#include <regex>

namespace rofi_hud {

HyprlandTracker::HyprlandTracker() {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        std::cerr << "[HyprlandTracker Error] Impossible to connect to Session Bus: " << e.what() << std::endl;
    }
}

std::optional<ActiveWindowInfo> HyprlandTracker::get_active_window_info() {
    if (!connection_) return std::nullopt;

    ActiveWindowInfo info;
    std::string json_output;

    try {
        std::array<char, 512> buffer;
        FILE* pipe = popen("hyprctl activewindow -j 2>/dev/null", "r");
        if (pipe) {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                json_output += buffer.data();
            }
            pclose(pipe);
        }
    } catch (...) {}

    if (json_output.empty()) return std::nullopt;

    // Estrae il pid
    std::regex pid_regex(R"("pid":\s*(\d+))");
    std::smatch match;
    if (std::regex_search(json_output, match, pid_regex) && match.size() > 1) {
        try {
            info.pid = static_cast<uint32_t>(std::stoul(match[1].str()));
        } catch (...) {}
    }

    if (info.pid == 0) return std::nullopt;

    // Estrae il titolo
    std::regex title_regex(R"raw("title":\s*"([^"]*)")raw");
    if (std::regex_search(json_output, match, title_regex) && match.size() > 1) {
        info.app_title = match[1].str();
    }

    // Estrae la classe
    std::regex class_regex(R"raw("class":\s*"([^"]*)")raw");
    if (std::regex_search(json_output, match, class_regex) && match.size() > 1) {
        info.app_class = match[1].str();
    }

    if (info.app_title.empty()) {
        info.app_title = DBusDiscovery::get_process_name(info.pid);
    }

    if (DBusDiscovery::resolve_dbus_menu_for_pid(*connection_, info.pid, info.service_name, info.object_path)) {
        return info;
    }

    return std::nullopt;
}

} // namespace rofi_hud
