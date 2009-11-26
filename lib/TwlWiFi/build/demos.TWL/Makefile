#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWifi - demos.TWL
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
# $Date:: 2008-12-02#$
# $Rev: 1102 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM	:=	TWL

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS		 = 
SUBDIRS_P	 = 

ifeq	($(TWLWIFI_SOCKET_PRIVATE),TRUE)
SUBDIRS		 = ../demos/netconnect ../demos/soc/shared

ifeq	($(TWLWIFI_NSSL_PRIVATE),TRUE)
ifneq	($(TWLWIFI_NOSSL),TRUE)
SUBDIRS_P	+= nssl
endif
endif
endif

ifeq	($(TWLWIFI_SYSMENU_PRIVATE),TRUE)
ifeq	($(TWLWIFI_NSSL_PRIVATE),TRUE)
SUBDIRS_P	+= nuc
endif
endif

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


