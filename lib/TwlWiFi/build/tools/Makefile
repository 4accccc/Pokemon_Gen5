#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWifi - tools
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

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS_P		= certconv

ifeq	($(TWLWIFI_HIDDEN),TRUE)
ifneq	($(TWLSYSTEM_ROOT),)
SUBDIRS_P		+= TwlNetUtil
endif
endif

ifeq	($(TWLWIFI_PRIVATE),TRUE)
SUBDIRS_P	+=  wifiindent
endif

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


