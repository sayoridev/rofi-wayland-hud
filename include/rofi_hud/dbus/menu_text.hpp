#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace rofi_hud {

// Removes dbusmenu mnemonic markers from a label: '&' and '_' are dropped
// while escaped sequences are preserved ("&&" -> "&", "__" -> "_").
// Examples: "&File" -> "File", "Save _As" -> "Save As", "A&&B" -> "A&B".
std::string clean_menu_label(std::string_view label);

// Formats a dbusmenu "shortcut" property (signature `aas`, an array of
// accelerator key arrays) as a human-readable string. Only the first
// combination is used, e.g. {"Control", "Shift", "P"} -> "Ctrl+Shift+P".
// "Control" is normalized to "Ctrl"; all other keys are kept verbatim.
std::string format_shortcut(const std::vector<std::vector<std::string>>& combinations);

} // namespace rofi_hud
