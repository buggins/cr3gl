LOCAL_PATH := $(call my-dir)
#LOCAL_PATH := /projects/coolreader/cr3gp/cr3android/jni

include $(CLEAR_VARS)

LOCAL_MODULE    := crenginegl

CRFLAGS = -DLINUX=1 -D_LINUX=1 -DFOR_ANDROID=1 -DCR3_PATCH -DFT2_BUILD_LIBRARY=1 \
     -DDOC_DATA_COMPRESSION_LEVEL=1 -DDOC_BUFFER_SIZE=0xA00000 \
     -DENABLE_CACHE_FILE_CONTENTS_VALIDATION=1 \
     -DLDOM_USE_OWN_MEM_MAN=0 \
     -DCR3_ANTIWORD_PATCH=1 -DENABLE_ANTIWORD=1 \
     -DMAX_IMAGE_SCALE_MUL=2 \
     -DCR_EMULATE_GETTEXT=1 \
     -DGL_GLEXT_PROTOTYPES=1 \
     -DDEBUG

CR3_ROOT = $(LOCAL_PATH)/cr3

CRENGINE_INCLUDES := \
    -I$(LOCAL_PATH)/cr3/crengine/include \
    -I$(LOCAL_PATH)/cr3/thirdparty/libpng \
    -I$(LOCAL_PATH)/cr3/thirdparty/freetype/include \
    -I$(LOCAL_PATH)/cr3/thirdparty/libjpeg \
    -I$(LOCAL_PATH)/cr3/thirdparty/antiword \
    -I$(LOCAL_PATH)/cr3/thirdparty/chmlib/src \
    -I$(LOCAL_PATH)/cr3/thirdparty/sqlite \
    -I$(LOCAL_PATH)/cr3/cr3db/include \
    -I$(LOCAL_PATH)/cr3/cr3gl/include \
    -I$(LOCAL_PATH)/cr3/cr3ui/include
    
LOCAL_CFLAGS += $(CRFLAGS) $(CRENGINE_INCLUDES) -Wno-psabi -Wno-unused-variable -Wno-sign-compare -Wno-write-strings -Wno-main -Wno-unused-but-set-variable -Wno-unused-function -Werror -Wall


CRENGINE_SRC_FILES := \
    cr3/crengine/src/cri18n.cpp \
    cr3/crengine/src/crconcurrent.cpp \
    cr3/crengine/src/bookformats.cpp \
    cr3/crengine/src/cp_stats.cpp \
    cr3/crengine/src/lvstring.cpp \
    cr3/crengine/src/props.cpp \
    cr3/crengine/src/lstridmap.cpp \
    cr3/crengine/src/rtfimp.cpp \
    cr3/crengine/src/lvmemman.cpp \
    cr3/crengine/src/lvstyles.cpp \
    cr3/crengine/src/crtxtenc.cpp \
    cr3/crengine/src/lvtinydom.cpp \
    cr3/crengine/src/lvstream.cpp \
    cr3/crengine/src/lvxml.cpp \
    cr3/crengine/src/chmfmt.cpp \
    cr3/crengine/src/epubfmt.cpp \
    cr3/crengine/src/pdbfmt.cpp \
    cr3/crengine/src/wordfmt.cpp \
    cr3/crengine/src/lvstsheet.cpp \
    cr3/crengine/src/txtselector.cpp \
    cr3/crengine/src/crtest.cpp \
    cr3/crengine/src/lvbmpbuf.cpp \
    cr3/crengine/src/lvfnt.cpp \
    cr3/crengine/src/hyphman.cpp \
    cr3/crengine/src/lvfntman.cpp \
    cr3/crengine/src/lvimg.cpp \
    cr3/crengine/src/crskin.cpp \
    cr3/crengine/src/lvdrawbuf.cpp \
    cr3/crengine/src/lvdocview.cpp \
    cr3/crengine/src/lvpagesplitter.cpp \
    cr3/crengine/src/lvtextfm.cpp \
    cr3/crengine/src/lvrend.cpp \
    cr3/crengine/src/wolutil.cpp \
    cr3/crengine/src/crgl.cpp \
    cr3/crengine/src/hist.cpp
#    cr3/crengine/src/cri18n.cpp
#    cr3/crengine/src/crgui.cpp \

PNG_SRC_FILES := \
    cr3/thirdparty/libpng/pngerror.c  \
    cr3/thirdparty/libpng/pngget.c  \
    cr3/thirdparty/libpng/pngpread.c \
    cr3/thirdparty/libpng/pngrio.c \
    cr3/thirdparty/libpng/pngrutil.c \
    cr3/thirdparty/libpng/pngvcrd.c \
    cr3/thirdparty/libpng/png.c \
    cr3/thirdparty/libpng/pngwrite.c \
    cr3/thirdparty/libpng/pngwutil.c \
    cr3/thirdparty/libpng/pnggccrd.c \
    cr3/thirdparty/libpng/pngmem.c \
    cr3/thirdparty/libpng/pngread.c \
    cr3/thirdparty/libpng/pngrtran.c \
    cr3/thirdparty/libpng/pngset.c \
    cr3/thirdparty/libpng/pngtrans.c \
    cr3/thirdparty/libpng/pngwio.c \
    cr3/thirdparty/libpng/pngwtran.c

JPEG_SRC_FILES := \
    cr3/thirdparty/libjpeg/jcapimin.c \
    cr3/thirdparty/libjpeg/jchuff.c \
    cr3/thirdparty/libjpeg/jcomapi.c \
    cr3/thirdparty/libjpeg/jctrans.c \
    cr3/thirdparty/libjpeg/jdcoefct.c \
    cr3/thirdparty/libjpeg/jdmainct.c \
    cr3/thirdparty/libjpeg/jdpostct.c \
    cr3/thirdparty/libjpeg/jfdctfst.c \
    cr3/thirdparty/libjpeg/jidctred.c \
    cr3/thirdparty/libjpeg/jutils.c \
    cr3/thirdparty/libjpeg/jcapistd.c \
    cr3/thirdparty/libjpeg/jcinit.c \
    cr3/thirdparty/libjpeg/jcparam.c \
    cr3/thirdparty/libjpeg/jdapimin.c \
    cr3/thirdparty/libjpeg/jdcolor.c \
    cr3/thirdparty/libjpeg/jdmarker.c \
    cr3/thirdparty/libjpeg/jdsample.c \
    cr3/thirdparty/libjpeg/jfdctint.c \
    cr3/thirdparty/libjpeg/jmemmgr.c \
    cr3/thirdparty/libjpeg/jccoefct.c \
    cr3/thirdparty/libjpeg/jcmainct.c \
    cr3/thirdparty/libjpeg/jcphuff.c \
    cr3/thirdparty/libjpeg/jdapistd.c \
    cr3/thirdparty/libjpeg/jddctmgr.c \
    cr3/thirdparty/libjpeg/jdmaster.c \
    cr3/thirdparty/libjpeg/jdtrans.c \
    cr3/thirdparty/libjpeg/jidctflt.c \
    cr3/thirdparty/libjpeg/jmemnobs.c \
    cr3/thirdparty/libjpeg/jccolor.c \
    cr3/thirdparty/libjpeg/jcmarker.c \
    cr3/thirdparty/libjpeg/jcprepct.c \
    cr3/thirdparty/libjpeg/jdatadst.c \
    cr3/thirdparty/libjpeg/jdhuff.c \
    cr3/thirdparty/libjpeg/jdmerge.c \
    cr3/thirdparty/libjpeg/jerror.c \
    cr3/thirdparty/libjpeg/jidctfst.c \
    cr3/thirdparty/libjpeg/jquant1.c \
    cr3/thirdparty/libjpeg/jcdctmgr.c \
    cr3/thirdparty/libjpeg/jcmaster.c \
    cr3/thirdparty/libjpeg/jcsample.c \
    cr3/thirdparty/libjpeg/jdatasrc.c \
    cr3/thirdparty/libjpeg/jdinput.c \
    cr3/thirdparty/libjpeg/jdphuff.c \
    cr3/thirdparty/libjpeg/jfdctflt.c \
    cr3/thirdparty/libjpeg/jidctint.c \
    cr3/thirdparty/libjpeg/jquant2.c

FREETYPE_SRC_FILES := \
    cr3/thirdparty/freetype/src/autofit/autofit.c \
    cr3/thirdparty/freetype/src/bdf/bdf.c \
    cr3/thirdparty/freetype/src/cff/cff.c \
    cr3/thirdparty/freetype/src/base/ftbase.c \
    cr3/thirdparty/freetype/src/base/ftbbox.c \
    cr3/thirdparty/freetype/src/base/ftbdf.c \
    cr3/thirdparty/freetype/src/base/ftbitmap.c \
    cr3/thirdparty/freetype/src/base/ftgasp.c \
    cr3/thirdparty/freetype/src/cache/ftcache.c \
    cr3/thirdparty/freetype/src/base/ftglyph.c \
    cr3/thirdparty/freetype/src/base/ftgxval.c \
    cr3/thirdparty/freetype/src/gzip/ftgzip.c \
    cr3/thirdparty/freetype/src/base/ftinit.c \
    cr3/thirdparty/freetype/src/lzw/ftlzw.c \
    cr3/thirdparty/freetype/src/base/ftmm.c \
    cr3/thirdparty/freetype/src/base/ftpatent.c \
    cr3/thirdparty/freetype/src/base/ftotval.c \
    cr3/thirdparty/freetype/src/base/ftpfr.c \
    cr3/thirdparty/freetype/src/base/ftstroke.c \
    cr3/thirdparty/freetype/src/base/ftsynth.c \
    cr3/thirdparty/freetype/src/base/ftsystem.c \
    cr3/thirdparty/freetype/src/base/fttype1.c \
    cr3/thirdparty/freetype/src/base/ftwinfnt.c \
    cr3/thirdparty/freetype/src/base/ftxf86.c \
    cr3/thirdparty/freetype/src/winfonts/winfnt.c \
    cr3/thirdparty/freetype/src/pcf/pcf.c \
    cr3/thirdparty/freetype/src/pfr/pfr.c \
    cr3/thirdparty/freetype/src/psaux/psaux.c \
    cr3/thirdparty/freetype/src/pshinter/pshinter.c \
    cr3/thirdparty/freetype/src/psnames/psmodule.c \
    cr3/thirdparty/freetype/src/raster/raster.c \
    cr3/thirdparty/freetype/src/sfnt/sfnt.c \
    cr3/thirdparty/freetype/src/smooth/smooth.c \
    cr3/thirdparty/freetype/src/truetype/truetype.c \
    cr3/thirdparty/freetype/src/type1/type1.c \
    cr3/thirdparty/freetype/src/cid/type1cid.c \
    cr3/thirdparty/freetype/src/type42/type42.c

CHM_SRC_FILES := \
    cr3/thirdparty/chmlib/src/chm_lib.c \
    cr3/thirdparty/chmlib/src/lzx.c 

ANTIWORD_SRC_FILES := \
    cr3/thirdparty/antiword/asc85enc.c \
    cr3/thirdparty/antiword/blocklist.c \
    cr3/thirdparty/antiword/chartrans.c \
    cr3/thirdparty/antiword/datalist.c \
    cr3/thirdparty/antiword/depot.c \
    cr3/thirdparty/antiword/doclist.c \
    cr3/thirdparty/antiword/fail.c \
    cr3/thirdparty/antiword/finddata.c \
    cr3/thirdparty/antiword/findtext.c \
    cr3/thirdparty/antiword/fontlist.c \
    cr3/thirdparty/antiword/fonts.c \
    cr3/thirdparty/antiword/fonts_u.c \
    cr3/thirdparty/antiword/hdrftrlist.c \
    cr3/thirdparty/antiword/imgexam.c \
    cr3/thirdparty/antiword/listlist.c \
    cr3/thirdparty/antiword/misc.c \
    cr3/thirdparty/antiword/notes.c \
    cr3/thirdparty/antiword/options.c \
    cr3/thirdparty/antiword/out2window.c \
    cr3/thirdparty/antiword/pdf.c \
    cr3/thirdparty/antiword/pictlist.c \
    cr3/thirdparty/antiword/prop0.c \
    cr3/thirdparty/antiword/prop2.c \
    cr3/thirdparty/antiword/prop6.c \
    cr3/thirdparty/antiword/prop8.c \
    cr3/thirdparty/antiword/properties.c \
    cr3/thirdparty/antiword/propmod.c \
    cr3/thirdparty/antiword/rowlist.c \
    cr3/thirdparty/antiword/sectlist.c \
    cr3/thirdparty/antiword/stylelist.c \
    cr3/thirdparty/antiword/stylesheet.c \
    cr3/thirdparty/antiword/summary.c \
    cr3/thirdparty/antiword/tabstop.c \
    cr3/thirdparty/antiword/unix.c \
    cr3/thirdparty/antiword/utf8.c \
    cr3/thirdparty/antiword/word2text.c \
    cr3/thirdparty/antiword/worddos.c \
    cr3/thirdparty/antiword/wordlib.c \
    cr3/thirdparty/antiword/wordmac.c \
    cr3/thirdparty/antiword/wordole.c \
    cr3/thirdparty/antiword/wordwin.c \
    cr3/thirdparty/antiword/xmalloc.c


SQLITE_FILES += \
    cr3/thirdparty/sqlite/sqlite3.c

NEWUI_FILES += \
    cr3/cr3db/src/basedb.cpp \
    cr3/cr3db/src/cr3db.cpp \
    cr3/cr3db/src/fileinfo.cpp \
    cr3/cr3gl/src/gldrawbuf.cpp \
    cr3/cr3gl/src/glfont.cpp \
    cr3/cr3gl/src/glscene.cpp \
    cr3/cr3ui/src/cruimain.cpp \
    cr3/cr3ui/src/crui.cpp \
    cr3/cr3ui/src/cruicontrols.cpp \
    cr3/cr3ui/src/cruievent.cpp \
    cr3/cr3ui/src/cruifolderwidget.cpp \
    cr3/cr3ui/src/cruihomewidget.cpp \
    cr3/cr3ui/src/cruilayout.cpp \
    cr3/cr3ui/src/cruilist.cpp \
    cr3/cr3ui/src/cruireadwidget.cpp \
    cr3/cr3ui/src/cruitheme.cpp \
    cr3/cr3ui/src/cruiwidget.cpp \
    cr3/cr3ui/src/cruipopup.cpp \
    cr3/cr3ui/src/crcoverpages.cpp \
    cr3/cr3ui/src/cruiconfig.cpp \
    cr3/cr3ui/src/stringresource.cpp

    
JNI_SRC_FILES := \
    crenginegl.cpp \
    cr3java.cpp

LOCAL_SRC_FILES := \
    $(JNI_SRC_FILES) \
    $(NEWUI_FILES) \
    $(SQLITE_FILES) \
    $(CRENGINE_SRC_FILES) \
    $(FREETYPE_SRC_FILES) \
    $(PNG_SRC_FILES) \
    $(JPEG_SRC_FILES) \
    $(CHM_SRC_FILES) \
    $(ANTIWORD_SRC_FILES)

LOCAL_LDLIBS    := -lm -llog -lz -lGLESv1_CM -ldl -Wl,-Map=cr3engine.map
#-ljnigraphics

include $(BUILD_SHARED_LIBRARY)
