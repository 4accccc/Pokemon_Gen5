#! make -f
#----------------------------------------------------------------------------
# Project:  Particle Demo - callback
# File:     Makefile
# $Log: Makefile,v $
# Revision 1.1  2004/12/28 02:20:06  okane_shinji
# èâî≈ìoò^
#
# Revision 1.2  2004/09/15 01:42:11  okane_shinji
# ÉÅÉCÉNÉtÉ@ÉCÉãèCê≥
#
# Revision 1.1  2004/09/15 01:29:53  okane_shinji
# èâä˙ìoò^
#
# $NoKeywords: $
#----------------------------------------------------------------------------



SPL_DEMOLIB = $(SPL_ROOT)/example/spl/PtclLib

LINCLUDES       =   $(SPL_DEMOLIB)/include
LLIBRARY_DIRS   =   $(SPL_DEMOLIB)/lib/$(NITRO_BUILDTYPE) \
                    $(SPL_ROOT)/spl/lib/$(NITRO_BUILDTYPE) \

LLIBRARIES      =   libptcl_demo.a \
                    libjn_spl.a \


INCDIR		+=	include \
                $(SPL_ROOT)/spl/include \

#----------------------------------------------------------------------------

SUBDIRS	= 

SRCS		=	main.c
TARGET_NEF	=	main.nef
TARGET_BIN	=	main.srl

NITRO_MAKEROM	=	TRUE
ROM_SPEC	=	main.rsf


#----------------------------------------------------------------------------
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs


do-build:		$(TARGETS)


include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
