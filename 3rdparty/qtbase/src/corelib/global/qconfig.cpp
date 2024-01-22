/* Installation Info */
static const char qt_configure_prefix_path_str  [12+256] = "qt_prfxpath=/opt/cad2x-converter/3rdparty/qtbase";
#ifdef QT_BUILD_QMAKE
static const char qt_configure_ext_prefix_path_str   [12+256] = "qt_epfxpath=/opt/cad2x-converter/3rdparty/qtbase";
static const char qt_configure_host_prefix_path_str  [12+256] = "qt_hpfxpath=/opt/cad2x-converter/3rdparty/qtbase";
#endif
static const short qt_configure_str_offsets[] = {
    0,
    8,
    12,
    20,
    24,
    32,
    36,
    38,
    40,
#ifdef QT_BUILD_QMAKE
    53,
    54,
    60,
    64,
    68,
    70,
    80,
#endif
};
static const char qt_configure_strs[] =
    "include\0"
    "lib\0"
    "libexec\0"
    "bin\0"
    "imports\0"
    "qml\0"
    ".\0"
    ".\0"
    "translations\0"
#ifdef QT_BUILD_QMAKE
    "\0"
    "false\0"
    "bin\0"
    "lib\0"
    ".\0"
    "linux-g++\0"
    "linux-g++\0"
#endif
;
#define QT_CONFIGURE_SETTINGS_PATH "etc/xdg"
#ifdef QT_BUILD_QMAKE
# define QT_CONFIGURE_SYSROOTIFY_PREFIX false
#endif
#define QT_CONFIGURE_PREFIX_PATH qt_configure_prefix_path_str + 12
#ifdef QT_BUILD_QMAKE
# define QT_CONFIGURE_EXT_PREFIX_PATH qt_configure_ext_prefix_path_str + 12
# define QT_CONFIGURE_HOST_PREFIX_PATH qt_configure_host_prefix_path_str + 12
#endif
