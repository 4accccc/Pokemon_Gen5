#! make -f
#----------------------------------------------------------------------------
# Project:  Pokemon GDS Network Library Demo
# File:     Makefile
#
#----------------------------------------------------------------------------

include ../depend.mk

SUBDIRS		=

#----------------------------------------------------------------------------

CRCDIR      =  ../TestPokeNet/Nitro
SRCS        =   $(wildcard src/*.c)
SRCS        +=   $(CRCDIR)/crc.cpp)

TARGET_BIN	=	gds_sample.srl
DEMOS_ROOT  =   ../

#SRCDIR		=	# using default
#LCFILE		=	# using default

# dbs ライブラリ
LLIBRARIES      += libdwcdbs$(LIBSUFFIX).a
LLIBRARY_DIRS   += $(DEMOS_ROOT)/dbs/lib/$(BUILDTYPE)
LINCLUDES       += $(DEMOS_ROOT)/dbs/include
LINCLUDES       += $(CRCDIR)

# common ライブラリ
GDSLIB_DIR   =  ../poke_net

LINCLUDES		+= $(GDSLIB_DIR)
LLIBRARY_DIRS	+= $(GDSLIB_DIR)/lib/$(BUILDTYPE)
LLIBRARIES		+= libgds$(LIBSUFFIX).a

NITRODWC_LIB_MODULES_   = ac auth bm base util account

CCFLAGS_MSGSTYLE = -msgstyle gcc

include	$(DWC_ROOT)/build/buildtools/commondefs
CCFLAGS += $(ADD_CCFLAGS)
#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(DWC_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
