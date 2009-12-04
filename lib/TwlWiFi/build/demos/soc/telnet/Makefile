#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - SOC - demos - telnet
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
# $Date:: 2008-09-18#$
# $Rev: 1024 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL NITRO

TARGET_BIN	= main.srl
SRCS		= main.c font.c vt100.c console.c telnet.c softkbd.c

#SoftKeyboardライブラリがある場合は場所を指定する
#WITH_KBD	= d:/dev/release/SoftKeyboard/kbd

#----------------------------------------------------------------------------

#SoftKeyboardライブラリはTwlSystemが必要
ifdef WITH_KBD
include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs
else
include	$(TWLSDK_ROOT)/build/buildtools/commondefs
endif

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs.demos

ifdef WITH_KBD
LINCLUDES		+= $(WITH_KBD)/include
LLIBRARY_DIRS	+= $(WITH_KBD)/lib/$(NITRO_BUILDTYPE)
LLIBRARIES		+= libkbd$(NITRO_LIBSUFFIX).a
LMACRO_FLAGS	+= -DWITH_KBD
else
SRCS            += kbd_tiny.c
endif


do-build:	$(TARGETS)

ifdef WITH_KBD
include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules
else
include	$(TWLSDK_ROOT)/build/buildtools/modulerules
endif

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
