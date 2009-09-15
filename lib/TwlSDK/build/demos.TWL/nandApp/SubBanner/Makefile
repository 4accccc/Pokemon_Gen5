#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos.TWL - nandApp - SubBanner
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
# $Date:: $
# $Rev: 8703 $
# $Author: ooshimay $
#----------------------------------------------------------------------------

SUBDIRS					=	banner

#-------------------------
#-- NAND アプリのビルドを指定します。

TWL_NANDAPP		=	TRUE

#----------------------------------------------------------------------------

TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

SRCS					=	main.c
TARGET_BIN				=	SubBanner.tad


#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。

ROM_SPEC			=	demo.rsf

MAKEROM_ROMROOT		=	./banner
MAKEROM_ROMFILES	=	sub_banner.bnr

include $(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
