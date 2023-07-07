#!/bin/env bash
g++ \
    -I.. -I../libswirl \
        main.cpp \
        ../libswirl/rend/soft/refrend_base.cpp \
        ../libswirl/rend/soft/refrend_debug.cpp \
        ../libswirl/rend/soft/refsw_pixel.cpp \
        ../libswirl/rend/soft/refsw.cpp \
        ../libswirl/hw/pvr/pvr_sb_regs.cpp \
        ../libswirl/oslib/posix/threading.cpp \
        ../libswirl/hw/pvr/pvr_mem.cpp \
        ../libswirl/rend/TexCache.cpp \
        ../libswirl/rend/gles/glestex.cpp \
        ../libswirl/deps/libpng/png.c \
        ../libswirl/deps/libpng/pngerror.c \
        ../libswirl/deps/libpng/pngget.c \
        ../libswirl/deps/libpng/pngmem.c \
        ../libswirl/deps/libpng/pngpread.c \
        ../libswirl/deps/libpng/pngread.c \
        ../libswirl/deps/libpng/pngrio.c \
        ../libswirl/deps/libpng/pngrtran.c \
        ../libswirl/deps/libpng/pngrutil.c \
        ../libswirl/deps/libpng/pngset.c \
        ../libswirl/deps/libpng/pngtrans.c \
        ../libswirl/deps/libpng/pngwio.c \
        ../libswirl/deps/libpng/pngwrite.c \
        ../libswirl/deps/libpng/pngwtran.c \
        ../libswirl/deps/libpng/pngwutil.c \
    -DTARGET_LINUX_x64 -DREFSW_OFFLINE -DFEAT_HAS_SOFTREND -DFEAT_TA=0x60000002 \
    -fopenmp -lz \
    -g -O0 \
    -o refsw-offline
