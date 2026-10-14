#include "rofi_hud/dbus/menu_text.hpp"

namespace rofi_hud {

std::string clean_menu_label(std::string_view label) {
    std::string cleaned;
    cleaned.reserve(label.size());
    for (size_t i = 0; i < label.size(); ++i) {
        if (label[i] == '&') {
            if (i + 1 < label.size() && label[i + 1] == '&') {
                cleaned += '&';
                ++i;
            }
            continue;
        }
        if (label[i] == '_') {
            if (i + 1 < label.size() && label[i + 1] == '_') {
                cleaned += '_';
                ++i;
            }
            continue;
        }
        cleaned += label[i];
    }
    return cleaned;
}

std::string format_shortcut(const std::vector<std::vector<std::string>>& combinations) {
    if (combinations.empty() || combinations[0].empty()) return {};

    std::string formatted;
    for (size_t k = 0; k < combinations[0].size(); ++k) {
        if (k > 0) formatted += '+';
        std::string key = combinations[0][k];
        if (key == "Control") key = "Ctrl";
        formatted += key;
    }
    return formatted;
}

} // namespace rofi_hud
