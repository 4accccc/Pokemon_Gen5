#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = camera_scroll

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)/fieldmap/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= original

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


#情報
LIST_FILE 	= list.txt


#binフォルダ
BINDIR = ../land_data/land_camera/
BIN2DIR = original/

BINDATA = $(wildcard $(BINDIR)*.bin)
BIN2DATA = $(wildcard $(BIN2DIR)*.bin)

BINLIST = $(BINDIR)$(LIST_FILE)

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


# コピー先のnarcが古い場合実行する
$(targetname).narc: Makefile $(BINDATA) $(BIN2DATA) $(BINLIST)
	$(COPY) $(BINDIR)*.bin ./
	$(COPY) $(BINLIST) ./

	-ls original/*.bin >> $(LIST_FILE)
	perl list_make.pl $(LIST_FILE)
	nnsarc -r -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	rm -f *.bin
	rm -f $(LIST_FILE)

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


