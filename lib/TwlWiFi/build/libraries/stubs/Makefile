#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - libraries - stubs
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-08#$
# $Rev: 1183 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL NITRO

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq	($(TWLWIFI_SOCKET_PRIVATE),TRUE)
SUBDIRS_P	= ssl
endif

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
