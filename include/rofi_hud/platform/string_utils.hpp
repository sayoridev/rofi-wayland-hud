#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace rofi_hud {

// Removes leading and trailing whitespace (" \t\n\r").
std::string trim(std::string_view text);

// Strictly parses a non-negative decimal integer that must span the whole
// (trimmed) input. Never throws; on failure `out` is left untouched.
bool parse_uint32(std::string_view text, uint32_t& out);

// Splits `text` into lines on '\n'; each line is trimmed. Trailing empty
// lines are dropped (matches the previous fgets-based collection).
std::vector<std::string> split_lines(const std::string& text);

} // namespace rofi_hud
