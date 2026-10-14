#include "rofi_hud/compositors/hyprland_tracker.hpp"
#include "rofi_hud/platform/json_field.hpp"
#include "rofi_hud/platform/process_runner.hpp"

namespace rofi_hud {

HyprlandTracker::HyprlandTracker() : CompositorTracker("HyprlandTracker") {}

std::optional<ActiveWindowInfo> HyprlandTracker::detect_active_window() {
    // hyprctl emits flat JSON with pid/title/class as top-level fields.
    const auto probe = run_command("hyprctl activewindow -j 2>/dev/null");
    const std::string& json = probe.output;
    if (json.empty()) return std::nullopt;

    ActiveWindowInfo info;
    json_find_uint(json, "pid", info.pid);
    if (info.pid == 0) return std::nullopt;

    json_find_string(json, "title", info.app_title);
    json_find_string(json, "class", info.app_class);
    return info;
}

} // namespace rofi_hud
