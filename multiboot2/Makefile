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

# NITROSDK3.0付属ののプリコンパイルを読み込まない
NITRO_NO_STD_PCHDR = True

#マルチブートにDSIバージョンはない
#TWLSDK_PLATFORM = NITRO

PARENT_SRC		=	../prog/src
PARENT_INC		=	../prog/include
PARENT_ARC		=	../prog/arc

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

#子機で親側のソースを見に行くので追加
LINCLUDES	=	./include \
				./arc \
				$(PARENT_INC) \
				$(PARENT_ARC) \


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
#パーティクルのためのライブラリを追加
#----------------------------------------------------------------------------
LLIBRARY_DIRS	+= ./library/spl/$(TWL_BUILDTYPE)
LLIBRARIES		+= ./libjn_spl.a

#----------------------------------------------------------------------------
#	バージョン定義
#----------------------------------------------------------------------------
VER_FILE	=	../prog/version
include	$(VER_FILE)

#(PM_DEBUG == yes)のときのみデバッグバージョン
ifeq	($(PM_DEBUG),yes)
MACRO_FLAGS	+=	-DPM_DEBUG
endif

ifdef	PM_VERSION
MACRO_FLAGS	+=	-DPM_VERSION=$(PM_VERSION)
endif

ifdef	PM_LANG
MACRO_FLAGS	+=	-DPM_LANG=$(PM_LANG)
endif

#----------------------------------------------------------------------------
#	NitroのデフォルトLCFファイルでなく、作成したものを読み込む	
#----------------------------------------------------------------------------
LCFILE_SPEC	=	main$(TWL_LIBSUFFIX).lsf
MAKELCF_FLAGS	+=	-DADDRESS_MAIN=0x02000000


# 個人テスト用定義
#ifeq	($(PM_DEBUG),yes)
MACRO_FLAGS += -DDEBUG_ONLY_FOR_$(USERNAME)
#endif

# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
TARGET_BIN		=	main.bin

# ソースファイル
SRCS			=	main.c $(PARENT_SRC)/system/gfl_use.c $(PARENT_SRC)/system/machine_use.c

include make_prog_files
#-include overlay/overlay_files

## xyzzyでエラージャンプを行う
ifeq	($(USERNAME),ohno)
CCFLAGS_MSGSTYLE	?= -msgstyle gcc
endif

# コンパイル時の警告抑止
CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps

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
ROM_SPEC= default$(TWL_LIBSUFFIX).rsf

#----------------------------------------------------------------------------
#	ターゲットファイル設定
#TARGET_NEF	=	main.nef	TARGET_BINに.srlを指定すると.nefも一緒に生成される
TARGET_BIN	=	main.srl

EXT_OBJS += $(patsubst %, arc/%, $(GAME_FILE_TREE:.narc=.o))

#include	$(TWLVCT_ROOT)/build/buildtools/commondefs
include $(TWLSYSTEM_ROOT)/build/buildtools/commondefs
include $(TWLSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(TWLSYSTEM_ROOT)/build/buildtools/modulerules
#include	$(TWLVCT_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------
#	makefileがインクルードするファイルが更新されたらmakelcfが動作するようにした
#----------------------------------------------------------------------------
#$(MAKEFILE):  make_prog_files overlay/overlay_files
#	ruby overlay/overlaytool.rb
#	//touch $(MAKEFILE)

#overlay/main.lsf:	make_prog_files
#	ruby overlay/overlaytool.rb

#overlay/overlay_files:	make_prog_files
#	ruby overlay/overlaytool.rb


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
#SOBJS_CC +=  $(MAKEROM_MB:.narc=.o)

#--------------------------------------- ----------------------------
# システム定義 &WiFi定義
#


#--------------------------------------- ----------------------------
do-build: precompile $(MAKEROM_MB:.narc=.o) $(TARGETS)

#----------------------------------------------------------------------------
#	ソースのみのリコンパイル
#----------------------------------------------------------------------------
source:
	-$(RM) $(DEPENDDIR)/*.d
	$(MAKE)

#----------------------------------------------------------------------------
#	子機srlを親機側に移動
#----------------------------------------------------------------------------

installsrl:..\prog\filetree\dl_rom\child2.srl
ifeq	($(TWLSDK_PLATFORM),NITRO)
..\prog\filetree\dl_rom\child2.srl:.\bin\ARM9-TS\Release\main.srl
	$(CP) .\bin\ARM9-TS\Release\main.srl ..\prog\filetree\dl_rom\child2.srl
else
..\prog\filetree\dl_rom\child2.srl:.\bin\ARM9-TS.HYB\Release\main.srl
	$(CP) .\bin\ARM9-TS.HYB\Release\main.srl ..\prog\filetree\dl_rom\child2.srl
endif

do-build: precompile $(TARGETS)


#----------------------------------------------------------------------------
# precompile
#----------------------------------------------------------------------------
PRECOMPILE_SRC		 =	../prog/include/precompile/precompile.pch
PRECOMPILE_CC_MCH	 =	$(OBJDIR)/precompile_cc.mch
PRECOMPILE_CCFLAGS	:=	$(CCFLAGS)
CCFLAGS			+=	-prefix $(PRECOMPILE_CC_MCH)

precompile: $(PRECOMPILE_CC_MCH)

#バージョン定義ファイルが更新されたらプリコンパイルヘッダも更新する
$(PRECOMPILE_CC_MCH):	$(VER_FILE)

-include $(PRECOMPILE_CC_MCH).d
$(PRECOMPILE_CC_MCH):%_cc.mch:
		$(CC) $(PRECOMPILE_CCFLAGS) $(INCLUDES) $(PRECOMPILE_SRC) -MD -precompile $(PRECOMPILE_CC_MCH)
##		$(MOVE_SRC_DEPEND)
##		$(MV) $(DEPENDDIR)/$(*F).d $@.d
$(OBJS):  $(PRECOMPILE_CC_MCH)

