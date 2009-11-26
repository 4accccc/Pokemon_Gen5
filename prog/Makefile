#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment 'pokemon_wb'
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

#export	NITRO_DEBUG=TRUE
export MAKEROM_ROMSPEED=1TROM
#export MAKEROM_ROMSPEED=MROM


TWL_ARCHGEN		:=	HYBRID

# NITROSDK3.0付属ののプリコンパイルを読み込まない
NITRO_NO_STD_PCHDR = True

.PHONY : source analyze overlaymap

#----------------------------------------------------------------------------
#	インクルードファイルディレクトリ
#----------------------------------------------------------------------------
LINCLUDES      = ./include \
		./arc


#----------------------------------------------------------------------------
# GF-Library
#----------------------------------------------------------------------------
GFLIB_DIR		=	../lib/gflib/
LINCLUDES		+= 	$(GFLIB_DIR)/include
LLIBRARY_DIRS	+=	$(GFLIB_DIR)/install/$(NITRO_BUILDTYPE)
LLIBRARIES		+=  	gflib.a


# wifiをオーバーレイで使用する場合のLibrary定義
LDEPENDS_NEF += $(TWLDWC_LIB_DEPENDS)
TWLDWC_LIB_MODULES_   = bm account common ac
TWLDWC_LIB_GSMODULES_ = 
NITROVCT_LIBS = 
TWLVCT_LIBS = 
TWLWIFI_LIBS = 

# JPEGライブラリをオーバーレイする為の定義
TWL_LIBS		= 								\
				   libcard$(TWL_LIBSUFFIX).a	\
				   libcp$(TWL_LIBSUFFIX).a		\
				   libel$(TWL_LIBSUFFIX).a		\
				   libenv$(TWL_LIBSUFFIX).a		\
				   libext$(TWL_LIBSUFFIX).a		\
				   libfs$(TWL_LIBSUFFIX).a		\
				   libfx$(TWL_LIBSUFFIX).a		\
				   libgx$(TWL_LIBSUFFIX).a		\
				   libmath$(TWL_LIBSUFFIX).a	\
				   libmi$(TWL_LIBSUFFIX).a		\
				   libnvram$(TWL_LIBSUFFIX).a	\
				   libos$(TWL_LIBSUFFIX).a		\
				   libprc$(TWL_LIBSUFFIX).a		\
				   libpxi$(TWL_LIBSUFFIX).a		\
				   librtc$(TWL_LIBSUFFIX).a		\
				   libscfg$(TWL_LIBSUFFIX).a	\
				   libsnd$(TWL_LIBSUFFIX).a		\
				   libspi$(TWL_LIBSUFFIX).a		\
				   libstd$(TWL_LIBSUFFIX).a		\
				   libwfs$(TWL_LIBSUFFIX).a		\
				   libwm$(TWL_LIBSUFFIX).a		\
				   libctrdg$(TWL_LIBSUFFIX).a	\
				   libcrypto$(TWL_LIBSUFFIX).a	\
			       libcht$(TWL_LIBSUFFIX).a		\
				   libwxc$(TWL_LIBSUFFIX).a		\
				   $(notdir $(LIBSYSCALL_))		\
				   $(notdir $(LIBSYSCALL_TWL_))	\
				   $(CRT0_O)

# nitrosystemをオーバーレイで使用する場合のLibrary定義
TWLSYSTEM_LIBS = \
		libnnsfnd$(NITRO_LIBSUFFIX).a \
		libnnsgfd$(NITRO_LIBSUFFIX).a \
		libnnsg2d$(NITRO_LIBSUFFIX).a \
		libnnsg3d$(NITRO_LIBSUFFIX).a \
		libnnssnd$(NITRO_LIBSUFFIX).a

#----------------------------------------------------------------------------
#パーティクルのためのライブラリを追加
#----------------------------------------------------------------------------
LLIBRARY_DIRS	+= ./library/spl/$(TWL_BUILDTYPE)
LLIBRARIES		+= ./libjn_spl.a

#----------------------------------------------------------------------------
#	バージョン定義
#----------------------------------------------------------------------------
VER_FILE	=	version
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
LCFILE_SPEC	=	overlay/main$(NITRO_LIBSUFFIX).lsf
MAKELCF_FLAGS	+=	-DADDRESS_MAIN=0x02000000


# 個人テスト用定義
#ifeq	($(PM_DEBUG),yes)
MACRO_FLAGS += -DDEBUG_ONLY_FOR_$(USERNAME)
#endif

# 最終的に出力されるファイル(lib/ARM9-xx/xxx/gflib.a)
#TARGET_BIN		=	main.bin

# ソースファイル
SRCS			=	system\main.c \
					system\machine_use.c \
					system\gfl_use.c \
					system\net_define.c

include make_prog_files
-include overlay/overlay_files

## xyzzyでエラージャンプを行う
ifeq	($(USERNAME),ohno)
CCFLAGS_MSGSTYLE	?= -msgstyle gcc
endif

#----------------------------------------------------------------------------
# コンパイル時の警告抑止
#----------------------------------------------------------------------------
#ifeq	($(USERNAME),tamada)
#CCFLAGS_WARNING	= -w all,noerr,nounusedarg,noextracomma,\
#	nonotused,nofilecaps,nosysfilecaps,noimpl_signedunsigned,noimplicit
#else
CCFLAGS_WARNING		=	-w all,nocmdline,noerr,nounused,noextracomma,nopedantic,noimplicit,\
				nostructclass,nopadding,nonotused,noanyptrintconv,nofilecaps,nosysfilecaps
#endif


#----------------------------------------------------------------------------
# 任天堂Cryptライブラリを追加
#----------------------------------------------------------------------------
LLIBARIES += libcrypto$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------
#	ロムに含まれるファイルの指定  default.rsfファイルで使用している
#----------------------------------------------------------------------------
#ロムファイル指定(GFL)
GFL_FILE_TREE	=	titledemo.narc \
					skb.narc \
					soundstatus.narc \
					wb_sound_data.sdat \
					gfl_font.dat \
					dl_rom/child.srl \
					dl_rom/icon.char \
					dl_rom/icon.plt


GFL_MAKEROM_ROMFILES = $(GFL_FILE_TREE)

#ロムファイル指定(GAME)
#GAME_FILE_TREE	= test_graphic/*.narc \
#					*.narc
#ロムファイル指定は、arc/arc_tool.lstに記述してください
#その記述内容から、file_tree.incを自動生成します

-include arc/file_tree.inc

MAKEROM_ROMFILES = $(GAME_FILE_TREE)

#リンク時の依存関係に含めるためにEDEPENDS_BINにFILE_TREEを入れておく
EDEPENDS_BIN += $(patsubst %, filetree/%, $(GFL_FILE_TREE))
EDEPENDS_BIN += $(patsubst %, filetree/%, $(GAME_FILE_TREE))

## ------------------------------------------------------------------------------------------
## GFL_MAKEROM_ROMFILESの定義をmakerom.exeに渡す MAKEROM_ROMFILESはデフォルトで定義されている
## makeした後に main.nlfファイルを開けば ファイルが正しく紐付けされたかがわかる
##-------------------------------------------------------------------------------------------
MAKEROM_FLAGS += -DGFL_MAKEROM_ROMFILES='$(GFL_MAKEROM_ROMFILES)'

#----------------------------------------------------------------------------
#	ロムスペックファイル読み込みのため追加
#----------------------------------------------------------------------------
ROM_SPEC= default$(TWL_LIBSUFFIX).rsf

#----------------------------------------------------------------------------
#	ターゲットファイル設定
#TARGET_NEF	=	main.nef	TARGET_BINに.srlを指定すると.nefも一緒に生成される
TARGET_BIN	=	main.srl

#--------------------------------------- ----------------------------
# システム定義
#

include	$(TWLVCT_ROOT)/build/buildtools/commondefs

include $(TWLSYSTEM_ROOT)/build/buildtools/commondefs
include $(TWLSYSTEM_ROOT)/build/buildtools/nnslibdefs
include $(TWLSYSTEM_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#	WiFi定義
#----------------------------------------------------------------------------
#include	$(TWLDWC_ROOT)/build/buildtools/commondefs
#include	$(TWLDWC_ROOT)/build/buildtools/modulerules
include	$(TWLVCT_ROOT)/build/buildtools/modulerules

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
programs = $(TARGETS)

do-build: precompile $(TARGETS)


#----------------------------------------------------------------------------
# precompile
#----------------------------------------------------------------------------
PRECOMPILE_SRC		 =	include/precompile/precompile.pch
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

CCFLAGS +=	-thumb



#----------------------------------------------------------------------------
#	ソースのみのリコンパイル
#----------------------------------------------------------------------------
source:
	-$(RM) $(GDIRT_CLEAN)
	-$(RM) $(DEPENDDIR)/*.d
	$(MAKE)

#----------------------------------------------------------------------------
#	ソースあたりオブジェクトサイズ算出
#----------------------------------------------------------------------------
analyze:
	ruby ../tools/analyze/objsize/get_objsize.rb ./bin/$(TWL_BUILDTYPE)/$(TARGET_BIN:.srl=.nef).xmap|sort > analyze.result

#----------------------------------------------------------------------------
#	オーバーレイマップ可視化
#----------------------------------------------------------------------------
overlaymap:
	../tools/analyze/overlaymap/overlaymap $(PROJECT_PROGDIR)/bin/$(TWL_BUILDTYPE)/$(TARGET_BIN:.srl=.nef).xmap
	#dot -Tpng ../tools/analyze/overlaymap/overlaymap.dot >overlaymap.png

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
filesize:
		ruby ../tools/analyze/filesize/filesize.rb $(PROJECT_PROGDIR)/bin/$(TWL_BUILDTYPE)/$(TARGET_BIN:.srl=.nlf)

#----------------------------------------------------------------------------
#	警告フィルタの適用
#----------------------------------------------------------------------------
warning:
	$(MAKE) source > result.txt
	ruby ../tools/warning/filter.rb result.txt > fresult.txt

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
echo_buildtype:
	@echo $(TWL_BUILDTYPE)

