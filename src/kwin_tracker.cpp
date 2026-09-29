#include "rofi_hud/kwin_tracker.hpp"
#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <fstream>
#include <algorithm>
#include <unistd.h>

namespace rofi_hud {

KWinTracker::KWinTracker() {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        std::cerr << "[KWinTracker Error] Connection failed: " << e.what() << std::endl;
    }
}

std::optional<ActiveWindowInfo> KWinTracker::get_active_window_info() {
    if (!connection_) return std::nullopt;

    usleep(100000);

    auto exec_cmd = [](const std::string& cmd) -> std::string {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    };

    try {
        // 1. get id and pid of the window from kdotool 
        std::string wid_str = exec_cmd("kdotool getactivewindow 2>/dev/null");
        if (wid_str.empty()) return std::nullopt;

        std::string pid_str = exec_cmd("kdotool getwindowpid " + wid_str + " 2>/dev/null");
        if (pid_str.empty()) return std::nullopt;

        int32_t active_pid = std::stoi(pid_str);
        if (active_pid <= 0) return std::nullopt;

        std::ifstream comm_file("/proc/" + std::to_string(active_pid) + "/comm");
        std::string desktop_file;
        if (comm_file.is_open()) {
            std::getline(comm_file, desktop_file);
            desktop_file.erase(std::remove(desktop_file.begin(), desktop_file.end(), '\n'), desktop_file.end());
            desktop_file.erase(std::remove(desktop_file.begin(), desktop_file.end(), '\r'), desktop_file.end());
        }

        if (desktop_file == "kitty" || desktop_file == "konsole" || desktop_file == "alacritty" || desktop_file.empty()) {
            return std::nullopt;
        }

        std::string dbus_unique_name;
        {
            auto dbus_proxy = sdbus::createProxy(
                *connection_,
                sdbus::ServiceName{"org.freedesktop.DBus"},
                sdbus::ObjectPath{"/org/freedesktop/DBus"}
            );

            std::vector<std::string> names;
            dbus_proxy->callMethod("ListNames")
                      .onInterface("org.freedesktop.DBus")
                      .storeResultsTo(names);

            for (const auto& name : names) {
                if (!name.empty() && name[0] == ':') {
                    try {
                        uint32_t pid = 0;
                        dbus_proxy->callMethod("GetConnectionUnixProcessID")
                                  .onInterface("org.freedesktop.DBus")
                                  .withArguments(name)
                                  .storeResultsTo(pid);
                        
                        if (pid == static_cast<uint32_t>(active_pid)) {
                            dbus_unique_name = name;
                            break;
                        }
                    } catch (...) {}
                }
            }
        }

        if (dbus_unique_name.empty()) {
            return std::nullopt;
        }

        ActiveWindowInfo info;
        info.service_name = dbus_unique_name; 
        info.object_path = "/MenuBar/2"; 
        return info;

    } catch (const std::exception& e) {
        std::cerr << "[KWinTracker Error] " << e.what() << std::endl;
    }

    return std::nullopt;
}

} 