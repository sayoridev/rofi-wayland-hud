#include "rofi_hud/platform/string_utils.hpp"
#include <charconv>

namespace rofi_hud {

namespace {
constexpr std::string_view kWhitespace = " \t\n\r";
}

std::string trim(std::string_view text) {
    const size_t first = text.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos) return {};
    const size_t last = text.find_last_not_of(kWhitespace);
    return std::string(text.substr(first, last - first + 1));
}

bool parse_uint32(std::string_view text, uint32_t& out) {
    const std::string cleaned = trim(text);
    if (cleaned.empty() || cleaned.front() == '-') return false;

    // Parse into a local: std::from_chars may partially write the output on a
    // failed parse, so `out` is committed only on full success.
    uint32_t parsed = 0;
    const char* begin = cleaned.data();
    const char* end = begin + cleaned.size();
    const auto result = std::from_chars(begin, end, parsed, 10);
    if (result.ec != std::errc() || result.ptr != end) return false;

    out = parsed;
    return true;
}

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;
    for (const char c : text) {
        if (c == '\n') {
            lines.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        lines.push_back(trim(current));
    }
    return lines;
}

} // namespace rofi_hud
