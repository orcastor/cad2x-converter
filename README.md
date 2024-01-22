# cad2x
A standalone tool that can convert cad file(dxf, dwg) to other formats(pdf, png, svg) from LibreCAD.

You can refer to original [BUILDING FROM SOURCE](https://github.com/LibreCAD/LibreCAD/wiki/Build-from-source#generic-unix) page.

``` sh
apt-get install g++ gcc make git-core libmuparser-dev libfreetype6-dev libicu-dev pkg-config
```

## How to config

- static library
``` sh
./configure -developer-build -release -no-iconv -no-icu -static -confirm-license -opensource
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -static -confirm-license -opensource
make -j20 staticlib # just in module dir
```

- shared library
``` sh
./configure -developer-build -release -no-iconv -no-icu -confirm-license -opensource -R .
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -confirm-license -opensource -R .
make -j20
```

## Features

- ✨ add font by dropping `.lff` into `cad2x-converter/output/fonts` without plugin
- ✅ remove dependency of QtWidgets & QtPrintSupport
- ✅ remove dependency of GUI app (QtGuiApplication / accessible / input / events / plugins)
- ✅ remove dependency of Render (OpenGL / Vulkan / KMS / VNC)
- ✅ remove dependency of qpa (X server and etc.)
- ✅ remove dependency of icu / pcre
- ✅ remove useless codes (Pixmap / Icon / Windows and etc.)
- ✅ minimal binary size (static link)

## TODO

- [ ] finish the README
- [ ] merge: cad2pdf + cad2pic = cad2x