#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = namein_data

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC	= $(PROJECT_ARCDIR)

XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -t

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
KEYMAPXLS:=$(wildcard keymap_*.xls)
CHANGEXLS:=$(wildcard change_*.xls)

# リソースが吐き出すファイルを自動でつくる
KEYMAPDAT = $(subst .xls,.dat,$(KEYMAPXLS))
CHANGEDAT = $(subst .xls,.dat,$(CHANGEXLS))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# パターンを定義することで 依存関係をサフィックス以外でも自動で判断してくれる

keymap_%.dat: keymap_%.xls
	perl namein_keymap_conv.pl $<
#	ruby namein_keymap_conv.rb $< $(subst .xls,.dat,$<)

change_%.dat: change_%.xls
	perl namein_change_conv.pl $<

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_ARC)/$(targetname).narc $(TARGETDIR_ARC)/$(targetname).naix
subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetname).naix: $(targetname).naix
	$(COPY) $(targetname).naix $(TARGETDIR_ARC) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(KEYMAPDAT) $(CHANGEDAT)
	nnsarc -r -l -n -i $(targetname) -S arc_list.txt
	$(NAIXCUT) $(targetname).naix
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.dat
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR_ARC)/$(targetname).narc
	rm -f $(TARGETDIR_ARC)/$(targetname).naix


