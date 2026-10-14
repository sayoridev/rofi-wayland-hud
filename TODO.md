# Project Roadmap & TODO List — `rofi-wayland-hud`

## Refactoring (2026-10-14) — COMPLETED

- [x] Repository hygiene: removed committed build trees, binaries, package tarball.
- [x] Layered architecture: `core/`, `platform/`, `dbus/`, `compositors/`, `rofi/`.
- [x] Platform layer: `process_runner`, `string_utils`, `json_field`, `log`.
- [x] Tracker deduplication via `CompositorTracker` base class.
- [x] D-Bus layer: named discovery steps, centralized timeouts, `menu_text` helpers.
- [x] Rofi layer: `RofiProtocol` payload codec; `main.cpp` as composition root.
- [x] Unit test suite (20 cases / 70 checks) + CTest + CI integration.
- [x] Documentation: `AGENTS.md` (operational guide), `docs/architecture.md`,
      README architecture/development sections; all docs and comments in English.

### Behavioral changes logged in AGENTS.md (approved by maintainer)

- [x] Removed dead numeric-only `ROFI_INFO` fallback in `main.cpp`.
- [x] Removed unused `MenuItem` tree struct and unused `render_menu` overload.
- [x] Stricter payload id parsing; stricter uint32 parsing.

### Technical notes (non-obvious decisions)

- JSON stays regex-based (`platform/json_field.hpp`) — no external deps allowed.
  The sway lookback window (1 KB back / 2 KB wide from `"focused"`) relies on
  sway's stable field serialization order (pid/name/app_id precede `focused`).
- `log` writes to stderr only; stderr is rendered by Rofi — message formats are
  behavioral surface, kept stable.
- `split_lines` preserves empty lines to keep pid/title/class alignment
  identical to the original `fgets` collection.
- `parse_uint32` parses into a local because `std::from_chars` partially writes
  the destination on failure.

---

## Feature Roadmap (unchanged, from the original plan)

### Phase 2: User Experience & Customization

- [ ] Enhanced application icon resolution (`.desktop` fallback when D-Bus
      provides no icon name).
- [ ] Config file support (`~/.config/rofi-hud/config.ini` or `.json`): theme
      overrides, ignored-app blacklist, custom fallback message.

### Phase 3: Performance, Stability & Edge Cases

- [ ] Submenu cache with invalidation for long-running heavy applications
      (GIMP, Inkscape, VS Code) to reach sub-millisecond render times.
- [ ] (idea) Unit tests for D-Bus-facing code behind an interface seam, once a
      mock strategy is designed.

### Phase 4: Distribution & Packaging

- [ ] AUR publishing (`rofi-wayland-hud`, `rofi-wayland-hud-git`).
- [ ] Fedora / COPR RPM repository (`.spec` + COPR pipeline).
- [ ] Distrobox setup helper script for immutable distros.
- [x] GitHub Actions CI (build + ctest) — done during refactoring.
