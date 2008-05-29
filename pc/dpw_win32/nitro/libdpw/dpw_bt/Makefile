#! make -f
#----------------------------------------------------------------------------
# Project:  DP WiFi Library
# File:     Makefile
#
# Copyright 2003-2006 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------


#----------------------------------------------------------------------------

SRCS		=	dpw_bt.c
TARGET_LIB	=	libdpwbt$(NITRO_LIBSUFFIX).a

#SRCDIR		=	# using default
#LCFILE		=	# using default

# DPW ‹¤’Ê ƒ‰ƒCƒuƒ‰ƒŠ
DPW_COMMON_DIR   =  ../../libdpw/common

LINCLUDES		+= $(DPW_COMMON_DIR)/include
LLIBRARY_DIRS	+= $(DPW_COMMON_DIR)/lib/$(NITRO_BUILDTYPE)
LLIBRARIES		+= libdpwcommon$(NITRO_LIBSUFFIX).a


include	$(NITRODWC_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(NITRODWC_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
