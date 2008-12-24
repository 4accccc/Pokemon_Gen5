#! make -f
#----------------------------------------------------------------------------
# Project:  テクスチャセットデータ作成
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
# 2008.12.18	WB環境へ移植
#
#----------------------------------------------------------------------------
#pkgsでは少し複雑なことをしているが、とりあえず単純に
#area_table.xlsから生成するべきimdとそれの元となるimdの関係を導き出す
#	imdをマージする。
#	imdをコンバートし、nsbtxを生成する
#	生成したnsbtxをまとめてnarcを生成する

#エリアコンバート共通定義ファイル
include ../area_common.def

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGET_ARC	=	area_map_tex.narc

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


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
DQ		="
#↑subst構文で"を使うとVimのコメント自動認識に狂いがでるので定義している

#DATABIN			= ../../../data/
#DATASRC			= $(DATABIN)rsc
TEXSETIMDDIR		= imd_files
NSBTXFILESDIR		= nsbtx_files
AREA_TABLE			= ../$(subst $(DQ),,$(SRC_TXT_FILENAME))
ARCLIST				= ../$(subst $(DQ),,$(ARC_TEX_LIST_FILENAME))

ARC_NSBTX_LIST		=	tex_arc_list.txt
CONVERT_TEX_LIST	=	g3list
#G3D_TEX_IMDの定義
#include g3list
-include $(CONVERT_TEX_LIST)

#DEPEND_TEX_FILESの定義
#	素材となる全imdが対象
include g3_texset
#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
#Makeで生成される*.binはmake cleanの削除対象にする
LDIRT_CLEAN	= $(NSBTXFILESDIR)/*.nsbtx

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
G3LISTMK	= g3listmk.rb
G3DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g3dcvtr.exe
TEXIMD_CMB = texset_cmb.rb
TEX_MAG = $(PROJECT_ROOT)tools/imdtools/tex_mag/tex_mag.exe

%.nsbtx: $(@:.nsbtx=.imd) $(subst $(TEXSETIMDDIR)/,$(NSBTXFILESDIR/),$@)
	$(G3DCVTR) $(@:.nsbtx=.imd) -o $(subst $(TEXSETIMDDIR)/,$(NSBTXFILESDIR)/,$@) -etex

.PHONY:	do-build clean
#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(TARGET_ARC)

$(TARGETDIR)/$(TARGET_ARC): $(TARGET_ARC)
	$(COPY) $(TARGET_ARC) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
$(TARGET_ARC): $(DEPEND_TEX_FILES) $(ARCLIST)
	$(MAKE) mkg3list			#NSBTX変換リスト
	$(MAKE) texset_cmb			#常駐|非常駐テクスチャセットマージ
	$(MAKE) imdconv				#IMD変換
	$(MAKE) texsetarc			#テクスチャセットアーカイブ
endif

#NSBTX変換用リスト作成
mkg3list:	$(CONVERT_TEX_LIST) $(ARC_NSBTX_LIST)

$(CONVERT_TEX_LIST): $(ARCLIST)
	#G3D_TEX_IMD定義ファイルの生成
	ruby $(G3LISTMK) $(ARCLIST) $(NSBTXFILESDIR)/ > $(CONVERT_TEX_LIST)
	sed -e 's/^"/"$(NSBTXFILESDIR)\//' $(ARCLIST) > $(ARC_NSBTX_LIST)

#常駐|非常駐テクスチャセットをマージ
texset_cmb:
	ruby $(TEXIMD_CMB) $(AREA_TABLE) $(TEXSETIMDDIR)/ $(TEX_MAG)

#IMDをNSBTXに変換
imdconv:$(G3D_TEX_IMD:.imd=.nsbtx)

#テクスチャセットのアーカイブ
texsetarc:
	#アーカイブにまとめるコンバート後テクスチャファイル名リストの生成
	ruby make_nnsarc_script.rb $(ARCLIST) $(NSBTXFILESDIR)/ > $(ARC_NSBTX_LIST)
	nnsarc -i -c -l -n $(TARGET_ARC) -S $(ARC_NSBTX_LIST)  > arc_result.txt
	#nnsarc -i -c -l -n $(TARGET_ARC) -S $(ARCLIST)  > arc_result.txt

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
clean:
	-rm $(LDIRT_CLEAN)
	-rm $(TARGETDIR)/$(TARGET_ARC)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
	-rm $(TARGET_ARC)
endif

