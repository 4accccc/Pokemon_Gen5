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

SUBDIRS			 = banner \
					$(ROOT)/build/demos/gx/UnitTours/DEMOLib

#-------------------------
#-- NAND アプリのビルドを指定します。

TWL_NANDAPP		=	TRUE


#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED

SRCS			 =	main.c
TARGET_BIN		 =	nandAppSample.tad


#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。

ROM_SPEC			=	demo.rsf


#-------------------------
#-- カードアプリと同様に ROM-FS にファイルを持たせます。

MAKEROM_ROMROOT	 = ./rom_data
MAKEROM_ROMFILES = *.*


#-------------------------
# rom 内のファイルは全て自動生成なので clean で削除されるようにします
LDIRT_CLEAN		 =	$(MAKEROM_ROMROOT)


include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

.PHONY:	build_time

do-build:		$(TARGETS)


#-------------------------
#-- SRL を作成する前に build_time.txt が作成されるようにします。

$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	build_time


#-------------------------
#-- rom_data/build_time.txt を作成します。

build_time:
	$(INSTALL) -d $(MAKEROM_ROMROOT)
	echo -n "build at `date \"+%Y/%m/%d %H:%M:%S\"` on \"`hostname`\"" > $(MAKEROM_ROMROOT)/build_time.txt


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
