#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - wvr - with_mb - child
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
# $Date:: 2009-06-11#$
# $Rev: 10743 $
# $Author: okajima_manabu $
#----------------------------------------------------------------------------

TARGET_PLATFORM :=  NITRO

INCDIR      =   ../common/include ./include
SRCDIR      =   ./src ../common/src 

LLIBRARIES  =   libwvr$(TWL_LIBSUFFIX).a

SRCS        =   main.c      \
                font.c      \
                disp.c      \
                common.c    \
                gmain.c

# WH モジュール は wireless_shared へ移動しました.
WH_DIR      =   $(ROOT)/build/demos/wireless_shared/wh
SRCDIR      +=  $(WH_DIR)
INCDIR      +=  $(WH_DIR)
SRCS        +=  $(WH_DIR)/wh.c
MACRO_FLAGS += -DSHARED_WH


TARGET_BIN  =   child.srl

#------------------------------------------------------#
# VRAM内に無線ライブラリが配置されるコンポーネント定義 #

DEFAULT_COMP_ARM7	=	ichneumon

#------------------------------------------------------#

include $(TWLSDK_ROOT)/build/buildtools/commondefs


#-------------------------------#
#   install target      #
#-------------------------------#
INSTALL_TARGETS =   $(TARGETS)

INSTALL_DIR = ../data


#----------------------------------------------------------------------------

do-build:       $(TARGETS)

include $(TWLSDK_ROOT)/build/buildtools/modulerules



#===== End of Makefile =====
