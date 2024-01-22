# cad2x
A standalone tool that can convert cad file(dxf, dwg) to other formats(pdf, png, svg) from LibreCAD.

``` sh
apt-get install g++ gcc make git-core libmuparser-dev libfreetype6-dev libicu-dev pkg-config
```

``` sh
./configure -developer-build -release -no-iconv -no-icu -static -confirm-license -opensource
./configure -developer-build -release -no-iconv -no-icu -confirm-license -opensource -R .
```

``` sh
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -static -confirm-license -opensource
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -confirm-license -opensource -R .
```

## Features

- remove depends of QtWidgets & QtPrintSupport
- remove depends of GUI (QtGuiApplication / events / plugins)
- remove depends of OpenGL / Vulkan
- remove depends of qpa (X server and etc.)
- remove depends of icu
- remove depends of Pixmap
- static link