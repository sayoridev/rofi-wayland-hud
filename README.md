# rofi-wayland-hud

A stable, instant-trigger **Global Menu HUD** for **Wayland** (KDE Plasma 6, Hyprland, Sway), powered by **Rofi**.

## 🚀 Features

- ⚡ **Instant and non-interactive**  
  Automatically detects the active window and its PID without requiring manual mouse clicks or crosshairs.

- 🖥️ **Multi-Compositor Auto-Detection**  
  Built-in polymorphic tracking support for **KDE Plasma 6** (via `kdotool`), **Hyprland** (via `hyprctl`), and **Sway / wlroots** (via `swaymsg`), automatically detected at runtime.

- ⌨️ **Keyboard Shortcut Hints & Search**  
  Parses native D-Bus menu accelerator combinations (e.g. `Ctrl+S`, `Alt+F4`, `Ctrl+Shift+P`) and renders them alongside menu labels, enabling instant searching by shortcut in Rofi.

- 🎯 **Deterministic Focus Dispatch**  
  Encodes D-Bus service and object paths directly into Rofi payloads to eliminate focus-switch race conditions when activating actions.

- 🚄 **High-Performance C++20 Backend**  
  Built using `sdbus-c++` with dynamic `/MenuBar/*` introspection, process hierarchy traversal (PPID for Electron/Chromium), and strict timeouts preventing UI freezes.

- 🎨 **Clean Rofi Output Formatting**  
  Renders formatted window titles, hierarchical breadcrumbs (`File > Export > PDF`), and application icons.

## 🗺️ Roadmap & Contributing

Want to see what's planned or contribute to `rofi-wayland-hud`?  
Check out the **[TODO & Roadmap](TODO.md)** for upcoming features and architectural milestones.

## 📋 Dependencies

To build and run `rofi-wayland-hud`, you need the following packages:

- Rofi with Wayland support, such as `rofi-wayland`
- `kdotool`
- `sdbus-c++`
- CMake
- GCC or another compatible C++ compiler

## 📦 Installation

### Arch Linux

This project includes a native `PKGBUILD` for easy installation and package management with `pacman`.

#### Install dependencies

```bash
sudo pacman -S --needed base-devel cmake sdbus-cpp rofi-wayland
paru -S kdotool
```

#### Build and install

Clone the repository and enter the project directory:

```bash
git clone https://github.com/sayoridev/rofi-wayland-hud.git
cd rofi-wayland-hud
```

Build and install the package:

```bash
makepkg -si
```

The binary will be installed globally as:

```text
/usr/bin/rofi_wayland_hud
```

### Fedora

Install the required dependencies:

```bash
sudo dnf install cmake gcc-c++ sdbus-c++-devel kdotool rofi
```

Clone and build the project:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

### Ubuntu / Debian

Install the available dependencies:

```bash
sudo apt update
sudo apt install cmake build-essential libsdbus-c++-dev rofi
```

> ⚠️ `kdotool` may not be available in the official repositories. If this is the case, install it from source by following the instructions provided by the `kdotool` project.

Then clone and build `rofi-wayland-hud`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

## ⚙️ Usage

### 🧪 Manual testing

You can test the HUD directly from a terminal:

```bash
rofi \
  -modi "hud:rofi_wayland_hud" \
  -show hud \
  -show-icons \
  -theme-str 'window { width: 40em; }'
```

If everything is configured correctly, Rofi will display the global menu for the currently active window.

## ⌨️ Setting up a global shortcut in KDE Plasma 6

On Wayland, commands launched by KDE global shortcuts may not inherit all the required environment variables automatically.

For this reason, it is recommended to use a wrapper script.

### 1. Create the launcher script

Create a new file in your home directory:

```bash
nano ~/launch_hud.sh
```

Add the following content:

```bash
#!/usr/bin/env bash

export XDG_RUNTIME_DIR="/run/user/$(id -u)"
export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}"
export DBUS_SESSION_BUS_ADDRESS="unix:path=${XDG_RUNTIME_DIR}/bus"

exec rofi \
  -modi "hud:rofi_wayland_hud" \
  -show hud \
  -show-icons \
  -theme-str 'window { width: 40em; }'
```

### 2. Make the script executable

```bash
chmod +x ~/launch_hud.sh
```

### 3. Add the shortcut in KDE Plasma

1. Open **System Settings**.
2. Go to **Keyboard > Shortcuts**.
3. Create a new **Custom Shortcut** or **Command Shortcut**.
4. Set the command to:

   ```text
   /home/YOUR_USERNAME/launch_hud.sh
   ```

5. Assign your preferred key combination.

### 🛠️ Shortcut troubleshooting

If the shortcut does not work immediately after being configured:

1. Delete the existing shortcut binding.
2. Recreate the shortcut.
3. Assign the key combination again.

This may be necessary to force KDE Plasma and `kglobalaccel` to refresh the command execution context.

## 🐛 Troubleshooting

### Rofi cannot find the HUD mode

Check whether the binary is installed and available in your `PATH`:

```bash
command -v rofi_wayland_hud
```

You can also run it using the absolute path:

```bash
rofi \
  -modi "hud:$(command -v rofi_wayland_hud)" \
  -show hud
```

### The D-Bus menu is not displayed

Make sure that:

- the active application exposes a D-Bus menu;
- `kdotool` is installed and working correctly;
- the script is executed inside the correct graphical session;
- `DBUS_SESSION_BUS_ADDRESS` points to the current session bus.

### `WAYLAND_DISPLAY` is incorrect

Check the current Wayland display:

```bash
echo "$WAYLAND_DISPLAY"
```

If the value is different from `wayland-0`, update the variable in the wrapper script accordingly.

## 📄 License

This project is distributed under the terms of the **MIT License**.

See the [`LICENSE`](LICENSE) file for more information.