#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※USERS　などをインクルード
#------------------------------------------------------------------------------
include ../mkdefine

USERS +=  tomoya_takahashi

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = tpoke_data

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
CONV_EXCEL = ../$(FMMDLLIST_XLS)

OBJCODE_HEADER = ../fldmmdl_objcode.h
SYMBOL_HEADER = ../fldmmdl_list_symbol.h
PMVERSION_HEADER = $(PROJECT_ROOT)prog/include/pm_version.h


##########################################
#ターゲットファイル
##########################################
#DAT

LIST_FILE 	= list.txt




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
$(targetname).narc: $(CONV_EXCEL) $(OBJCODE_HEADER) $(SYMBOL_HEADER) $(PMVERSION_HEADER)

	

	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o tmp.tab -p 2 -s tab $(CONV_EXCEL)

	perl conv.pl tpoke.dat tmp.tab $(OBJCODE_HEADER) $(PMVERSION_HEADER) $(SYMBOL_HEADER)

	-ls *.dat > $(LIST_FILE)

	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)
	-rm tmp.tab
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
	rm -f *.tab
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif



