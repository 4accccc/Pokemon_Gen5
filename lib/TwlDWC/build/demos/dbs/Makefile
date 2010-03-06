#! make -f
#----------------------------------------------------------------------------
# Project:  TWLDWC
#
# Copyright 2005-2010 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#----------------------------------------------------------------------------

SUBDIRS     =

#----------------------------------------------------------------------------

SRCS    = dbs_print.c dbs_console.c dbs_demo.c

TARGET_LIB	= libdwcdbs$(TWL_LIBSUFFIX).a

CCFLAGS_MSGSTYLE = -msgstyle gcc

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:       $(TARGETS)

include $(TWLSDK_ROOT)/build/buildtools/modulerules
