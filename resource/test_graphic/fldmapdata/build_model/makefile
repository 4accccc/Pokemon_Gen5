#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= all_build_model
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

EXCEL2TAB	= $(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe


XLS_DATA	= $(targetname).xls
DEP_LIST	= depend_list.txt
ARC_LIST	= archive_file.txt
RESDIR		= nsbmdfiles
SRCDIR		= imdfiles

-include $(DEP_LIST)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.nsbmd .imd

#imdデータからの生成ルール
%.nsbmd:	../$(SRCDIR)/%.imd
	g3dcvtr $< -o $@
endif


.PHONY:	do-build lists clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir
	$(MAKE) lists
	$(MAKE) arc

subdir:
	@$(MAKE_SUBDIR)

arc:	$(TARGETDIR)/$(NARCNAME)

lists:	$(DEP_LIST)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(DEPEND_FILES)
	nnsarc -c -l -n -i $(NARCNAME) -S $(ARC_LIST) > arc_result.txt
	$(NAIXCUT) $(NAIXNAME)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、リスト生成
$(DEP_LIST):	$(XLS_DATA)
	$(EXCEL2TAB) $(XLS_DATA)
	ruby makelist.rb $(XLS_DATA:.xls=.txt) $(DEP_LIST) $(ARC_LIST)
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
	-rm -f *.txt
endif


