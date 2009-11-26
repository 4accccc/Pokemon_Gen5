#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWifi - demos
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
# $Date:: 2009-06-22#$
# $Rev: 1186 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL NITRO

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS		 = 
SUBDIRS_P	 = 

ifeq	($(TWLWIFI_SOCKET_PRIVATE),TRUE)
SUBDIRS		+= netconnect soc/shared
SUBDIRS_P	+= wcm soc

ifeq	($(TWLWIFI_NCFG_PRIVATE),TRUE)
SUBDIRS_P	+= ncfg
endif
endif

ifneq	($(TWLWIFI_NOSSL),TRUE)
ifeq	($(TWLWIFI_SOCKET_PRIVATE),TRUE)
ifeq	($(TWLWIFI_NHTTP_PRIVATE),TRUE)
SUBDIRS_P	+= nhttp
endif
endif
ifeq	($(TWLWIFI_HIDDEN),TRUE)
SUBDIRS_P	+= cps ssl
endif
endif

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


