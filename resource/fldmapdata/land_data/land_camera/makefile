#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
include ../users_def

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
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


#情報
LIST_FILE 	= list.txt

#マップマトリックス
MAP_MATRIX = ../../map_matrix/map_matrix.xls
MAP_MATRIX_FILE = map_matrix.txt

#エクセルコンバーター
CSV_CVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

#imdフォルダ
IMD_FOLDER = ../land_res/
IMD_FILE = $(wildcard $(IMD_FOLDER)*.imd)
BIN_FILE = $(wildcard *.bin)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(targetname).txt

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).txt: Makefile $(IMD_FILE) $(MAP_MATRIX)

	$(CSV_CVTR) -o $(MAP_MATRIX_FILE) -p 1 $(MAP_MATRIX)
	perl conv.pl $(MAP_MATRIX_FILE) $(IMD_FOLDER) $(LIST_FILE)
	ls *.bin > $(targetname).txt

else

# コピー先のnarcが古い場合実行する
$(targetname).narc: Makefile $(BIN_FILE)

	ls *.bin > $(targetname).txt

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	-rm -r *.bin
	-rm -r *.bin.*
endif
	-rm -f $(targetname).narc
	-rm -f $(targetname).naix
	-rm -f $(targetname).txt


