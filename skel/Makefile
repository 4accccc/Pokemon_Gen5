#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment 'skelton'
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#
# $LastChangedDate$
# $Id$
#
#----------------------------------------------------------------------------

.PHONY : gflib

# GF-Library
GFLIB_DIR		=	../gflib
LINCLUDES		= 	$(GFLIB_DIR)/include
LLIBRARY_DIRS	=	$(GFLIB_DIR)/install/$(NITRO_BUILDTYPE)
LLIBRARIES		=  	gflib.a


#イクニューモンコンポーネントのためのライブラリを追加 by Mori 2005.7.27
LLIBRARY_DIRS	+= ./library/spl/$(NITRO_BUILDTYPE)
LLIBRARIES		+= ./libjn_spl.a libwvr$(CODEGEN).a

# 個人テスト用定義
#ifeq	($(PM_DEBUG),yes)
MACRO_FLAGS += -DDEBUG_ONLY_FOR_$(USERNAME)
#endif

# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
TARGET_BIN		=	main.bin

# ソースファイル
SRCS			=	main.c gfl_use.c

include make_prog_files

## xyzzyでエラージャンプを行う
ifeq	($(USERNAME),ohno)
CCFLAGS_MSGSTYLE	?= -msgstyle gcc
endif

# コンパイル時の警告抑止
CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps

#------------------------------------------------------#
# イクニューモンコンポーネントを追加 by Mori 2005.7.27
# VRAM内に無線ライブラリが配置されるコンポーネント定義 #
MY_COMPONENT_NAME	= ichneumon
MAKEROM_ARM7	= $(NITRO_COMPONENTSDIR)/$(MY_COMPONENT_NAME)/$(NITRO_BUILDTYPE_ARM7)/$(MY_COMPONENT_NAME)_sub$(TS_VERSION).nef

#----------------------------------------------------------------------------
#	ロムに含まれるファイルの指定
#----------------------------------------------------------------------------
GFLIBGRADIR	= src/gfl_graphic/
SAMPLE_GRADIR = src/sample_graphic/

FILE_TREE	= $(GFLIBGRADIR)gfl_font.dat \
			  $(SAMPLE_GRADIR)*.narc \
			  $(SAMPLE_GRADIR)*.NCLR \
			  $(SAMPLE_GRADIR)*.NCGR \
			  $(SAMPLE_GRADIR)*.NCER \
			  $(SAMPLE_GRADIR)*.NANR
			  
#ロムファイルシステムのルートディレクトリ指定
MAKEROM_ROMROOT  = ./src/

MAKEROM_ROMFILES = $(subst src/,,$(FILE_TREE))


#--------------------------------------- ----------------------------
# システム定義
#
include $(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include $(NITROSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(NITROSYSTEM_ROOT)/build/buildtools/modulerules


#--------------------------------------- ----------------------------
do-build:		$(TARGETS)

#-----
#  gflibをmakeする
gflib:
	$(MAKE) -C $(GFLIB_DIR)


