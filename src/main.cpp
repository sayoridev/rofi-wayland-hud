#include "rofi_hud/compositors/tracker_factory.hpp"
#include "rofi_hud/dbus/dbus_menu_client.hpp"
#include "rofi_hud/rofi/rofi_formatter.hpp"
#include "rofi_hud/rofi/rofi_protocol.hpp"

// Rofi script-mode entry point.
//
// Rofi invokes this binary twice:
//   1. ROFI_RETV unset/0: detect the active window, fetch its D-Bus menu and
//      print the Rofi protocol stream.
//   2. ROFI_RETV=1 with ROFI_INFO="<service>|<path>|<id>": reactivate the
//      selected item directly from the encoded payload (no re-detection).
int main() {
    const rofi_hud::RofiRequest request = rofi_hud::read_rofi_request();

    if (request.retv == 1) {
        if (request.selection) {
            rofi_hud::DBusMenuClient client(request.selection->service_name,
                                            request.selection->object_path);
            client.trigger_action(request.selection->item_id);
        }
        // A malformed payload cannot be dispatched; exit quietly (Rofi simply
        // closes). The legacy numeric-id fallback was removed as dead code: the
        // formatter always emits the full service|path|id payload.
        return 0;
    }

    auto tracker = rofi_hud::TrackerFactory::create_tracker();
    const auto window_info = tracker->get_active_window_info();

    if (!window_info) {
        rofi_hud::RofiFormatter::render_error(
            "Impossible to detect D-BUS menu for the active window");
        return 0;
    }

    rofi_hud::DBusMenuClient client(window_info->service_name, window_info->object_path);
    const auto menu_items = client.fetch_flat_menu();
    rofi_hud::RofiFormatter::render_menu(window_info->app_title,
                                         window_info->service_name,
                                         window_info->object_path,
                                         menu_items);
    return 0;
}
