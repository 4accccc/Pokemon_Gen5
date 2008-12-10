#---------------------------------------------------------------
# Project:  ゾーンマトリクス作成
# File:     Makefile
# Copyright 2005 GameFreak.inc  All rights reserved.
#---------------------------------------------------------------

#---------------------------------------------------------------
#	共通ルールファイルのインクルード
#---------------------------------------------------------------
include $(PROJECT_RSCDIR)/macro_define

#---------------------------------------------------------------
#	ツールへのパス指定
#---------------------------------------------------------------
TARGET_DIR		= $(PROJECT_ARCDIR)/test_graphic
TARGET_ARC		= map_matrix.narc
TARGET_ARCIDX	= map_matrix.naix
MAPMATRIX_XLS	= map_matrix.xls
MAPMATRIXTXT	= map_matrix.txt
MAPBLOCKNUMTXT	= blocknum.txt
ARCLIST			= arclist
XLS				= *.xls
BINDATA			= *.bin
ENUM_HEADER		= map_matrix_id.h
FLDCSV_FOR_MAT	= field_list_for_mat.csv
LAND_XLS_DATA	= ../land_data/map_list.xls

LAND_NAIX_PATH  = $(PROJECT_RSCDIR)fldmapdata/land_data/map_land_data.naix
TMPDIR_PATH		= $(PROJECT_RSCDIR)fldmapdata/map_matrix/tmp

#---------------------------------------------------------------
#	Ｃソース→バイナリ変換を可能にするための設定
#---------------------------------------------------------------
include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#Makeで生成される*.binはmake cleanの削除対象にする
LDIRT_CLEAN	= $(TARGET_ARC) $(TARGET_ARCIDX) $(ENUM_HEADER) $(TMPDIR_PATH)/$(BINDATA) $(TARGET_DIR)/$(TARGET_ARC) $(TARGET_DIR)/$(TARGET_ARCIDX)

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

#---------------------------------------------------------------
#	ツールへのパス指定
#---------------------------------------------------------------
LISTMK		= listmk.rb
CSV_CVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

#---------------------------------------------------------------
#	ルール定義
#---------------------------------------------------------------
do-build: $(TARGET_DIR)/$(TARGET_ARC)

$(TARGET_DIR)/$(TARGET_ARC): $(XLS)
	$(MAKE) mklandlst			#ランドリスト作成
	$(MAKE) convxls				#エクセルコンバート
	$(MAKE) mklst				#一覧を作成
	$(MAKE) convert				#マトリクスコンバート
	$(MAKE) makearc				#アーカイブ
	$(COPY) $(TARGET_ARC) $(TARGET_DIR)
	$(COPY) $(TARGET_ARCIDX) $(TARGET_DIR)
	
#ランドリスト作成
mklandlst:
	$(CSV_CVTR) -o $(FLDCSV_FOR_MAT) -p 1 -s csv $(LAND_XLS_DATA)
	ruby make_land_list.rb $(FLDCSV_FOR_MAT)

#エクセルコンバート
convxls:
	$(CSV_CVTR) -p 1 map_matrix.xls
	$(CSV_CVTR) -o blocknum.txt -p 2 map_matrix.xls
	$(CSV_CVTR) -o field_zone.txt -p 3 map_matrix.xls
	$(CSV_CVTR) -o field_land.txt -p 4 map_matrix.xls
	$(CSV_CVTR) -o wb_spring.txt -p 5 map_matrix.xls
	$(CSV_CVTR) -o wb_summer.txt -p 6 map_matrix.xls
	$(CSV_CVTR) -o wb_autumn.txt -p 7 map_matrix.xls
	$(CSV_CVTR) -o wb_winter.txt -p 8 map_matrix.xls

#対象ソース一覧作成
mklst:
	ruby listmk.rb $(MAPMATRIXTXT)

#コンバート
convert:
	ruby mapmatcv.rb $(MAPMATRIXTXT) $(MAPBLOCKNUMTXT) $(LAND_NAIX_PATH) $(TMPDIR_PATH)

#アーカイブ作成
makearc:
	nnsarc -i -c -l -n $(TARGET_ARC) -S $(ARCLIST)  > arc_result.txt
