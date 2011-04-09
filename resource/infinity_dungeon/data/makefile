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
XLS_FILE = room_data.xls
POS_CSV_FILE = tr_move_type.csv
POS_FILE = infinity_dungeon_tr_move_type.cdat
ROOM_CSV_FILE = room_type.csv
ROOM_FILE = infinity_dungeon_room_pos_type.cdat
ROOM_DATA_CSV_FILE = room_data.csv
ROOM_DATA_FILE = infinity_dungeon_room_setup_data.cdat

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
LDIRT_CLEAN += $(POS_CSV_FILE) $(ROOM_CSV_FILE) $(POS_FILE) $(ROOM_FILE) $(ROOM_DATA_FILE)
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成
do-build: $(POS_CSV_FILE) $(ROOM_CSV_FILE) $(POS_FILE) $(ROOM_FILE) $(ROOM_DATA_FILE)

$(POS_CSV_FILE): $(XLS_FILE) trainer_move_type_cnv.rb
	@ruby $(EXCELCONV) $(XLS_FILE) -n 0 -s > $(POS_CSV_FILE)

$(ROOM_CSV_FILE): $(XLS_FILE) trainer_room_pos_type_cnv.rb
	@ruby $(EXCELCONV) $(XLS_FILE) -n 1 -s > $(ROOM_CSV_FILE)

$(ROOM_DATA_CSV_FILE): $(XLS_FILE) room_data_cnv.rb
	@ruby $(EXCELCONV) $(XLS_FILE) -n 2 -s > $(ROOM_DATA_CSV_FILE)

$(POS_FILE):$(POS_CSV_FILE)
	@ruby trainer_move_type_cnv.rb $(POS_FILE) $(POS_CSV_FILE)

$(ROOM_FILE):$(ROOM_CSV_FILE)
	@ruby trainer_room_pos_type_cnv.rb $(ROOM_FILE) $(ROOM_CSV_FILE)

$(ROOM_DATA_FILE):$(ROOM_DATA_CSV_FILE)
	@ruby room_data_cnv.rb $(ROOM_DATA_FILE) $(ROOM_DATA_CSV_FILE)

endif

