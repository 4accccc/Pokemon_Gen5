#------------------------------------------------------------------------------
#  ショップデータを作るmakefile  
#  2008.10.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= mori tetsuji_ohta

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = shop_data

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ROOT)prog/src/field

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .xls

endif

.PHONY:	do-build clean subdir

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).cdat

subdir:
	@$(MAKE_SUBDIR)

# cdat のコピーだけ実行する
$(TARGETDIR)/$(targetname).cdat: $(targetname).cdat
	$(COPY) $(targetname).cdat $(TARGETDIR)
	$(COPY) bpshop.cdat $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).cdat: fs_item.xls
#	$(COPY) $(PROJECT_PROGDIR)include/item/itemsym.h ./itemsym.h
	ruby $(CSVCVTR) -c fs_item.xls >fs_item.csv
	ruby shopdata.rb fs_item.csv itemsym.h $(targetname).cdat $(targetname)_index.h
	ruby $(CSVCVTR) -c bp_item.xls >bp_item.csv
	ruby bpshopdata.rb bp_item.csv itemsym.h bpshop.cdat bpshop_index.h
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.cdat
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


