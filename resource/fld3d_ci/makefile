
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= fld3d_ci
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

targetname2	= fld3d_ci_setup
NARCNAME2	= $(targetname2).narc
NAIXNAME2	= $(targetname2).naix

#ツールパス

XLS_CNV = ../../tools/exceltool/xls2xml/tab_out.rb

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
#ifeq ($(CONVERTUSER),true)
#SUBDIRS	= ../../../../pokemon_wb_doc/particle/battleeffect
#endif

LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix $(TARGETDIR)/$(targetname2).narc $(TARGETDIR)/$(targetname2).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt $(targetname2).narc $(targetname2).naix arc_result2.txt
endif


#コンバート対象のファイルツリーを生成

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname2).narc
	

#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

$(TARGETDIR)/$(targetname2).narc:  $(targetname2).narc 
	@echo copying $(targetname2).narc
	@$(COPY) $(targetname2).narc $(TARGETDIR)
	@$(COPY) $(targetname2).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
# コピー先のnarcが古い場合実行する
$(targetname).narc:
	@g3dcvtr test_cart.imd
	nnsarc -cl -l -n -i $(NARCNAME) -S fld_spa.lst > arc_rsult.txt

$(targetname2).narc: fld3d_ci.xls
	ruby $(XLS_CNV) -c fld3d_ci.xls > fld3d_ci.csv
	ruby setupdat.rb fld3d_ci.csv fld_spa.lst
	nnsarc -cl -l -n -i $(NARCNAME2) -S ci_list > arc_rsult2.txt
endif

