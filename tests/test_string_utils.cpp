#include "rofi_hud/platform/string_utils.hpp"
#include "test_framework.hpp"

using namespace rofi_hud;
#include <string>

TEST_CASE(trim_removes_surrounding_whitespace) {
    CHECK_EQ(std::string("hello"), trim("  hello \n\r\t"));
    CHECK_EQ(std::string(""), trim("   "));
    CHECK_EQ(std::string(""), trim(""));
}

TEST_CASE(parse_uint32_accepts_plain_decimal) {
    uint32_t value = 0;
    CHECK(parse_uint32("1234", value));
    CHECK_EQ(1234u, value);
    CHECK(parse_uint32("  77 \n", value)); // surrounding whitespace is tolerated
    CHECK_EQ(77u, value);
}

TEST_CASE(parse_uint32_rejects_malformed_input) {
    uint32_t value = 42;
    CHECK(!parse_uint32("", value));
    CHECK(!parse_uint32("abc", value));
    CHECK(!parse_uint32("12x", value));
    CHECK(!parse_uint32("-1", value));
    CHECK(!parse_uint32("4294967296", value)); // > UINT32_MAX
    CHECK(!parse_uint32("0x10", value));
    CHECK_EQ(42u, value); // failed parses never touch the output
}

TEST_CASE(split_lines_splits_and_trims) {
    const auto lines = split_lines("first \nsecond\n\n  third  \n");
    CHECK_EQ(4u, lines.size());
    CHECK_EQ(std::string("first"), lines[0]);
    CHECK_EQ(std::string("second"), lines[1]);
    CHECK_EQ(std::string(""), lines[2]); // empty lines are preserved
    CHECK_EQ(std::string("third"), lines[3]);
}

