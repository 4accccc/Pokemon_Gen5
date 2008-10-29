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

#----------------------------------------------------------------------------
#	ロム作成用定義　（ＲＯＭ提出の際は有効にしてsrl作成）
#----------------------------------------------------------------------------
#NITRO_FINALROM = yes

.PHONY : gflib installsrl


#MAKE環境設定ファイルの読み込み
MAKE_ENV_FILE	=	make.env
include		$(MAKE_ENV_FILE)

ifeq	($(MULTI_BOOT_MAKE),yes)
MACRO_FLAGS	+=	-DMULTI_BOOT_MAKE
endif

LINCLUDES	=	./include \
				./arc


# GF-Library
GFLIB_DIR		=	../lib/gflib
LINCLUDES		+= 	$(GFLIB_DIR)/include
LLIBRARY_DIRS	=	$(GFLIB_DIR)/install/$(NITRO_BUILDTYPE)
LLIBRARIES		=  	gflib.a

# wifiを使用する場合のLibrary定義
#LDEPENDS_NEF = $(NITRODWC_LIB_DEPENDS)
#NITRODWC_LIB_MODULES_   = base auth ac bm account
#NITRODWC_LIB_GSMODULES_ = common ghttp gp gstats gt2 natneg qr2 serverbrowsing chat

#----------------------------------------------------------------------------
#パーティクル＋イクニューモンコンポーネントのためのライブラリを追加
#----------------------------------------------------------------------------
LLIBRARY_DIRS	+= ./library/spl/$(NITRO_BUILDTYPE)
LLIBRARIES		+= ./libjn_spl.a libwvr$(CODEGEN).a
#LLIBRARIES		+= libwvr$(CODEGEN).a
# 任天堂Cryptライブラリを追加 by mituhara 2006.07.05
LLIBRARY_DIRS	+= ./library/crypto/$(NITRO_BUILDTYPE)
LLIBRARIES	+= ./libcrypto.a

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
SRCS			=	main.c gfl_use.c machine_use.c

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
#	ロムに含まれるファイルの指定
#----------------------------------------------------------------------------
GFLIBGRADIR	= src/gfl_graphic/
SAMPLE_GRADIR = src/sample_graphic/
SAMPLE_SNDDIR = src/sample_sound/
SAMPLE_MBRDIR = src/sample_multi_boot_rom/

#FILE_TREE	=	$(GFLIBGRADIR)utility.bin \
#			$(GFLIBGRADIR)wmi.narc \
#			$(GFLIBGRADIR)gfl_font.dat \
#			$(SAMPLE_GRADIR)*.narc \
			  
#ロムファイルシステムのルートディレクトリ指定
MAKEROM_ROMROOT  = ./

##MAKEROM_ROMFILES = $(subst ,,$(FILE_TREE))

-include arc/file_tree.inc
EDEPENDS_BIN += $(patsubst %, arc/%, $(GAME_FILE_TREE:.narc=.o))


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
#include	$(NITROVCT_ROOT)/build/buildtools/commondefs
#include	$(NITROVCT_ROOT)/build/buildtools/modulerules

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


#マルチブート対応のMAKEをするかどうかをフラグで判定

#ワイルドカードで指定されているファイル名を展開しておく
#MAKEROM_MB:=$(wildcard $(GFL_MAKEROM_ROMFILES))
#EXT_OBJS+=$(wildcard $(GFL_MAKEROM_ROMFILES:.narc=.o))

#----------------------------------------------------------------------------
#	.narcから.oを生成するためのファイルツリー作成
#----------------------------------------------------------------------------
MAKEROM_MB = $(patsubst %, arc/%, $(GAME_FILE_TREE))

#----------------------------------------------------------------------------
#	.narcから.oを生成
#----------------------------------------------------------------------------
#do-build: $(MAKEROM_MB:.narc=.o)
%.o	: %.narc
	$(BIN2OBJ) -b _start_%b -e _end_%b $< $*.o
	touch -c makefile

#----------------------------------------------------------------------------
#	.narcから生成された.oをリンクに含めるための記述
#----------------------------------------------------------------------------
EXT_OBJS += $(patsubst %, arc/%, $(GAME_FILE_TREE:.narc=.o))


#--------------------------------------- ----------------------------
do-build: $(MAKEROM_MB:.narc=.o) $(TARGETS)


#----------------------------------------------------------------------------
#	ソースのみのリコンパイル
#----------------------------------------------------------------------------
source:
	-$(RM) $(DEPENDDIR)/*.d
	$(MAKE)

#----------------------------------------------------------------------------
#	子機srlを親機側に移動
#----------------------------------------------------------------------------

installsrl:
	$(CP) .\bin\ARM9-TS\Release\main.srl ..\prog\filetree\dl_rom\child.srl

