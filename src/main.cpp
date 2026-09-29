#include "rofi_hud/kwin_tracker.hpp"
#include "rofi_hud/dbus_menu_client.hpp"
#include "rofi_hud/rofi_formatter.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    //read variable from rofi
    const char* retv_env = std::getenv("ROFI_RETV");
    const char* info_env = std::getenv("ROFI_INFO");

    int rofi_retv = retv_env ? std::atoi(retv_env) : 0;

    if (rofi_retv == 1 && info_env != nullptr) {
        int32_t selected_id = std::atoi(info_env);

        rofi_hud::KWinTracker tracker;
        auto window_info = tracker.get_active_window_info();

        if (window_info) {
            rofi_hud::DBusMenuClient client(window_info->service_name, window_info->object_path);
            client.trigger_action(selected_id);
        }
        return 0;
    }
    
    
    rofi_hud::KWinTracker tracker;
    auto window_info = tracker.get_active_window_info();

    if (!window_info) {
        rofi_hud::RofiFormatter::render_error("Impossible to detect D-BUS menu for the active window");
        return 0;
    }

    rofi_hud::DBusMenuClient client(window_info->service_name, window_info->object_path);
    auto menu_items = client.fetch_flat_menu();
    rofi_hud::RofiFormatter::render_menu(window_info->app_title, menu_items);
    return 0;
}