#include "rofi_hud/rofi_formatter.hpp"
#include <iostream>
#include <algorithm>

namespace rofi_hud {

void RofiFormatter::render_menu(const std::string& app_title, const std::vector<FlatMenuItem>& items) {
    std::cout << "\0prompt\x1fHUD [" << app_title << "]\n";

    if (items.empty()) {
        std::cout << "No menu or function available for this window";
        std::cout.put('\0');
        std::cout << "icon\x1f" << "dialog-warning" << "\x1f" << "nonselectable" << "\x1f" << "true\n";
        return;
    }

    for (const auto& item : items) {
        if (!item.enabled) continue;

        std::cout << item.full_path;
        std::cout.put('\0'); 
        std::cout << "icon\x1f" << item.icon
                  << "\x1finfo\x1f" << item.id << "\n";
    }
}

void RofiFormatter::render_error(const std::string& error_message) {
    std::string clean_msg = error_message;
    
    clean_msg.erase(
        std::remove(clean_msg.begin(), clean_msg.end(), '['), 
        clean_msg.end()
    );
    clean_msg.erase(
        std::remove(clean_msg.begin(), clean_msg.end(), ']'), 
        clean_msg.end()
    );

    std::cout << "\0message\x1f" << clean_msg << "\n";
}

} 