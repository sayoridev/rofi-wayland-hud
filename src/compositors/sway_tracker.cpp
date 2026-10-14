#include "rofi_hud/compositors/sway_tracker.hpp"
#include "rofi_hud/platform/json_field.hpp"
#include "rofi_hud/platform/process_runner.hpp"

namespace rofi_hud {

SwayTracker::SwayTracker() : CompositorTracker("SwayTracker") {}

std::optional<ActiveWindowInfo> SwayTracker::detect_active_window() {
    const auto probe = run_command("swaymsg -t get_tree 2>/dev/null");
    if (probe.output.empty()) return std::nullopt;

    // sway serializes node fields in a stable order (pid, name, app_id precede
    // "focused" in each node object); see sway_focused_node_block().
    const std::string_view block = sway_focused_node_block(probe.output);
    if (block.empty()) return std::nullopt;

    ActiveWindowInfo info;
    json_find_uint(block, "pid", info.pid);
    if (info.pid == 0) return std::nullopt;

    json_find_string(block, "name", info.app_title);
    json_find_string(block, "app_id", info.app_class);
    return info;
}

} // namespace rofi_hud
