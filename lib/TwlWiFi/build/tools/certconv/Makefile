#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWifi - tools - certconv
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
TARGET_PLATFORM	= TWL NITRO

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

TARGETS			= certconv.exe

OBJECTS			= make_cacert.o

INSTALL_DIR     = $(TWIFI_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	= $(TARGETS)

LDIRT_CLEAN		= $(OBJECTS) $(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules.x86
WARNING =

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

make_cacert.o:		make_cacert.c
