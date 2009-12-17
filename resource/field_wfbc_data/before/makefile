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
targetname0 = bcblock.mev
targetname1 = wfblock.mev

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_RSCDIR)fldmapdata/eventdata/data/

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
do-build:	subdir $(TARGETDIR)/$(targetname0) $(TARGETDIR)/$(targetname1)

subdir:
	@$(MAKE_SUBDIR)

$(TARGETDIR)/$(targetname0): $(targetname0)
	$(COPY) $(targetname0) $(TARGETDIR)

$(TARGETDIR)/$(targetname1): $(targetname1)
	$(COPY) $(targetname1) $(TARGETDIR)
	

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
$(targetname0): $(XLSFILE)
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 2 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_event_conv.pl $(TABFILE) $(targetname0) $(targetname1)

$(targetname1): $(XLSFILE)
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 2 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_event_conv.pl $(TABFILE) $(targetname0) $(targetname1)

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname0)
	rm -f $(TARGETDIR)/$(targetname1)

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.txt
	rm -f $(targetname0)
	rm -f $(targetname1)
endif



