#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi ohmori

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = field_wfbc_data

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

##########################################
#コンバートファイル
##########################################
XLSFILE = ../wb_wfbc.xls
TABFILE = wb_wfbc.txt
BLOCK_WF_FILE = wb_wfbc_block_wf.txt
BLOCK_BC_FILE = wb_wfbc_block_bc.txt
POS_WF_FILE = wb_wfbc_pos_wf.txt
POS_BC_FILE = wb_wfbc_pos_bc.txt
CAMERA_FILE = wb_wfbc_camera.txt
EVENT_FILE = wb_wfbc_event.txt

BLOCK_WF_BIN = wb_wfbc_block_wf.block
BLOCK_BC_BIN = wb_wfbc_block_bc.block
POS_WF_BIN = wb_wfbc_block_wf.pos
POS_BC_BIN = wb_wfbc_block_bc.pos
CAMERA_BIN = wb_wfbc_camera.cam

EVENT_BC_PARA = bc_block_event.evp
EVENT_WF_PARA = wf_block_event.evp


#ブロックヘッダー情報
BLOCK_WF_HEADER = wb_wfbc_block_wf.h
BLOCK_BC_HEADER = wb_wfbc_block_bc.h



##########################################
#ターゲットファイル
##########################################
LIST_FILE 	= list.txt

OUTPUT_HEADER = field_wfbc_people_def.h
COPY_DIR	= $(PROJECT_PROGDIR)include/field/

##########################################
#参照ヘッダー
##########################################
OBJCODE_HEADER = ../../fldmmdl/fldmmdl_objcode.h
TRTYPE_HEADER = ../../trdata/trtype_def.h
MONSNO_HEADER = ../../personal/monsno_def.h
MESSAGE_HEADER = ../../message/dst/script/msg_bcwftest.h
SCRIPT_BC_HEADER = ../../fldmapdata/script/bc10_def.h
SCRIPT_WF_HEADER = ../../fldmapdata/script/wc10_def.h
ITEM_HEADER = ../../itemconv/itemsort_abc_def.h
TRDATA_HEADER = ../../trdata/trno_def.h

LAND_DATA_PATCH = ../../fldmapdata/land_data_patch/land_data_patch_def.h
EVENT_BC_HEADER = ../../fldmapdata/eventdata/tmp/bc10.h
EVENT_WF_HEADER = ../../fldmapdata/eventdata/tmp/wc10.h


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
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(COPY_DIR)$(OUTPUT_HEADER)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)
	
$(COPY_DIR)$(OUTPUT_HEADER): $(OUTPUT_HEADER)
	$(COPY) $(OUTPUT_HEADER) $(COPY_DIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(XLSFILE) $(OBJCODE_HEADER) $(TRTYPE_HEADER) $(MONSNO_HEADER) $(SCRIPT_BC_HEADER) $(SCRIPT_WF_HEADER) $(ITEM_HEADER) $(TRDATA_HEADER) $(EVENT_BC_HEADER) $(EVENT_WF_HEADER)
	
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 1 -s tab $(XLSFILE)
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(BLOCK_WF_FILE) -p 4 -s tab $(XLSFILE)
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(BLOCK_BC_FILE) -p 3 -s tab $(XLSFILE)
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(POS_WF_FILE) -p 6 -s tab $(XLSFILE)
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(POS_BC_FILE) -p 5 -s tab $(XLSFILE)
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(CAMERA_FILE) -p 7 -s tab $(XLSFILE)
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(EVENT_FILE) -p 2 -s tab $(XLSFILE)


	#コンバート
	perl wfbc_people_conv.pl $(TABFILE) $(OBJCODE_HEADER) $(TRTYPE_HEADER) $(MONSNO_HEADER) $(SCRIPT_BC_HEADER) $(SCRIPT_WF_HEADER) $(ITEM_HEADER) $(TRDATA_HEADER) $(OUTPUT_HEADER)

	perl wfbc_block_conv.pl $(BLOCK_WF_FILE) $(LAND_DATA_PATCH)  $(BLOCK_WF_BIN)
	perl wfbc_block_conv.pl $(BLOCK_BC_FILE) $(LAND_DATA_PATCH)  $(BLOCK_BC_BIN)

	perl wfbc_peoplepos_conv.pl $(POS_WF_FILE) $(POS_WF_BIN)
	perl wfbc_peoplepos_conv.pl $(POS_BC_FILE) $(POS_BC_BIN)

	perl wfbc_camera_conv.pl $(CAMERA_FILE) $(CAMERA_BIN)

	perl wfbc_event_conv.pl $(EVENT_FILE) $(LAND_DATA_PATCH) $(BLOCK_BC_HEADER) $(BLOCK_WF_HEADER) $(EVENT_BC_HEADER) $(EVENT_WF_HEADER) $(EVENT_BC_PARA) $(EVENT_WF_PARA)

	

	-ls *.bin > $(LIST_FILE)
	-ls *.block >> $(LIST_FILE)
	-ls *.evp >> $(LIST_FILE)
	-ls *.pos >> $(LIST_FILE)
	-ls *.cam >> $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)
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
	rm -f *.pos
	rm -f *.block
	rm -f *.bin
	rm -f *.txt
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif



