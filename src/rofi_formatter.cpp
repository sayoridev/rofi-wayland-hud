#include "rofi_hud/rofi_formatter.hpp"
#include <iostream>
#include <algorithm>

namespace rofi_hud {

constexpr char DELIM = '\x1f';

void RofiFormatter::render_menu(const std::string& app_title,
                                const std::string& service_name,
                                const std::string& object_path,
                                const std::vector<FlatMenuItem>& items) {
    // Protocollo Rofi script: byte nullo \0 seguito da comando di controllo
    std::string title = app_title.empty() ? "Window" : app_title;
    std::cout << '\0' << "prompt" << DELIM << "HUD [" << title << "]\n";

    if (items.empty()) {
        std::cout << "No menu or function available for this window";
        std::cout.put('\0');
        std::cout << "icon" << DELIM << "dialog-warning"
                  << DELIM << "nonselectable" << DELIM << "true\n";
        return;
    }

    for (const auto& item : items) {
        if (!item.enabled) continue;

        std::string display_text = item.full_path;
        if (!item.shortcut.empty()) {
            display_text += "  (" + item.shortcut + ")";
        }

        // Codifica service_name|object_path|item_id per dispatch deterministico
        std::string info_payload;
        if (!service_name.empty() && !object_path.empty()) {
            info_payload = service_name + "|" + object_path + "|" + std::to_string(item.id);
        } else {
            info_payload = std::to_string(item.id);
        }

        std::cout << display_text;
        std::cout.put('\0');
        std::cout << "icon" << DELIM << item.icon
                  << DELIM << "info" << DELIM << info_payload;

        if (!item.shortcut.empty()) {
            std::cout << DELIM << "meta" << DELIM << item.shortcut;
        }

        std::cout << "\n";
    }
}

void RofiFormatter::render_menu(const std::string& app_title, const std::vector<FlatMenuItem>& items) {
    render_menu(app_title, "", "", items);
}

void RofiFormatter::render_error(const std::string& error_message) {
    std::string clean_msg = error_message;
    clean_msg.erase(std::remove(clean_msg.begin(), clean_msg.end(), '['), clean_msg.end());
    clean_msg.erase(std::remove(clean_msg.begin(), clean_msg.end(), ']'), clean_msg.end());

    // Protocollo Rofi: \0message\x1f<testo>
    std::cout << '\0' << "message" << DELIM << clean_msg << "\n";
}

} // namespace rofi_hud