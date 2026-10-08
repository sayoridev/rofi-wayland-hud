# Maintainer: sayoridev <sayoridevuwu@gmail.com>
pkgname=rofi-wayland-hud
pkgver=1.0.0
pkgrel=1
pkgdesc="Stable, instant-trigger Global Menu HUD for Wayland (KDE Plasma 6, Hyprland, Sway) using Rofi"
arch=('x86_64')
url="https://github.com/sayoridev/rofi-wayland-hud"
license=('MIT')
depends=('rofi' 'sdbus-cpp')
optdepends=(
    'kdotool: for KDE Plasma support'
    'hyprland: for Hyprland support'
    'sway: for Sway support'
)
makedepends=('cmake' 'gcc')
provides=('rofi-wayland-hud')
options=('!debug')
source=()

build() {
    cmake -B build -S "$startdir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    DESTDIR="$pkgdir" cmake --install build
}