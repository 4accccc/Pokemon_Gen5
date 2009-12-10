#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = fld_trade_poke

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#コピーしてくるディレクトリ
MONSNO_H_DIR		= ..\\personal\\
ITEMSYM_H_DIR		= ..\\..\\prog\\include\\item\\
TOKUSYU_H_DIR		= ..\\..\\prog\\include\\poke_tool\\
PM_VER_H_DIR		= ..\\..\\prog\\include\\

#コピー先
CPY_DIR	= .

#依存するファイル
MAKE_FILE	= fld_trade_poke.xls \
			      monsno_def.h \
			      itemsym.h \
			      tokusyu_def.h \
			      pm_version.h

#ターゲットファイル
MAKE_TARGET = fld_trade_00.dat
DAT_FILE	= $(wildcard *.dat)

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir file_cp $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

#ファイルコピー
file_cp:
	xcopy /d /y $(MONSNO_H_DIR)monsno_def.h $(CPY_DIR)
	xcopy /d /y $(ITEMSYM_H_DIR)itemsym.h $(CPY_DIR)
	xcopy /d /y $(TOKUSYU_H_DIR)tokusyu_def.h $(CPY_DIR)
	xcopy /d /y $(PM_VER_H_DIR)pm_version.h $(CPY_DIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(MAKE_FILE)
	perl conv.pl
	nnsarc -c -i -n $(targetname).narc -S fld_trade_list.lst
	$(NAIXCUT) $(targetname).naix

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(CONVERTDATA)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


