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
targetname = entrance_camera

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#------------------------------------------------------------------------------
#共通ルールファイルのインクルード
#------------------------------------------------------------------------------
include	$(PROJECT_RSCDIR)\macro_define


#------------------------------------------------------------------------------
# マクロ定義
#------------------------------------------------------------------------------
XLS_TAB_CONVERTOR = ../../../tools/exceltool/xls2xml/tab_out_sheet.rb
BIN               = ./bin
EXITTYPE_DEF_FILE = ../../../prog/include/field/eventdata_type.h

CONVERT_DATA = $(BIN)/*.bin
DEBUG_DATA = $(BIN)/*.txt


ifeq ($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 
endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# コンバート
# タブ区切り
entrance_camera.txt: entrance_camera.xls
	ruby $(XLS_TAB_CONVERTOR) -n 0 $< > $@ 
endif

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: entrance_camera.txt
	$(COPY) $(EXITTYPE_DEF_FILE) ./
	ruby entrance_camera.rb $< $(BIN)
	nnsarc -r -l -n -i $(targetname) $(CONVERT_DATA)
	$(NAIXCUT) $(targetname).naix 
endif



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(CONVERT_DATA)
	rm -f $(DEBUG_DATA)
	rm -f *.txt
	rm -f ./eventdata_type.h
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)$(targetname).narc
	rm -f $(TARGETDIR)$(targetname).naix
