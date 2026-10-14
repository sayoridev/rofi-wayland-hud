#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace rofi_hud {

// Payload stored in Rofi's `info` field for deterministic dispatch:
// "<service_name>|<object_path>|<item_id>". Encoding the full D-Bus target in
// the payload avoids re-querying the compositor when the selection is
// activated, eliminating focus-switch race conditions.
struct SelectionPayload {
    std::string service_name;
    std::string object_path;
    int32_t item_id{0};

    std::string encode() const;

    // Inverse of encode(). Returns std::nullopt when the payload is malformed
    // (missing separators, non-numeric id, empty service or path).
    static std::optional<SelectionPayload> decode(std::string_view encoded);
};

// A parsed Rofi script-mode request, built from the ROFI_RETV / ROFI_INFO
// environment variables.
struct RofiRequest {
    int retv{0};                                    // Rofi return code
    std::optional<SelectionPayload> selection;      // set iff retv == 1 and the
                                                    // info payload decodes cleanly
};

// Reads ROFI_RETV / ROFI_INFO from the environment.
RofiRequest read_rofi_request();

} // namespace rofi_hud
