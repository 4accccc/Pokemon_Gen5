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

# GF-Library
GFLIB_DIR		=	../gflib
LINCLUDES		= 	$(GFLIB_DIR)/include
LLIBRARY_DIRS		=	$(GFLIB_DIR)/install/$(NITRO_BUILDTYPE)
LLIBRARIES		=  	gflib.a


# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
TARGET_BIN		=	main.bin

# ソースファイル
SRCS			=	main.c

# コンパイル時の警告抑止
CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps


#--------------------------------------- ----------------------------
# システム定義
#
include $(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include $(NITROSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(NITROSYSTEM_ROOT)/build/buildtools/modulerules


#--------------------------------------- ----------------------------
do-build:		$(TARGETS)

#-----
