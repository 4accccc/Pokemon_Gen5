#------------------------------------------------------------------------------
#
#	配置モデルデータコンバート用Makefile
#
#	2009.04.26	tamada	エリア別リストデータの生成まで完了
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada lee_hyunjung

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
#targetname	= all_build_model
TARGET_OUTDOOR	= buildmodel_outdoor
NARC_OUTDOOR	= $(TARGET_OUTDOOR).narc
NAIX_OUTDOOR	= $(TARGET_OUTDOOR).naix

TARGET_INDOOR		= buildmodel_indoor
NARC_INDOOR	= $(TARGET_INDOOR).narc
NAIX_INDOOR	= $(TARGET_INDOOR).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

XLS2TAB	= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb


WKDIR				= wkdir
RESDIR		= nsbmdfiles
SRCDIR		= imdfiles
INDEXDIR	= indexfiles

XLS_DATA_OUTDOOR	= $(TARGET_OUTDOOR).xls
TXT_DATA_OUTDOOR	= $(WKDIR)/$(XLS_DATA_OUTDOOR:.xls=.txt)
DEP_LIST_OUTDOOR	= $(WKDIR)/depend_outdoor.txt
ARC_LIST_OUTDOOR	= $(WKDIR)/archive_outdoor.txt
INDEX_OUTDOOR			= $(INDEXDIR)/bmodel_index_outdoor.narc

XLS_DATA_INDOOR	= $(TARGET_INDOOR).xls
TXT_DATA_INDOOR	= $(WKDIR)/$(XLS_DATA_INDOOR:.xls=.txt)
DEP_LIST_INDOOR	= $(WKDIR)/depend_indoor.txt
ARC_LIST_INDOOR	= $(WKDIR)/archive_indoor.txt
INDEX_INDOOR			= $(INDEXDIR)/bmodel_index_indoor.narc

.PHONY:	do-build lists clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir
	$(MAKE) lists
	$(MAKE) arc
	$(MAKE) index

-include $(DEP_LIST_OUTDOOR)
-include $(DEP_LIST_INDOOR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.nsbmd .imd

#imdデータからの生成ルール
%.nsbmd:	../$(SRCDIR)/%.imd
	@echo $<をコンバート
	@g3dcvtr $< -o $@
endif


subdir:
	@$(MAKE_SUBDIR)

arc:	$(TARGETDIR)/$(NARC_OUTDOOR) $(TARGETDIR)/$(NARC_INDOOR)

lists:	$(DEP_LIST_OUTDOOR) $(DEP_LIST_INDOOR)

index: $(TARGETDIR)/$(INDEX_OUTDOOR) $(TARGETDIR)/$(INDEX_INDOOR)

$(TARGETDIR)/$(NARC_OUTDOOR):	$(NARC_OUTDOOR)
	@echo $(TARGETDIR)に$(NARC_OUTDOOR)をコピー
	@$(COPY)	$(NARC_OUTDOOR) $(TARGETDIR)
	@$(COPY) $(NAIX_OUTDOOR) $(TARGETDIR)

$(TARGETDIR)/$(NARC_INDOOR):	$(NARC_INDOOR)
	@echo $(TARGETDIR)に$(NARC_INDOOR)をコピー
	@$(COPY)	$(NARC_INDOOR) $(TARGETDIR)
	@$(COPY) $(NAIX_INDOOR) $(TARGETDIR)

$(NARC_OUTDOOR):	$(DEP_OUTDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(NARC_OUTDOOR) -S $(ARC_LIST_OUTDOOR) > $(WKDIR)/arc_result.txt
	@$(NAIXCUT) $(NAIX_OUTDOOR)

$(NARC_INDOOR):	$(DEP_INDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(NARC_INDOOR) -S $(ARC_LIST_INDOOR) > $(WKDIR)/arc_result.txt
	@$(NAIXCUT) $(NAIX_INDOOR)

$(TARGETDIR)/$(INDEX_OUTDOOR):	$(INDEX_OUTDOOR)
	@echo $(TARGETDIR)に$<をコピー
	@$(COPY) $(INDEX_OUTDOOR) $(TARGETDIR)/

$(TARGETDIR)/$(INDEX_INDOOR):	$(INDEX_INDOOR)
	@echo $(TARGETDIR)に$<をコピー
	@$(COPY) $(INDEX_INDOOR) $(TARGETDIR)/


#------------------------------------------------------------------------------
#	以下はコンバート対象者のみ動作する
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから

#
$(TXT_DATA_OUTDOOR):	$(XLS_DATA_OUTDOOR)
	@echo $<からリスト生成中
	@$(XLS2TAB) $(XLS_DATA_OUTDOOR) > $(TXT_DATA_OUTDOOR)

$(TXT_DATA_INDOOR):	$(XLS_DATA_INDOOR)
	@echo $<からリスト生成中
	@$(XLS2TAB) $(XLS_DATA_INDOOR) > $(TXT_DATA_INDOOR)

#
$(DEP_LIST_OUTDOOR):	$(TXT_DATA_OUTDOOR)
	@echo コンバート対象配置モデルのリストアップ中
	@ruby makelist.rb $(TXT_DATA_OUTDOOR) DEP_OUTDOOR $(DEP_LIST_OUTDOOR) $(ARC_LIST_OUTDOOR)

$(DEP_LIST_INDOOR):	$(TXT_DATA_INDOOR)
	@echo コンバート対象配置モデルのリストアップ中
	@ruby makelist.rb $(TXT_DATA_INDOOR) DEP_INDOOR $(DEP_LIST_INDOOR) $(ARC_LIST_INDOOR)

#
$(INDEX_OUTDOOR):		$(TXT_DATA_OUTDOOR)
	@echo エリア別配置モデルリスト（屋外）生成中
	@ruby makeindex.rb	$(TXT_DATA_OUTDOOR) out $(INDEXDIR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(INDEX_OUTDOOR) -S $(INDEXDIR)/out.list > $(WKDIR)/arc_result.txt

$(INDEX_INDOOR):		$(TXT_DATA_INDOOR)
	@echo エリア別配置モデルリスト（屋内）生成中
	@ruby makeindex.rb	$(TXT_DATA_INDOOR) in $(INDEXDIR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(INDEX_INDOOR) -S $(INDEXDIR)/in.list > $(WKDIR)/arc_result.txt

endif		#コンバート対象者のみ動作、ここまで

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	@-rm -f $(TARGETDIR)/$(NARC_OUTDOOR) $(TARGETDIR)/$(NAIX_OUTDOOR)
	@-rm -f $(TARGETDIR)/$(NARC_INDOOR) $(TARGETDIR)/$(NAIX_INDOOR)
	@-rm -f $(NARC_OUTDOOR) $(NAIX_OUTDOOR)
	@-rm -f $(NARC_INDOOR) $(NAIX_INDOOR)
	@-rm -f $(TARGETDIR)/$(INDEX_OUTDOOR) $(TARGETDIR)/$(INDEX_INDOOR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	@-rm -f $(DEP_OUTDOOR) $(DEP_INDOOR)
	@-rm -f $(WKDIR)/*.txt *.txt
	@-rm -f $(INDEXDIR)/*
endif


