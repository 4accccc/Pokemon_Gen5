
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou unno

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

ARC_TARGET = arc_target_data

include $(ARC_TARGET)

XLS_DATA = fld3d_ci.xls
CSV_DATA = fld3d_ci.csv
FLD_CI_LIST = fld_ci.lst

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
do-build: $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname2).narc
	
clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
	@rm -f $(TARGETDIR)/$(targetname2).narc
	@rm -f $(TARGETDIR)/$(targetname2).naix

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f $(targetname2).narc
	@rm -f $(targetname2).naix
	@rm -f bin/*.bin
endif

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

$(CSV_DATA): $(XLS_DATA)
#	@echo __CVS CONVERT
	ruby $(XLS_CNV) -c $(XLS_DATA) > $(CSV_DATA)

$(FLD_CI_LIST): $(CSV_DATA)
#	@echo __NAKE ARC TARGET LST
	ruby make_arc_target_lst.rb $(CSV_DATA) $(FLD_CI_LIST)

$(ARC_TARGET): $(CSV_DATA)
#	@echo __NAKE ARC TARGET
	ruby make_arc_target.rb $(CSV_DATA) $(ARC_TARGET)

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(ARC_TARGET) $(ARC_TARGET_DATA)
#	@echo __NAKE TARGET1
	nnsarc -cl -l -n -i $(NARCNAME) -S $(FLD_CI_LIST) > arc_rsult.txt

$(targetname2).narc: $(CSV_DATA)
#	@echo __NAKE TARGET2
	ruby setupdat.rb $(CSV_DATA) $(FLD_CI_LIST)
	nnsarc -cl -l -n -i $(NARCNAME2) -S ci_list > arc_rsult2.txt

#3Dアニメコンバート
%.nsbmd: data/%.imd
	@g3dcvtr $<

%.nsbca: data/%.ica
	@g3dcvtr $<

%.nsbta: data/%.ita
	@g3dcvtr $<

%.nsbma: data/%.ima
	@g3dcvtr $<

%.nsbtp: data/%.itp
	@g3dcvtr $<

%.nsbva: data/%.iva
	@g3dcvtr $<	
	
endif

