#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
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


TMP				= tmp
RESDIR		= nsbmdfiles
SRCDIR		= imdfiles

XLS_DATA_OUTDOOR	= $(TARGET_OUTDOOR).xls
TXT_DATA_OUTDOOR	= $(TMP)/$(XLS_DATA_OUTDOOR:.xls=.txt)
DEP_LIST_OUTDOOR	= $(TMP)/depend_outdoor.txt
ARC_LIST_OUTDOOR	= $(TMP)/archive_outdoor.txt

XLS_DATA_INDOOR	= $(TARGET_INDOOR).xls
TXT_DATA_INDOOR	= $(TMP)/$(XLS_DATA_INDOOR:.xls=.txt)
DEP_LIST_INDOOR	= $(TMP)/depend_indoor.txt
ARC_LIST_INDOOR	= $(TMP)/archive_indoor.txt

.PHONY:	do-build lists clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir
	$(MAKE) lists
	$(MAKE) arc

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

$(TARGETDIR)/$(NARC_OUTDOOR):	$(NARC_OUTDOOR)
	@echo proc/arcに$(NARC_OUTDOOR)をコピー
	@$(COPY)	$(NARC_OUTDOOR) $(TARGETDIR)
	@$(COPY) $(NAIX_OUTDOOR) $(TARGETDIR)

$(TARGETDIR)/$(NARC_INDOOR):	$(NARC_INDOOR)
	@echo proc/arcに$(NARC_INDOOR)をコピー
	@$(COPY)	$(NARC_INDOOR) $(TARGETDIR)
	@$(COPY) $(NAIX_INDOOR) $(TARGETDIR)

$(NARC_OUTDOOR):	$(DEP_OUTDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(NARC_OUTDOOR) -S $(ARC_LIST_OUTDOOR) > $(TMP)/arc_result.txt
	@$(NAIXCUT) $(NAIX_OUTDOOR)

$(NARC_INDOOR):	$(DEP_INDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(NARC_INDOOR) -S $(ARC_LIST_INDOOR) > $(TMP)/arc_result.txt
	@$(NAIXCUT) $(NAIX_INDOOR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、リスト生成
$(DEP_LIST_OUTDOOR):	$(XLS_DATA_OUTDOOR)
	@echo $<からリスト生成中
	@$(XLS2TAB) $(XLS_DATA_OUTDOOR) > $(TXT_DATA_OUTDOOR)
	@ruby makelist.rb $(TXT_DATA_OUTDOOR) DEP_OUTDOOR $(DEP_LIST_OUTDOOR) $(ARC_LIST_OUTDOOR)

$(DEP_LIST_INDOOR):	$(XLS_DATA_INDOOR)
	@echo $<からリスト生成中
	@$(XLS2TAB) $(XLS_DATA_INDOOR) > $(TXT_DATA_INDOOR)
	@ruby makelist.rb $(TXT_DATA_INDOOR) DEP_INDOOR $(DEP_LIST_INDOOR) $(ARC_LIST_INDOOR)
endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	@-rm -f $(TARGETDIR)/$(NARC_OUTDOOR) $(TARGETDIR)/$(NAIX_OUTDOOR)
	@-rm -f $(TARGETDIR)/$(NARC_INDOOR) $(TARGETDIR)/$(NAIX_INDOOR)
	@-rm -f $(NARC_OUTDOOR) $(NAIX_OUTDOOR)
	@-rm -f $(NARC_INDOOR) $(NAIX_INDOOR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	@-rm -f $(DEP_OUTDOOR) $(DEP_INDOOR)
	@-rm -f $(TMP)/*.txt *.txt
endif


