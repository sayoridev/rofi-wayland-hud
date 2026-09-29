# rofi-wayland-hud

> A stable, instant-trigger Global Menu HUD for **KDE Plasma 6 (Wayland)** powered by **Rofi**.

---

## 🚀 Features

* **Instant & Non-Interactive**: Automatically detects the active window and its PID using `kdotool` instantly, without requiring manual mouse clicks or crosshairs.
* **Wayland & KDE Plasma 6 Native**: Built specifically to overcome Wayland focus limitations and seamlessly bridge D-Bus menu data into Rofi.
* **High Performance C++ Backend**: Written in modern C++ using `sdbus-c++` for reliable and fast D-Bus communication.
* **Clean UI Formatting**: Custom output formatter for Rofi supporting clean window titles, fallback messages, and application icons.

---

## 📋 Dependencies

To build and run `rofi-wayland-hud`, you need the following packages installed on your system:

* **Rofi** (Wayland-compatible fork, e.g., `rofi-wayland`)
* **kdotool**
* **sdbus-cpp**
* **CMake** & **GCC** (for building)

---

## 📦 Installation (Arch Linux)

This project includes a native `PKGBUILD` for easy installation and package management via `pacman`.

1. Navigate to the project directory:
   ```bash
   cd rofi-wayland-hud
   ```

2. Build and install the package using `makepkg`:
   ```bash
   makepkg -si
   ```

This will automatically compile the source code in Release mode and install the binary globally to `/usr/bin/rofi_wayland_hud`.

---

## ⚙️ Configuration & Usage

### 1. Manual Testing
You can test the HUD directly from your terminal:
```bash
rofi -modi "hud:rofi_wayland_hud" -show hud -show-icons -theme-str "window { width: 40em; }"
```

### 2. Setting up a Global Shortcut in KDE Plasma 6
Due to Wayland's security context isolation, global shortcuts running background commands need proper environment variables.

1. Create a simple wrapper script in your home directory (e.g., `~/launch_hud.sh`):
   ```bash
   #!/bin/bash
   export XDG_RUNTIME_DIR="/run/user/$(id -u)"
   export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}"
   export DBUS_SESSION_BUS_ADDRESS="unix:path=$XDG_RUNTIME_DIR/bus"

   exec rofi -modi "hud:rofi_wayland_hud" -show hud -show-icons -theme-str "window { width: 40em; }"
   ```

2. Make it executable:
   ```bash
   chmod +x ~/launch_hud.sh
   ```

3. Open **System Settings > Shortcuts**, add a new **Command Shortcut**, and point it to your wrapper script:
   ```bash
   /home/YOUR_USERNAME/launch_hud.sh
   ```
   *(Note: If the shortcut doesn't trigger immediately on KDE Plasma 6, delete and re-add the shortcut binding to flush `kglobalaccel`'s execution context cache).*.

---

## 🛡️ License

Distributed under the **MIT License**. See `LICENSE` for more information.
