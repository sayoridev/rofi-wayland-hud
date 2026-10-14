# AGENTS.md — Operational Guide

Technical operational guide for human contributors and AI agents working on
`rofi-wayland-hud`. It documents the **intended** architecture, the rules that
keep it healthy, and the day-to-day workflows. For the deep module-by-module
design documentation, see [docs/architecture.md](docs/architecture.md).

---

## 1. Project Overview

`rofi-wayland-hud` is a C++20 Rofi script-mode plugin that renders a searchable
HUD of the global menu (D-Bus `com.canonical.dbusmenu`) exported by the
currently focused Wayland window. Supported compositors: KDE Plasma 6
(`kdotool`), Hyprland (`hyprctl`), Sway/wlroots (`swaymsg`).

- **Language:** C++20, no external dependencies except `sdbus-c++`.
- **Build:** CMake >= 3.20.
- **Binary:** `rofi_wayland_hud` (installed to `bin/`, invoked by Rofi).
- The core library `rofi_hud_core` (alias `rofi_hud::core`) is **internal**;
  it is not installed and has no stable public API.

## 2. Target Architecture

Layered module structure under `include/rofi_hud/` and `src/`:

| Module | Responsibility | May depend on |
| :--- | :--- | :--- |
| `core/` | Domain types (`ActiveWindowInfo`, `FlatMenuItem`), `WindowTracker` interface, `CompositorTracker` base | platform |
| `platform/` | OS/toolkit-free helpers: `process_runner`, `string_utils`, `json_field`, `log` | — (leaf layer) |
| `dbus/` | D-Bus concerns: `DBusDiscovery` (menu coordinates resolution), `DBusMenuClient` (dbusmenu protocol), `menu_text`, `timeouts` | platform |
| `compositors/` | `KWinTracker`, `HyprlandTracker`, `SwayTracker`, `TrackerFactory` | core, platform, dbus |
| `rofi/` | Rofi boundary: `RofiProtocol` (env vars + `service|path|id` payload codec), `RofiFormatter` (script-mode output) | core, platform |

**Dependency rules (enforced by review):**

- `platform` depends on nothing inside the project (leaf layer).
- `core` depends only on `platform`.
- `dbus` and `compositors` depend on `core` + `platform` (+ `dbus` for compositors).
- `rofi` depends only on `core` + `platform`.
- `main.cpp` is the only composition root and may include any module.
- Dependencies between peer modules (e.g. `rofi` -> `dbus`) are **prohibited**;
  communicate through `core` types or function arguments.

## 3. Coding Conventions

- All code, identifiers, comments and documentation are written in **English**.
- One class per header; free functions for stateless logic (see `menu_text`,
  `string_utils`, `json_field`).
- No `using namespace` in headers or sources; always qualify (`rofi_hud::log`).
- Error handling:
  - Never let exceptions escape `main()`; catch `sdbus::Error` where the
    original code did and log via `rofi_hud::log`.
  - Parsing helpers must be **no-throw** and must not partially modify their
    outputs on failure (see `parse_uint32`).
  - `catch (...)` is allowed only where the original code silenced errors by
    design (D-Bus probes); add a comment explaining what is being ignored.

## 4. Hard Constraints

1. **No new external dependencies.** JSON is parsed with `std::regex`
   (`platform/json_field.hpp`) because a real JSON parser is forbidden; the
   assumptions are documented there and in docs/architecture.md.
2. **stdout carries only the Rofi protocol.** Log exclusively through
   `rofi_hud::log`, which writes to stderr.
3. **stderr is user-visible:** in Rofi script-mode, stderr lines are rendered by
   Rofi as message entries. Keep stderr output concise and stable; error
   message formats are part of the observable behavior.
4. **Behavior preservation over cosmetics.** Refactorings must not change the
   emitted protocol bytes, the detection order of strategies, or D-Bus timeouts
   (`dbus/timeouts.hpp`).

## 5. Prohibited Patterns

- Adding a JSON library, process library, or test framework.
- Printing to `std::cout` from anywhere except `RofiFormatter`.
- `system()` calls; use `platform::run_command`.
- Bare `popen`/`fgets` outside `process_runner.cpp`.
- Magic timeout values outside `dbus/timeouts.hpp`.
- `std::stoi`/`std::stoul` on unvalidated input; use `parse_uint32`.
- Compositor-specific logic outside `compositors/`.

## 6. Build & Test Workflow

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure   # unit tests (tests/, zero-dep framework)
```

Useful options: `-DROFI_HUD_BUILD_TESTS=OFF`, `-DROFI_HUD_WARNINGS_AS_ERRORS=ON`.

Formatting: `.clang-format` (LLVM base, 4-space indent, 100 columns). Run
`clang-format -i` on touched files before committing.

CI (`.github/workflows/ci.yml`) builds Release and runs `ctest` on Ubuntu.

## 7. Testing Policy

- Tests live in `tests/`, use the self-contained framework in
  `tests/test_framework.hpp` (`TEST_CASE`, `CHECK`, `CHECK_EQ`, one `TEST_MAIN`
  in `tests/test_main.cpp`), and link `rofi_hud::core`.
- Every pure helper (platform layer, `menu_text`, `rofi_protocol`) must have
  unit tests. D-Bus-facing code is verified manually (see docs/architecture.md
  section "Manual verification").
- Regressions found during refactoring become new test cases.

## 8. Refactoring Changelog

2026-10-14 — Full repository refactoring (all phases approved by maintainer):

- **Repository hygiene:** removed committed build trees (`build/`, `src/build/`),
  binaries and the prebuilt package tarball.
- **New layered structure** (`core/`, `platform/`, `dbus/`, `compositors/`,
  `rofi/`) replacing the flat layout; internal include path `rofi_hud/<layer>/…`.
- **Platform layer extracted:** `run_command` (RAII popen), `trim` /
  `parse_uint32` / `split_lines`, `json_field` regex helpers (with the sway
  lookback-window assumption documented), centralized `log` (stderr, level-gated).
- **Tracker deduplication:** new `CompositorTracker` base owning the D-Bus
  session connection and the common menu-resolution tail; the three trackers now
  implement only `detect_active_window()`. Stderr messages kept byte-compatible.
- **D-Bus layer:** `resolve_dbus_menu_for_pid` split into named steps
  (registrar / owner bus name / known paths / `/MenuBar` introspection); timeouts
  centralized in `dbus/timeouts.hpp` (same values); label cleaning and shortcut
  formatting extracted to `dbus/menu_text.hpp` as pure, tested functions.
- **Rofi layer + main:** new `RofiProtocol` (`SelectionPayload` codec +
  `read_rofi_request`); `main.cpp` reduced to pure orchestration.
- **Tests:** 20 unit test cases (70 checks) for the pure helpers; CTest
  integration; CI runs the suite.
- **Docs:** this guide rewritten in English; deep design docs added under
  `docs/`; TODO.md updated.

### Behavioral changes (approved)

- Removed the legacy numeric-only `ROFI_INFO` fallback in `main.cpp` (dead code:
  the formatter always encodes the full `service|path|id` payload).
- Removed the unused nested `MenuItem` tree struct and the unused 2-arg
  `render_menu` overload.
- `SelectionPayload::decode` now rejects ids with trailing garbage (previously
  `std::stoi` silently accepted a numeric prefix).
- `parse_uint32` rejects values > UINT32_MAX (previously `stoul` truncated).
