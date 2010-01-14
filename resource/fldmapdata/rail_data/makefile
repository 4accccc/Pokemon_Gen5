#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi nakatsui takahashi obata_toshihiro #ohno #tamada 

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = field_rail_setup

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= rail scene

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
XLSFILE = rail_data.xls
XLS_CSV_FILE = rail_data.txt

DAT_FILE = $(wildcard *.bin)

#エクセルコンバーター
CSV_CVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

#naix
RAILDATA_NAIX = rail/field_rail_data.naix 
SCENEDATA_NAIX = scene/field_scenearea_data.naix 

LIST_FILE 	= list.txt

#------------------------------------------------------------------------------
#ヘッダ参照先
#------------------------------------------------------------------------------
HEADERDIR = $(PROJECT_PROGDIR)src/field/

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
$(targetname).narc: $(XLSFILE) $(RAILDATA_NAIX) $(SCENEDATA_NAIX)
	$(CSV_CVTR) -o $(XLS_CSV_FILE) -p 1 $(XLSFILE)
	perl conv.pl $(XLS_CSV_FILE) $(RAILDATA_NAIX) $(SCENEDATA_NAIX)
	
	-ls *.bin > $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)

else

$(targetname).narc: $(DAT_FILE)
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
	rm -f $(targetname).narc
	rm -f $(targetname).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.bin
endif




