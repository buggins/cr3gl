# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

MY_LOCAL_PATH := $(call my-dir)/../..

SAMPLE_PATH := ../../../../GamePlay/gameplay/src
#SAMPLE_PATH := $(call my-dir)/../../../../../GamePlay/gameplay/src
LIBPNG_PATH := ../../../../GamePlay/external-deps/libpng/lib/android/$(TARGET_ARCH)
ZLIB_PATH := ../../../../GamePlay/external-deps/zlib/lib/android/$(TARGET_ARCH)
#LUA_PATH := ../../../../GamePlay/external-deps/lua/lib/android/$(TARGET_ARCH_ABI)
#BULLET_PATH := ../../../../GamePlay/external-deps/bullet/lib/android/$(TARGET_ARCH_ABI)
#VORBIS_PATH := ../../../../GamePlay/external-deps/oggvorbis/lib/android/$(TARGET_ARCH_ABI)
#OPENAL_PATH := ../../../../GamePlay/external-deps/openal/lib/android/$(TARGET_ARCH_ABI)

CRFLAGS = -DLINUX=1 -D_LINUX=1 -DFOR_ANDROID=1 -DCR3_PATCH -DFT2_BUILD_LIBRARY=1 \
     -DDOC_DATA_COMPRESSION_LEVEL=1 -DDOC_BUFFER_SIZE=0xA00000 \
     -DENABLE_CACHE_FILE_CONTENTS_VALIDATION=1 \
     -DLDOM_USE_OWN_MEM_MAN=0 \
     -DCR3_ANTIWORD_PATCH=1 -DENABLE_ANTIWORD=1 \
     -DMAX_IMAGE_SCALE_MUL=2

CR3_ROOT = ../../crengine


# gameplay
LOCAL_PATH := $(MY_LOCAL_PATH)/../../../GamePlay/gameplay/android/obj/local/$(TARGET_ARCH_ABI)
include $(CLEAR_VARS)
LOCAL_MODULE    := libgameplay
LOCAL_SRC_FILES := libgameplay.a
include $(PREBUILT_STATIC_LIBRARY)



CRENGINE_SRC_FILES := \
    $(CR3_ROOT)/crengine/src/cp_stats.cpp \
    $(CR3_ROOT)/crengine/src/lvstring.cpp \
    $(CR3_ROOT)/crengine/src/props.cpp \
    $(CR3_ROOT)/crengine/src/lstridmap.cpp \
    $(CR3_ROOT)/crengine/src/rtfimp.cpp \
    $(CR3_ROOT)/crengine/src/lvmemman.cpp \
    $(CR3_ROOT)/crengine/src/lvstyles.cpp \
    $(CR3_ROOT)/crengine/src/crtxtenc.cpp \
    $(CR3_ROOT)/crengine/src/lvtinydom.cpp \
    $(CR3_ROOT)/crengine/src/lvstream.cpp \
    $(CR3_ROOT)/crengine/src/lvxml.cpp \
    $(CR3_ROOT)/crengine/src/chmfmt.cpp \
    $(CR3_ROOT)/crengine/src/epubfmt.cpp \
    $(CR3_ROOT)/crengine/src/pdbfmt.cpp \
    $(CR3_ROOT)/crengine/src/wordfmt.cpp \
    $(CR3_ROOT)/crengine/src/lvstsheet.cpp \
    $(CR3_ROOT)/crengine/src/txtselector.cpp \
    $(CR3_ROOT)/crengine/src/crtest.cpp \
    $(CR3_ROOT)/crengine/src/lvbmpbuf.cpp \
    $(CR3_ROOT)/crengine/src/lvfnt.cpp \
    $(CR3_ROOT)/crengine/src/hyphman.cpp \
    $(CR3_ROOT)/crengine/src/lvfntman.cpp \
    $(CR3_ROOT)/crengine/src/lvimg.cpp \
    $(CR3_ROOT)/crengine/src/crskin.cpp \
    $(CR3_ROOT)/crengine/src/lvdrawbuf.cpp \
    $(CR3_ROOT)/crengine/src/lvdocview.cpp \
    $(CR3_ROOT)/crengine/src/lvpagesplitter.cpp \
    $(CR3_ROOT)/crengine/src/lvtextfm.cpp \
    $(CR3_ROOT)/crengine/src/lvrend.cpp \
    $(CR3_ROOT)/crengine/src/wolutil.cpp \
    $(CR3_ROOT)/crengine/src/hist.cpp
#    $(CR3_ROOT)/crengine/src/cri18n.cpp
#    $(CR3_ROOT)/crengine/src/crgui.cpp \

PNG_SRC_FILES := \
    $(CR3_ROOT)/thirdparty/libpng/pngerror.c  \
    $(CR3_ROOT)/thirdparty/libpng/pngget.c  \
    $(CR3_ROOT)/thirdparty/libpng/pngpread.c \
    $(CR3_ROOT)/thirdparty/libpng/pngrio.c \
    $(CR3_ROOT)/thirdparty/libpng/pngrutil.c \
    $(CR3_ROOT)/thirdparty/libpng/pngvcrd.c \
    $(CR3_ROOT)/thirdparty/libpng/png.c \
    $(CR3_ROOT)/thirdparty/libpng/pngwrite.c \
    $(CR3_ROOT)/thirdparty/libpng/pngwutil.c \
    $(CR3_ROOT)/thirdparty/libpng/pnggccrd.c \
    $(CR3_ROOT)/thirdparty/libpng/pngmem.c \
    $(CR3_ROOT)/thirdparty/libpng/pngread.c \
    $(CR3_ROOT)/thirdparty/libpng/pngrtran.c \
    $(CR3_ROOT)/thirdparty/libpng/pngset.c \
    $(CR3_ROOT)/thirdparty/libpng/pngtrans.c \
    $(CR3_ROOT)/thirdparty/libpng/pngwio.c \
    $(CR3_ROOT)/thirdparty/libpng/pngwtran.c

JPEG_SRC_FILES := \
    $(CR3_ROOT)/thirdparty/libjpeg/jcapimin.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jchuff.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcomapi.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jctrans.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdcoefct.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdmainct.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdpostct.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jfdctfst.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jidctred.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jutils.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcapistd.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcinit.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcparam.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdapimin.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdcolor.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdmarker.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdsample.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jfdctint.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jmemmgr.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jccoefct.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcmainct.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcphuff.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdapistd.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jddctmgr.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdmaster.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdtrans.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jidctflt.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jmemnobs.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jccolor.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcmarker.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcprepct.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdatadst.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdhuff.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdmerge.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jerror.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jidctfst.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jquant1.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcdctmgr.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcmaster.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jcsample.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdatasrc.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdinput.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jdphuff.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jfdctflt.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jidctint.c \
    $(CR3_ROOT)/thirdparty/libjpeg/jquant2.c

FREETYPE_SRC_FILES := \
    $(CR3_ROOT)/thirdparty/freetype/src/autofit/autofit.c \
    $(CR3_ROOT)/thirdparty/freetype/src/bdf/bdf.c \
    $(CR3_ROOT)/thirdparty/freetype/src/cff/cff.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftbase.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftbbox.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftbdf.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftbitmap.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftgasp.c \
    $(CR3_ROOT)/thirdparty/freetype/src/cache/ftcache.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftglyph.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftgxval.c \
    $(CR3_ROOT)/thirdparty/freetype/src/gzip/ftgzip.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftinit.c \
    $(CR3_ROOT)/thirdparty/freetype/src/lzw/ftlzw.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftmm.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftpatent.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftotval.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftpfr.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftstroke.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftsynth.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftsystem.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/fttype1.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftwinfnt.c \
    $(CR3_ROOT)/thirdparty/freetype/src/base/ftxf86.c \
    $(CR3_ROOT)/thirdparty/freetype/src/winfonts/winfnt.c \
    $(CR3_ROOT)/thirdparty/freetype/src/pcf/pcf.c \
    $(CR3_ROOT)/thirdparty/freetype/src/pfr/pfr.c \
    $(CR3_ROOT)/thirdparty/freetype/src/psaux/psaux.c \
    $(CR3_ROOT)/thirdparty/freetype/src/pshinter/pshinter.c \
    $(CR3_ROOT)/thirdparty/freetype/src/psnames/psmodule.c \
    $(CR3_ROOT)/thirdparty/freetype/src/raster/raster.c \
    $(CR3_ROOT)/thirdparty/freetype/src/sfnt/sfnt.c \
    $(CR3_ROOT)/thirdparty/freetype/src/smooth/smooth.c \
    $(CR3_ROOT)/thirdparty/freetype/src/truetype/truetype.c \
    $(CR3_ROOT)/thirdparty/freetype/src/type1/type1.c \
    $(CR3_ROOT)/thirdparty/freetype/src/cid/type1cid.c \
    $(CR3_ROOT)/thirdparty/freetype/src/type42/type42.c

CHM_SRC_FILES := \
    $(CR3_ROOT)/thirdparty/chmlib/src/chm_lib.c \
    $(CR3_ROOT)/thirdparty/chmlib/src/lzx.c 

ANTIWORD_SRC_FILES := \
    $(CR3_ROOT)/thirdparty/antiword/asc85enc.c \
    $(CR3_ROOT)/thirdparty/antiword/blocklist.c \
    $(CR3_ROOT)/thirdparty/antiword/chartrans.c \
    $(CR3_ROOT)/thirdparty/antiword/datalist.c \
    $(CR3_ROOT)/thirdparty/antiword/depot.c \
    $(CR3_ROOT)/thirdparty/antiword/doclist.c \
    $(CR3_ROOT)/thirdparty/antiword/fail.c \
    $(CR3_ROOT)/thirdparty/antiword/finddata.c \
    $(CR3_ROOT)/thirdparty/antiword/findtext.c \
    $(CR3_ROOT)/thirdparty/antiword/fontlist.c \
    $(CR3_ROOT)/thirdparty/antiword/fonts.c \
    $(CR3_ROOT)/thirdparty/antiword/fonts_u.c \
    $(CR3_ROOT)/thirdparty/antiword/hdrftrlist.c \
    $(CR3_ROOT)/thirdparty/antiword/imgexam.c \
    $(CR3_ROOT)/thirdparty/antiword/listlist.c \
    $(CR3_ROOT)/thirdparty/antiword/misc.c \
    $(CR3_ROOT)/thirdparty/antiword/notes.c \
    $(CR3_ROOT)/thirdparty/antiword/options.c \
    $(CR3_ROOT)/thirdparty/antiword/out2window.c \
    $(CR3_ROOT)/thirdparty/antiword/pdf.c \
    $(CR3_ROOT)/thirdparty/antiword/pictlist.c \
    $(CR3_ROOT)/thirdparty/antiword/prop0.c \
    $(CR3_ROOT)/thirdparty/antiword/prop2.c \
    $(CR3_ROOT)/thirdparty/antiword/prop6.c \
    $(CR3_ROOT)/thirdparty/antiword/prop8.c \
    $(CR3_ROOT)/thirdparty/antiword/properties.c \
    $(CR3_ROOT)/thirdparty/antiword/propmod.c \
    $(CR3_ROOT)/thirdparty/antiword/rowlist.c \
    $(CR3_ROOT)/thirdparty/antiword/sectlist.c \
    $(CR3_ROOT)/thirdparty/antiword/stylelist.c \
    $(CR3_ROOT)/thirdparty/antiword/stylesheet.c \
    $(CR3_ROOT)/thirdparty/antiword/summary.c \
    $(CR3_ROOT)/thirdparty/antiword/tabstop.c \
    $(CR3_ROOT)/thirdparty/antiword/unix.c \
    $(CR3_ROOT)/thirdparty/antiword/utf8.c \
    $(CR3_ROOT)/thirdparty/antiword/word2text.c \
    $(CR3_ROOT)/thirdparty/antiword/worddos.c \
    $(CR3_ROOT)/thirdparty/antiword/wordlib.c \
    $(CR3_ROOT)/thirdparty/antiword/wordmac.c \
    $(CR3_ROOT)/thirdparty/antiword/wordole.c \
    $(CR3_ROOT)/thirdparty/antiword/wordwin.c \
    $(CR3_ROOT)/thirdparty/antiword/xmalloc.c

# cr3gp
LOCAL_PATH := $(SAMPLE_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE    := cr3gp
LOCAL_SRC_FILES := 

LOCAL_PATH := $(MY_LOCAL_PATH)
CR3GP_SRC_FILES := \
    src/CR3Main.cpp \
    ../../../GamePlay/gameplay/src/gameplay-main-android.cpp 

LOCAL_C_INCLUDES := \
    -I ../../../crengine/thirdparty/antiword \
    -I ../../../crengine/include \
    -I ../../../crengine/thirdparty/libpng \
    -I ../../../crengine/thirdparty/freetype/include \
    -I ../../../crengine/thirdparty/libjpeg \
    -I ../../../crengine/thirdparty/antiword \
    -I ../../../crengine/thirdparty/chmlib/src

LOCAL_SRC_FILES := \
    $(CR3GP_SRC_FILES) \
    $(CRENGINE_SRC_FILES) \
    $(FREETYPE_SRC_FILES) \
    $(PNG_SRC_FILES) \
    $(JPEG_SRC_FILES) \
    $(CHM_SRC_FILES) \
    $(ANTIWORD_SRC_FILES)



LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lOpenSLES -lz
LOCAL_CFLAGS    := $(CRFLAGS) $(CRENGINE_INCLUDES) -Wno-strict-overflow -Wno-unused-variable -Wno-sign-compare -Wno-write-strings -Wno-main -Wno-unused-but-set-variable -Wno-unused-function -Werror -Wall
LOCAL_CFLAGS    += -D__ANDROID__ -Wno-psabi -I"../../../../GamePlay/gameplay/src" $(CRFLAGS)
#-I"../../../../../GamePlay/external-deps/lua/include" -I"../../../../GamePlay/external-deps/bullet/include" 
#-I"../../../../GamePlay/external-deps/libpng/include" 
#-I"../../../../GamePlay/external-deps/oggvorbis/include" 
#-I"../../../../GamePlay/external-deps/openal/include" 
#-I"../../../../GamePlay/gameplay/src"

LOCAL_STATIC_LIBRARIES := android_native_app_glue libgameplay

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)
