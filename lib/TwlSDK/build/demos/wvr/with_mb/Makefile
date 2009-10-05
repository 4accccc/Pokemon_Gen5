#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - wvr - with_mb
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
# $Date:: 2009-06-24#$
# $Rev: 10808 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

TARGET_PLATFORM :=  NITRO TWL
TWL_ARCHGEN		:=	HYBRID

# 子機ブート用バイナリを作成
SUBDIRS     =   child


# MBライブラリのサンプルはマルチブート機能を使うため、
# 同じ通信環境(無線あるいは有線)どうしの開発機材を複数必要とします。
# $NitroSDK/bin/ARM9-TS/Release/ ディレクトリにある
# mb_child.bin プログラムが最終実機におけるマルチブート子機と同等の
# 機能を提供するサンプルになっていますので、
# このバイナリをサンプルプログラムと同じ方法で他の機材に読み込み、
# 一緒に実行してください。


#----------------------------------------------------------------------------

INCDIR      =   common/include parent/include
SRCDIR      =   common/src parent/src
SRCS        =   main.c      \
                common.c    \
                disp.c      \
                font.c      \
                gmain.c

LLIBRARIES  =   libwvr$(TWL_LIBSUFFIX).a

# WH モジュール は wireless_shared へ移動しました.

WH_DIR      =   $(ROOT)/build/demos/wireless_shared/wh
SRCDIR      +=  $(WH_DIR)
INCDIR      +=  $(WH_DIR)
SRCS        +=  $(WH_DIR)/wh.c

# MBP モジュール は wireless_shared へ移動しました.

MBP_DIR     =   $(ROOT)/build/demos/wireless_shared/mbp
SRCDIR      +=  $(MBP_DIR)
INCDIR      +=  $(MBP_DIR)
SRCS        +=  $(MBP_DIR)/mbp.c

TARGET_NEF	 =	main.nef
TARGET_BIN  =   main.srl

# MBのコールバック内で複雑な処理をする場合には、lcfファイルで
# IRQスタックサイズを少し大きめに設定しておいた方が安全です。
# 特にOS_Printf()は大量のスタックを消費するので、コールバック内では
# できるだけ軽量版のOS_TPrintf()を使用するようにしてください。

# makerom settings
MAKEROM_ROMROOT     =   ./data
MAKEROM_ROMFILES    =   *.srl

#------------------------------------------------------#
# VRAM内に無線ライブラリが配置されるコンポーネント定義 #

DEFAULT_COMP_ARM7	=	ichneumon

#------------------------------------------------------#

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

# ichneumon.TWL はサイズの関係で CWforDSi 版にのみ含まれるため、それを使用するデモはビルドしない
ifneq	($(SDK_CW_BUILD_VERSION_CC),)
ifeq	($(shell expr "$(SDK_CW_BUILD_VERSION_CC)" "<=" 3.0),1)
TARGET_PLATFORM_LIST		:= $(filter-out TWL, $(TARGET_PLATFORM_LIST))
endif
endif

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			 =	main.TWL.rsf
else
ROM_SPEC			 =	main.rsf
endif

do-build:	$(TARGETS)

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN

#===== End of Makefile =====
