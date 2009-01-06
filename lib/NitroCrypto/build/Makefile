#! make -f
#----------------------------------------------------------------------------
# Project:  NitroCrypto - build
# File:     Makefile
#
# Copyright 2006 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile,v $
# Revision 1.10  2006/03/09 04:22:57  okubata_ryoma
# demosのみをmakeするように変更
#
# Revision 1.7  2006/03/08 09:48:25  okubata_ryoma
# buildtoolsをmakeするように修正
#
# Revision 1.6  2006/03/01 11:31:22  okubata_ryoma
# buildtoolsをmakeしないように変更
#
# Revision 1.5  2006/02/28 06:20:42  okubata_ryoma
# toolsの追加
#
# Revision 1.4  2006/02/27 03:35:51  okubata_ryoma
# buildtoolsを先にビルドするように変更
#
# Revision 1.3  2006/02/27 00:48:24  okubata_ryoma
# Copyrightの修正
#
# Revision 1.2  2006/02/24 09:09:27  okubata_ryoma
# buildsetupをビルドするように修正
#
# Revision 1.1  2006/02/24 01:15:29  okubata_ryoma
# NitroCrypto の新設
#
#
# $NoKeywords: $
#----------------------------------------------------------------------------

include	$(NITROSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------
SUBDIRS_BUILDTOOLS	= buildtools
SUBDIRS_SETUP		= buildsetup/versions
SUBDIRS_TOOLS		= tools
SUBDIRS_LIBS		= libraries
SUBDIRS_DEMOS		= demos
SUBDIRS_CLEANUP		= buildtools

# if no ONLY defs, do build all
ifndef	NITRO_ONLY_BUILDTOOLS
ifndef	NITRO_ONLY_SETUP
ifndef	NITRO_ONLY_TOOLS
ifndef	NITRO_ONLY_LIBS
ifndef	NITRO_ONLY_DEMOS
ifndef	NITRO_ONLY_CLEANUP
#NITRO_ONLY_BUILDTOOLS	= True
#NITRO_ONLY_SETUP	= True
#NITRO_ONLY_TOOLS	= True
#NITRO_ONLY_LIBS	= True
NITRO_ONLY_DEMOS	= True
#NITRO_ONLY_CLEANUP	= True
endif
endif
endif
endif
endif
endif

# build BUILDTOOLS
ifdef	NITRO_ONLY_BUILDTOOLS
SUBDIRS	+= $(SUBDIRS_BUILDTOOLS)
endif

# build SETUP
ifdef	NITRO_ONLY_SETUP
SUBDIRS		+= $(SUBDIRS_SETUP)
endif

# build TOOLS
ifdef	NITRO_ONLY_TOOLS
SUBDIRS		+= $(SUBDIRS_TOOLS)
endif

# build LIBS
ifdef	NITRO_ONLY_LIBS
SUBDIRS		+= $(SUBDIRS_LIBS)
endif

# build DEMOS
ifdef	NITRO_ONLY_DEMOS
SUBDIRS		+= $(SUBDIRS_DEMOS)
endif

# build CLEANUP
ifdef	NITRO_ONLY_CLEANUP
SUBDIRS	+= $(SUBDIRS_CLEANUP)
endif

#----------------------------------------------------------------------------
include	$(NITROSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
