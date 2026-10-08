#pragma once 

#include "menu_item.hpp"
#include <vector>
#include <string>

namespace rofi_hud {

class RofiFormatter {
public:
    // Formatta e stampa le voci di menu per Rofi includendo metadata, scorciatoie e payload deterministico
    static void render_menu(const std::string& app_title,
                            const std::string& service_name,
                            const std::string& object_path,
                            const std::vector<FlatMenuItem>& items);

    static void render_menu(const std::string& app_title, const std::vector<FlatMenuItem>& items);

    // Stampa messaggio di errore o avviso formattato secondo il protocollo script di Rofi
    static void render_error(const std::string& error_message);
};

}