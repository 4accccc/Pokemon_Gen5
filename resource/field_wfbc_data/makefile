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
XLSFILE = wb_wfbc.xls
TABFILE = wb_wfbc.txt



##########################################
#ターゲットファイル
##########################################
LIST_FILE 	= list.txt

OUTPUT_HEADER = field_wfbc_people_def.h
COPY_DIR	= $(PROJECT_PROGDIR)include/field/

##########################################
#参照ヘッダー
##########################################
OBJCODE_HEADER = ../fldmmdl/fldmmdl_objcode.h
TRTYPE_HEADER = ../trdata/trtype_def.h
MONSNO_HEADER = ../personal/monsno_def.h
MESSAGE_HEADER = ../message/dst/script/msg_bcwftest.h
SCRIPT_HEADER = ../fldmapdata/script/bcwftest_def.h
ITEM_HEADER = ../itemconv/itemsort_abc_def.h


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
$(targetname).narc: $(XLSFILE) $(OBJCODE_HEADER) $(TRTYPE_HEADER) $(MONSNO_HEADER) $(SCRIPT_HEADER) $(ITEM_HEADER)
	
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 0 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_people_conv.pl $(TABFILE) $(OBJCODE_HEADER) $(TRTYPE_HEADER) $(MONSNO_HEADER) $(SCRIPT_HEADER) $(ITEM_HEADER) $(OUTPUT_HEADER)

	-ls *.bin > $(LIST_FILE)
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
	rm -f *.bin
	rm -f *.txt
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif



