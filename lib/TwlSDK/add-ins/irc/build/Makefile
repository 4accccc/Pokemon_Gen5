#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - IRC - build
# File:     Makefile
#
# Copyright 2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-04-16#$
# $Rev: 10419 $
# $Author: yosizaki $
#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------
SUBDIRS_SETUP		= buildtools
SUBDIRS_TOOLS		=
SUBDIRS_LIBS		= libraries
SUBDIRS_DEMOS		= demos
SUBDIRS_CLEANUP		= buildtools

# if no ONLY defs, do build all
ifndef	NITRO_ONLY_SETUP
ifndef	NITRO_ONLY_TOOLS
ifndef	NITRO_ONLY_LIBS
ifndef	NITRO_ONLY_DEMOS
ifndef	NITRO_ONLY_CLEANUP
NITRO_ONLY_SETUP	= True
NITRO_ONLY_TOOLS	= True
NITRO_ONLY_LIBS		= True
NITRO_ONLY_DEMOS	= True
NITRO_ONLY_CLEANUP	= True
endif
endif
endif
endif
endif


ifdef NITRO_WITHOUT_DEMOS
NITRO_ONLY_DEMOS	=
endif

# build SETUP
ifdef	NITRO_ONLY_SETUP
SUBDIRS		+= $(SUBDIRS_SETUP)
endif

# build TOOLS
ifdef	NITRO_ONLY_TOOLS
SUBDIRS			+= $(SUBDIRS_TOOLS)
endif

# build LIBS
ifdef	NITRO_ONLY_LIBS
SUBDIRS			+= $(SUBDIRS_LIBS)
endif

# build DEMOS
ifdef	NITRO_ONLY_DEMOS
SUBDIRS			+= $(SUBDIRS_DEMOS)
endif

# build CLEANUP
ifdef	NITRO_ONLY_CLEANUP
SUBDIRS		+= $(SUBDIRS_CLEANUP)
endif

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
