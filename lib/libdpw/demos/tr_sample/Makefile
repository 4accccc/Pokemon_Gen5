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

SRCS		=	main.c init.c userdata.c

TARGET_BIN	=	tr_sample.srl

#SRCDIR		=	# using default
#LCFILE		=	# using default

# デバッグスクリーンライブラリ
LINCLUDES     = ../dbs/include
LLIBRARIES    = libdwcdbs$(NITRO_LIBSUFFIX).a
LLIBRARY_DIRS = ../dbs/lib/$(NITRO_BUILDTYPE)

# common ヘッダ
DPW_COMMON_DIR   =  ../../common
LINCLUDES		+= $(DPW_COMMON_DIR)/include

# DPW Tr ライブラリ
DPW_TR_DIR   =  ../../dpw_tr

LINCLUDES		+= $(DPW_TR_DIR)/include
LLIBRARY_DIRS	+= $(DPW_TR_DIR)/lib/$(NITRO_BUILDTYPE)
LLIBRARIES		+= libdpwtr$(NITRO_LIBSUFFIX).a


NITRODWC_LIB_MODULES_   = ac auth bm base util account
NITRODWC_LIB_GSMODULES_ = common ghttp gp gstats gt2 natneg qr2 serverbrowsing

include	$(NITRODWC_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(NITRODWC_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
