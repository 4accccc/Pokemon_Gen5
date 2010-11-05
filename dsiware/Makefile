#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos.TWL - nandApp - simple
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-26#$
# $Rev: 8694 $
# $Author: hatamoto_minoru $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#メイン側のバージョン定義を取り込む
include	../prog/version

#PARENT_DIR	= ../prog
#PARENT_SRC	= $(PARENT_DIR)/src
#PARENT_INC	= $(PARENT_DIR)/include
#PARENT_ARC	= $(PARENT_DIR)/arc


LINCLUDES		+= ./include	\
							 ./arc \

# GF-Library
GFLIB_DIR		=	../lib/gflib
LINCLUDES		+= 	$(GFLIB_DIR)/include
LLIBRARY_DIRS	=	$(GFLIB_DIR)/install/$(NITRO_BUILDTYPE)
LLIBRARIES		=  	gflib.a

# NITROSDK3.0付属ののプリコンパイルを読み込まない
# これを定義しないと独自プリコンパイルを行えない
NITRO_NO_STD_PCHDR = True


#(PM_DEBUG == yes)のときのみデバッグバージョン
ifeq	($(PM_DEBUG),yes)
MACRO_FLAGS	+=	-DPM_DEBUG
endif

ifdef	PM_VERSION
MACRO_FLAGS	+=	-DPM_VERSION=$(PM_VERSION)
endif

ifdef	PM_LANG
MACRO_FLAGS	+=	-DPM_LANG=$(PM_LANG)
endif


CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps
#----------------------------------------------------------------------------
SUBDIRS			 = banner \
							 resource \
							 arc \
					#$(ROOT)/build/demos/gx/UnitTours/DEMOLib

#-------------------------
#-- NAND アプリのビルドを指定します。

TWL_NANDAPP		=	TRUE


#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED

SRCS			=	main.c \
		gfl_use.c \
		machine_use.c \
		net_dev.c \
		net_icondata.c \


#-include	make_prog_files

# 最終的に生成されるファイル
TARGET_BIN		 =	nandAppSample.tad


#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。

ROM_SPEC			=	dsiware.rsf


#-------------------------
#-- カードアプリと同様に ROM-FS にファイルを持たせます。

MAKEROM_ROMROOT	 = ./rom_data
#MAKEROM_ROMROOT	 = ./
MAKEROM_ROMFILES = *.*  $(subst rom_data/,,$(wildcard rom_data/a/*/*/*))


#-------------------------
# rom 内のファイルは全て自動生成なので clean で削除されるようにします
LDIRT_CLEAN		 =	$(MAKEROM_ROMROOT)


include $(TWLSYSTEM_ROOT)/build/buildtools/commondefs
include $(TWLSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(TWLSYSTEM_ROOT)/build/buildtools/modulerules
#include	$(TWLSDK_ROOT)/build/buildtools/commondefs
#include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

.PHONY:	build_time

do-build: build_time precompile		$(TARGETS)


#-------------------------
#-- SRL を作成する前に build_time.txt が作成されるようにします。

$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	build_time


#-------------------------
#-- rom_data/build_time.txt を作成します。

build_time:
	$(INSTALL) -d $(MAKEROM_ROMROOT)
	echo -n "build at `date \"+%Y/%m/%d %H:%M:%S\"` on \"`hostname`\"" > $(MAKEROM_ROMROOT)/build_time.txt


#include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------
# precompile
#----------------------------------------------------------------------------
#PRECOMPILE_SRC		 =	../prog/include/precompile/precompile.pch
PRECOMPILE_SRC		 =	include/precompile/precompile.pch
PRECOMPILE_CC_MCH	 =	$(OBJDIR)/precompile_cc.mch
PRECOMPILE_CCFLAGS	:=	$(CCFLAGS)
CCFLAGS			+=	-prefix $(PRECOMPILE_CC_MCH)

precompile: $(PRECOMPILE_CC_MCH)

#バージョン定義ファイルが更新されたらプリコンパイルヘッダも更新する
$(PRECOMPILE_CC_MCH):	$(VER_FILE)

#-include $(PRECOMPILE_CC_MCH).d
$(PRECOMPILE_CC_MCH):%_cc.mch:
		$(CC) $(PRECOMPILE_CCFLAGS) $(INCLUDES) $(PRECOMPILE_SRC) -MD -precompile $(PRECOMPILE_CC_MCH)
##		$(MOVE_SRC_DEPEND)
##		$(MV) $(DEPENDDIR)/$(*F).d $@.d
$(OBJS):  $(PRECOMPILE_CC_MCH)

info:
	@echo prefix:$(PRECOMPILE_CC_MCH)
	@echo pchdr_ccflag:$(PCHDR_CCFLAGS)
	@echo use_std_pchdr:$(USE_STD_PCHDR)
	@echo twl_std_pchdr:$(TWL_STD_PCHDR)
	@echo twl_no_std_pchdr:$(TWL_NO_STD_PCHDR)
#===== End of Makefile =====
