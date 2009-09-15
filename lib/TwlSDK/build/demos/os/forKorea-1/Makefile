#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - OS - forKorea-1
# File:     Makefile
#
# Copyright 2006-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-10-30#$
# $Rev: 9161 $
# $Author: hatamoto_minoru $
#----------------------------------------------------------------------------
TARGET_PLATFORM	    = NITRO

SUBDIRS             =

#----------------------------------------------------------------------------

TARGET_BIN          = main.srl
SRCS                = main.c

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

ROM_SPEC            = forKorea.rsf

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
