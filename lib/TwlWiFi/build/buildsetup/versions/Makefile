#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - buildsetup - versions
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
#
# These coded insructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-10-20#$
# $Rev: 1071 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

override PARALLEL_BUILD=FALSE

TARGET_PLATFORM		= TWL NITRO

TWL_FORCE_BUILD		= True
include	$(TWLWIFI_ROOT)/build/buildtools/commondefs
-include $(SDK_VERINFO_CW_CC) $(SDK_VERINFO_CW_LD)

TARGETS				= version.h

TARGETS_MAKEFILE	= $(TWIFI_ROOT)/release/Makefile
INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR			= $(TWIFI_INSTALL_INCDIR)/nitroWiFi
LDIRT_CLEAN			= $(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

do-build:	$(TARGETS)

SDK_BUILDINFO_CW_CC	= $(shell echo $(SDK_CW_BUILD_NUMBER_CC) | sed -e 's/^0*\(.\)/\1/')
SDK_BUILDINFO_CW_LD	= $(shell echo $(SDK_CW_BUILD_NUMBER_LD) | sed -e 's/^0*\(.\)/\1/')

MAKEVER_FLAGS		= VERSIONFILE=$(TARGETS) \
					  SDK_BUILDINFO_CW_CC=$(SDK_BUILDINFO_CW_CC) \
					  SDK_BUILDINFO_CW_LD=$(SDK_BUILDINFO_CW_LD)

$(TARGETS):	$(TARGETS_MAKEFILE)
		@$(ECHO) SDK_BUILDINFO_CW_CC=$(SDK_BUILDINFO_CW_CC)
		@$(ECHO) SDK_BUILDINFO_CW_LD=$(SDK_BUILDINFO_CW_LD)
		@$(MAKE) -s -f $(TARGETS_MAKEFILE) $(MAKEVER_FLAGS) version > /dev/null

#===== End of Makefile =====
