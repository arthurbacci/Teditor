# Maintainer:  Guilherme Torquato <guibaux@tuta.io>

_pkgname=Teditor
_binname=ted
pkgname=$_pkgname-git
pkgver=0.1.468b8db
pkgrel=1
pkgdesc="Teditor - A simple text/code editor"
url="https://github.com/ArthurBacci64/Teditor"
arch=('i686' 'x86_64')
license=('GPL3')
depends=('ncurses')
makedepends=('git')
provides=($_binname)
conflicts=($_binname)
source=(git+https://github.com/ArthurBacci64/Teditor.git)
sha256sums=('SKIP')

build(){
  cd $_pkgname
  make
}

package() {
  cd $_pkgname
  make PREFIX=/usr DESTDIR="$pkgdir" install
  install -Dm644 LICENSE "$pkgdir"/usr/share/licenses/$pkgname/LICENSE
}

