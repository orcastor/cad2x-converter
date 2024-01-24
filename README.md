<p align="center">
  <a href="https://orcastor.github.io/doc/">
    <img src="https://orcastor.github.io/doc/logo.svg">
  </a>
</p>

<h1 align="center"><strong>cad2x</strong> - sub tool of <a href="https://github.com/orcastor/addon-previewer">orcastor/addon-previewer</a></h1>

Minimal CLI tool convert CAD files (DXF / DWG) to other formats (PDF / PNG / SVG) trimming from [LibreCAD](https://github.com/LibreCAD/LibreCAD).

# Features

- 💼 **Portable** - no GUI & no dependencies
- 🌈 **Customized** - with trimmed Qt 5.12.12 (QtCore & QtGUI)
- 🚀 **Extremely small** - 2.9MB binary file size at all (on aarch64-linux)
- 🗳️ **Cross platform** - support Mac OSX / Windows / Linux (Most linux based operating systems)

## How to build trimmed `qtbase(Qt 5.12.12)`

- static QtCore & QtGUI library
``` sh
./configure -developer-build -release -no-iconv -no-icu -static -strip -confirm-license -opensource

cd ./3rdparty/qtbase/src/tools
make -j20 # build qmake & bootstrap & moc

cd -

cd ./3rdparty/qtbase/src/corelib
make -j20 staticlib

cd -

cd ./3rdparty/qtbase/src/gui
make -j20 staticlib
```

- shared QtCore & QtGUI library
``` sh
./configure -developer-build -release -no-iconv -no-icu -strip -confirm-license -opensource -R .

cd ./3rdparty/qtbase/src
make -j20 # build qmake & bootstrap & moc / corelib / gui
make install
```

## How to build cad2x

``` sh
qmake -qt=qt5 -r
make -j20
```

## Changelog

- ✨ add font by easily dropping `.lff` into `cad2x-converter/output/fonts`
- ✅ trim dependency of QtWidgets & QtPrintSupport
- ✅ trim dependency of GUI app (QtGuiApplication / accessible / input / events / plugins)
- ✅ trim dependency of Render (OpenGL / Vulkan / KMS / VNC)
- ✅ trim dependency of qpa (X server and etc.)
- ✅ trim dependency of icu / pcre2
- ✅ inbuild font database
- ✅ trim useless codes (Pixmap / Icon / Windows and etc.)
- ✅ minimal binary size (static link) (2.9MB after `strip` and `./upx -9 --ultra-brute`)
    ``` sh
    root@e69def756a3b:/opt/cad2x-converter/output# ldd cad2pdf
            libdl.so.2 => /lib/aarch64-linux-gnu/libdl.so.2 (0x00000055021dc000)
            libglib-2.0.so.0 => /lib/aarch64-linux-gnu/libglib-2.0.so.0 (0x00000055021f0000)
            libpng16.so.16 => /lib/aarch64-linux-gnu/libpng16.so.16 (0x000000550232b000)
            libm.so.6 => /lib/aarch64-linux-gnu/libm.so.6 (0x000000550236f000)
            libfreetype.so.6 => /lib/aarch64-linux-gnu/libfreetype.so.6 (0x000000550241a000)
            libz.so.1 => /lib/aarch64-linux-gnu/libz.so.1 (0x00000055024d9000)
            libmuparser.so.2 => /lib/aarch64-linux-gnu/libmuparser.so.2 (0x0000005502505000)
            libpthread.so.0 => /lib/aarch64-linux-gnu/libpthread.so.0 (0x000000550256b000)
            libstdc++.so.6 => /lib/aarch64-linux-gnu/libstdc++.so.6 (0x000000550259c000)
            libgcc_s.so.1 => /lib/aarch64-linux-gnu/libgcc_s.so.1 (0x0000005502781000)
            libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x00000055027a5000)
            /lib/ld-linux-aarch64.so.1 (0x0000005500000000)
            libpcre.so.3 => /lib/aarch64-linux-gnu/libpcre.so.3 (0x0000005502918000)
    ```
    - only `libmuparser.so.2` is 3rd party library
- [ ] finish the README
- [ ] merge: cad2pdf + cad2pic = cad2x

## Referrence

You can refer to original [LibreCAD Wiki - BUILD FROM SOURCE](https://github.com/LibreCAD/LibreCAD/wiki/Build-from-source) page first.

#### How to prepare toolchain

``` sh
apt-get install g++ gcc make git-core pkg-config -y --no-install-recommends
```
