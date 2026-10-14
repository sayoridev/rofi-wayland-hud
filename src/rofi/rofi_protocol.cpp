#include "rofi_hud/rofi/rofi_protocol.hpp"
#include "rofi_hud/platform/string_utils.hpp"
#include <cstdlib>

namespace rofi_hud {

std::string SelectionPayload::encode() const {
    if (service_name.empty() || object_path.empty()) {
        return std::to_string(item_id);
    }
    return service_name + "|" + object_path + "|" + std::to_string(item_id);
}

std::optional<SelectionPayload> SelectionPayload::decode(std::string_view encoded) {
    const size_t first_delim = encoded.find('|');
    if (first_delim == std::string_view::npos) return std::nullopt;
    const size_t second_delim = encoded.find('|', first_delim + 1);
    if (second_delim == std::string_view::npos) return std::nullopt;

    SelectionPayload payload;
    payload.service_name = std::string(encoded.substr(0, first_delim));
    payload.object_path = std::string(encoded.substr(first_delim + 1, second_delim - first_delim - 1));

    // The id must be a valid integer spanning the whole remainder (stricter
    // than the original std::stoi, which ignored trailing garbage).
    const std::string id_text = std::string(encoded.substr(second_delim + 1));
    try {
        size_t parsed = 0;
        const int value = std::stoi(id_text, &parsed);
        if (parsed != id_text.size()) return std::nullopt;
        payload.item_id = value;
    } catch (...) {
        return std::nullopt;
    }

    if (payload.service_name.empty() || payload.object_path.empty()) return std::nullopt;
    return payload;
}

RofiRequest read_rofi_request() {
    RofiRequest request;

    if (const char* retv_env = std::getenv("ROFI_RETV")) {
        uint32_t retv = 0;
        if (parse_uint32(retv_env, retv)) {
            request.retv = static_cast<int>(retv);
        }
    }

    if (request.retv == 1) {
        if (const char* info_env = std::getenv("ROFI_INFO")) {
            request.selection = SelectionPayload::decode(info_env);
        }
    }
    return request;
}

} // namespace rofi_hud
