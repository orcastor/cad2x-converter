QT_CPU_FEATURES.x86_64 = mmx sse sse2
QT.global_private.enabled_features = private_tests sse2 alloca_h alloca avx2 gc_binaries gui posix_fallocate reduce_exports reduce_relocations system-zlib
QT.global_private.disabled_features = alloca_malloc_h android-style-assets release_tools stack-protector-strong
PKG_CONFIG_EXECUTABLE = /usr/bin/pkg-config
QT_COORD_TYPE = double
QMAKE_LIBS_ZLIB = /usr/lib/x86_64-linux-gnu/libz.so
CONFIG += use_gold_linker sse2 aesni sse3 ssse3 sse4_1 sse4_2 avx avx2 avx512f avx512bw avx512cd avx512dq avx512er avx512ifma avx512pf avx512vbmi avx512vl enable_new_dtags f16c headersclean largefile precompile_header rdrnd shani warnings_are_errors x86SimdAlways
QT_BUILD_PARTS += libs tools
