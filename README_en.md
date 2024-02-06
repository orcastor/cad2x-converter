[Simplified Chinese README | 简体中文说明](README.md)

<p align="center">
  <a href="https://orcastor.github.io/doc/">
    <img src="https://orcastor.github.io/doc/logo.svg">
  </a>
</p>

<h1 align="center"><strong>🆒 cad2x</strong> <a href="https://github.com/orcastor/addon-previewer">@orcastor-previewer</a></h1>

<p align="center">Releases: <a href="https://github.com/orcastor/addon-previewer/tree/main/back/cad2x/common"><strong>fonts & patterns</strong></a> (3.53MB) | <a href="https://github.com/orcastor/addon-previewer/tree/main/back/cad2x/linux_arm64"><strong>linux_arm64</strong></a> (2.88MB) | <a href="https://github.com/orcastor/addon-previewer/tree/main/back/cad2x/linux_x64"><strong>linux_x64</strong></a> (3.42MB)
</p>

`cad2x` - a minimal CLI tool that convert CAD files (DXF / DWG) to other formats (DXF / PDF / PNG / SVG) which is derived from [LibreCAD (commit: 0601535)](https://github.com/LibreCAD/LibreCAD/commit/0601535822c66a69da615463e42285616cfadedf).

# Features

- 💼 **Portable** - no GUI & no dependencies
- 🌈 **Customized** - with trimmed `Qt 5.12.12` (`QtCore` & `QtGUI`)
- 🚀 **Extremely Small** - **2.9MB** binary file size at all (on `aarch64-linux`)
- � **Unicode Display** - support display of malformed Unicode characters - [DXF File Encoding](https://ezdxf.readthedocs.io/en/stable/dxfinternals/fileencoding.html)
- 🗳️ **Cross Platform** - support Mac OSX / Windows / Linux / Android (Most linux based operating systems)
- 🔠 **TrueType Font Reuse** - lazy load `ttf`/`ttc` font (not only `lff`/`cxf`) - share system and other apps' fonts with less overhead

## Best Paractices

``` sh
Usage: ./cad2x [options] <dxf/dwg files>

Convert DXF/DWG files to DXF v2007/PDF/PNG/SVG files.

Examples:

  cad2x *.dxf|*.dwg|*
    -- print all dxf/dwg files to pdf files with the same base names.

  cad2x -o svg *.dxf|*.dwg|*
    -- print all dxf/dwg files to svg format.

  cad2x -o b.png a.dwg
    -- print a.dwg file to b.png.

Options:
  -h, --help                           Displays this help.
  -v, --version                        Displays version information.
  -a, --auto-orientation               Auto paper orientation (landscape or
                                       portrait) based on document bounding box.
  -b, --monochrome                     Print monochrome (black/white).
  -c, --fit                            Auto fit and center drawing to page.
  -e, --code-page <codepage>           Set default code page (default is
                                       ANSI_1252).
  -f, --default-font <font>            Set default font (default is standard).
  -l, --font-dirs <dir1,dir2,dir3...>  Set more font directories.
  -m, --margins <L,T,R,B>              Paper margins in mm (integer or float).
  -n, --pages <HxV>                    Print on multiple pages (Horiz. x
                                       Vert.).
  -o, --outfile <file>                 Output DXF v2007/PDF/PNG/SVG file.
  -p, --paper <WxH>                    Paper size (Width x Height) in mm.
  -r, --resolution <integer>           Output resolution (DPI).
  -s, --scale <double>                 Output scale. E.g.: 0.01 (for 1:100
                                       scale).
  -t, --directory <path>               Target output directory.

Arguments:
  <dxf/dwg files>                      Input DXF/DWG file(s)
```

### Convert all the `dwg` files to `pdf` format
- detect the orientation (-a)
- auto fit & center drawing in the page (-c)
- to `output` directory (-t)

> ./cad2x -o pdf *.dwg -t output -ac

### Convert `a.dxf` to `b.png`
- detect the orientation (-a)
- auto fit & center drawing in the page (-c)
- set page size to 2970 x 2100 (-p 2970x2100)
- set margins (-m 2.0,2.0,2.0,2.0)

> ./cad2x -o b.png a.dxf -ac -p 2970x2100 -m 2.0,2.0,2.0,2.0

### Convert `a.dwg` to `b.dxf` (v2007)

> ./cad2x -o b.dxf a.dwg

### Convert `a.dxf`(QCad / v2004 / v2000 / v14 / v12) to `b.dxf`(v2007)

> ./cad2x -o b.dxf a.dxf

### Convert `a.dxf` to `b.pdf`
- detect the orientation (-a)
- auto fit & center drawing in the page (-c)
- set default code page chinese (-e ANSI_936)
- set default font (-f simsun)
- add font list (-l /one/fonts,/two/fonts)

> ./cad2x -o b.pdf a.dxf -e ANSI_936 -f simsun -ac -l /one/fonts,/two/fonts

### Code Page References
|Language|DXF CodePage|
|-|-|
|Thai|ANSI_874|
|Japanese|ANSI_932|
|UnifiedChinese|ANSI_936|
|Korean|ANSI_949|
|TradChinese|ANSI_950|
|CentralEurope|ANSI_1250|
|Cyrillic|ANSI_1251|
|WesternEurope|ANSI_1252|
|Greek|ANSI_1253|
|Turkish|ANSI_1254|
|Hebrew|ANSI_1255|
|Arabic|ANSI_1256|
|Baltic|ANSI_1257|
|Vietnam|ANSI_1258|

## Build Tutorials

### Preparation

``` sh
apt-get install g++ gcc make git-core pkg-config qt5-qmake libfreetype-dev -y --no-install-recommends
```

### How to build trimmed `qtbase(Qt 5.12.12)`

- **[strongly recommended]** static QtCore & QtGUI library
``` sh
cd ./3rdparty/qtbase/
./configure -developer-build -release -no-iconv -no-icu -static -strip -confirm-license -opensource
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -static -strip -confirm-license -opensource
make -j20

cd src/corelib
make install_targ_headers

make -j20 staticlib

cd ../gui
make install_targ_headers

make -j20 staticlib
```

- [available alternative] shared QtCore & QtGUI library
``` sh
cd ./3rdparty/qtbase/
./configure -developer-build -release -no-iconv -no-icu -strip -confirm-license -opensource -R .
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -strip -confirm-license -opensource -R .
make -j20
make install
```

### How to build cad2x

``` sh
qmake -qt=qt5 -r
make -j20
```

## Changelog

- ✨ add font by easily dropping into `cad2x-converter/output/fonts`
- ✂️ trim QtBase
  - ✂️ trim dependency of QtWidgets & QtPrintSupport
  - ✂️ trim dependency of GUI app (QtGuiApplication / accessible / input / events / plugins)
  - ✂️ trim dependency of Render (OpenGL / Vulkan / KMS / VNC)
  - ✂️ trim dependency of qpa (X server and etc.)
  - ✂️ trim dependency of ICU / PCRE2
  - ✂️ trim useless codes (Pixmap / Icon / Windows and etc.)
  - ✂️ inbuilt font database without plugin
- ✂️ pick out dependent codes
- ✂️ trim dependency of boost
- ✂️ minimal binary size (static link) (2.9MB after `strip` and `upx -9 --ultra-brute`)
    ``` sh
    > ldd cad2pdf
        libdl.so.2 => /lib/aarch64-linux-gnu/libdl.so.2 (0x00000055021dc000)
        libglib-2.0.so.0 => /lib/aarch64-linux-gnu/libglib-2.0.so.0 (0x00000055021f0000)
        libpng16.so.16 => /lib/aarch64-linux-gnu/libpng16.so.16 (0x000000550232b000)
        libm.so.6 => /lib/aarch64-linux-gnu/libm.so.6 (0x000000550236f000)
        libfreetype.so.6 => /lib/aarch64-linux-gnu/libfreetype.so.6 (0x000000550241a000)
        libz.so.1 => /lib/aarch64-linux-gnu/libz.so.1 (0x00000055024d9000)
        libpthread.so.0 => /lib/aarch64-linux-gnu/libpthread.so.0 (0x000000550256b000)
        libstdc++.so.6 => /lib/aarch64-linux-gnu/libstdc++.so.6 (0x000000550259c000)
        libgcc_s.so.1 => /lib/aarch64-linux-gnu/libgcc_s.so.1 (0x0000005502781000)
        libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x00000055027a5000)
        /lib/ld-linux-aarch64.so.1 (0x0000005500000000)
        libpcre.so.3 => /lib/aarch64-linux-gnu/libpcre.so.3 (0x0000005502918000)
    ```
- ✂️ merge: cad2pdf + cad2pic = cad2x
- 🛠️ fix: png size auto setting
- 🛠️ fix: output file argument
- 🛠️ fix: malformed Unicode characters - [DXF File Encoding](https://ezdxf.readthedocs.io/en/stable/dxfinternals/fileencoding.html)
- [ ] 🛠️ fix: QCad format DXF incorrect offset of entities
- 🆕 feature: support auto orientation detection
- 🆕 feature: lazy load `ttf`/`ttc` font file support (not only `lff`/`cxf`)
- 🆕 feature: auto setting system font directories
  - **Windows** - `C:\Windows\Fonts`
  - **MacOSX** - `/Library/Fonts` `/System/Library/Fonts`
  - **Linux** - `/usr/share/fonts` `/usr/local/share/fonts` `~/.fonts`
- [ ] 🆕 feature: support more export formats
  - [ ] a quick way from DWG to BMP thumbnail
  - [ ] DWG / DXF to JSON for [cad.js](https://github.com/ghemingway/cad.js)
  - [ ] DWG / DXF to XML
  - [ ] DWG / DXF to TXT

## Referrence

You can refer to original [LibreCAD Wiki - BUILD FROM SOURCE](https://github.com/LibreCAD/LibreCAD/wiki/Build-from-source) page first.

## FAQ

Q: How to export other image formats like bmp / jpeg / tiff / ico / tga?
> A: Create a directory named 'imageformats' and put the `libq<fmt>.so`/`q<fmt>.dll` in it (can be found from official Qt5.12， \<fmt\> is the specified image format), app will automatically load the plugins.
