#include "rofi_hud/dbus/dbus_menu_client.hpp"
#include "rofi_hud/dbus/menu_text.hpp"
#include "rofi_hud/dbus/timeouts.hpp"
#include "rofi_hud/platform/log.hpp"
#include <chrono>
#include <cstdint>
#include <utility>

namespace rofi_hud {

namespace {
// sdbus-c++ takes timeouts as integer microseconds.
uint64_t microseconds(std::chrono::microseconds duration) {
    return static_cast<uint64_t>(duration.count());
}

// Reads a typed property from a dbusmenu property map. Returns false when the
// key is absent or holds a different type; never throws.
template <typename T>
bool read_prop(const std::map<std::string, sdbus::Variant>& props,
               const char* key,
               T& out) {
    const auto it = props.find(key);
    if (it == props.end()) return false;
    try {
        if (!it->second.containsValueOfType<T>()) return false;
        out = it->second.get<T>();
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace

DBusMenuClient::DBusMenuClient(const std::string& service_name, const std::string& object_path)
    : service_name_(service_name), object_path_(object_path) {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        log::error("[DBusMenuClient Error] D-Bus connection failed: ", e.what());
    }
}

std::unique_ptr<sdbus::IProxy> DBusMenuClient::create_proxy() const {
    return sdbus::createProxy(*connection_,
                              sdbus::ServiceName(service_name_),
                              sdbus::ObjectPath(object_path_));
}

std::vector<FlatMenuItem> DBusMenuClient::fetch_flat_menu() {
    std::vector<FlatMenuItem> items;
    if (!connection_) return items;

    try {
        auto proxy = create_proxy();

        const std::vector<std::string> property_names = {
            "label", "enabled", "visible", "shortcut", "icon-name", "type",
        };

        uint32_t revision = 0;
        MenuNode root_node;
        proxy->callMethod("GetLayout")
             .onInterface("com.canonical.dbusmenu")
             .withArguments(0, -1, property_names)
             .withTimeout(microseconds(dbus_timeouts::kGetLayout))
             .storeResultsTo(revision, root_node);
        (void)revision; // unused; present in the reply signature

        parse_menu_node(root_node, "", items);
    } catch (const sdbus::Error& e) {
        log::error("[DBusMenuClient Exception] Impossible to recover the layout menu: ", e.what());
    }
    return items;
}

void DBusMenuClient::parse_menu_node(const MenuNode& node,
                                     const std::string& parent_path,
                                     std::vector<FlatMenuItem>& out_items) const {
    const int32_t id = std::get<0>(node);
    const auto& props = std::get<1>(node);
    const auto& children = std::get<2>(node);

    std::string label;
    std::string icon;
    std::string item_type;
    std::string shortcut;
    bool enabled = true;
    bool visible = true;

    read_prop(props, "label", label);
    read_prop(props, "enabled", enabled);
    read_prop(props, "visible", visible);
    read_prop(props, "type", item_type);
    read_prop(props, "icon-name", icon);

    // "shortcut" has signature `aas`; a few exporters send a plain string.
    std::vector<std::vector<std::string>> combinations;
    if (read_prop(props, "shortcut", combinations)) {
        shortcut = format_shortcut(combinations);
    } else {
        read_prop(props, "shortcut", shortcut);
    }

    const std::string clean_label = clean_menu_label(label);

    // Separators and decorative markers are hidden.
    if (item_type == "separator" || clean_label == "-" || clean_label == "[" || clean_label == "]") {
        visible = false;
    }

    const std::string current_path = parent_path.empty() ? clean_label
                                                         : parent_path + " > " + clean_label;

    // Only leaf items are listed, so every row in Rofi is directly actionable.
    if (id != 0 && visible && !clean_label.empty() && children.empty()) {
        FlatMenuItem item;
        item.id = id;
        item.full_path = current_path;
        item.shortcut = shortcut;
        item.enabled = enabled;
        item.icon = icon.empty() ? "system-run" : icon;
        out_items.push_back(std::move(item));
    }

    for (const auto& child_var : children) {
        try {
            const auto child_struct = child_var.get<MenuNode>();
            parse_menu_node(child_struct, id == 0 ? "" : current_path, out_items);
        } catch (...) {
            // Skip malformed child entries.
        }
    }
}

bool DBusMenuClient::trigger_action(int32_t item_id) {
    if (!connection_) return false;

    try {
        auto proxy = create_proxy();

        const sdbus::Variant data{std::string("")};
        const uint32_t timestamp = 0;
        proxy->callMethod("Event")
             .onInterface("com.canonical.dbusmenu")
             .withArguments(item_id, "clicked", data, timestamp)
             .withTimeout(microseconds(dbus_timeouts::kEvent));
        return true;
    } catch (const sdbus::Error& e) {
        log::error("[DBusMenuClient Trigger Error] ", e.what());
        return false;
    }
}

} // namespace rofi_hud
