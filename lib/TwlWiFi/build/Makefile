#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - build
# File:     Makefile
#
# Copyright 2003-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-10-20#$
# $Rev: 1071 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM		 = TWL NITRO

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------
ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)

ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),)
SUBMAKES.DEMOS		+=	demos/Makefile
SUBMAKES.DEMOS		+=	demos.TWL/Makefile

ifeq	($(TWLWIFI_HIDDEN),TRUE)
SUBMAKES.TESTS		+=	tests/Makefile
endif
endif

endif	# TWL

#----------------------------------------------------------------------------
ifneq	($(filter NITRO,$(TARGET_PLATFORM_LIST)),)

ifeq	($(TWL_WITHOUT_DEMOS)$(NITRO_WITHOUT_DEMOS),)
#
# build demo if not NITRO_WITHOUT_DEMOS
#
SUBMAKES.DEMOS		+= demos/Makefile
ifeq	($(TWLWIFI_HIDDEN),TRUE)
SUBMAKES.TESTS		+= tests/Makefile
endif

endif

endif	# NITRO


#----------------------------------------------------------------------------
ifdef	TWLWIFI_NOSUPPORT
SUBMAKES	+=	buildsetup/versions/Makefile
endif


SUBMAKES 	+=	libraries/Makefile
SUBMAKES 	+=	tools/Makefile

SUBMAKES 	+=	$(sort $(SUBMAKES.DEMOS))			\
				$(sort $(SUBMAKES.TESTS))
#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
