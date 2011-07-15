#------------------------------------------------------------------------------
#
#	メダルデータ用Makefile
#
#	2011.01.7	onoue_masayuki
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	onoue_masayuki ito_hiroto


#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)
CONVERTERDIR	= ../converter/
CONVERTER			= achieve_table_cnv.rb
MDLRECID_H		= ../data/mdlrec_id.h
MDLRECID_TABLE		= mdlachieve_proc_table.cdat
PROC_TABLE_XLS		= proc_table.xls
PROC_TABLE_CSV		= proc_table.csv
EXCELCONV = ../../../tools/exceltool/ExcelSeetConv.exe

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(MDLRECID_TABLE)
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

do-build: $(MDLRECID_TABLE)

$(PROC_TABLE_CSV) : $(PROC_TABLE_XLS)
	@$(EXCELCONV) -o $(PROC_TABLE_CSV) -p 1 -s csv $(PROC_TABLE_XLS)

$(MDLRECID_TABLE) : $(MDLRECID_H) $(CONVERTERDIR)$(CONVERTER) $(PROC_TABLE_CSV)
	ruby $(CONVERTERDIR)$(CONVERTER) $(MDLRECID_H) $(PROC_TABLE_CSV) $(MDLRECID_TABLE)

endif

