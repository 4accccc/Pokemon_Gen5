#------------------------------------------------------------------------------
#
#	ポケモンパーソナル用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------

USERS	=	sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

NO_HEADER = trno_def.h
TYPE_HEADER = trtype_def.h
GENDER_HEADER = trtype_sex.h
NAME_GMMFILE = trname.gmm
TYPE_GMMFILE = trtype.gmm
SCRFILE = trfgra_wb.scr

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../../prog/include/tr_tool/
GMMDIR	= ../../resource/message/src/

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

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成

do-build: out_end

out_end: trainer_wb.csv
	ruby ../../tools/trdata_conv/trdata_conv.rb $< ../message/template.gmm
endif

do-build: $(TARGETDIR)$(NO_HEADER) $(TARGETDIR)$(TYPE_HEADER) $(TARGETDIR)$(GENDER_HEADER) $(GMMDIR)$(NAME_GMMFILE) $(GMMDIR)$(TYPE_GMMFILE)

$(TARGETDIR)$(NO_HEADER):	$(NO_HEADER)
	$(COPY)	$(NO_HEADER) $(TARGETDIR)

$(TARGETDIR)$(TYPE_HEADER):	$(TYPE_HEADER)
	$(COPY)	$(TYPE_HEADER) $(TARGETDIR)

$(TARGETDIR)$(GENDER_HEADER):	$(GENDER_HEADER)
	$(COPY)	$(GENDER_HEADER) $(TARGETDIR)

$(GMMDIR)$(NAME_GMMFILE):	$(NAME_GMMFILE)
	$(COPY)	$(NAME_GMMFILE) $(GMMDIR)

$(GMMDIR)$(TYPE_GMMFILE):	$(TYPE_GMMFILE)
	$(COPY)	$(TYPE_GMMFILE) $(GMMDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.s
	-rm -f $(NO_HEADER)
	-rm -f $(TYPE_HEADER)
	-rm -f $(GENDER_HEADER)
	-rm -f $(NAME_GMMFILE)
	-rm -f $(TYPE_GMMFILE)
	-rm -f *.scr
	-rm -f out_end
endif
	-rm -f $(TARGETDIR)$(NO_HEADER)
	-rm -f $(TARGETDIR)$(TYPE_HEADER)
	-rm -f $(TARGETDIR)$(GENDER_HEADER)
	-rm -f $(GMMDIR)$(NAME_GMMFILE)
	-rm -f $(GMMDIR)$(TYPE_GMMFILE)

