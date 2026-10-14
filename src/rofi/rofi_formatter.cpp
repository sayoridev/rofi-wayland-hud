#include "rofi_hud/rofi/rofi_formatter.hpp"
#include <algorithm>
#include <iostream>

namespace rofi_hud {

namespace {
// Rofi script-mode field separator.
constexpr char kFieldDelimiter = '\x1f';
}

void RofiFormatter::render_menu(const std::string& app_title,
                                const std::string& service_name,
                                const std::string& object_path,
                                const std::vector<FlatMenuItem>& items) {
    // Rofi control commands are a NUL byte followed by "command<delim>value".
    const std::string title = app_title.empty() ? "Window" : app_title;
    std::cout << '\0' << "prompt" << kFieldDelimiter << "HUD [" << title << "]\n";

    if (items.empty()) {
        std::cout << "No menu or function available for this window";
        std::cout.put('\0');
        std::cout << "icon" << kFieldDelimiter << "dialog-warning"
                  << kFieldDelimiter << "nonselectable" << kFieldDelimiter << "true\n";
        return;
    }

    for (const auto& item : items) {
        if (!item.enabled) continue;

        std::string display_text = item.full_path;
        if (!item.shortcut.empty()) {
            display_text += "  (" + item.shortcut + ")";
        }

        // Deterministic dispatch payload: service|object_path|item_id.
        std::string info_payload;
        if (!service_name.empty() && !object_path.empty()) {
            info_payload = service_name + "|" + object_path + "|" + std::to_string(item.id);
        } else {
            info_payload = std::to_string(item.id);
        }

        std::cout << display_text;
        std::cout.put('\0');
        std::cout << "icon" << kFieldDelimiter << item.icon
                  << kFieldDelimiter << "info" << kFieldDelimiter << info_payload;
        if (!item.shortcut.empty()) {
            std::cout << kFieldDelimiter << "meta" << kFieldDelimiter << item.shortcut;
        }
        std::cout << "\n";
    }
}

void RofiFormatter::render_error(const std::string& error_message) {
    std::string clean_msg = error_message;
    clean_msg.erase(std::remove(clean_msg.begin(), clean_msg.end(), '['), clean_msg.end());
    clean_msg.erase(std::remove(clean_msg.begin(), clean_msg.end(), ']'), clean_msg.end());

    std::cout << '\0' << "message" << kFieldDelimiter << clean_msg << "\n";
}

} // namespace rofi_hud
