#pragma once

#include <chrono>

namespace rofi_hud::dbus_timeouts {

// All D-Bus call timeouts. Keep these short: the HUD runs synchronously inside
// Rofi and any slow application would otherwise freeze the launcher UI.
using namespace std::chrono_literals;

// Probes used while discovering a menu (GetLayout probes, introspection).
inline constexpr auto kDiscoveryCall = 50ms;

// com.canonical.AppMenu.Registrar query.
inline constexpr auto kRegistrarCall = 50ms;

// Full menu layout fetch (com.canonical.dbusmenu.GetLayout).
inline constexpr auto kGetLayout = 1500ms;

// Menu activation event (com.canonical.dbusmenu.Event).
inline constexpr auto kEvent = 1000ms;

// org.freedesktop.DBus.ListNames during owner resolution.
inline constexpr auto kListNames = 100ms;

// org.freedesktop.DBus.GetConnectionUnixProcessID per unique name.
inline constexpr auto kGetConnectionPid = 30ms;

} // namespace rofi_hud::dbus_timeouts
