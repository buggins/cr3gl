#-------------------------------------------------
#
# Project created by QtCreator 2013-08-22T11:47:47
#
#-------------------------------------------------

QT       += core gui network opengl openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cr3
TEMPLATE = app


SOURCES += main.cpp\
        cr3qt.cpp \
        cr3mainwindow.cpp

HEADERS  += cr3mainwindow.h \
            cr3qt.h
            
win32 {
    DEFINES += _WIN32=1 \
        WIN32=1 \
        _CRT_SECURE_NO_WARNINGS \
        CR_EMULATE_GETTEXT=1
    LIBS += -lgdi32 -Lc:/Qt/OpenSSL/lib
    debug:LIBS += -lQt5PlatformSupportd -lQt5Cored
#    !debug:LIBS += -lQt5PlatformSupport -lQt5Core
#    LIBS += -lQt5PlatformSupport -lQt5Core
    RC_FILE = cr3.rc
}
!win32 {
    DEFINES += _LINUX=1 \
        LINUX=1
    INCLUDEPATH += /usr/include/freetype2
}

DEFINES += _DEBUG=1 DEBUG=1
#debug:DEFINES += _DEBUG=1 DEBUG=1
#!debug:DEFINES += NDEBUG _ITERATOR_DEBUG_LEVEL=0
#DEFINES += NDEBUG _ITERATOR_DEBUG_LEVEL=0

DEFINES += USE_FREETYPE=1 \
    COLOR_BACKBUFFER=1 \
    USE_DOM_UTF8_STORAGE=1 \
    CR3_PATCH \
    DOC_DATA_COMPRESSION_LEVEL=1 \
    DOC_BUFFER_SIZE=0xA00000 \
    ENABLE_CACHE_FILE_CONTENTS_VALIDATION=1 \
    LDOM_USE_OWN_MEM_MAN=0 \
    CR3_ANTIWORD_PATCH=1 \
    ENABLE_ANTIWORD=1 \
    MAX_IMAGE_SCALE_MUL=2 \
    QT_GL=1 \
    NO_WIN32_DRAWING=1 \
    CR_EMULATE_GETTEXT=1 \
    FT2_BUILD_LIBRARY=1

macx:DEFINES += USE_FONTCONFIG=0 CR3_OSX=1

INCLUDEPATH += ../../../cr3/crengine/include \
    ../../../cr3/thirdparty/libpng \
    ../../../cr3/thirdparty/sqlite \
    ../../../cr3/thirdparty/zlib \
    ../../../cr3/thirdparty/chmlib \
    ../../../cr3/thirdparty/antiword \
    ../../../cr3/thirdparty/freetype/include \
    ../../../cr3/thirdparty/libjpeg

SOURCES += \
    ../../../cr3/crengine/src/crconcurrent.cpp \
    ../../../cr3/crengine/src/wordfmt.cpp \
    ../../../cr3/crengine/src/wolutil.cpp \
    ../../../cr3/crengine/src/txtselector.cpp \
    ../../../cr3/crengine/src/rtfimp.cpp \
    ../../../cr3/crengine/src/props.cpp \
    ../../../cr3/crengine/src/pdbfmt.cpp \
    ../../../cr3/crengine/src/lvxml.cpp \
    ../../../cr3/crengine/src/lvtinydom.cpp \
    ../../../cr3/crengine/src/lvtextfm.cpp \
    ../../../cr3/crengine/src/lvstyles.cpp \
    ../../../cr3/crengine/src/lvstsheet.cpp \
    ../../../cr3/crengine/src/lvstring.cpp \
    ../../../cr3/crengine/src/lvstream.cpp \
    ../../../cr3/crengine/src/lvrend.cpp \
    ../../../cr3/crengine/src/lvpagesplitter.cpp \
    ../../../cr3/crengine/src/lvmemman.cpp \
    ../../../cr3/crengine/src/lvimg.cpp \
    ../../../cr3/crengine/src/lvfntman.cpp \
    ../../../cr3/crengine/src/lvfnt.cpp \
    ../../../cr3/crengine/src/lvdrawbuf.cpp \
    ../../../cr3/crengine/src/lvdocview.cpp \
    ../../../cr3/crengine/src/lvbmpbuf.cpp \
    ../../../cr3/crengine/src/lstridmap.cpp \
    ../../../cr3/crengine/src/hyphman.cpp \
    ../../../cr3/crengine/src/hist.cpp \
    ../../../cr3/crengine/src/epubfmt.cpp \
    ../../../cr3/crengine/src/crtxtenc.cpp \
    ../../../cr3/crengine/src/cri18n.cpp \
    ../../../cr3/crengine/src/cp_stats.cpp \
    ../../../cr3/crengine/src/bookformats.cpp \
    ../../../cr3/crengine/src/chmfmt.cpp \
    ../../../cr3/thirdparty/libpng/pngerror.c  \
    ../../../cr3/thirdparty/libpng/pngget.c  \
    ../../../cr3/thirdparty/libpng/pngpread.c \
    ../../../cr3/thirdparty/libpng/pngrio.c \
    ../../../cr3/thirdparty/libpng/pngrutil.c \
    ../../../cr3/thirdparty/libpng/pngvcrd.c \
    ../../../cr3/thirdparty/libpng/png.c \
    ../../../cr3/thirdparty/libpng/pngwrite.c \
    ../../../cr3/thirdparty/libpng/pngwutil.c \
    ../../../cr3/thirdparty/libpng/pnggccrd.c \
    ../../../cr3/thirdparty/libpng/pngmem.c \
    ../../../cr3/thirdparty/libpng/pngread.c \
    ../../../cr3/thirdparty/libpng/pngrtran.c \
    ../../../cr3/thirdparty/libpng/pngset.c \
    ../../../cr3/thirdparty/libpng/pngtrans.c \
    ../../../cr3/thirdparty/libpng/pngwio.c \
    ../../../cr3/thirdparty/libpng/pngwtran.c \
    ../../../cr3/thirdparty/libjpeg/jcapimin.c \
    ../../../cr3/thirdparty/libjpeg/jchuff.c \
    ../../../cr3/thirdparty/libjpeg/jcomapi.c \
    ../../../cr3/thirdparty/libjpeg/jctrans.c \
    ../../../cr3/thirdparty/libjpeg/jdcoefct.c \
    ../../../cr3/thirdparty/libjpeg/jdmainct.c \
    ../../../cr3/thirdparty/libjpeg/jdpostct.c \
    ../../../cr3/thirdparty/libjpeg/jfdctfst.c \
    ../../../cr3/thirdparty/libjpeg/jidctred.c \
    ../../../cr3/thirdparty/libjpeg/jutils.c \
    ../../../cr3/thirdparty/libjpeg/jcapistd.c \
    ../../../cr3/thirdparty/libjpeg/jcinit.c \
    ../../../cr3/thirdparty/libjpeg/jcparam.c \
    ../../../cr3/thirdparty/libjpeg/jdapimin.c \
    ../../../cr3/thirdparty/libjpeg/jdcolor.c \
    ../../../cr3/thirdparty/libjpeg/jdmarker.c \
    ../../../cr3/thirdparty/libjpeg/jdsample.c \
    ../../../cr3/thirdparty/libjpeg/jfdctint.c \
    ../../../cr3/thirdparty/libjpeg/jmemmgr.c \
    ../../../cr3/thirdparty/libjpeg/jccoefct.c \
    ../../../cr3/thirdparty/libjpeg/jcmainct.c \
    ../../../cr3/thirdparty/libjpeg/jcphuff.c \
    ../../../cr3/thirdparty/libjpeg/jdapistd.c \
    ../../../cr3/thirdparty/libjpeg/jddctmgr.c \
    ../../../cr3/thirdparty/libjpeg/jdmaster.c \
    ../../../cr3/thirdparty/libjpeg/jdtrans.c \
    ../../../cr3/thirdparty/libjpeg/jidctflt.c \
    ../../../cr3/thirdparty/libjpeg/jmemnobs.c \
    ../../../cr3/thirdparty/libjpeg/jccolor.c \
    ../../../cr3/thirdparty/libjpeg/jcmarker.c \
    ../../../cr3/thirdparty/libjpeg/jcprepct.c \
    ../../../cr3/thirdparty/libjpeg/jdatadst.c \
    ../../../cr3/thirdparty/libjpeg/jdhuff.c \
    ../../../cr3/thirdparty/libjpeg/jdmerge.c \
    ../../../cr3/thirdparty/libjpeg/jerror.c \
    ../../../cr3/thirdparty/libjpeg/jidctfst.c \
    ../../../cr3/thirdparty/libjpeg/jquant1.c \
    ../../../cr3/thirdparty/libjpeg/jcdctmgr.c \
    ../../../cr3/thirdparty/libjpeg/jcmaster.c \
    ../../../cr3/thirdparty/libjpeg/jcsample.c \
    ../../../cr3/thirdparty/libjpeg/jdatasrc.c \
    ../../../cr3/thirdparty/libjpeg/jdinput.c \
    ../../../cr3/thirdparty/libjpeg/jdphuff.c \
    ../../../cr3/thirdparty/libjpeg/jfdctflt.c \
    ../../../cr3/thirdparty/libjpeg/jidctint.c \
    ../../../cr3/thirdparty/libjpeg/jquant2.c \
    ../../../cr3/thirdparty/freetype/src/autofit/autofit.c \
    ../../../cr3/thirdparty/freetype/src/bdf/bdf.c \
    ../../../cr3/thirdparty/freetype/src/cff/cff.c \
    ../../../cr3/thirdparty/freetype/src/base/ftbase.c \
    ../../../cr3/thirdparty/freetype/src/base/ftbbox.c \
    ../../../cr3/thirdparty/freetype/src/base/ftbdf.c \
    ../../../cr3/thirdparty/freetype/src/base/ftbitmap.c \
    ../../../cr3/thirdparty/freetype/src/base/ftgasp.c \
    ../../../cr3/thirdparty/freetype/src/cache/ftcache.c \
    ../../../cr3/thirdparty/freetype/src/base/ftglyph.c \
    ../../../cr3/thirdparty/freetype/src/base/ftgxval.c \
    ../../../cr3/thirdparty/freetype/src/gzip/ftgzip.c \
    ../../../cr3/thirdparty/freetype/src/base/ftinit.c \
    ../../../cr3/thirdparty/freetype/src/lzw/ftlzw.c \
    ../../../cr3/thirdparty/freetype/src/base/ftmm.c \
    ../../../cr3/thirdparty/freetype/src/base/ftpatent.c \
    ../../../cr3/thirdparty/freetype/src/base/ftotval.c \
    ../../../cr3/thirdparty/freetype/src/base/ftpfr.c \
    ../../../cr3/thirdparty/freetype/src/base/ftstroke.c \
    ../../../cr3/thirdparty/freetype/src/base/ftsynth.c \
    ../../../cr3/thirdparty/freetype/src/base/ftsystem.c \
    ../../../cr3/thirdparty/freetype/src/base/fttype1.c \
    ../../../cr3/thirdparty/freetype/src/base/ftwinfnt.c \
    ../../../cr3/thirdparty/freetype/src/base/ftxf86.c \
    ../../../cr3/thirdparty/freetype/src/winfonts/winfnt.c \
    ../../../cr3/thirdparty/freetype/src/pcf/pcf.c \
    ../../../cr3/thirdparty/freetype/src/pfr/pfr.c \
    ../../../cr3/thirdparty/freetype/src/psaux/psaux.c \
    ../../../cr3/thirdparty/freetype/src/pshinter/pshinter.c \
    ../../../cr3/thirdparty/freetype/src/psnames/psmodule.c \
    ../../../cr3/thirdparty/freetype/src/raster/raster.c \
    ../../../cr3/thirdparty/freetype/src/sfnt/sfnt.c \
    ../../../cr3/thirdparty/freetype/src/smooth/smooth.c \
    ../../../cr3/thirdparty/freetype/src/truetype/truetype.c \
    ../../../cr3/thirdparty/freetype/src/type1/type1.c \
    ../../../cr3/thirdparty/freetype/src/cid/type1cid.c \
    ../../../cr3/thirdparty/freetype/src/type42/type42.c \
    ../../../cr3/thirdparty/zlib/adler32.c \
    ../../../cr3/thirdparty/zlib/crc32.c \
    ../../../cr3/thirdparty/zlib/infback.c \
    ../../../cr3/thirdparty/zlib/inflate.c \
    ../../../cr3/thirdparty/zlib/uncompr.c \
    ../../../cr3/thirdparty/zlib/compress.c \
    ../../../cr3/thirdparty/zlib/deflate.c \
    ../../../cr3/thirdparty/zlib/gzio.c \
    ../../../cr3/thirdparty/zlib/inffast.c \
    ../../../cr3/thirdparty/zlib/inftrees.c \
    ../../../cr3/thirdparty/zlib/trees.c \
    ../../../cr3/thirdparty/zlib/zutil.c

HEADERS += \
    ../../../cr3/crengine/include/crconcurrent.h \
    ../../../cr3/crengine/include/wordfmt.h \
    ../../../cr3/crengine/include/wolutil.h \
    ../../../cr3/crengine/include/txtselector.h \
    ../../../cr3/crengine/include/rtfimp.h \
    ../../../cr3/crengine/include/rtfcmd.h \
    ../../../cr3/crengine/include/props.h \
    ../../../cr3/crengine/include/pdbfmt.h \
    ../../../cr3/crengine/include/lvxml.h \
    ../../../cr3/crengine/include/lvtypes.h \
    ../../../cr3/crengine/include/lvtinydom.h \
    ../../../cr3/crengine/include/lvthread.h \
    ../../../cr3/crengine/include/lvtextfm.h \
    ../../../cr3/crengine/include/lvstyles.h \
    ../../../cr3/crengine/include/lvstsheet.h \
    ../../../cr3/crengine/include/lvstring.h \
    ../../../cr3/crengine/include/lvstream.h \
    ../../../cr3/crengine/include/lvrend.h \
    ../../../cr3/crengine/include/lvrefcache.h \
    ../../../cr3/crengine/include/lvref.h \
    ../../../cr3/crengine/include/lvptrvec.h \
    ../../../cr3/crengine/include/lvpagesplitter.h \
    ../../../cr3/crengine/include/lvmemman.h \
    ../../../cr3/crengine/include/lvimg.h \
    ../../../cr3/crengine/include/lvhashtable.h \
    ../../../cr3/crengine/include/lvqueue.h \
    ../../../cr3/crengine/include/lvfntman.h \
    ../../../cr3/crengine/include/lvfnt.h \
    ../../../cr3/crengine/include/lvdrawbuf.h \
    ../../../cr3/crengine/include/lvdocview.h \
    ../../../cr3/crengine/include/lvbmpbuf.h \
    ../../../cr3/crengine/include/lvarray.h \
    ../../../cr3/crengine/include/lstridmap.h \
    ../../../cr3/crengine/include/hyphman.h \
    ../../../cr3/crengine/include/hist.h \
    ../../../cr3/crengine/include/gammatbl.h \
    ../../../cr3/crengine/include/fb2def.h \
    ../../../cr3/crengine/include/epubfmt.h \
    ../../../cr3/crengine/include/dtddef.h \
    ../../../cr3/crengine/include/cssdef.h \
    ../../../cr3/crengine/include/crtxtenc.h \
    ../../../cr3/crengine/include/crtrace.h \
    ../../../cr3/crengine/include/crsetup.h \
    ../../../cr3/crengine/include/cri18n.h \
    ../../../cr3/crengine/include/crengine.h \
    ../../../cr3/crengine/include/cr3version.h \
    ../../../cr3/crengine/include/cp_stats.h \
    ../../../cr3/crengine/include/chmfmt.h \
    ../../../cr3/crengine/include/crlocks.h \
    ../../../cr3/crengine/include/bookformats.h \
    ../../../cr3/crengine/include/lvdocviewprops.h \
    ../../../cr3/crengine/include/lvdocviewcmd.h \
    ../../../cr3/crengine/include/lvautoptr.h


#!win32 {
    unix:!macx:LIBS += -ljpeg
#    win32:LIBS += libjpeg.lib
#}
win32 {
#    INCLUDEPATH += D:\Qt\5.2.0\Src\qtbase\src\3rdparty\libjpeg
#    INCLUDEPATH += D:\Qt\Qt5.4.0\5.4\msvc2013_64_opengl\include\QtZlib 
#    INCLUDEPATH += ../../../cr3/thirdparty/libjpeg
#    SOURCES += ../../../cr3/thirdparty/libjpeg/jcapimin.c \
#        ../../../cr3/thirdparty/libjpeg/jcapistd.c \
#        ../../../cr3/thirdparty/libjpeg/jccoefct.c \
#        ../../../cr3/thirdparty/libjpeg/jccolor.c \
#        ../../../cr3/thirdparty/libjpeg/jcdctmgr.c \
#        ../../../cr3/thirdparty/libjpeg/jchuff.c \
#        ../../../cr3/thirdparty/libjpeg/jcinit.c \
#        ../../../cr3/thirdparty/libjpeg/jcmainct.c \
#        ../../../cr3/thirdparty/libjpeg/jcmarker.c \
#        ../../../cr3/thirdparty/libjpeg/jcmaster.c \
#        ../../../cr3/thirdparty/libjpeg/jcomapi.c \
#        ../../../cr3/thirdparty/libjpeg/jcparam.c \
#        ../../../cr3/thirdparty/libjpeg/jcphuff.c \
#        ../../../cr3/thirdparty/libjpeg/jcprepct.c \
#        ../../../cr3/thirdparty/libjpeg/jcsample.c \
#        ../../../cr3/thirdparty/libjpeg/jctrans.c \
#        ../../../cr3/thirdparty/libjpeg/jdapimin.c \
#        ../../../cr3/thirdparty/libjpeg/jdapistd.c \
#        ../../../cr3/thirdparty/libjpeg/jdatadst.c \
#        ../../../cr3/thirdparty/libjpeg/jdatasrc.c \
#        ../../../cr3/thirdparty/libjpeg/jdcoefct.c \
#        ../../../cr3/thirdparty/libjpeg/jdcolor.c \
#        ../../../cr3/thirdparty/libjpeg/jddctmgr.c \
#        ../../../cr3/thirdparty/libjpeg/jdhuff.c \
#        ../../../cr3/thirdparty/libjpeg/jdinput.c \
#        ../../../cr3/thirdparty/libjpeg/jdmainct.c \
#        ../../../cr3/thirdparty/libjpeg/jdmarker.c \
#        ../../../cr3/thirdparty/libjpeg/jdmaster.c \
#        ../../../cr3/thirdparty/libjpeg/jdmerge.c \
#        ../../../cr3/thirdparty/libjpeg/jdphuff.c \
#        ../../../cr3/thirdparty/libjpeg/jdpostct.c \
#        ../../../cr3/thirdparty/libjpeg/jdsample.c \
#        ../../../cr3/thirdparty/libjpeg/jdtrans.c \
#        ../../../cr3/thirdparty/libjpeg/jerror.c \
#        ../../../cr3/thirdparty/libjpeg/jfdctflt.c \
#        ../../../cr3/thirdparty/libjpeg/jfdctfst.c \
#        ../../../cr3/thirdparty/libjpeg/jfdctint.c \
#        ../../../cr3/thirdparty/libjpeg/jidctflt.c \
#        ../../../cr3/thirdparty/libjpeg/jidctfst.c \
#        ../../../cr3/thirdparty/libjpeg/jidctint.c \
#        ../../../cr3/thirdparty/libjpeg/jidctred.c \
#        ../../../cr3/thirdparty/libjpeg/jmemmgr.c \
#        ../../../cr3/thirdparty/libjpeg/jquant1.c \
#        ../../../cr3/thirdparty/libjpeg/jquant2.c \
#        ../../../cr3/thirdparty/libjpeg/jutils.c \
#        ../../../cr3/thirdparty/libjpeg/jmemnobs.c
}
#!win32 {
    unix:!macx:LIBS += -lpng -ldl
#    win32:LIBS += libpng.lib
#}
win32 {
#    INCLUDEPATH += ../../../cr3/thirdparty/libpng
#    INCLUDEPATH += C:\Qt\5.2.0\Src\qtbase\src\3rdparty\libpng
#    SOURCES += ../../../cr3/thirdparty/libpng/png.c \
#        ../../../cr3/thirdparty/libpng/pngset.c \
#        ../../../cr3/thirdparty/libpng/pngget.c \
#        ../../../cr3/thirdparty/libpng/pngrutil.c \
#        ../../../cr3/thirdparty/libpng/pngtrans.c \
#        ../../../cr3/thirdparty/libpng/pngwutil.c \
#        ../../../cr3/thirdparty/libpng/pngread.c \
#        ../../../cr3/thirdparty/libpng/pngrio.c \
#        ../../../cr3/thirdparty/libpng/pngwio.c \
#        ../../../cr3/thirdparty/libpng/pngwrite.c \
#        ../../../cr3/thirdparty/libpng/pngrtran.c \
#        ../../../cr3/thirdparty/libpng/pngwtran.c \
#        ../../../cr3/thirdparty/libpng/pngmem.c \
#        ../../../cr3/thirdparty/libpng/pngerror.c \
#        ../../../cr3/thirdparty/libpng/pngpread.c
}
#!win32 {
    unix:!macx:LIBS += -lfreetype -lfontconfig
#    win32:LIBS += libfreetype.lib
#}
win32 {
#    DEFINES += FT2_BUILD_LIBRARY=1
#    INCLUDEPATH += ../../../cr3/thirdparty/freetype/include
    INCLUDEPATH += C:\Qt\5.2.0\Src\qtbase\src\3rdparty\freetype\include
#    SOURCES += ../../../cr3/thirdparty/freetype/src/autofit/autofit.c \
#        ../../../cr3/thirdparty/freetype/src/bdf/bdf.c \
#        ../../../cr3/thirdparty/freetype/src/cff/cff.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftbase.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftbbox.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftbdf.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftbitmap.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftgasp.c \
#        ../../../cr3/thirdparty/freetype/src/cache/ftcache.c \
#        ../../../cr3/thirdparty/freetype/builds/win32/ftdebug.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftglyph.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftgxval.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftinit.c \
#        ../../../cr3/thirdparty/freetype/src/lzw/ftlzw.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftmm.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftotval.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftpfr.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftstroke.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftsynth.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftsystem.c \
#        ../../../cr3/thirdparty/freetype/src/base/fttype1.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftwinfnt.c \
#        ../../../cr3/thirdparty/freetype/src/base/ftxf86.c \
#        ../../../cr3/thirdparty/freetype/src/pcf/pcf.c \
#        ../../../cr3/thirdparty/freetype/src/pfr/pfr.c \
#        ../../../cr3/thirdparty/freetype/src/psaux/psaux.c \
#        ../../../cr3/thirdparty/freetype/src/pshinter/pshinter.c \
#        ../../../cr3/thirdparty/freetype/src/psnames/psmodule.c \
#        ../../../cr3/thirdparty/freetype/src/raster/raster.c \
#        ../../../cr3/thirdparty/freetype/src/sfnt/sfnt.c \
#        ../../../cr3/thirdparty/freetype/src/smooth/smooth.c \
#        ../../../cr3/thirdparty/freetype/src/truetype/truetype.c \
#        ../../../cr3/thirdparty/freetype/src/type1/type1.c \
#        ../../../cr3/thirdparty/freetype/src/cid/type1cid.c \
#        ../../../cr3/thirdparty/freetype/src/type42/type42.c \
#        ../../../cr3/thirdparty/freetype/src/winfonts/winfnt.c
}
#!win32 {
    unix:!macx:LIBS += -lz
#    win32:LIBS += libz.lib
#}
win32 {
    #INCLUDEPATH += ../../../cr3/thirdparty/zlib
#    INCLUDEPATH += C:\Qt\5.2.0\Src\qtbase\src\3rdparty\zlib
#    SOURCES += ../../../cr3/thirdparty/zlib/adler32.c \
#        ../../../cr3/thirdparty/zlib/compress.c \
#        ../../../cr3/thirdparty/zlib/crc32.c \
#        ../../../cr3/thirdparty/zlib/gzio.c \
#        ../../../cr3/thirdparty/zlib/uncompr.c \
#        ../../../cr3/thirdparty/zlib/deflate.c \
#        ../../../cr3/thirdparty/zlib/trees.c \
#        ../../../cr3/thirdparty/zlib/zutil.c \
#        ../../../cr3/thirdparty/zlib/inflate.c \
#        ../../../cr3/thirdparty/zlib/infback.c \
#        ../../../cr3/thirdparty/zlib/inftrees.c \
#        ../../../cr3/thirdparty/zlib/inffast.c
}

SOURCES += \
    ../../../cr3/thirdparty/chmlib/src/lzx.c \
    ../../../cr3/thirdparty/chmlib/src/chm_lib.c

HEADERS += \
    ../../../cr3/thirdparty/chmlib/src/chm_lib.h \
    ../../../cr3/thirdparty/chmlib/src/lzx.h

INCLUDEPATH += \
    ../../../cr3/thirdparty/chmlib/src

SOURCES += \
    ../../../cr3/thirdparty/antiword/xmalloc.c \
    ../../../cr3/thirdparty/antiword/wordwin.c \
    ../../../cr3/thirdparty/antiword/wordole.c \
    ../../../cr3/thirdparty/antiword/wordmac.c \
    ../../../cr3/thirdparty/antiword/wordlib.c \
    ../../../cr3/thirdparty/antiword/worddos.c \
    ../../../cr3/thirdparty/antiword/word2text.c \
    ../../../cr3/thirdparty/antiword/utf8.c \
    ../../../cr3/thirdparty/antiword/unix.c \
    ../../../cr3/thirdparty/antiword/tabstop.c \
    ../../../cr3/thirdparty/antiword/summary.c \
    ../../../cr3/thirdparty/antiword/stylesheet.c \
    ../../../cr3/thirdparty/antiword/stylelist.c \
    ../../../cr3/thirdparty/antiword/sectlist.c \
    ../../../cr3/thirdparty/antiword/rowlist.c \
    ../../../cr3/thirdparty/antiword/propmod.c \
    ../../../cr3/thirdparty/antiword/properties.c \
    ../../../cr3/thirdparty/antiword/prop8.c \
    ../../../cr3/thirdparty/antiword/prop6.c \
    ../../../cr3/thirdparty/antiword/prop2.c \
    ../../../cr3/thirdparty/antiword/prop0.c \
    ../../../cr3/thirdparty/antiword/pictlist.c \
    ../../../cr3/thirdparty/antiword/pdf.c \
    ../../../cr3/thirdparty/antiword/out2window.c \
    ../../../cr3/thirdparty/antiword/options.c \
    ../../../cr3/thirdparty/antiword/notes.c \
    ../../../cr3/thirdparty/antiword/misc.c \
    ../../../cr3/thirdparty/antiword/listlist.c \
    ../../../cr3/thirdparty/antiword/imgexam.c \
    ../../../cr3/thirdparty/antiword/hdrftrlist.c \
    ../../../cr3/thirdparty/antiword/fonts_u.c \
    ../../../cr3/thirdparty/antiword/fonts.c \
    ../../../cr3/thirdparty/antiword/fontlist.c \
    ../../../cr3/thirdparty/antiword/findtext.c \
    ../../../cr3/thirdparty/antiword/finddata.c \
    ../../../cr3/thirdparty/antiword/fail.c \
    ../../../cr3/thirdparty/antiword/doclist.c \
    ../../../cr3/thirdparty/antiword/depot.c \
    ../../../cr3/thirdparty/antiword/datalist.c \
    ../../../cr3/thirdparty/antiword/chartrans.c \
    ../../../cr3/thirdparty/antiword/blocklist.c \
    ../../../cr3/thirdparty/antiword/asc85enc.c

HEADERS += \
    ../../../cr3/thirdparty/antiword/antiword.h \
    ../../../cr3/thirdparty/antiword/version.h

INCLUDEPATH += \
    ../../../cr3/thirdparty/antiword

INCLUDEPATH += ../../../cr3/thirdparty/sqlite
SOURCES += \
    ../../../cr3/thirdparty/sqlite/sqlite3.c

HEADERS += \
    ../../../cr3/thirdparty/sqlite/sqlite3.h \
    ../../../cr3/thirdparty/sqlite/sqlite3ext.h

INCLUDEPATH += ../../cr3db/include \
    ../../cr3gl/include \
    ../../cr3ui/include

SOURCES += \
    ../../cr3db/src/basedb.cpp \
    ../../cr3db/src/cr3db.cpp \
    ../../cr3db/src/fileinfo.cpp \
    ../../cr3gl/src/gldrawbuf.cpp \
    ../../cr3gl/src/glfont.cpp \
    ../../cr3gl/src/glscene.cpp \
    ../../cr3gl/src/glwrapper.cpp \
    ../../cr3ui/src/cruimain.cpp \
    ../../cr3ui/src/crui.cpp \
    ../../cr3ui/src/cruicontrols.cpp \
    ../../cr3ui/src/cruievent.cpp \
    ../../cr3ui/src/cruifolderwidget.cpp \
    ../../cr3ui/src/cruihomewidget.cpp \
    ../../cr3ui/src/cruilayout.cpp \
    ../../cr3ui/src/cruilist.cpp \
    ../../cr3ui/src/cruiscrollwidget.cpp \
    ../../cr3ui/src/cruireadwidget.cpp \
    ../../cr3ui/src/cruitheme.cpp \
    ../../cr3ui/src/cruiwidget.cpp \
    ../../cr3ui/src/cruipopup.cpp \
    ../../cr3ui/src/crcoverpages.cpp \
    ../../cr3ui/src/cruiconfig.cpp \
    ../../cr3ui/src/cruiaction.cpp \
    ../../cr3ui/src/cruiwindow.cpp \
    ../../cr3ui/src/cruicoverwidget.cpp \
    ../../cr3ui/src/cruisettingswidget.cpp \
    ../../cr3ui/src/vkeyboard.cpp \
    ../../cr3ui/src/opdsbrowser.cpp \
    ../../cr3ui/src/cruiopdsprops.cpp \
    ../../cr3ui/src/cruiopdsbook.cpp \
    ../../cr3ui/src/stringresource.cpp

HEADERS += \
    ../../cr3db/include/basedb.h \
    ../../cr3db/include/cr3db.h \
    ../../cr3db/include/fileinfo.h \
    ../../cr3gl/include/gldrawbuf.h \
    ../../cr3gl/include/glfont.h \
    ../../cr3gl/include/glscene.h \
    ../../cr3gl/include/glwrapper.h \
    ../../cr3ui/include/cruimain.h \
    ../../cr3ui/include/crui.h \
    ../../cr3ui/include/cruicontrols.h \
    ../../cr3ui/include/cruievent.h \
    ../../cr3ui/include/cruifolderwidget.h \
    ../../cr3ui/include/cruihomewidget.h \
    ../../cr3ui/include/cruilayout.h \
    ../../cr3ui/include/cruilist.h \
    ../../cr3ui/include/cruiscrollwidget.h \
    ../../cr3ui/include/cruireadwidget.h \
    ../../cr3ui/include/cruitheme.h \
    ../../cr3ui/include/cruiwidget.h \
    ../../cr3ui/include/cruipopup.h \
    ../../cr3ui/include/crcoverpages.h \
    ../../cr3ui/include/cruiconfig.h \
    ../../cr3ui/include/cruiaction.h \
    ../../cr3ui/include/cruiactiondef.h \
    ../../cr3ui/include/cruiwindow.h \
    ../../cr3ui/include/cruisettingswidget.h \
    ../../cr3ui/include/cruicoverwidget.h \
    ../../cr3ui/include/vkeyboard.h \
    ../../cr3ui/include/opdsbrowser.h \
    ../../cr3ui/include/cruiopdsprops.h \
    ../../cr3ui/include/cruiopdsbook.h \
    ../../cr3ui/include/cruisettings.h \
    ../../cr3ui/include/stringresource.h
