#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - demos - netconnect
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
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

#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL NITRO
TWL_ARCHGEN			= ALL

SRCS		 = netconnect.c

# for WCM control
SRCS		+= wcm_control.c

SITEDEFS	?= $(shell \
			if [ -r $(SRCDIR)/sitedefs_custom.c ];then \
				echo sitedefs_custom.c;\
			else \
				echo sitedefs.c;\
			fi)
SRCS		+= $(SITEDEFS)

TARGET_LIB	?= libnetconnect$(TWIFI_LIBSUFFIX_EX).a

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs
#----------------------------------------------------------------------------

INSTALL_TARGETS	= $(TARGETS)
INSTALL_DIR		= ./lib/$(TWIFI_LIBTYPE)

do-build:	$(TARGETS)

ifeq	($(TWL_AUTOTEST),TRUE)

ifdef TWLWIFI_WORKING_ROOT
SITEDEFS_PRESET		= $(call eupath,$(TWLWIFI_WORKING_ROOT)/TwlWiFi/build/demos/netconnect/src/_sitedefs_custom.c)
else
SITEDEFS_PRESET		= $(SRCDIR)/_sitedefs_custom.c
endif

SITEDEFS_OVERRIDE	= $(SRCDIR)/sitedefs_custom.c
LDIRT_CLEAN		+= $(SITEDEFS_OVERRIDE)
build:	$(SITEDEFS_OVERRIDE)

$(SITEDEFS_OVERRIDE): $(SITEDEFS_PRESET)
	$(CP) $< $@

endif

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
