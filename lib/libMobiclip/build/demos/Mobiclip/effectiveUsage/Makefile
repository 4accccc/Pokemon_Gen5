#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK -     -
# File:     Makefile
#
# Copyright 2003 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-02-26#$
# $Rev: 4346 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TWL_LIBMOBICLIP_ROOT ?= C:/libMobiclip

SUBDIRS	=

#----------------------------------------------------------------------------
TARGET_PLATFORM	=	TWL NITRO
TWL_ARCHGEN		=	ALL

SRCS			=	main.cpp
TARGET_BIN		=	effectiveUsage.srl

LINCLUDES		+=	$(TWL_LIBMOBICLIP_ROOT)/include \
					$(TWL_LIBMOBICLIP_ROOT)/include/nitro
LLIBRARY_DIRS	+=	$(TWL_LIBMOBICLIP_ROOT)/lib/$(TWL_LIBTYPE)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MAKEROM_ROMROOT	=	../data

ifeq ($(TARGET_PLATFORM),TWL)
LLIBRARIES		+=	libMobiclip.TWL$(ARCHGEN_TYPE).a
ROM_SPEC        = effectiveUsage.TWL.rsf
LCFILE_SPEC		= effectiveUsage.TWL.lsf
else
LLIBRARIES		+=	libMobiclip.a
LCFILE_SPEC		= effectiveUsage.Nitro.lsf
ROM_SPEC        = effectiveUsage.Nitro.rsf
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
