#pragma once

#include <string>
#include <optional>
#include <memory>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

struct ActiveWindowInfo {
    std::string service_name; //dbus service name
    std::string object_path; //object path
    std::string app_title; //app title
};

class KWinTracker {
public:
    KWinTracker();
    ~KWinTracker() = default;

    //recover window info from dbus
    std::optional<ActiveWindowInfo> get_active_window_info();

private:
    std::unique_ptr<sdbus::IConnection> connection_;
};

}