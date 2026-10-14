#pragma once

#include <cstdint>
#include <string>

namespace rofi_hud {

// A single leaf entry of an application menu, flattened with its breadcrumb path.
// Nested submenus are encoded in `full_path` as "File > Export > PDF".
struct FlatMenuItem {
    int32_t id{0};
    std::string full_path;
    std::string shortcut;
    std::string icon;
    bool enabled{true};
};

} // namespace rofi_hud
