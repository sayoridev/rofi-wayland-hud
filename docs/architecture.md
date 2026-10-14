# Architecture — `rofi-wayland-hud`

Deep technical documentation of the module structure, data flow, external
protocols and the non-obvious constraints of each component. For coding rules
and workflows see [../AGENTS.md](../AGENTS.md).

## 1. Execution Model (Rofi Script Mode)

Rofi invokes `rofi_wayland_hud` as an *script mode* program:

1. **Render phase** (`ROFI_RETV` unset or `0`): the binary prints the Rofi
   protocol stream on stdout — a `\0prompt\x1f…` control line followed by one
   row per menu item (`text \0 icon\x1f… \x1finfo\x1f<payload> \x1fmeta\x1f…`).
2. **Selection phase** (`ROFI_RETV=1`, `ROFI_INFO=<payload>`): the binary
   receives back the `info` payload of the chosen row and must trigger the
   corresponding menu action.

Hard protocol facts:

- Control lines start with a **NUL byte**; a C string literal `"\0prompt…"`
  would stop at the first byte, which is why the code writes `std::cout << '\0'`
  explicitly.
- Fields inside a row are separated by `0x1F` (`kFieldDelimiter`).
- **stderr is rendered by Rofi as message rows.** Only `RofiFormatter` may
  write to stdout; everything diagnostic goes through `rofi_hud::log` (stderr).

## 2. Module Map

```
main.cpp (composition root)
├── compositors/
│   ├── TrackerFactory        env-based strategy selection
│   ├── CompositorTracker     base: D-Bus connection + menu resolution tail
│   ├── KWinTracker           kdotool (KDE Plasma 6)
│   ├── HyprlandTracker       hyprctl activewindow -j
│   └── SwayTracker           swaymsg -t get_tree
├── dbus/
│   ├── DBusDiscovery         pid -> (service, object path)
│   ├── DBusMenuClient        com.canonical.dbusmenu GetLayout/Event
│   ├── menu_text             label mnemonic cleaning, shortcut formatting
│   └── timeouts              all D-Bus call timeouts (µs values, unchanged)
├── rofi/
│   ├── RofiProtocol          ROFI_RETV/ROFI_INFO parsing, SelectionPayload codec
│   └── RofiFormatter         script-mode stdout emission
├── core/                     WindowTracker, ActiveWindowInfo, FlatMenuItem,
│                             CompositorTracker base
└── platform/                 process_runner, string_utils, json_field, log
```

Dependency direction: `platform -> core -> {dbus, compositors} -> main`, with
`rofi` depending only on `core`/`platform`. See AGENTS.md §2 for the full rules.

## 3. Window Detection (compositors/)

Each tracker implements only `detect_active_window()` (fill at least `pid`);
`CompositorTracker::get_active_window_info()` then:

1. falls back `app_title` to `/proc/<pid>/comm`,
2. resolves the menu coordinates via `DBusDiscovery`,
3. returns `std::nullopt` when either step fails.

Environment-based selection (`TrackerFactory`), in priority order:
`HYPRLAND_INSTANCE_SIGNATURE` → Hyprland; `SWAYSOCK` → Sway;
`XDG_CURRENT_DESKTOP` substring (`hyprland`/`sway`/`kde`/`plasma`);
fallback → KWin.

### JSON parsing constraint

No JSON library is allowed, so compositor output is scanned with `std::regex`
(`platform/json_field.hpp`):

- **Hyprland:** `hyprctl activewindow -j` emits flat JSON; `pid`, `title`,
  `class` are searched over the whole document.
- **Sway:** `swaymsg -t get_tree` emits nested node trees. sway serializes node
  fields in a **stable order** (`pid`, `name`, `app_id`, … before `focused`),
  so the focused node is located by searching **backwards** from
  `"focused": true` with a 1 KB lookback / 2 KB window
  (`sway_focused_node_block`). If sway ever reorders its serialization, this
  heuristic must be revisited.

## 4. Menu Discovery (dbus/DBusDiscovery)

`resolve_dbus_menu_for_pid(connection, pid, &service, &path)` tries, in order:

1. **AppMenu registrar** (`com.canonical.AppMenu.Registrar.GetMenuForWindow`).
   Designed for X11 window ids; on Wayland it usually fails or misbehaves, so
   the returned service name is validated (must be a well-known name containing
   `.` or a unique name starting with `:`); otherwise the bus name is
   re-resolved from the pid.
2. **Owner bus name** via `org.freedesktop.DBus.GetConnectionUnixProcessID`
   over `ListNames`. This is a linear scan over unique connections — O(N) on
   the session bus — kept for behavior compatibility. Timeouts: `timeouts.hpp`.
3. **Well-known menu object paths**, probed with a shallow `GetLayout`
   (depth 1): `/MenuBar/1` … `/MenuBar/8`, `/com/canonical/dbusmenu`,
   `/org/ayatana/NotificationItem/MenuBar`, `/org/gtk/menus`, `/MenuBar`,
   `/AppMenu`.
4. **Dynamic introspection** of `/MenuBar` (`org.freedesktop.DBus.Introspectable`),
   probing every advertised child `/MenuBar/<N>` — this is what catches
   dynamically-indexed Qt/KDE menus (e.g. Dolphin exporting `/MenuBar/2`).

Steps 1–4 are retried against the **parent pid** as well, because
Electron/Chromium renderers do not own the exported menu (the browser process
does). Every probe uses short timeouts (`kDiscoveryCall = 50 ms`) so a
misbehaving application cannot freeze the launcher.

## 5. Menu Protocol (dbus/DBusMenuClient)

Implements the consumer side of `com.canonical.dbusmenu`:

- `fetch_flat_menu()` — one `GetLayout(0, -1, props)` call with a 1.5 s
  timeout; the reply tree is flattened depth-first into `FlatMenuItem` leaves
  with breadcrumb paths (`File > Export > PDF`). Only **leaf** items are
  emitted, so every Rofi row is directly actionable. Properties consumed:
  `label`, `enabled`, `visible`, `shortcut`, `icon-name`, `type`. Separators
  and decorative labels (`-`, `[`, `]`) are hidden. Disabled items are emitted
  but skipped by the formatter. `icon-name` falls back to `system-run`.
- `trigger_action(id)` — `Event(id, "clicked", "", 0)` with a 1 s timeout.

Label mnemonics (`&x`, `_x`) are stripped by `clean_menu_label` while escaped
`&&`/`__` collapse to a single character. Shortcuts (`aas` signature) are
formatted by `format_shortcut` using only the first combination and
normalizing `Control` → `Ctrl`.

## 6. Deterministic Dispatch (rofi/RofiProtocol)

To avoid re-querying the compositor when the user activates a row (a focus
race on Wayland layer-shell), the formatter encodes the full D-Bus target into
the row's `info` field:

```
<service_name>|<object_path>|<item_id>
```

`SelectionPayload::encode/decode` is the single codec for this format; decode
is strict (three parts, integer id spanning the whole remainder, non-empty
service/path). `read_rofi_request()` reads `ROFI_RETV`/`ROFI_INFO` from the
environment. `main()` branches on it: `retv == 1` → trigger; otherwise render.

## 7. Manual Verification (no D-Bus in CI)

The unit tests cover the pure helpers. The D-Bus path is verified manually on a
live session:

```bash
# Protocol bytes (render phase), independent of Rofi:
ROFI_RETV=0 ./rofi_wayland_hud | cat -v   # expect ^@prompt^_<title>

# Selection phase:
ROFI_RETV=1 ROFI_INFO=":1.42|/MenuBar/2|17" ./rofi_wayland_hud
```

Check: prompt line present, rows carry `icon`/`info`/`meta` fields, stderr
stays empty on the happy path.
