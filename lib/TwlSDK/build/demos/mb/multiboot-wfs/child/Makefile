#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB
# File:     Makefile
#
# Copyright 2005-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------
# 子機として配信可能なのは NITRO もしくは TWL-HYBRID ビルドした srl
# MB ライブラリが配信するのは NITRO アプリですので、
# 配信先の本体が TWL であっても HYBRID ロムは NTR モードでブートします。
#
# 詳しくは、関数リファレンスの MB ライブラリの概要をご覧下さい。

TARGET_PLATFORM	:=	ALL
TWL_ARCHGEN		:=	HYBRID

include	../make_common


###############################################################################
# 子機固有の設定.

SRCS			+=	main.c child.c
# オーバーレイテスト用モジュール.
SRCS_OVERLAY	+=	overlay_test/func_1.c	\
					overlay_test/func_2.c	\
					overlay_test/func_3.cpp

MAKEROM_ROMROOT	 =	.
MAKEROM_ROMFILES =	./data

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
LCFILE_SPEC		=	main.TWL.lsf
else
LCFILE_SPEC		 =	main.lsf
endif

INSTALL_TARGETS	 =	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR		 =	../parent/data

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====

