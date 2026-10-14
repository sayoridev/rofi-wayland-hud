#include "rofi_hud/rofi/rofi_protocol.hpp"
#include "test_framework.hpp"
#include <cstdlib>
#include <string>

TEST_CASE(payload_roundtrip) {
    const rofi_hud::SelectionPayload payload{":1.42", "/MenuBar/2", 17};
    const auto decoded = rofi_hud::SelectionPayload::decode(payload.encode());
    CHECK(decoded.has_value());
    CHECK_EQ(std::string(":1.42"), decoded->service_name);
    CHECK_EQ(std::string("/MenuBar/2"), decoded->object_path);
    CHECK_EQ(17, decoded->item_id);
}

TEST_CASE(payload_encode_falls_back_to_bare_id) {
    const rofi_hud::SelectionPayload payload{"", "", 3};
    CHECK_EQ(std::string("3"), payload.encode());
}

TEST_CASE(payload_decode_rejects_malformed_input) {
    CHECK(!rofi_hud::SelectionPayload::decode("").has_value());
    CHECK(!rofi_hud::SelectionPayload::decode("42").has_value());            // legacy numeric id
    CHECK(!rofi_hud::SelectionPayload::decode(":1.4|/MenuBar/2|x").has_value());
    CHECK(!rofi_hud::SelectionPayload::decode(":1.4|/MenuBar/2|5extra").has_value()); // trailing garbage
    CHECK(!rofi_hud::SelectionPayload::decode("|/MenuBar/2|5").has_value()); // empty service
    CHECK(!rofi_hud::SelectionPayload::decode(":1.4||5").has_value());       // empty path
}

TEST_CASE(read_rofi_request_reads_environment) {
    setenv("ROFI_RETV", "1", 1);
    setenv("ROFI_INFO", "org.example.App|/MenuBar/1|9", 1);
    const auto request = rofi_hud::read_rofi_request();
    CHECK_EQ(1, request.retv);
    CHECK(request.selection.has_value());
    CHECK_EQ(std::string("org.example.App"), request.selection->service_name);
    CHECK_EQ(std::string("/MenuBar/1"), request.selection->object_path);
    CHECK_EQ(9, request.selection->item_id);
    unsetenv("ROFI_RETV");
    unsetenv("ROFI_INFO");
}

TEST_CASE(read_rofi_request_defaults_to_render_mode) {
    unsetenv("ROFI_RETV");
    unsetenv("ROFI_INFO");
    const auto request = rofi_hud::read_rofi_request();
    CHECK_EQ(0, request.retv);
    CHECK(!request.selection.has_value());
}

