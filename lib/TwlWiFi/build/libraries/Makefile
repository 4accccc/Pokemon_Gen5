#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - libraries
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
# $Date:: 2009-06-08#$
# $Rev: 1183 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	 = TWL NITRO

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS_P		 = stubs

ifeq	($(TWLWIFI_SOCKET_PRIVATE),TRUE)
SUBDIRS_P	+= soc wcm

ifeq	($(TWLWIFI_NHTTP_PRIVATE),TRUE)
SUBDIRS_P	+= nhttp
endif

ifeq	($(TWLWIFI_NSSLSRC_PRIVATE),TRUE)
SUBDIRS_P		+= nssl
endif
endif

ifeq	($(TWLWIFI_HIDDEN),TRUE)
SUBDIRS_P		+= ip eftp
endif

ifeq	($(TWLWIFI_NCFG_PRIVATE),TRUE)
SUBDIRS_P		+= ncfg
endif

ifeq	($(TWLWIFI_SYSMENU_PRIVATE),TRUE)
ifeq	($(TWLWIFI_NSSL_PRIVATE),TRUE)
SUBDIRS_P		+= nuc
endif
endif

ifeq	($(TWLWIFI_CPS_PRIVATE),TRUE)
SUBDIRS_P		+= $(TWIFI_PRIVATE_ROOT)/build/libraries/cps 	\
                   $(TWIFI_PRIVATE_ROOT)/build/libraries/ssl
endif

ifeq	($(TWLWIFI_INET),TRUE)
SUBDIRS_P		:= nhttp eftp nssl
endif


#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
