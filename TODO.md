# 📌 Project Roadmap & TODO List — `rofi-wayland-hud`

This document outlines planned features, structural improvements, and platform support extensions for `rofi-wayland-hud`.

---

## 🏎️ Phase 1: Modular Architecture & Multi-Compositor Support
> *Goal: Abstract the window tracking mechanism so the HUD works seamlessly across various Wayland compositors beyond KDE Plasma.*

- [ ] **Refactor `WindowTracker` to Polymorphic Strategy Pattern**
  - Define an abstract C++ interface/base class `WindowTracker`.
  - Move current `kdotool` implementation into `KWinTracker : public WindowTracker`.
- [ ] **Implement Hyprland Support**
  - Create `HyprlandTracker` using `hyprctl activewindow -j` for zero-delay active window PID and title extraction.
- [ ] **Implement Sway / wlroots Support**
  - Create `SwayTracker` using `swaymsg -t get_tree` IPC socket calls.
  - Explore native `wlr-foreign-toplevel-management-unstable-v1` integration.
- [ ] **Automatic Compositor Auto-Detection**
  - Parse `$XDG_CURRENT_DESKTOP` and `$WAYLAND_DISPLAY` at startup to dynamically instantiate the appropriate tracker.

---

## 🎨 Phase 2: User Experience & Customization

- [ ] **Keyboard Shortcut Hints in Rofi Menu**
  - Parse D-Bus menu accelerator strings (e.g., `Ctrl+S`, `Alt+F4`) and display them on the right side of the Rofi entry (`\0meta\x1f...` or custom styling).
- [ ] **Enhanced Application Icon Resolution**
  - Fallback icon matching using `.desktop` files when D-Bus doesn't provide an explicit icon name.
- [ ] **Config File Support (`~/.config/rofi-hud/config.ini` or `.json`)**
  - Allow users to configure default parameters without wrapping Rofi commands:
    - Custom Rofi theme overrides.
    - Ignored application blacklists (e.g., ignore Desktop background or Panels).
    - Custom fallback message text.
- [ ] **Submenu Breadcrumbs Navigation**
  - Add visual breadcrumb indicators (e.g., `File > Export > PDF`) for deeply nested submenus.

---

## 🔧 Phase 3: Performance, Stability & Edge Cases

- [ ] **Asynchronous D-Bus Extraction with Timeout Protection**
  - Prevent Rofi interface freezing when querying unresponsive or frozen background applications.
- [ ] **Submenu Cache with Invalidation**
  - Cache static menu structures for long-running heavy applications (like GIMP, Inkscape, VS Code) to achieve sub-millisecond render times.
- [ ] **Qt / GTK / Electron Specific Workarounds**
  - Handle edge cases where Chromium/Electron apps register menu bars under non-standard D-Bus paths.

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