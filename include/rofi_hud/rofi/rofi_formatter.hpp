#pragma once

#include "rofi_hud/core/menu_item.hpp"
#include <string>
#include <vector>

namespace rofi_hud {

// Emits Rofi script-mode output. stdout carries ONLY protocol data; control
// lines start with a NUL byte ("\0prompt\x1f...", "\0message\x1f...").
class RofiFormatter {
public:
    // Prints the prompt header ("HUD [<app_title>]") followed by one row per
    // enabled item: display text, icon, info payload and shortcut metadata.
    static void render_menu(const std::string& app_title,
                            const std::string& service_name,
                            const std::string& object_path,
                            const std::vector<FlatMenuItem>& items);

    // Prints a "\0message\x1f..." banner. Square brackets are stripped from
    // the message (legacy behavior, kept for output compatibility).
    static void render_error(const std::string& error_message);
};

} // namespace rofi_hud
