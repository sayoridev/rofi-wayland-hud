#include "rofi_hud/dbus/menu_text.hpp"
#include "test_framework.hpp"

using namespace rofi_hud;
#include <string>

TEST_CASE(clean_menu_label_strips_mnemonics) {
    CHECK_EQ(std::string("File"), clean_menu_label("&File"));
    CHECK_EQ(std::string("Save As"), clean_menu_label("Save _As"));
    CHECK_EQ(std::string("File"), clean_menu_label("&Fi&le"));
    CHECK_EQ(std::string("a b"), clean_menu_label("a _b"));
}

TEST_CASE(clean_menu_label_preserves_escaped_markers) {
    CHECK_EQ(std::string("A&B"), clean_menu_label("A&&B"));
    CHECK_EQ(std::string("a_b"), clean_menu_label("a__b"));
    CHECK_EQ(std::string("&x_"), clean_menu_label("&&x__"));
}

TEST_CASE(clean_menu_label_handles_trailing_marker) {
    CHECK_EQ(std::string("Open"), clean_menu_label("Open&"));
    CHECK_EQ(std::string("Open"), clean_menu_label("Open_"));
}

TEST_CASE(format_shortcut_joins_first_combination) {
    CHECK_EQ(std::string("Ctrl+Shift+P"),
             format_shortcut({{"Control", "Shift", "P"}}));
    CHECK_EQ(std::string("Alt+F4"), format_shortcut({{"Alt", "F4"}}));
    CHECK_EQ(std::string("Ctrl+S"),
             format_shortcut({{"Control", "S"}, {"Meta", "K"}})); // only the first combination
}

TEST_CASE(format_shortcut_handles_empty_input) {
    CHECK_EQ(std::string(""), format_shortcut({}));
    CHECK_EQ(std::string(""), format_shortcut({{}}));
}

