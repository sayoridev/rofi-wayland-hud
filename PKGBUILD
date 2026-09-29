# Maintainer: Tuo Nome <tua@email.com>
pkgname=rofi-wayland-hud
pkgver=1.0.0
pkgrel=1
pkgdesc="Stable, instant-trigger Global Menu HUD for KDE Plasma 6 Wayland using Rofi"
arch=('x86_64')
url="https://github.com/tuonome/rofi-wayland-hud"
license=('MIT')
depends=('rofi' 'kdotool' 'sdbus-cpp')
makedepends=('cmake' 'gcc')
provides=('rofi-wayland-hud')
conflicts=('rofi-wayland-hud')
options=('!debug') # <-- Questa riga dice a makepkg di saltare la generazione del pacchetto di debug
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