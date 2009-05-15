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
USERS	=	tamada nakatsui fuchino lee_hyunjung

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGET		=	$(subst $(DQ),,$(TARGET_BIN_FILENAME))		#"area_data.bin
TARGET_ARC	=	area_data.narc

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS		=	area_map_tex area_map_ita area_map_itp

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#EXCEL2TAB	= $(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe
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
do-build:	$(TARGETDIR)/$(TARGET) subdir

subdir:
	$(MAKE_SUBDIR)

$(TARGETDIR)/$(TARGET):	$(TARGET)
	$(COPY) $(TARGET) $(TARGETDIR)
	$(COPY) $(TARGET_HEADER) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
$(TARGET):	$(SRC_XLS_FILE)
	#エクセルコンバート
	#$(EXCEL2TAB) $(SRC_XLS_FILE)
	$(XLS2TAB) $(SRC_XLS_FILE) > $(SRC_TXT_FILE)
	#エリアデータコンバート＋各種定義リスト生成
	ruby area.rb $(SRC_TXT_FILE)
	#地形モデルテクスチャコンバート用定義生成
	ruby area_map_tex.rb $(SRC_TXT_FILE)
endif


$(TARGET_ARC): $(XLS)
	#エクセルコンバート
	#$(EXCEL2TAB) $(SRC_XLS_FILE)
	$(XLS2TAB) $(SRC_XLS_FILE) > $(SRC_TXT_FILE)
	#$(MAKE) convert				#コンバート
	ruby area.rb $(SRC_TXT_FILE)
	$(MAKE) mktexsetlst			#テクスチャセット元データリスト作成
	$(MAKE) addnew				#新規空データ追加
	$(MAKE) movelist			#リストファイル移動
	$(MAKE) -C $(BUILD_DIR)		#配置データアーカイブ
	#$(MAKE) -C $(TEXSET_DIR)

#新規に空データをディレクトリに追加
addnew:
	$(NEWFILE) $(BUILDXLSLIST) $(BUILD_DIR) $(ORGAREAXLS)
	$(NEWFILE) $(TEX_IMD_LIST) $(TEX_IMD_DIR) $(ORGTEXSET)

#リストファイル移動
movelist:
	ruby $(LISTMK) $(BUILD_LIST) tmp $(BUILD_DIR)/$(BUILD_LIST)
	#-rm $(BUILD_LIST)
#	ruby $(LISTMK) $(TEX_LIST) $(NSBTXFILES_DIR) $(TEXSET_DIR)/$(TEX_LIST)
	#-rm $(TEX_LIST)
	ruby $(LISTMK) $(G_ANM_LIST) g_nsbta area_ground_anime/$(G_ANM_LIST)
	-rm $(G_ANM_LIST) 

#マップテクスチャセットリスト作成
mktexsetlst:
	$(TEXSET_MK) $(SRC_TXT_FILE) $(TEXSET_LIST)

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
