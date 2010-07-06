#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - demos.TWL
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-03-27#$
# $Rev: 1151 $
# $Author: adachi_hiroaki $
#----------------------------------------------------------------------------


ifneq ($(USE_NSSL),)
TARGET_PLATFORM		= TWL
TWL_ARCHGEN	= LIMITED
LLIBRARIES	= libnhttp.nssl$(TWIFI_LIBSUFFIX).a
else
TARGET_PLATFORM		= ALL
LLIBRARIES	= libnhttp$(TWIFI_LIBSUFFIX).a
endif

TARGET_BIN	= main.srl
SRCS		= main.c

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs.demos

do-build:	$(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
