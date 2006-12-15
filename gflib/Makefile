#! make -f
#--------------------------------------- ----------------------------
# Project:  GAMEFREAK  Standard library.
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
# $LastChangedDate$
# $Id$
#
#--------------------------------------- ----------------------------
#

.PHONY : doc

# プリコンパイルをアプリケーション側へ開放するため、
# gflibではその準備のみを行う構造とします
#

# NITROSDK3.0付属ののプリコンパイルを読み込まない
NITRO_NO_STD_PCHDR	=	True

# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
TARGET_LIB		=	gflib.a

# ソースファイル
SRCS			=	initialize.c filesystem.c heap.c heapsys.c \
					tcb.c procsys.c saveload_system.c \
					ui/ui.c \
					ui/touchpanel.c \
					ui/key.c \
                    network/tool/net_ring_buff.c \
                    network/tool/net_queue.c \
                    network/tool/net_tool.c \
					network/device/wh.c \
					network/device/net_wireless.c \
					network/net_system.c \
					network/net_command.c \
					network/net.c \
					standard/gf_standard.c \
					calctool.c \
					bgl/bg_sys.c bgl/display.c \
					bitmap/bmp.c bitmap/bmp_win.c \
					string/strbuf.c string/msgdata.c string/systemfont.c \
					g3d/g3d_system.c \
					arc/arc_tool.c arc/arc_util.c \
					clact.c \
					string/textprint.c

# MCS(Multiple Channel Stream)を有効にする時はUSE_MCSを定義
ifdef USE_MCS
SRCS			+=	mcs.c
endif

# gflib.aがインストールされるディレクトリ
INSTALL_DIR		=	./install/$(NITRO_BUILDTYPE)/
INSTALL_TARGETS		=	$(LIBDIR)/$(TARGET_LIB)

# コンパイル時の警告抑止
CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps

## xyzzyでエラージャンプを行う
ifeq	($(USERNAME),ohno)
CCFLAGS_MSGSTYLE	?= -msgstyle gcc
endif

# make clean時に一緒に抹消したいファイル
LDIRT_CLEAN		=	$(INSTALL_DIR)

#--------------------------------------- ----------------------------
# システム定義
#
include $(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include $(NITROSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#--------------------------------------- ----------------------------
do-build:	$(TARGETS)

#-----
#  ドキュメントの確認を行う
doc:
	if ! [ -d "doc" ]; then \
		mkdir doc; \
	fi;
	doxygen ./doxygen.conf
