#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment 'skelton'
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#
# $LastChangedDate$
# $Id$
#
#----------------------------------------------------------------------------

.PHONY : gflib

# GF-Library
GFLIB_DIR		=	gflib
LINCLUDES		= 	$(GFLIB_DIR)/include
LLIBRARY_DIRS	=	$(GFLIB_DIR)/install/$(NITRO_BUILDTYPE)
LLIBRARIES		=  	gflib.a

# wifiを使用する場合のLibrary定義
LDEPENDS_NEF = $(NITRODWC_LIB_DEPENDS)
NITRODWC_LIB_MODULES_   = base auth ac bm account
NITRODWC_LIB_GSMODULES_ = common ghttp gp gstats gt2 natneg qr2 serverbrowsing chat

#----------------------------------------------------------------------------
#パーティクル＋イクニューモンコンポーネントのためのライブラリを追加
#----------------------------------------------------------------------------
LLIBRARY_DIRS	+= ./library/spl/$(NITRO_BUILDTYPE)
LLIBRARIES		+= ./libjn_spl.a libwvr$(CODEGEN).a

#----------------------------------------------------------------------------
#	インクルードファイルのディレクトリの追加
#----------------------------------------------------------------------------

LINCLUDES      = ./gflib/include \
				 ./include


#----------------------------------------------------------------------------
#	NitroのデフォルトLCFファイルでなく、作成したものを読み込む	
#----------------------------------------------------------------------------
LCFILE_SPEC	=	overlay/main.lsf
MAKELCF_FLAGS	+=	-DADDRESS_MAIN=0x02000000


# 個人テスト用定義
#ifeq	($(PM_DEBUG),yes)
MACRO_FLAGS += -DDEBUG_ONLY_FOR_$(USERNAME)
#endif

# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
TARGET_BIN		=	main.bin

# ソースファイル
SRCS			=	system\main.c \
					system\machine_use.c \
					system\gfl_use.c

include make_prog_files
-include overlay/overlay_files

## xyzzyでエラージャンプを行う
ifeq	($(USERNAME),ohno)
CCFLAGS_MSGSTYLE	?= -msgstyle gcc
endif

# コンパイル時の警告抑止
CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps

#------------------------------------------------------#
# イクニューモンコンポーネントを追加 by Mori 2005.7.27
# VRAM内に無線ライブラリが配置されるコンポーネント定義 #
MY_COMPONENT_NAME	= ichneumon
MAKEROM_ARM7	= $(NITRO_COMPONENTSDIR)/$(MY_COMPONENT_NAME)/$(NITRO_BUILDTYPE_ARM7)/$(MY_COMPONENT_NAME)_sub$(TS_VERSION).nef

#----------------------------------------------------------------------------
#	ロムに含まれるファイルの指定  default.rsfファイルで使用している
#----------------------------------------------------------------------------
#ロムファイル指定(GFL)
GFL_FILE_TREE	=	\
					wmi.narc \
					gfl_font.dat

GFL_MAKEROM_ROMFILES = $(GFL_FILE_TREE)

#ロムファイル指定(GAME)
GAME_FILE_TREE	= test_graphic/*.narc
			  
MAKEROM_ROMFILES = $(GAME_FILE_TREE)

## ------------------------------------------------------------------------------------------
## GFL_MAKEROM_ROMFILESの定義をmakerom.exeに渡す MAKEROM_ROMFILESはデフォルトで定義されている
## makeした後に main.nlfファイルを開けば ファイルが正しく紐付けされたかがわかる
##-------------------------------------------------------------------------------------------
MAKEROM_FLAGS += -DGFL_MAKEROM_ROMFILES='$(GFL_MAKEROM_ROMFILES)'

#----------------------------------------------------------------------------
#	ロムスペックファイル読み込みのため追加
#----------------------------------------------------------------------------
ROM_SPEC= default.rsf

#----------------------------------------------------------------------------
#	ターゲットファイル設定
#TARGET_NEF	=	main.nef	TARGET_BINに.srlを指定すると.nefも一緒に生成される
TARGET_BIN	=	main.srl

#--------------------------------------- ----------------------------
# システム定義
#
include $(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include $(NITROSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#	WiFi定義
#----------------------------------------------------------------------------
#include	$(NITRODWC_ROOT)/build/buildtools/commondefs
#include	$(NITRODWC_ROOT)/build/buildtools/modulerules
include	$(NITROVCT_ROOT)/build/buildtools/commondefs
include	$(NITROVCT_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#	makefileがインクルードするファイルが更新されたらmakelcfが動作するようにした
#----------------------------------------------------------------------------
#$(MAKEFILE):  make_prog_files overlay/overlay_files
#	ruby overlay/overlaytool.rb
#	//touch $(MAKEFILE)

overlay/main.lsf:	make_prog_files
	ruby overlay/overlaytool.rb

overlay/overlay_files:	make_prog_files
	ruby overlay/overlaytool.rb

#--------------------------------------- ----------------------------
do-build:		$(TARGETS)

#-----
#  gflibをmakeする
gflib:
	$(MAKE) -C $(GFLIB_DIR)


#----------------------------------------------------------------------------
#	ソースのみのリコンパイル
#----------------------------------------------------------------------------
source:
	-$(RM) $(DEPENDDIR)/*.d
	$(MAKE)
