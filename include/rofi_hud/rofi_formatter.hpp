#pragma once 

#include "menu_item.hpp"
#include <vector>
#include <string>

namespace rofi_hud {

class RofiFormatter {
public:
    // print rofi menu items in a format that rofi can understand
    static void render_menu(const std::string& app_title, const std::vector<FlatMenuItem>& items);
    static void render_error(const std::string& error_message);
};

}