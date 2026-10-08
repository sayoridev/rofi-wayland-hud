# 📌 Project Roadmap & TODO List — `rofi-wayland-hud`

This document outlines planned features, structural improvements, and platform support extensions for `rofi-wayland-hud`.

---

## 🏎️ Phase 1: Modular Architecture & Multi-Compositor Support
> *Goal: Abstract the window tracking mechanism so the HUD works seamlessly across various Wayland compositors beyond KDE Plasma.*

- [x] **Refactor `WindowTracker` to Polymorphic Strategy Pattern**
  - Defined abstract C++ base class `WindowTracker`.
  - Moved and optimized `kdotool` implementation into `KWinTracker : public WindowTracker`.
- [x] **Implement Hyprland Support**
  - Created `HyprlandTracker` using `hyprctl activewindow -j` for active window PID, class, and title extraction.
- [x] **Implement Sway / wlroots Support**
  - Created `SwayTracker` using `swaymsg -t get_tree` IPC socket calls.
- [x] **Automatic Compositor Auto-Detection**
  - Created `TrackerFactory` checking `$HYPRLAND_INSTANCE_SIGNATURE`, `$SWAYSOCK`, and `$XDG_CURRENT_DESKTOP` at startup to dynamically instantiate the appropriate tracker.

---

## 🎨 Phase 2: User Experience & Customization

- [x] **Keyboard Shortcut Hints in Rofi Menu**
  - Parsed D-Bus menu accelerator strings (e.g., `Ctrl+S`, `Alt+F4`, `Ctrl+Shift+P`) and displayed them formatted alongside the entry name, with searchable `meta` tags.
- [x] **Submenu Breadcrumbs Navigation**
  - Implemented visual breadcrumb indicators (e.g., `File > Export > PDF`) for nested submenus.
- [ ] **Enhanced Application Icon Resolution**
  - Fallback icon matching using `.desktop` files when D-Bus doesn't provide an explicit icon name.
- [ ] **Config File Support (`~/.config/rofi-hud/config.ini` or `.json`)**
  - Allow users to configure default parameters without wrapping Rofi commands:
    - Custom Rofi theme overrides.
    - Ignored application blacklists (e.g., ignore Desktop background or Panels).
    - Custom fallback message text.

---

## 🔧 Phase 3: Performance, Stability & Edge Cases

- [x] **D-Bus Extraction with Timeout Protection**
  - Added strict timeouts (1.5s on GetLayout, 1.0s on Event, 50ms on discovery) to prevent freezing when querying unresponsive applications.
- [x] **Deterministic Focus Payload Dispatch**
  - Encoded `service|object_path|item_id` in Rofi's info field to eliminate focus-loss race conditions on menu item activation.
- [x] **Qt / GTK / Electron Specific Workarounds**
  - Scans `/MenuBar/1` through `/MenuBar/8`, `/com/canonical/dbusmenu`, introspects `/MenuBar` dynamically, and resolves parent PIDs for Chromium/Electron renderers.
- [ ] **Submenu Cache with Invalidation**
  - Cache static menu structures for long-running heavy applications (like GIMP, Inkscape, VS Code) to achieve sub-millisecond render times.

---

## 📦 Phase 4: Distribution & Packaging

- [ ] **AUR (Arch User Repository) Publishing**
  - Submit `rofi-wayland-hud` and `rofi-wayland-hud-git` to PKGBUILD repositories.
- [ ] **Fedora / COPR RPM Repository**
  - Create `.spec` file and set up COPR build pipeline for easy installation on Fedora Workstation & Kinoite.
- [ ] **Distrobox Setup Helper Script**
  - Provide an automated one-line setup script for users on immutable operating systems (Fedora Kinoite, SteamOS, openSUSE MicroOS).
- [ ] **GitHub Actions CI/CD**
  - Set up automated CMake build checks and linting for pull requests.

---

## 💡 Community Ideas & Notes
*Have an idea or feature request? Feel free to open an issue or submit a pull request on GitHub!*