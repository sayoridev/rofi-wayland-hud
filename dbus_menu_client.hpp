#pragma once

#include "menu_item.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

class DBusMenuClient {
public:
    DBusMenuClient(const std::string& service_name, const std::string& object_path);
    ~DBusMenuClient() = default;

    std::vector<FlatMenuItem> fetch_flat_menu();
    bool trigger_action(int32_t item_id);

private:
    std::string service_name_;
    std::string object_path_;
    std::unique_ptr<sdbus::IConnection> connection_;

    void parse_dbus_node(const sdbus::Struct<int32_t, std::map<std::string, sdbus::Variant>, std::vector<sdbus::Variant>>& node,
                         const std::string& parent_path,
                         std::vector<FlatMenuItem>& out_items);

};

}