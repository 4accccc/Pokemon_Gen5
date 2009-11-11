#! make -f
#----------------------------------------------------------------------------
# Project:  libVCT - build
# File:     Makefile
#
# Copyright (c) 2005-2006 Abiosso Networks, Inc. ALL RIGHTS RESERVED.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Abiosso Networks, Inc., and are protected
# by Federal copyright law.  They may not be disclosed to third parties
# or copied or duplicated in any form, in whole or in part,  without
# the prior written consent of Abiosso.
#
# April 17, 2009
# Supports Twl Libraries.
#
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------

SRCDIR  = src


SRCS    = \
	gamemain.c \
	logon.c \
	main.c \
	registerfriend.c \
	sound.c \
	transport.c \
	debug_menu.cpp \
	demo_util.cpp \
	thread_statistics.c \
	phone.cpp \
	transceiver.cpp \
	conference.cpp \
	font.c \
	init.c \
	mp.c \
	screen.c \
	utility.c \
	wc.c \
	demo_menu.cpp \
	vct_menu.cpp

EXT_OBJS           += ./data/sound_data.o
TARGET_BIN	=	main.srl

MAKEROM_CODEC_MODE = NTR
LDEPENDS_NEF = $(TWLDWC_LIB_DEPENDS)

TWLDWC_LIB_MODULES_   = auth ac bm util account common nonport nas match
TWLDWC_LIB_GSMODULES_ = common gp gstats gt2 natneg qr2 serverbrowsing

LINCLUDES	= ./include $(TWLDWC_ROOT)/build/demos/dbs/include
CCFLAGS_MSGSTYLE = -msgstyle gcc

ifeq ($(PR_USERDATA), TRUE)
MACRO_FLAGS	+=	-DUSE_PRIVATE_USERDATA
endif

ifdef TWL_PROFILE
TWL_PROFILE_TYPE  = FUNCTIONCOST
SRCS +=	cost.c
endif
#----------------------------------------------------------------------------
include	$(TWLLIBVCT_ROOT)/build/buildtools/commondefs

ifneq	($(TARGET_PLATFORM),TWL)
LLIBRARIES	   += libdwcdbs$(NITRO_LIBSUFFIX).a
LLIBRARY_DIRS	+= $(TWLDWC_ROOT)/build/demos/dbs/lib/$(NITRO_BUILDTYPE)
ROM_SPEC        = libvct_demo.rsf
LCFILE_SPEC     = libvct_demo.lsf
else
#LLIBRARIES	    = libdwcdbs.a
LLIBRARIES	    += libdwcdbs$(TWL_LIBSUFFIX).a
LLIBRARY_DIRS	+= $(TWLDWC_ROOT)/build/demos/dbs/lib/$(TWL_BUILDTYPE)
ROM_SPEC        = libvct_demo.TWL.rsf
LCFILE_SPEC     = libvct_demo.TWL.lsf
endif


do-build:		$(TARGETS)

include	$(TWLLIBVCT_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
