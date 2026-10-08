#include "rofi_hud/sway_tracker.hpp"
#include "rofi_hud/dbus_discovery.hpp"
#include <iostream>
#include <array>
#include <regex>

namespace rofi_hud {

SwayTracker::SwayTracker() {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        std::cerr << "[SwayTracker Error] Impossible to connect to Session Bus: " << e.what() << std::endl;
    }
}

std::optional<ActiveWindowInfo> SwayTracker::get_active_window_info() {
    if (!connection_) return std::nullopt;

    ActiveWindowInfo info;
    std::string json_output;

    try {
        std::array<char, 1024> buffer;
        FILE* pipe = popen("swaymsg -t get_tree 2>/dev/null", "r");
        if (pipe) {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                json_output += buffer.data();
            }
            pclose(pipe);
        }
    } catch (...) {}

    if (json_output.empty()) return std::nullopt;

    // Cerca il blocco contenente "focused": true
    size_t focused_pos = json_output.find("\"focused\": true");
    if (focused_pos == std::string::npos) {
        focused_pos = json_output.find("\"focused\":true");
    }

    if (focused_pos != std::string::npos) {
        // Cerca all'interno di una finestra ragionevole di caratteri prima e dopo la proprietà focused
        size_t start_pos = (focused_pos > 1024) ? focused_pos - 1024 : 0;
        size_t length = std::min<size_t>(2048, json_output.size() - start_pos);
        std::string block = json_output.substr(start_pos, length);

        std::regex pid_regex(R"("pid":\s*(\d+))");
        std::smatch match;
        if (std::regex_search(block, match, pid_regex) && match.size() > 1) {
            try {
                info.pid = static_cast<uint32_t>(std::stoul(match[1].str()));
            } catch (...) {}
        }

        std::regex name_regex(R"raw("name":\s*"([^"]*)")raw");
        if (std::regex_search(block, match, name_regex) && match.size() > 1) {
            info.app_title = match[1].str();
        }

        std::regex app_id_regex(R"raw("app_id":\s*"([^"]*)")raw");
        if (std::regex_search(block, match, app_id_regex) && match.size() > 1) {
            info.app_class = match[1].str();
        }
    }

    if (info.pid == 0) return std::nullopt;

    if (info.app_title.empty()) {
        info.app_title = DBusDiscovery::get_process_name(info.pid);
    }

    if (DBusDiscovery::resolve_dbus_menu_for_pid(*connection_, info.pid, info.service_name, info.object_path)) {
        return info;
    }

    return std::nullopt;
}

} // namespace rofi_hud
