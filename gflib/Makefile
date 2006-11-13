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
# プリコンパイルをアプリケーション側へ開放するため、
# gflibではその準備のみを行う構造とします
#

# NITROSDK3.0付属ののプリコンパイルを読み込まない
NITRO_NO_STD_PCHDR	=	True

# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
TARGET_LIB		=	gflib.a

# ソースファイル
SRCS			=	initialize.c filesystem.c heapsys.c tcb.c procsys.c \
					touchpanel.c

# MCS(Multiple Channel Stream)を有効にする時はUSE_MCSを定義
ifdef USE_MCS
SRCS			+=	mcs.c
endif

# gflib.aがインストールされるディレクトリ
INSTALL_DIR		=	./install/$(NITRO_BUILDTYPE)/
INSTALL_TARGETS		=	$(LIBDIR)/$(TARGET_LIB)

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
