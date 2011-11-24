#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda hiro_nakamura shimoyamada genya_hosaka tetsuji_ohta ito_hiroto iwasawa ichiraku_katsuhiko onoue_masayuki

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = item_data
targetname2 = item_battle_pocket

POCKET_DATA = item_pocket_data.cdat
ONLYONE_DATA = item_onlyone_flags.cdat

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
TARGETDIR_CDAT = $(PROJECT_PROGDIR)src/item

#------------------------------------------------------------------------------
#※gmm -> mstxt 変換関係
#------------------------------------------------------------------------------
UPDATEWITHGMM = ../../tools/message/UpdateWithGmm.bat
ITEMNAME_MSTXT = src/itemname.mstxt
ITEMINFO_MSTXT = src/iteminfo.mstxt
MSPROJDIR	= ../message/msproj/


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
CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe
RUBYDIR = $(PROJECT_ROOT)/tools/item/
HASHDIR	= $(PROJECT_ROOT)/tools/hash/

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(MSPROJDIR)$(ITEMNAME_MSTXT) $(MSPROJDIR)$(ITEMINFO_MSTXT) $(TARGETDIR_CDAT)/$(ONLYONE_DATA)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する	ハッシュテーブルのコピーもする
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) item_hash.rb $(HASHDIR)
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

$(TARGETDIR_CDAT)/$(ONLYONE_DATA):$(ONLYONE_DATA)
	$(COPY) $(ONLYONE_DATA) $(TARGETDIR_CDAT)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: wb_item.xls $(RUBYDIR)item_conv.rb
	$(COPY) $(PROJECT_PROGDIR)include/item/itemuse_def.h ./itemuse_def.h
	$(CSVCVTR) -o wb_item_sort_in.csv -p 1 -s csv wb_item.xls
	$(RUBYDIR)sort_remover.rb wb_item_sort_in.csv wb_item.csv swan_item_sort.csv
	$(RUBYDIR)item_conv.rb wb_item.csv ../message/template.gmm $(POCKET_DATA)
	itemconv.bat $(PROJECT_PROGDIR) $(PROJECT_ROOT)
	$(NAIXCUT) $(targetname).naix

$(MSPROJDIR)$(ITEMNAME_MSTXT) : itemname.gmm
	$(UPDATEWITHGMM) . itemname.gmm $(ITEMNAME_MSTXT)

$(MSPROJDIR)$(ITEMINFO_MSTXT) : iteminfo.gmm
	$(UPDATEWITHGMM) . iteminfo.gmm $(ITEMINFO_MSTXT)

endif

do-build:	subdir $(TARGETDIR)/$(targetname2).narc

# narc のコピーだけ実行する	ハッシュテーブルのコピーもする
$(TARGETDIR)/$(targetname2).narc: $(targetname2).narc
	$(COPY) $(targetname2).narc $(TARGETDIR)


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(ONLYONE_DATA)
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(PROJECT_PROGDIR)src/app/bag/itemsort_absolute_def.cdat 
	rm -f $(PROJECT_PROGDIR)src/app/bag/itemsort_jp_def.cdat 
	rm -f $(TARGETDIR_CDAT)/$(ONLYONE_DATA)

