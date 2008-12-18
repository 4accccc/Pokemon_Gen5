#! make -f
#----------------------------------------------------------------------------
# Project:  エリアデータ作成
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGET		=	area_data.bin
TARGET_ARC	=	area_data.narc

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS		=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

EXCEL2TAB	= $(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe


ARCLIST	=	area_list.txt
BUILDXLSLIST	=	build_xls_list.txt
TEX_IMD_LIST	= tex_imd_list.txt
ORGAREAXLS	= org_areaxxo.xls
ORGTEXSET	= org_tex_set.imd
XLS	=	*.xls
BINDATA	=	*.bin
TXTDATA	=	*.txt
ENUMHEADER	=	area_id.h

BUILD_DIR		:= area_build_model
AREABUILD_ARC	:= $(BUILD_DIR)/area_build.narc
TEXSET_DIR		:= area_map_tex
AREATEX_ARC		:= $(TEXSET_DIR)/map_tex_set.narc
TEXSET_LIST		:= $(TEXSET_DIR)/g3_texset
TEX_IMD_DIR		:= $(TEXSET_DIR)/imd_files

NSBTXFILES_DIR	= nsbtx_files

AREA_TABLE	= area_table.txt
AREABM_LIST = abmlst_files


#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
#Makeで生成される*.binはmake cleanの削除対象にする
#CLEANFILES	= $(TARGET_ARC) $(ENUMHEADER) $(BINDATA) $(TXTDATA) $(AREABUILD_ARC) $(AREATEX_ARC) $(AREABM_LIST)
CLEANFILES	=  $(AREABM_LIST) $(TXTDATA)
CLEANFILES	+= $(TARGETDIR)/$(ENUMHEADER) $(TARGETDIR)/$(TARGET)

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
LISTMK		= listmk.rb
NEWFILE	= make_new_file.exe
ABMLIST_MK	= ruby abmlstmk.rb
TEXSET_MK	 = ruby mklst_texset.rb

#----------------------------------------------------------------------------
#	リスト作成時の読み込み元ファイル
#----------------------------------------------------------------------------
BUILD_LIST	=	build_list.txt
TEX_LIST	=	tex_list.txt
G_ANM_LIST	=	g_anm_list.txt

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
#do-build: subdir $(TARGET_ARC) $(AREABM_LIST)
do-build:	subdir	$(TARGETDIR)/$(TARGET)

$(TARGETDIR)/$(TARGET):	$(TARGET)
	$(COPY)	$(TARGET) $(TARGETDIR)
	$(COPY) $(ENUMHEADER) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
$(TARGET):	area_table.xls
	#エクセルコンバート
	$(EXCEL2TAB) area_table.xls
	#$(MAKE) convert				#コンバート
	ruby area.rb $(AREA_TABLE)
endif


$(TARGET_ARC): $(XLS)
	#エクセルコンバート
	$(EXCEL2TAB) area_table.xls
	#$(MAKE) convert				#コンバート
	ruby area.rb $(AREA_TABLE)
	#$(MAKE) mkabmlst			#エリア配置モデルファイルリスト作成
	ruby abmlstmk.rb $(AREA_TABLE) $(AREABM_LIST)
#	$(MAKE) mktexsetlst			#テクスチャセット元データリスト作成
	#$(MAKE) makearc				#エリアデータアーカイブ
	#nnsarc -i -c -l -n $(TARGET_ARC) -S $(ARCLIST)  > arc_result.txt
	$(MAKE) addnew				#新規空データ追加
	$(MAKE) movelist			#リストファイル移動
	$(MAKE) -C $(BUILD_DIR)		#配置データアーカイブ
	$(MAKE) -C $(TEXSET_DIR)

#アーカイブ作成
makearc:
	nnsarc -i -c -l -n $(TARGET_ARC) -S $(ARCLIST)  > arc_result.txt

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

#area build list作成
mkabmlst:
	$(ABMLIST_MK) $(AREA_TABLE) $(AREABM_LIST)

#マップテクスチャセットリスト作成
mktexsetlst:
	$(TEXSET_MK) $(AREA_TABLE) $(TEXSET_LIST)

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
clean:
	-@rm $(CLEANFILES)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ削除
	-@rm $(TARGET) $(ENUMHEADER)
endif

