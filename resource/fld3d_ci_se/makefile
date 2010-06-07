
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou adachi_minako kageyama_shota unno

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
SE_DAT_FILE	= ci_se_tbl.cdat

#ツールパス
XLS_CNV = ../../tools/exceltool/xls2xml/tab_out.rb

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

SE_XLS_DATA = fld3d_ci_se.xls
SE_CSV_DATA = fld3d_ci_se.csv

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
#ifeq ($(CONVERTUSER),true)
#SUBDIRS	= ../../../../pokemon_wb_doc/particle/battleeffect
#endif

#LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix $(TARGETDIR)/$(targetname2).narc $(TARGETDIR)/$(targetname2).naix
#コンバート対象者のみ、中間ファイル削除
#ifeq	($(CONVERTUSER),true)	
#LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt $(targetname2).narc $(targetname2).naix arc_result2.txt
#endif


#コンバート対象のファイルツリーを生成

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(SE_DAT_FILE)
	
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(SE_DAT_FILE)
endif

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

$(SE_DAT_FILE): $(SE_CSV_DATA)
	ruby ci_se.rb $(SE_CSV_DATA) $(SE_DAT_FILE)

$(SE_CSV_DATA): $(SE_XLS_DATA)
	@echo __SE_CVS CONVERT
	ruby $(XLS_CNV) -c $(SE_XLS_DATA) > $(SE_CSV_DATA)
endif

