#include "rofi_hud/platform/json_field.hpp"
#include "test_framework.hpp"

using namespace rofi_hud;
#include <string>
#include <string_view>

namespace {
constexpr std::string_view kHyprctlOutput = R"json(
{
    "address": "0x7f2e",
    "mapped": true,
    "title": "README.md - rofi-wayland-hud",
    "class": "code",
    "pid": 4321,
    "xwayland": false
}
)json";
}

TEST_CASE(json_find_uint_extracts_numeric_fields) {
    uint32_t pid = 0;
    CHECK(json_find_uint(kHyprctlOutput, "pid", pid));
    CHECK_EQ(4321u, pid);
}

TEST_CASE(json_find_string_extracts_text_fields) {
    std::string title;
    CHECK(json_find_string(kHyprctlOutput, "title", title));
    CHECK_EQ(std::string("README.md - rofi-wayland-hud"), title);

    std::string app_class;
    CHECK(json_find_string(kHyprctlOutput, "class", app_class));
    CHECK_EQ(std::string("code"), app_class);
}

TEST_CASE(json_find_reports_missing_keys) {
    uint32_t value = 7;
    CHECK(!json_find_uint(kHyprctlOutput, "missing", value));
    CHECK_EQ(7u, value); // output untouched on failure

    std::string text;
    CHECK(!json_find_string(kHyprctlOutput, "missing", text));
    CHECK(text.empty());
}

TEST_CASE(sway_block_covers_fields_preceding_focused) {
    // sway serializes pid/name/app_id before "focused" inside each node, so a
    // node located anywhere in the tree is covered by the lookback window.
    const std::string tree = std::string(2000, ' ') +
        R"json({"id": 7, "name": "editor", "app_id": "foot", "pid": 99, "focused": true})json" +
        std::string(2000, ' ');
    const std::string_view block = sway_focused_node_block(tree);
    CHECK(!block.empty());

    uint32_t pid = 0;
    CHECK(json_find_uint(block, "pid", pid));
    CHECK_EQ(99u, pid);

    std::string name;
    CHECK(json_find_string(block, "name", name));
    CHECK_EQ(std::string("editor"), name);

    std::string app_id;
    CHECK(json_find_string(block, "app_id", app_id));
    CHECK_EQ(std::string("foot"), app_id);
}

TEST_CASE(sway_block_supports_compact_flag) {
    const std::string tree = R"json({"pid": 5, "name": "x", "app_id": "y", "focused":true})json";
    CHECK(!sway_focused_node_block(tree).empty());
}

TEST_CASE(sway_block_empty_when_nothing_focused) {
    CHECK(sway_focused_node_block(R"json({"pid": 5, "focused": false})json").empty());
}

