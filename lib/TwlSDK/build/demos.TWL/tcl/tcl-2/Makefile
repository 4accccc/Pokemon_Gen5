#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tcl - demos.TWL - tcl-2
# File:     Makefile
#
#  Copyright 2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 2008-10-07#$
#  $Rev: 8888 $
#  $Author: onozawa_yuki $
#----------------------------------------------------------------------------

SUBDIRS			 =	banner

#-------------------------
#-- NAND アプリのビルドを指定します。

TWL_NANDAPP		=	TRUE


#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED

SRCS			 =	main.c screen.c font.c
TARGET_BIN		 =	tcl_2.tad


MAKEROM_ROMROOT	 = ./data
MAKEROM_ROMFILES = *.bmp


#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。

ROM_SPEC			=	demo.rsf


include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib


#-------------------------
#-- commondefsにtclが正式追加されるまでの仮対応です。

TWL_LIBS_EX		+=	libtcl$(TWL_LIBSUFFIX).a


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
