#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB - multiboot-PowerSave
# File:     Makefile
#
# Copyright 2005-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS				 =	

# MBライブラリのサンプルはワイヤレス機能を使うため、
# 同じ通信環境(ワイヤレスあるいは有線LAN)同士の開発機材を複数必要とします。
# $TwlSDK/bin/ARM9-TS/Rom/ ディレクトリにある mb_child_NITRO.srl, mb_child_TWL.srl プログラムが
# 製品実機のIPLにおけるDSダウンロードプレイ子機と同等の機能を
# 提供するサンプルになっていますので、このバイナリを
# サンプルプログラムと同じ方法で他の機材に読み込み、
# 一緒に実行してください。

#----------------------------------------------------------------------------
TARGET_PLATFORM		 =	TWL NITRO

SRCS				 =	common.c data.c dispfunc.c menu.c parent.c main.c

TARGET_NEF			 =	main.nef
TARGET_BIN			 =	main.srl

# makerom settings
MAKEROM_ROMROOT		 =	./data
MAKEROM_ROMFILES	 =	*.srl


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			 =	main.TWL.rsf
LCFILE_SPEC			 =	main.TWL.lsf
else
ROM_SPEC			 =	main.rsf
LCFILE_SPEC			 =	main.lsf
endif

#----------------------------------------------------------------------------

do-build:			$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

ifdef	TWL_ELFTOBIN
$(TARGETS): $(ELFTOBIN_ARM7)

else	#TWL_ELFTOBIN
$(TARGETS): $(MAKEROM_ARM7)

endif	#TWL_ELFTOBIN


#===== End of Makefile =====
