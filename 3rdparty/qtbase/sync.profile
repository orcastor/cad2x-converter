%modules = ( # path to module name map
    "QtGui" => "$basedir/src/gui",
    "QtCore" => "$basedir/src/corelib",
    "QtZlib" => "!>$basedir/src/corelib;$basedir/src/3rdparty/zlib",
    "QtFontDatabaseSupport" => "$basedir/src/platformsupport/fontdatabases",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
@allmoduleheadersprivate = (
);
%classnames = (
    "qglobal.h" => "QtGlobal",
    "qendian.h" => "QtEndian",
    "qconfig.h" => "QtConfig",
    "qalgorithms.h" => "QtAlgorithms",
    "qcontainerfwd.h" => "QtContainerFwd",
    "qdebug.h" => "QtDebug",
    "qnamespace.h" => "Qt",
    "qnumeric.h" => "QtNumeric",
    "qvariant.h" => "QVariantHash,QVariantList,QVariantMap"
);
%deprecatedheaders = (
    "QtGui" =>  {
        "QGenericPlugin" => "QtGui/QGenericPlugin",
        "QGenericPluginFactory" => "QtGui/QGenericPluginFactory"
    }
);

@qpa_headers = ( qr/^qplatform/ );
my @internal_zlib_headers = ( "crc32.h", "deflate.h", "gzguts.h", "inffast.h", "inffixed.h", "inflate.h", "inftrees.h", "trees.h", "zutil.h" );
my @zlib_headers = ( "zconf.h", "zlib.h" );
@ignore_headers = ( @internal_zlib_headers );
@ignore_for_include_check = ( "qsystemdetection.h", "qcompilerdetection.h", "qprocessordetection.h", @zlib_headers);
@ignore_for_qt_begin_namespace_check = ( "qt_windows.h", @zlib_headers);
%inject_headers = (
    "$basedir/src/corelib/global" => [ "qconfig.h", "qconfig_p.h" ]
);
