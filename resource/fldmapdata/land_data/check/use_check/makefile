
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include

#------------------------------------------------------------------------------
#ツール
#------------------------------------------------------------------------------

XLS_CNV = ../../../../../tools/exceltool/xls2xml/tab_out_sheet.rb

#------------------------------------------------------------------------------
#読み込むエクセル
#------------------------------------------------------------------------------
MAP_LIST_XLS = ../../map_list.xls
MAP_REPLACE_XLS = ../../../map_replace/map_replace.xls
MAP_MATRIX_XLS = ../../../map_matrix/map_matrix.xls

.PHONY:	do-build clean



do-build: check

check:
	ruby $(XLS_CNV) -c -n 0 $(MAP_LIST_XLS) > 01.csv
	ruby $(XLS_CNV) -c -n 1 $(MAP_LIST_XLS) > 02.csv
	ruby $(XLS_CNV) -c -n 2 $(MAP_LIST_XLS) > 03.csv
	ruby $(XLS_CNV) -c -n 3 $(MAP_LIST_XLS) > 04.csv
	ruby $(XLS_CNV) -c -n 0 $(MAP_REPLACE_XLS) > rep.csv
	ruby $(XLS_CNV) -c -n 0 $(MAP_MATRIX_XLS) > mat.csv
	ruby chk_list.rb 01.csv 02.csv 03.csv 04.csv rep.csv mat.csv

delete:
	ruby file_del.rb result.txt ../../land_res/

clean:



