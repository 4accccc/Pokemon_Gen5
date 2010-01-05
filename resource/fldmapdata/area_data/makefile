#! make -f
#----------------------------------------------------------------------------
# Project:  エリアデータ作成
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#	2008.12.18	WB環境へ移植開始
#----------------------------------------------------------------------------

include area_common.def

DQ		="
#"
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#area_table.xls
SRC_XLS_FILE	= $(subst $(DQ),,$(SRC_XLS_FILENAME))
#area_table.txt
SRC_TXT_FILE	= $(subst .xls,.txt,$(SRC_XLS_FILE))
TEX_IMD_LIST	= $(subst $(DQ),,$(TEX_SRC_LIST_FILENAME))
BUILDXLSLIST	=	build_xls_list.txt
ORGAREAXLS	= org_areaxxo.xls
ORGTEXSET	= org_tex_set.imd
XLS	=	*.xls
TXTDATA	=	*.txt
TARGET_HEADER	=	$(subst $(DQ),,$(TARGET_HEADER_FILENAME))

#"area_build_model
BUILD_DIR		:= $(subst $(DQ),,$(BM_ARCHIVER_DIR))
#AREABUILD_ARC	:= $(BUILD_DIR)/area_build.narc

#NSBTXFILES_DIR	= nsbtx_files

#AREA_TABLE	= area_table.txt

TEX_DEPEND_FILES	= $(NSBTX_DEPEND_FILENAME) $(NNSARC_DEPEND_FILENAME) $(NNSARC_TEXLIST_FILENAME)

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada nakatsui fuchino lee_hyunjung tomoya_takahashi

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGET		=	$(subst $(DQ),,$(TARGET_BIN_FILENAME))		#"area_data.bin

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS		=	edgemarking area_map_tex area_map_ita area_map_itp area_mmdl_color

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb

#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
#Makeで生成される*.binはmake cleanの削除対象にする
CLEANFILES	= $(TARGETDIR)/$(TARGET_HEADER) $(TARGETDIR)/$(TARGET)
CLEANFILES	+=  $(TXTDATA)

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
LISTMK		= listmk.rb
NEWFILE	= make_new_file.exe
#TEXSET_MK	 = ruby mklst_texset.rb

#----------------------------------------------------------------------------
#	リスト作成時の読み込み元ファイル
#----------------------------------------------------------------------------
BUILD_LIST	=	build_list.txt
TEX_LIST	=	$(subst $(DQ),,$(ARC_TEX_LIST_FILENAME))
G_ANM_LIST	=	$(subst $(DQ),,$(ARC_ANM_LIST_FILENAME))

#----------------------------------------------------------------------------
#	中間ファイル
#----------------------------------------------------------------------------
#BUILD	=	build
#TEXSET	=	texset

#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
.PHONY:	do-build subdir clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
do-build:	area_map_tex.d  subdir $(TARGETDIR)/$(TARGET)
else
do-build:	subdir $(TARGETDIR)/$(TARGET)
endif

subdir:
	$(MAKE_SUBDIR)

$(TARGETDIR)/$(TARGET):	$(TARGET)
	$(COPY) $(TARGET) $(TARGETDIR)
	$(COPY) $(TARGET_HEADER) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
$(SRC_TXT_FILE):	$(SRC_XLS_FILE)
	#エクセルコンバート
	$(XLS2TAB) $(SRC_XLS_FILE) > $(SRC_TXT_FILE)

$(TARGET):	$(SRC_TXT_FILE)
	#エリアデータコンバート＋各種定義リスト生成
	ruby area.rb $(SRC_TXT_FILE)

area_map_tex.d: $(SRC_TXT_FILE)
	#地形モデルテクスチャコンバート用定義生成
	ruby area_map_tex.rb $(SRC_TXT_FILE)
endif



#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ削除
	-@rm $(TARGET) $(TARGET_HEADER)
	-@rm $(subst $(DQ),,$(TEX_DEPEND_FILES))
endif
	@$(MAKE_SUBDIR)
	-@rm $(CLEANFILES)

test:
	echo $(TEXSET_LIST)
	echo $(TEX_LIST)
