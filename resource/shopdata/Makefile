#------------------------------------------------------------------------------
#  ショップデータを作るmakefile  
#  2008.10.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= mori tetsuji_ohta iwao_kazumasa ichiraku_katsuhiko nagihashi_toru

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = shop_data
targetname_bp 	= bpshop.cdat
targetname_waza = waza_oshie.cdat

list_file = list.txt

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ROOT)prog/src/field
TARGETDIR_ARC = $(PROJECT_ARCDIR)

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
do-build:	subdir $(TARGETDIR_ARC)/$(targetname)_list.narc $(TARGETDIR_ARC)/$(targetname)_num.narc $(TARGETDIR)/$(targetname_bp) $(TARGETDIR)/$(targetname_waza)

subdir:
	@$(MAKE_SUBDIR)

# cdat のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname)_list.narc: $(targetname)_list.narc
	$(COPY) $(targetname)_list.narc $(TARGETDIR_ARC)
	$(COPY) $(targetname)_list.naix $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetname)_num.narc: $(targetname)_num.narc
	$(COPY) $(targetname)_num.narc $(TARGETDIR_ARC)
	$(COPY) $(targetname)_num.naix $(TARGETDIR_ARC)

$(TARGETDIR)/$(targetname_bp): $(targetname_bp)
	$(COPY) $(targetname_bp) $(TARGETDIR)

$(TARGETDIR)/$(targetname_waza): $(targetname_waza)
	$(COPY) $(targetname_waza) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname_bp): bp_item.xls
	$(COPY) $(PROJECT_PROGDIR)include/item/itemsym.h ./itemsym.h
	ruby $(CSVCVTR) -c bp_item.xls >bp_item.csv
	ruby bpshopdata.rb bp_item.csv itemsym.h $(targetname_bp) bpshop_index.h

$(targetname_waza): waza_oshie.xls
	ruby $(CSVCVTR) -c waza_oshie.xls >waza_oshie.csv
	ruby waza_oshie.rb waza_oshie.csv $(targetname_waza)

$(targetname)_list.narc $(targetname)_num.narc : fs_item.xls
	ruby $(CSVCVTR) -c fs_item.xls >fs_item.csv
	ruby shopdata.rb fs_item.csv itemsym.h $(targetname) $(targetname)_index.h
	find dat -name "*.dat" > $(list_file)
	perl $(PROJECT_ROOT)tools/arc_list_make.pl $(list_file)
	nnsarc -r -l -n -i $(targetname)_list -S $(list_file)
	nnsarc -r -l -n -i $(targetname)_num shop_data_num.dat

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.cdat
	rm -f *.dat
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f dat/*.dat
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


