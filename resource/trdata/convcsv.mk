#------------------------------------------------------------------------------
#
#	ポケモンパーソナル用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

NO_HEADER = trno_def.h
NAME_GMMFILE = trname.gmm

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../../prog/include/tr_tool/
GMMDIR	= ../../resource/message/src/
GRADIR	= ../../resource/trgra/
GRAHEADERDIR	= ../../prog/src/system/

ALL_TARGETS = $(TARGETDIR)$(NO_HEADER) $(GMMDIR)$(NAME_GMMFILE)
LOCAL_TARGETS = $(notdir $(ALL_TARGETS))
#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(ALL_TARGETS)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += *.s *.scr out_end $(LOCAL_TARGETS)
endif

.PHONY:	do-build

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成

do-build: out_end

out_end: trainer_wb.csv
	ruby ../../tools/trdata_conv/trdata_conv.rb $< ../message/template.gmm
	ruby breeder_list.rb trno_def.h > breeder_id_list.cdat
endif

do-build: $(ALL_TARGETS)

$(TARGETDIR)$(NO_HEADER):	$(NO_HEADER)
	$(COPY)	$(NO_HEADER) $(TARGETDIR)


$(GMMDIR)$(NAME_GMMFILE):	$(NAME_GMMFILE)
	$(COPY)	$(NAME_GMMFILE) $(GMMDIR)

clean:
	@rm -f $(LDIRT_CLEAN)
