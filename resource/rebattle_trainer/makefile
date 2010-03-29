#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi masafumi_saitou nishino

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = rebattle_trainer
targetname2 = $(targetname).h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap/
TARGETDIR2 = $(PROJECT_ROOT)prog/src/field/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

##########################################
#コンバートファイル
##########################################
EXCELFILE = rebattle_trainer.xls

##########################################
#ターゲットファイル
##########################################

HEADER = $(PROJECT_ROOT)resource/trdata/trno_def.h
HEADER2 = $(PROJECT_ROOT)prog/arc/fieldmap/fldmmdl_objcode.h

LIST_FILE 	= list.txt


#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	

endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(TARGETDIR2)/$(targetname2)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

$(TARGETDIR2)/$(targetname2): $(targetname2)
	$(COPY) $(targetname2) $(TARGETDIR2)



#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
# コピー先のnarcが古い場合実行する
$(targetname).narc:$(EXCELFILE) $(HEADER) $(HEADER2)

	#エクセルをコンバート
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o out.tab -p 0 -s tab $(EXCELFILE)

	#ターゲットを生成
	perl conv.pl out.tab $(HEADER) $(targetname) $(HEADER2)
	-rm out.tab

	#アーカイブ
	-ls *.dat > $(LIST_FILE)
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

	rm -f $(TARGETDIR2)/$(targetname2)

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f $(targetname).narc
	rm -f $(targetname2)
endif



