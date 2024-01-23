# cad2x
A standalone & portable tool that can convert CAD files(dxf, dwg) to other formats(pdf, png, svg) modified from LibreCAD.

## How to build modified `qtbase(Qt 5.12.12)`

- static library
``` sh
./configure -developer-build -release -no-iconv -no-icu -static -confirm-license -opensource

# qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -static -confirm-license -opensource

cd ./3rdparty/qtbase/src/tools
make -j20 # build bootstrap & moc

cd -

cd ./3rdparty/qtbase/src/corelib
make -j20 staticlib
make install

cd -

cd ./3rdparty/qtbase/src/gui
make -j20 staticlib
make install
```

- shared library
``` sh
./configure -developer-build -release -no-iconv -no-icu -confirm-license -opensource -R .

# qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -confirm-license -opensource -R .

cd ./3rdparty/qtbase/src/tools
make -j20 # build bootstrap & moc

cd -

cd ./3rdparty/qtbase/src/corelib
make -j20
make install

cd -

cd ./3rdparty/qtbase/src/gui
make -j20
make install
```

## How to build cad2x

``` sh
qmake -qt=qt5 -r
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

## Referrence

You can refer to original [LibreCAD Wiki - BUILD FROM SOURCE](https://github.com/LibreCAD/LibreCAD/wiki/Build-from-source#generic-unix) page first.

#### Install toolchain

``` sh
apt-get install g++ gcc make git-core pkg-config -y --no-install-recommends
```