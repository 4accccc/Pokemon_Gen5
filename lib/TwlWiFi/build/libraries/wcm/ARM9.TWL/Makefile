#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - WCM - libraries
# File:    Makefile
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

SUBDIRS		=

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL
TWL_PROC		= ARM9
TWL_ARCHGEN		= ALL
TWL_CODEGEN		= ALL

TARGET_LIB		= libwcm$(TWL_LIBSUFFIX).a

LINCLUDES		= $(ROOT)/build/libraries/wm/ARM9.TWL/nwm/inc \
				  $(ROOT)/build/libraries/wm/common.TWL/inc
SRCS		       += ../../ARM9.TWL/src/nwmif.c

include ../common/Makefile.common

#===== End of Makefile =====
