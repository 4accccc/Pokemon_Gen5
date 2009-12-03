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
targetname0 = bc_block_event.mev
targetname1 = wf_block_event.mev
targetname2 = bc_block_event.evp
targetname3 = wf_block_event.evp

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_RSCDIR)fldmapdata/eventdata/data/
TARGETDIR2 = $(PROJECT_RSCDIR)field_wfbc_data/after/

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
do-build:	subdir $(TARGETDIR)/$(targetname0) $(TARGETDIR)/$(targetname1) $(TARGETDIR2)/$(targetname2) $(TARGETDIR2)/$(targetname3)

subdir:
	@$(MAKE_SUBDIR)

$(TARGETDIR)/$(targetname0): $(targetname0)
	$(COPY) $(targetname0) $(TARGETDIR)

$(TARGETDIR)/$(targetname1): $(targetname1)
	$(COPY) $(targetname1) $(TARGETDIR)

$(TARGETDIR2)/$(targetname2): $(targetname2)
	$(COPY) $(targetname2) $(TARGETDIR2)

$(TARGETDIR2)/$(targetname3): $(targetname3)
	$(COPY) $(targetname3) $(TARGETDIR2)
	

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
$(targetname0): $(XLSFILE)
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 2 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_event_conv.pl $(TABFILE) $(targetname0) $(targetname1) $(targetname2) $(targetname3)

$(targetname1): $(XLSFILE)
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 2 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_event_conv.pl $(TABFILE) $(targetname0) $(targetname1) $(targetname2) $(targetname3)

$(targetname2): $(XLSFILE)
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 2 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_event_conv.pl $(TABFILE) $(targetname0) $(targetname1) $(targetname2) $(targetname3)

$(targetname3): $(XLSFILE)
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $(TABFILE) -p 2 -s tab $(XLSFILE)
	#コンバート
	perl wfbc_event_conv.pl $(TABFILE) $(targetname0) $(targetname1) $(targetname2) $(targetname3)
	
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname0)
	rm -f $(TARGETDIR)/$(targetname1)
	rm -f $(TARGETDIR2)/$(targetname2)
	rm -f $(TARGETDIR2)/$(targetname3)

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.txt
	rm -f $(targetname0)
	rm -f $(targetname1)
	rm -f $(targetname2)
	rm -f $(targetname3)
endif



