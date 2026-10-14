#pragma once

#include "rofi_hud/core/menu_item.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sdbus-c++/sdbus-c++.h>

namespace rofi_hud {

// Client for the com.canonical.dbusmenu protocol on a remote menu object:
// fetches the full layout and triggers items by id.
class DBusMenuClient {
public:
    DBusMenuClient(const std::string& service_name, const std::string& object_path);
    ~DBusMenuClient() = default;

    // Fetches the complete menu layout, flattened to leaf items with breadcrumb
    // paths. Returns an empty vector when the layout cannot be retrieved.
    std::vector<FlatMenuItem> fetch_flat_menu();

    // Sends a "clicked" event for `item_id`. Returns false on D-Bus errors.
    bool trigger_action(int32_t item_id);

private:
    // (revision, properties, children) node of a GetLayout reply.
    using MenuNode = sdbus::Struct<int32_t,
                                   std::map<std::string, sdbus::Variant>,
                                   std::vector<sdbus::Variant>>;

    std::unique_ptr<sdbus::IConnection> connection_;
    std::string service_name_;
    std::string object_path_;

    std::unique_ptr<sdbus::IProxy> create_proxy() const;
    void parse_menu_node(const MenuNode& node,
                         const std::string& parent_path,
                         std::vector<FlatMenuItem>& out_items) const;
};

} // namespace rofi_hud
