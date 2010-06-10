#! make -f
#----------------------------------------------------------------------------
# Project:  Pokemon GDS Network Library
# File:     Makefile
#
#----------------------------------------------------------------------------
#NITRO_FINALROM = yes
#TWL_FINALROM = yes

include ../depend.mk

SUBDIRS		=

#----------------------------------------------------------------------------


#----------------------------------------------------------------------------

SRCS		=	poke_net.c poke_net_ds.c poke_net_gds.c poke_net_opt.c poke_net_gds_debug.c poke_net_ds_ssl.c
TARGET_LIB	=	libgds$(LIBSUFFIX).a

#SRCDIR		=	# using default
#LCFILE		=	# using default

CCFLAGS_MSGSTYLE = -msgstyle gcc 

include	$(DWC_ROOT)/build/buildtools/commondefs
CCFLAGS += $(ADD_CCFLAGS)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(DWC_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
