cmake_minimum_required(VERSION 3.20)
project(rofi_wayland_hud VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Trova la libreria sdbus-c++
find_package(sdbus-c++ REQUIRED)

include_directories(include)

# Modulo libreria interna (riutilizzabile da main e dai test)
add_library(rofi_hud_core STATIC
    src/kwin_tracker.cpp
    src/dbus_menu_client.cpp
    src/rofi_formatter.cpp
)

# Usa il target importato corretto SDBusCpp::sdbus-c++
target_link_libraries(rofi_hud_core PUBLIC SDBusCpp::sdbus-c++)

# Eseguibile Principale
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE rofi_hud_core)

# Opzioni di compilazione
target_compile_options(rofi_hud_core PRIVATE
    -Wall -Wextra -Wpedantic
)

install(TARGETS rofi_wayland_hud
    RUNTIME DESTINATION bin
)