#include "rofi_hud/dbus_menu_client.hpp"
#include <iostream>

namespace rofi_hud {

DBusMenuClient::DBusMenuClient(const std::string& service_name, const std::string& object_path)
    : service_name_(service_name), object_path_(object_path) {
    try {
        connection_ = sdbus::createSessionBusConnection();
    } catch (const sdbus::Error& e) {
        std::cerr << "[DBusMenuClient Error] D-Bus connection failed: " << e.what() << std::endl;
    }
}

std::vector<FlatMenuItem> DBusMenuClient::fetch_flat_menu() {
    std::vector<FlatMenuItem> items;
    if (!connection_) return items;

    try {
        auto proxy = sdbus::createProxy(
            *connection_,
            sdbus::ServiceName(service_name_),
            sdbus::ObjectPath(object_path_)
        );

        std::vector<std::string> property_names = {"label", "enabled", "visible", "shortcut", "icon-name", "type"};
        
        uint32_t revision = 0;
        sdbus::Struct<int32_t, std::map<std::string, sdbus::Variant>, std::vector<sdbus::Variant>> root_node;

        proxy->callMethod("GetLayout")
             .onInterface("com.canonical.dbusmenu")
             .withArguments(0, -1, property_names)
             .withTimeout(1500000)
             .storeResultsTo(revision, root_node);

        (void)revision;
        parse_dbus_node(root_node, "", items);

    } catch (const sdbus::Error& e) {
        std::cerr << "[DBusMenuClient Exception] Impossible to recover the layout menu: " << e.what() << std::endl;
    }

    return items;
}

void DBusMenuClient::parse_dbus_node(
    const sdbus::Struct<int32_t, std::map<std::string, sdbus::Variant>, std::vector<sdbus::Variant>>& node,
    const std::string& parent_path,
    std::vector<FlatMenuItem>& out_items) {

    int32_t id = std::get<0>(node);
    const auto& props = std::get<1>(node);
    const auto& children = std::get<2>(node);

    std::string label;
    bool enabled = true;
    bool visible = true;
    std::string icon;
    std::string item_type;
    std::string shortcut;

    if (auto it = props.find("label"); it != props.end()) {
        try {
            if (it->second.containsValueOfType<std::string>()) {
                label = it->second.get<std::string>();
            }
        } catch (...) {}
    }

    if (auto it = props.find("enabled"); it != props.end()) {
        try {
            if (it->second.containsValueOfType<bool>()) {
                enabled = it->second.get<bool>();
            }
        } catch (...) {}
    }

    if (auto it = props.find("visible"); it != props.end()) {
        try {
            if (it->second.containsValueOfType<bool>()) {
                visible = it->second.get<bool>();
            }
        } catch (...) {}
    }

    if (auto it = props.find("type"); it != props.end()) {
        try {
            if (it->second.containsValueOfType<std::string>()) {
                item_type = it->second.get<std::string>();
            }
        } catch (...) {}
    }

    if (auto it = props.find("icon-name"); it != props.end()) {
        try {
            if (it->second.containsValueOfType<std::string>()) {
                icon = it->second.get<std::string>();
            }
        } catch (...) {}
    }

    if (auto it = props.find("shortcut"); it != props.end()) {
        try {
            if (it->second.containsValueOfType<std::vector<std::vector<std::string>>>()) {
                auto combinations = it->second.get<std::vector<std::vector<std::string>>>();
                if (!combinations.empty() && !combinations[0].empty()) {
                    std::string sc;
                    for (size_t k = 0; k < combinations[0].size(); ++k) {
                        if (k > 0) sc += "+";
                        std::string key = combinations[0][k];
                        if (key == "Control") key = "Ctrl";
                        sc += key;
                    }
                    shortcut = sc;
                }
            } else if (it->second.containsValueOfType<std::string>()) {
                shortcut = it->second.get<std::string>();
            }
        } catch (...) {}
    }

    // clean the label by removing '&' and '_' characters while preserving escaped && and __
    std::string clean_label;
    for (size_t i = 0; i < label.size(); ++i) {
        if (label[i] == '&') {
            if (i + 1 < label.size() && label[i + 1] == '&') {
                clean_label += '&';
                ++i;
            }
            continue;
        }
        if (label[i] == '_') {
            if (i + 1 < label.size() && label[i + 1] == '_') {
                clean_label += '_';
                ++i;
            }
            continue;
        }
        clean_label += label[i];
    }

    if (item_type == "separator" || clean_label == "-" || clean_label == "[" || clean_label == "]") {
        visible = false;
    }

    std::string current_path = parent_path.empty() ? clean_label : parent_path + " > " + clean_label;

    if (id != 0 && visible && !clean_label.empty() && children.empty()) {
        FlatMenuItem item;
        item.id = id;
        item.full_path = current_path;
        item.shortcut = shortcut;
        item.enabled = enabled;
        item.icon = icon.empty() ? "system-run" : icon;
        out_items.push_back(item);
    }

    for (const auto& child_var : children) {
        try {
            auto child_struct = child_var.get<sdbus::Struct<int32_t, std::map<std::string, sdbus::Variant>, std::vector<sdbus::Variant>>>();
            parse_dbus_node(child_struct, id == 0 ? "" : current_path, out_items);
        } catch (...) {}
    }
}

bool DBusMenuClient::trigger_action(int32_t item_id) {
    if (!connection_) return false;

    try {
        auto proxy = sdbus::createProxy(
            *connection_,
            sdbus::ServiceName(service_name_),
            sdbus::ObjectPath(object_path_)
        );
        
        sdbus::Variant data{std::string("")};
        uint32_t timestamp = 0;

        proxy->callMethod("Event")
             .onInterface("com.canonical.dbusmenu")
             .withArguments(item_id, "clicked", data, timestamp)
             .withTimeout(1000000);

        return true;
    } catch (const sdbus::Error& e) {
        std::cerr << "[DBusMenuClient Trigger Error] " << e.what() << std::endl;
        return false;
    }
}

} 