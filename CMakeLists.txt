cmake_minimum_required(VERSION 3.20)
project(rofi_wayland_hud VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(sdbus-c++ REQUIRED)

add_library(rofi_hud_core STATIC
    src/dbus_discovery.cpp
    src/kwin_tracker.cpp
    src/hyprland_tracker.cpp
    src/sway_tracker.cpp
    src/tracker_factory.cpp
    src/dbus_menu_client.cpp
    src/rofi_formatter.cpp
)

target_include_directories(rofi_hud_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(rofi_hud_core PUBLIC SDBusCpp::sdbus-c++)

target_compile_options(rofi_hud_core PRIVATE
    -Wall -Wextra -Wpedantic
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE rofi_hud_core)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
)

install(TARGETS rofi_wayland_hud
    RUNTIME DESTINATION bin
)