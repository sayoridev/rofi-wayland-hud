cmake_minimum_required(VERSION 3.20)
project(rofi_wayland_hud VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ROFI_HUD_BUILD_TESTS "Build the unit test suite" ON)
option(ROFI_HUD_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

find_package(sdbus-c++ REQUIRED)

# --- Core library (internal) -------------------------------------------------
add_library(rofi_hud_core STATIC
    src/core/compositor_tracker.cpp
    src/platform/log.cpp
    src/platform/string_utils.cpp
    src/platform/process_runner.cpp
    src/platform/json_field.cpp
    src/dbus/dbus_discovery.cpp
    src/dbus/menu_text.cpp
    src/dbus/dbus_menu_client.cpp
    src/compositors/kwin_tracker.cpp
    src/compositors/hyprland_tracker.cpp
    src/compositors/sway_tracker.cpp
    src/compositors/tracker_factory.cpp
    src/rofi/rofi_protocol.cpp
    src/rofi/rofi_formatter.cpp
)

add_library(rofi_hud::core ALIAS rofi_hud_core)

target_include_directories(rofi_hud_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rofi_hud_core PUBLIC SDBusCpp::sdbus-c++)

target_compile_options(rofi_hud_core PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${ROFI_HUD_WARNINGS_AS_ERRORS}>:-Werror>
)

# --- Executable ---------------------------------------------------------------
add_executable(rofi_wayland_hud src/main.cpp)
target_link_libraries(rofi_wayland_hud PRIVATE rofi_hud::core)
target_compile_options(rofi_wayland_hud PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${ROFI_HUD_WARNINGS_AS_ERRORS}>:-Werror>
)

install(TARGETS rofi_wayland_hud
    RUNTIME DESTINATION bin
)

# --- Tests --------------------------------------------------------------------
if(ROFI_HUD_BUILD_TESTS)
    enable_testing()
    add_executable(rofi_hud_tests
        tests/test_main.cpp
        tests/test_string_utils.cpp
        tests/test_json_field.cpp
        tests/test_menu_text.cpp
        tests/test_rofi_protocol.cpp
    )
    target_link_libraries(rofi_hud_tests PRIVATE rofi_hud::core)
    target_include_directories(rofi_hud_tests PRIVATE tests)
    add_test(NAME rofi_hud_unit COMMAND rofi_hud_tests)
endif()
