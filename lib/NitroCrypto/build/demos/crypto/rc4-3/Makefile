#! make -f
#----------------------------------------------------------------------------
# Project:  NitroCrypto - build - demos
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
# $Log: Makefile,v $
# Revision 1.1  2006/03/08 08:53:41  seiki_masashi
# rc4-3 ƒfƒ‚‚Ì’Ç‰Á
#
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------

SRCS		=	main.c rc4enc.c prng.c

TARGET_BIN	=	main.srl

LINCLUDES     += $(NCRYPTO_ROOT)/include
LLIBRARY_DIRS += $(NCRYPTO_ROOT)/lib/$(NITRO_LIBTYPE)
LLIBRARIES    += libcrypto$(NITRO_LIBSUFFIX).a

include	$(NITROSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(NITROSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
