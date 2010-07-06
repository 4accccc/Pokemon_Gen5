#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - SOC - shared
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
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
TARGET_PLATFORM		= TWL NITRO
TWL_ARCHGEN			= ALL

SRCS		 = padkeyboard.c font.c output.c screen.c ftp.c

TARGET_LIB	?= libkeypad$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs.demos

do-build:	$(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
