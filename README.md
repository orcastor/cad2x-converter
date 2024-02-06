[English README | 英文说明](README_en.md)

<p align="center">
  <a href="https://orcastor.github.io/doc/">
    <img src="https://orcastor.github.io/doc/logo.svg">
  </a>
</p>

<h1 align="center"><strong>🆒 cad2x</strong> <a href="https://github.com/orcastor/addon-previewer">@orcastor-previewer</a></h1>

<p align="center">预编译版本: <a href="https://github.com/orcastor/addon-previewer/tree/main/back/cad2x/common"><strong>字体与图案</strong></a> (3.53MB) | <a href="https://github.com/orcastor/addon-previewer/tree/main/back/cad2x/linux_arm64"><strong>linux_arm64</strong></a> (2.88MB) | <a href="https://github.com/orcastor/addon-previewer/tree/main/back/cad2x/linux_x64"><strong>linux_x64</strong></a> (3.42MB)
</p>

`cad2x` - 一个最小化的命令行工具，用于将 CAD 文件（DXF / DWG）转换为其他格式（DXF / PDF / PNG / SVG），衍生自 [LibreCAD (commit: 0601535)](https://github.com/LibreCAD/LibreCAD/commit/0601535822c66a69da615463e42285616cfadedf)。

# 特性

- 💼 **便携** - 无图形界面 & 无依赖项
- 🌈 **定制** - 使用精简的 `Qt 5.12.12` (`QtCore` & `QtGUI`)
- 🚀 **极小** - **2.9MB** 的二进制文件大小（在 `aarch64-linux` 平台上）
- � **显示** - 支持显示格式错误的 Unicode 字符 - [DXF 文件编码](https://ezdxf.readthedocs.io/en/stable/dxfinternals/fileencoding.html)
- 🗳️ **跨平台** - 支持 Mac OSX / Windows / Linux / Android（大多数基于 Linux 的操作系统）
- 🔠 **重用** - 懒加载 `ttf`/`ttc` TrueType 字体（不仅限于 `lff`/`cxf`）- 以较少的开销共享系统和其他应用程序的字体

## 最佳实践

``` sh
使用方法: ./cad2x [选项] <dxf/dwg 文件>

将 DXF/DWG 文件转换为 DXF v2007/PDF/PNG/SVG 文件。

示例:

  cad2x *.dxf|*.dwg|*
    -- 将所有 dxf/dwg 文件打印为具有相同文件名的 pdf 文件。

  cad2x -o svg *.dxf|*.dwg|*
    -- 将所有 dxf/dwg 文件转换为 svg 格式。

  cad2x -o b.png a.dwg
    -- 将 a.dwg 文件打印为 b.png。

选项:
  -h, --help                           显示此帮助。
  -v, --version                        显示版本信息。
  -a, --auto-orientation               根据文档边界框自动设置纸张方向（横向或纵向）。
  -b, --monochrome                     打印单色（黑/白）。
  -c, --fit                            自动适应并居中绘图到页面。
  -e, --code-page <codepage>           设置默认的代码页（默认为 ANSI_1252）。
  -f, --default-font <font>            设置默认字体（默认为 standard）。
  -l, --font-dirs <dir1,dir2,dir3...>  设置更多字体目录。
  -m, --margins <L,T,R,B>              页面边距（毫米）（整数或浮点数）。
  -n, --pages <HxV>                    在多页上打印（水平 x 垂直）。
  -o, --outfile <file>                 输出 DXF v2007/PDF/PNG/SVG 文件。
  -p, --paper <WxH>                    页面尺寸（宽 x 高）（毫米）。
  -r, --resolution <integer>           输出分辨率（DPI）。
  -s, --scale <double>                 输出比例。例如：0.01（表示 1:100 比例）。
  -t, --directory <path>               目标输出目录。

参数:
  <dxf/dwg 文件>                      输入 DXF/DWG 文件（们）
```

### 将所有 `dwg` 文件转换为 `pdf` 格式
- 检测方向 (-a)
- 自动适应并居中绘图到页面 (-c)
- 到 `output` 目录 (-t)

> ./cad2x -o pdf *.dwg -t output -ac

### 将 `a.dxf` 转换为 `b.png`
- 检测方向 (-a)
- 自动适应并居中绘图到页面 (-c)
- 将页面大小设置为 2970 x 2100 (-p 2970x2100)
- 设置边距 (-m 2.0,2.0,2.0,2.0)

> ./cad2x -o b.png a.dxf -ac -p 2970x2100 -m 2.0,2.0,2.0,2.0

### 将 `a.dwg` 转换为 `b.dxf`（v2007）

> ./cad2x -o b.dxf a.dwg

### 将 `a.dxf`（QCad / v2004 / v2000 / v14 / v12）转换为 `b.dxf`（v2007）

> ./cad2x -o b.dxf a.dxf

### 将 `a.dxf` 转换为 `b.pdf`
- 检测方向 (-a)
- 自动适应并居中绘图到页面 (-c)
- 设置默认代码页为中文 (-e ANSI_936)
- 设置默认字体 (-f simsun)
- 添加字体列表 (-l /one/fonts,/two/fonts)

> ./cad2x -o b.pdf a.dxf -e ANSI_936 -f simsun -ac -l /one/fonts,/two/fonts

### 代码页参考
|语言|DXF 代码页|
|-|-|
|泰语|ANSI_874|
|日语|ANSI_932|
|简体中文|ANSI_936|
|韩语|ANSI_949|
|繁体中文|ANSI_950|
|中欧|ANSI_1250|
|西里尔语|ANSI_1251|
|西欧|ANSI_1252|
|希腊语|ANSI_1253|
|土耳其语|ANSI_1254|
|希伯来语|ANSI_1255|
|阿拉伯语|ANSI_1256|
|波罗的海语|ANSI_1257|
|越南语|ANSI_1258|

## 构建教程

### 准备

``` sh
apt-get install g++ gcc make git-core pkg-config qt5-qmake libfreetype-dev -y --no-install-recommends
```

### 构建精简版的 `qtbase(Qt 5.12.12)`

- **[强烈推荐]** 静态 QtCore & QtGUI 库
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

- [可用备选方案] 共享 QtCore & QtGUI 库
``` sh
cd ./3rdparty/qtbase/
./configure -developer-build -release -no-iconv -no-icu -strip -confirm-license -opensource -R .
qmake -qt=qt5 -r -- -developer-build -release -no-iconv -no-icu -strip -confirm-license -opensource -R .
make -j20
make install
```

### 构建 cad2x

``` sh
qmake -qt=qt5 -r
make -j20
```

## 更改日志

- ✨ 通过简单拖放添加字体到 `cad2x-converter/output/fonts`
- ✂️ 裁剪QtBase
  - ✂️ 裁剪 QtWidgets & QtPrintSupport 依赖项
  - ✂️ 裁剪 GUI 应用程序的依赖项（QtGuiApplication / accessible / input / events / plugins）
  - ✂️ 裁剪 Render 的依赖项（OpenGL / Vulkan / KMS / VNC）
  - ✂️ 裁剪 qpa 的依赖项（X 服务器等）
  - ✂️ 裁剪 ICU / PCRE2 的依赖项
  - ✂️ 裁剪无用代码（Pixmap / Icon / Windows 等）
  - ✂️ 内置字体数据库，无需插件
- ✂️ 摘选出关键代码
- ✂️ 裁剪 boost 的依赖项
- ✂️ 最小的二进制文件大小（静态链接）（在 `strip` 和 `upx -9 --ultra-brute` 之后为 2.9MB）
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
- ✂️ 合并: cad2pdf + cad2pic = cad2x
- 🛠️ 修复: png 大小自动设置
- 🛠️ 修复: 输出文件参数
- 🛠️ 修复: 格式错误的 Unicode 字符 - [DXF 文件编码](https://ezdxf.readthedocs.io/en/stable/dxfinternals/fileencoding.html)
- [ ] 🛠️ 修复: QCad 格式 DXF 中实体的不正确偏移
- 🆕 功能: 支持自动方向检测
- 🆕 功能: 懒加载 `ttf`/`ttc` 字体文件支持（不仅限于 `lff`/`cxf`）
- 🆕 功能: 自动设置系统字体目录
  - **Windows** - `C:\Windows\Fonts`
  - **MacOSX** - `/Library/Fonts` `/System/Library/Fonts`
  - **Linux** - `/usr/share/fonts` `/usr/local/share/fonts` `~/.fonts`
- [ ] 🆕 功能: 支持更多导出格式
  - [ ] 从 DWG 到 BMP 缩略图的快速方式
  - [ ] DWG / DXF 到 JSON 用于 [cad.js](https://github.com/ghemingway/cad.js)
  - [ ] DWG / DXF 到 XML
  - [ ] DWG / DXF 到 TXT

## 参考

您可以首先参考原始的 [LibreCAD Wiki - 从源代码构建](https://github.com/LibreCAD/LibreCAD/wiki/Build-from-source) 页面。

## 常见问题解答

问: 如何导出其他图像格式，如 bmp / jpeg / tiff / ico / tga？
> 答: 创建一个名为 'imageformats' 的目录，并将 `libq<fmt>.so`/`q<fmt>.dll` 放入其中（可以从官方 Qt5.12 找到，\<fmt\>为具体格式），应用程序将自动加载插件。
