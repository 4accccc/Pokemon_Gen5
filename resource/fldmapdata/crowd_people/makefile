#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi nakatsui

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = field_crowd_people

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

##########################################
#コンバートファイル
##########################################
XLSFILE = crowd_people.xls
TABFILE = crowd_people.txt

##########################################
#ターゲットファイル
##########################################
LIST_FILE 	= list.txt

LIST_BIN 	= list_data.dat
SCRIPT_BIN 	= script.dat

##########################################
#必要ヘッダー
##########################################
ZONEID_H = ../zonetable/zone_id.h
OBJCODE_H = ../../fldmmdl/fldmmdl_objcode.h
DIR_H = ../../../prog/include/field/field_dir.h
SCRIPT_H = ../script/crowd_people_scr_def.h

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
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(XLSFILE) $(ZONEID_H) $(OBJCODE_H) $(DIR_H) $(SCRIPT_H)
	
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 0 -s tab $(XLSFILE)
	#コンバート
	perl conv.pl $(TABFILE) $(OBJCODE_H) $(DIR_H) $(ZONEID_H) $(LIST_BIN) $(LIST_FILE) $(SCRIPT_H) $(SCRIPT_BIN)

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
	rm -f *.dat
	rm -f *.bin
	rm -f *.txt
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif




