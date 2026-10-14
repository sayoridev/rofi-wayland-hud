#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace rofi_hud {

// ---------------------------------------------------------------------------
// CONSTRAINT: the project must not depend on external libraries, so JSON
// documents (hyprctl / swaymsg output) are scanned with std::regex instead of
// a real JSON parser. These helpers therefore assume flat, well-formed output
// produced by the compositor tools. Any change to those tools' output format
// must be reflected here and in docs/architecture.md.
// ---------------------------------------------------------------------------

// Finds the first `"key": <digits>` occurrence and parses it into `out`.
// Returns false when the key is absent or the value overflows uint32; `out` is
// left untouched in both cases.
bool json_find_uint(std::string_view json, std::string_view key, uint32_t& out);

// Finds the first `"key": "value"` occurrence and copies `value` into `out`.
bool json_find_string(std::string_view json, std::string_view key, std::string& out);

// swaymsg -t get_tree serializes node fields in a stable order: the identifying
// fields (pid, name, app_id) always appear BEFORE "focused" within each node
// object. This returns the substring covering the node that contains
// `"focused": true` (with or without a space), looking backwards from the flag.
// Returns an empty view when no focused node exists.
std::string_view sway_focused_node_block(std::string_view tree_json);

} // namespace rofi_hud
