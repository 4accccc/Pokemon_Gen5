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
targetname	= buildmodel_outdoor
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

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
XLS_DATA	= $(targetname).xls
TXT_DATA	= $(TMP)/$(XLS_DATA:.xls=.txt)
DEP_LIST	= $(TMP)/depend_list.txt
ARC_LIST	= $(TMP)/archive_file.txt
RESDIR		= nsbmdfiles
SRCDIR		= imdfiles

.PHONY:	do-build lists clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir
	$(MAKE) lists
	$(MAKE) arc

-include $(DEP_LIST)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.nsbmd .imd

#imdデータからの生成ルール
%.nsbmd:	../$(SRCDIR)/%.imd
	g3dcvtr $< -o $@
endif


subdir:
	@$(MAKE_SUBDIR)

arc:	$(TARGETDIR)/$(NARCNAME)

lists:	$(DEP_LIST)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(DEPEND_FILES)
	nnsarc -c -l -n -i $(NARCNAME) -S $(ARC_LIST) > $(TMP)/arc_result.txt
	$(NAIXCUT) $(NAIXNAME)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、リスト生成
$(DEP_LIST):	$(XLS_DATA)
	$(XLS2TAB) $(XLS_DATA) > $(TXT_DATA)
	ruby makelist.rb $(TXT_DATA) $(DEP_LIST) $(ARC_LIST)
endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	-rm -f $(DEPEND_FILES)
	-rm -f $(TMP)/*.txt *.txt
endif


