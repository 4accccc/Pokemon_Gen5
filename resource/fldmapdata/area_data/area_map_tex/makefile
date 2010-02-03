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
#pkgsでは少し複雑なことをしているが、とりあえず単純に...
#上位のMakefileが、area_table.xlsから生成するべきテクスチャと元となるimdの関係を導き出す
#	imdをマージする。
#	imdをコンバートし、nsbtxを生成する
#	生成したnsbtxをまとめてnarcを生成する

#エリアコンバート共通定義ファイル
include ../area_common.def

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada nakatsui fuchino lee_hyunjung fujiwara_maiko ibe_mana tanoue_reiko \
				ohkubo_tomohiko unno james_turner obata_toshihiro tomoya_takahashi

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
NARCNAME	=	area_map_tex.narc
NAIXNAME	=	area_map_tex.naix

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

#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
#Makeで生成される*.binはmake cleanの削除対象にする
CLEANFILES	= $(subst $(DQ),,$(NSBTXFILESDIR)/*.nsbtx $(MARGED_TEXIMD_DIR)/*.imd)

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
G3DCVTR	= g3dcvtr.exe
#G3DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g3dcvtr.exe
TEX_MAG = $(PROJECT_ROOT)tools/imdtools/tex_mag/tex_mag.exe

#----------------------------------------------------------------------------
#	アーカイブのみ行う場合の依存関係
#----------------------------------------------------------------------------
NSBTXFILES = $(wildcard $(subst $(DQ),,$(NSBTXFILESDIR))/*.nsbtx)

.PHONY:	do-build clean
#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME): $(NARCNAME)
	$(COPY) $(NARCNAME) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成

# source imd --> marged imd --> nsbtxの依存関係＆コンパイルルール定義
NSBTX_DEPEND_FILE	= $(subst $(DQ),,$(NSBTX_DEPEND_FILENAME))
-include ../$(NSBTX_DEPEND_FILE)

# NNSARC_DEPEND_FILES（nsbtx --> narcの依存関係）定義
NNSARC_DEPEND_FILE	= $(subst $(DQ),,$(NNSARC_DEPEND_FILENAME))
-include ../$(NNSARC_DEPEND_FILE)

# nnsarcで固めるデータ指定リスト
NNSARC_LIST_FILE	= ../$(subst $(DQ),,$(NNSARC_TEXLIST_FILENAME))

$(NARCNAME): $(NNSARC_DEPEND_FILES)
	nnsarc -i -c -l -n $(NARCNAME) -S $(NNSARC_LIST_FILE)  > arc_result.txt

else	#その他の人はアーカイブのみ

# nnsarcで固めるデータ指定リスト
NNSARC_LIST_FILE	= ../$(subst $(DQ),,$(NNSARC_TEXLIST_FILENAME))

#アーカイブ
$(NARCNAME): $(NNSARC_LIST_FILE) $(NSBTXFILES)
	nnsarc -i -c -l -n $(NARCNAME) -S $(NNSARC_LIST_FILE)  > arc_result.txt

endif

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
clean:
	@-rm $(TARGETDIR)/$(NARCNAME)
	@-rm $(NARCNAME) $(NAIXNAME)
	@-rm arc_result.txt
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
	@-rm $(CLEANFILES)
endif

