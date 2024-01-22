QT_CPU_FEATURES.arm64 = cx16 neon
QT.global_private.enabled_features = private_tests alloca_h alloca gc_binaries gui posix_fallocate reduce_exports system-zlib xml
QT.global_private.disabled_features = sse2 alloca_malloc_h android-style-assets avx2 reduce_relocations release_tools stack-protector-strong
PKG_CONFIG_EXECUTABLE = /usr/bin/pkg-config
QT_COORD_TYPE = double
QMAKE_LIBS_ZLIB = /usr/lib/aarch64-linux-gnu/libz.so
CONFIG += use_gold_linker enable_new_dtags headersclean largefile neon precompile_header warnings_are_errors
QT_BUILD_PARTS += libs tools
