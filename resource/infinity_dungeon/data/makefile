#------------------------------------------------------------------------------
#
#	無限ダンジョン用トレーナー移動タイプコンバータ
#
#	2011.04.06	onoue_masayuki
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include ../user.def

#------------------------------------------------------------------------------
#　生成ファイル名
#------------------------------------------------------------------------------
TUTORIAL_XLS_FILE = tutorial_data.xls
XLS_FILE = room_data.xls
POS_CSV_FILE = tr_move_type.csv
TUTORIAL_CSV_FILE = tutorial_data.csv
POS_FILE = infinity_dungeon_tr_move_type.cdat
MOVE_TYPE_FILE = infinity_dungeon_tr_move_type.h
ROOM_CSV_FILE = room_type.csv
ROOM_FILE = infinity_dungeon_room_pos_type.cdat
ROOM_TYPE_HEADER_FILE = infinity_dungeon_room_pos_type.h
ROOM_DATA_CSV_FILE = room_data.csv
TUTORIAL_FILE = infinity_dungeon_tutorial_data.cdat
AREA_DATA_FILE = inf_area_data.cdat
FLOOR_NUM_DATA_FILE = inf_floor_num.h
FLOOR_SIZE_DATA_FILE = inf_floor_size.cdat

# エリアデータ
W_AREA_CSV_FILE = w_area_data_csv.csv
B_AREA_CSV_FILE = b_area_data_csv.csv

# トレーナー抽選データ
TR_XLS_FILE = tr_area_data.xls
TR_DATA_CSV_FILE = tr_area_data.csv
TR_DATA_FILE = infinity_dungeon_tr_area_data.cdat
TR_DATA_HEADER_FILE = infinity_dungeon_tr_area_data.h
#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

EXCELCONV = ../../../tools/exceltool/xls2xml/tab_out_direct_sheet.rb

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(POS_CSV_FILE) $(ROOM_CSV_FILE) $(ROOM_DATA_CSV_FILE) $(POS_FILE) $(ROOM_FILE) $(TR_DATA_CSV_FILE) $(TUTORIAL_FILE) $(TUTORIAL_CSV_FILE) $(W_AREA_CSV_FILE) $(B_AREA_CSV_FILE) $(AREA_DATA_FILE) $(FLOOR_NUM_DATA_FILE) $(FLOOR_SIZE_DATA_FILE)
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成
do-build: $(POS_CSV_FILE) $(ROOM_CSV_FILE) $(TR_DATA_CSV_FILE) $(ROOM_DATA_CSV_FILE) $(POS_FILE) $(ROOM_FILE) $(TR_DATA_FILE) $(TUTORIAL_FILE) $(W_AREA_CSV_FILE) $(B_AREA_CSV_FILE) $(AREA_DATA_FILE)

$(POS_CSV_FILE): $(XLS_FILE) trainer_move_type_cnv.rb
	@ruby $(EXCELCONV) $(XLS_FILE) -n 0 -s > $(POS_CSV_FILE)

$(TUTORIAL_CSV_FILE): $(TUTORIAL_XLS_FILE)
	@ruby $(EXCELCONV) $(TUTORIAL_XLS_FILE) -n 0 -s > $(TUTORIAL_CSV_FILE)

$(ROOM_CSV_FILE): $(XLS_FILE) trainer_room_pos_type_cnv.rb
	@ruby $(EXCELCONV) $(XLS_FILE) -n 1 -s > $(ROOM_CSV_FILE)

$(ROOM_DATA_CSV_FILE): $(XLS_FILE) 
	@ruby $(EXCELCONV) $(XLS_FILE) -n 2 -s > $(ROOM_DATA_CSV_FILE)

$(TR_DATA_CSV_FILE): $(TR_XLS_FILE) trainer_area_data_cnv.rb
	@ruby $(EXCELCONV) $(TR_XLS_FILE) -n 0 -s > $(TR_DATA_CSV_FILE)

$(W_AREA_CSV_FILE): $(XLS_FILE) 
	@ruby $(EXCELCONV) $(XLS_FILE) -n 2 -s > $(W_AREA_CSV_FILE)

$(B_AREA_CSV_FILE): $(XLS_FILE) 
	@ruby $(EXCELCONV) $(XLS_FILE) -n 3 -s > $(B_AREA_CSV_FILE)

$(TUTORIAL_FILE) : $(TUTORIAL_CSV_FILE) tutorial_data_cnv.rb
	@ruby tutorial_data_cnv.rb $(TUTORIAL_FILE) $(TUTORIAL_CSV_FILE)

$(POS_FILE):$(POS_CSV_FILE)
	@ruby trainer_move_type_cnv.rb $(POS_FILE) $(MOVE_TYPE_FILE) $(POS_CSV_FILE)

$(ROOM_FILE):$(ROOM_CSV_FILE)
	@ruby trainer_room_pos_type_cnv.rb $(ROOM_FILE) $(ROOM_TYPE_HEADER_FILE) $(ROOM_CSV_FILE)

$(AREA_DATA_FILE):$(W_AREA_CSV_FILE) $(B_AREA_CSV_FILE) area_data_cnv.rb
	@ruby area_data_cnv.rb $(W_AREA_CSV_FILE) $(B_AREA_CSV_FILE) $(AREA_DATA_FILE) $(FLOOR_NUM_DATA_FILE) $(FLOOR_SIZE_DATA_FILE)

$(TR_DATA_FILE):$(TR_DATA_CSV_FILE) trainer_area_data_cnv.rb
	@ruby trainer_area_data_cnv.rb $(TR_DATA_FILE) $(TR_DATA_HEADER_FILE) $(TR_DATA_CSV_FILE)

endif

