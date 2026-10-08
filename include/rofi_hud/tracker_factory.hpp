#pragma once

#include "window_tracker.hpp"
#include <memory>

namespace rofi_hud {

class TrackerFactory {
public:
    // Crea l'istanza appropriata di WindowTracker in base al compositore Wayland attivo
    static std::unique_ptr<WindowTracker> create_tracker();
};

} // namespace rofi_hud
