#include "rofi_hud/tracker_factory.hpp"
#include "rofi_hud/dbus_menu_client.hpp"
#include "rofi_hud/rofi_formatter.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Lettura delle variabili d'ambiente fornite da Rofi
    const char* retv_env = std::getenv("ROFI_RETV");
    const char* info_env = std::getenv("ROFI_INFO");

    int rofi_retv = retv_env ? std::atoi(retv_env) : 0;

    // Quando l'utente seleziona una voce in Rofi (ROFI_RETV == 1)
    if (rofi_retv == 1 && info_env != nullptr) {
        std::string info_str = info_env;

        // Verifica se info_env contiene il payload deterministico "service|object_path|item_id"
        size_t first_delim = info_str.find('|');
        size_t second_delim = (first_delim != std::string::npos) ? info_str.find('|', first_delim + 1) : std::string::npos;

        if (first_delim != std::string::npos && second_delim != std::string::npos) {
            std::string service = info_str.substr(0, first_delim);
            std::string path = info_str.substr(first_delim + 1, second_delim - first_delim - 1);
            try {
                int32_t selected_id = std::stoi(info_str.substr(second_delim + 1));
                rofi_hud::DBusMenuClient client(service, path);
                client.trigger_action(selected_id);
                return 0;
            } catch (...) {}
        }

        // Fallback: se info contiene solo l'ID numerico, cerca la finestra attiva
        int32_t selected_id = std::atoi(info_env);
        auto tracker = rofi_hud::TrackerFactory::create_tracker();
        auto window_info = tracker->get_active_window_info();

        if (window_info) {
            rofi_hud::DBusMenuClient client(window_info->service_name, window_info->object_path);
            client.trigger_action(selected_id);
        }
        return 0;
    }

    // Modalità rendering: rileva finestra attiva tramite il tracker appropriato per il compositore corrente
    auto tracker = rofi_hud::TrackerFactory::create_tracker();
    auto window_info = tracker->get_active_window_info();

    if (!window_info) {
        rofi_hud::RofiFormatter::render_error("Impossible to detect D-BUS menu for the active window");
        return 0;
    }

    rofi_hud::DBusMenuClient client(window_info->service_name, window_info->object_path);
    auto menu_items = client.fetch_flat_menu();
    rofi_hud::RofiFormatter::render_menu(window_info->app_title,
                                         window_info->service_name,
                                         window_info->object_path,
                                         menu_items);
    return 0;
}