#pragma once

#include <string>
#include <vector>

namespace rofi_hud {

    struct MenuItem {
        int32_t id{0};
        std::string label;
        std::string shortcut;
        std::string icon;
        bool enabled{true};
        bool is_separator{false};
        std::vector<MenuItem> children;
    };

struct FlatMenuItem {
    int32_t id{0};
    std::string full_path;
    std::string shortcut;
    std::string icon;
    bool enabled{true};
};

}