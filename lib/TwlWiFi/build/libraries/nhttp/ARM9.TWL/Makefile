#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - NHTTP - libraries
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
# $Date:: 2008-09-18#$
# $Rev: 1024 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL
TWL_PROC		= ARM9
TWL_CODEGEN		= ALL
TWL_ARCHGEN		= ALL

TARGET_LIB		= libnhttp$(TWIFI_LIBSUFFIX_EX).a

include ../common/Makefile.common

ifeq	($(TWLWIFI_HIDDEN),TRUE)
SUBMAKES		= Makefile.nssl
endif

#===== End of Makefile =====
