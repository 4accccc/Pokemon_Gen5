#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - SOC - demos - httpclient
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
# $Date:: 2008-12-15#$
# $Rev: 1112 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL NITRO

# SOC_SHARED ルートの定義.
SOC_SHARED	=  $(TWIFI_ROOT)/build/demos/soc/shared

TARGET_BIN	= main.srl
SRCS		= main.c http.c ssl.c

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs.demos

# SOC デモ共通初期化ライブラリ.
ifeq ($(TARGET_PLATFORM_TWL),TRUE)
LLIBRARIES		+= libkeypad$(TWL_LIBSUFFIX).a
LLIBRARY_DIRS	+= ../shared/lib/$(TWL_BUILDTYPE)
else
LLIBRARIES		+= libkeypad$(NITRO_LIBSUFFIX).a
LLIBRARY_DIRS	+= ../shared/lib/$(NITRO_BUILDTYPE)
endif

LINCLUDES		+= $(SOC_SHARED)/include

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
