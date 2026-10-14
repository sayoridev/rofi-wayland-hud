#include "rofi_hud/platform/json_field.hpp"
#include "rofi_hud/platform/string_utils.hpp"
#include <algorithm>
#include <regex>
#include <string>

namespace rofi_hud {

namespace {
// Matches `"key": <digits>` (the format emitted by hyprctl / swaymsg).
std::regex number_field_regex(std::string_view key) {
    return std::regex{"\\\"" + std::string(key) + "\\\":\\s*(\\d+)"};
}

// Matches `"key": "value"` with a value that contains no escaped quotes.
std::regex string_field_regex(std::string_view key) {
    return std::regex{"\\\"" + std::string(key) + "\\\":\\s*\\\"([^\\\"]*)\\\""};
}
} // namespace

bool json_find_uint(std::string_view json, std::string_view key, uint32_t& out) {
    const std::string haystack(json);
    std::smatch match;
    if (std::regex_search(haystack, match, number_field_regex(key)) && match.size() > 1) {
        return parse_uint32(match[1].str(), out);
    }
    return false;
}

bool json_find_string(std::string_view json, std::string_view key, std::string& out) {
    const std::string haystack(json);
    std::smatch match;
    if (std::regex_search(haystack, match, string_field_regex(key)) && match.size() > 1) {
        out = match[1].str();
        return true;
    }
    return false;
}

std::string_view sway_focused_node_block(std::string_view tree_json) {
    size_t focused_pos = tree_json.find("\"focused\": true");
    if (focused_pos == std::string_view::npos) {
        focused_pos = tree_json.find("\"focused\":true");
    }
    if (focused_pos == std::string_view::npos) return {};

    // The identifying fields precede "focused" in sway's serialization, so the
    // owning node is covered by looking backwards from the flag. Window sizes
    // match the original implementation (1 KB lookback, 2 KB window).
    constexpr size_t kLookback = 1024;
    constexpr size_t kWindow = 2048;
    const size_t start = (focused_pos > kLookback) ? focused_pos - kLookback : 0;
    const size_t length = std::min(kWindow, tree_json.size() - start);
    return tree_json.substr(start, length);
}

} // namespace rofi_hud
