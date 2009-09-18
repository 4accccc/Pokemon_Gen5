#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi nakatsui tamada fuchino

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = camera_scroll_original

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
XLSFILE = original_camera_scroll.xls
XLS_CSV_FILE = original_camera_scroll.txt
#エクセルコンバーター
CSV_CVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

LIST_FILE 	= list.txt

#------------------------------------------------------------------------------
#ヘッダ参照先
#------------------------------------------------------------------------------
HEADERDIR = $(PROJECT_PROGDIR)src/field/

HEADERDUMMY_TARGET = headerdummy.txt

#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	


.SUFFIXES:

endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(HEADERDUMMY_TARGET) $(targetname).narc 
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(XLSFILE) $(HEADERDUMMY_TARGET)
	$(CSV_CVTR) -o $(XLS_CSV_FILE) -p 1 $(XLSFILE)
	perl conv.pl $(XLS_CSV_FILE) $(HEADERDIR)field_camera.h
	-ls *.bin > $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)

$(HEADERDUMMY_TARGET):$(HEADERDIR)field_camera.h
	rm -f *.bin
	echo このファイルは、headerが更新されたときにbinを作り直すために用意されています。 > $(HEADERDUMMY_TARGET)
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.bin
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif




