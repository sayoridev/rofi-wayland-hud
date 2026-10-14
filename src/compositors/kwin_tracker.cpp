#include "rofi_hud/compositors/kwin_tracker.hpp"
#include "rofi_hud/platform/process_runner.hpp"
#include "rofi_hud/platform/string_utils.hpp"

namespace rofi_hud {

KWinTracker::KWinTracker() : CompositorTracker("KWinTracker") {}

std::optional<ActiveWindowInfo> KWinTracker::detect_active_window() {
    ActiveWindowInfo info;

    // Single kdotool invocation requesting pid, title and class; kdotool
    // prints one value per line. The line layout mirrors the original
    // implementation and relies on kdotool's chained-output semantics.
    const auto probe = run_command(
        "kdotool getactivewindow getwindowpid %1 getwindowname %1 getwindowclassname %1 2>/dev/null");
    const auto lines = split_lines(probe.output);

    if (!lines.empty() && !lines[0].empty()) {
        parse_uint32(lines[0], info.pid);
    }
    if (lines.size() > 1) {
        info.app_title = lines[1];
    }
    if (lines.size() > 2) {
        info.app_class = lines[2];
    }

    // Fallback: some kdotool versions reject the multi-action form; probe the
    // pid with the base command sequence instead.
    if (info.pid == 0) {
        const auto pid_probe = run_command("kdotool getactivewindow getwindowpid 2>/dev/null");
        const auto pid_lines = split_lines(pid_probe.output);
        if (!pid_lines.empty()) {
            parse_uint32(pid_lines[0], info.pid);
        }
    }

    if (info.pid == 0) return std::nullopt;
    return info;
}

} // namespace rofi_hud
