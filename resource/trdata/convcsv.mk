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
TYPE_HEADER = trtype_def.h
GENDER_HEADER = trtype_sex.h
NAME_GMMFILE = trname.gmm
TYPE_GMMFILE = trtype.gmm
HASH = trtype_hash.rb
SCRFILE = trfgra_wb.scr
LSTFILE = trfgra_wb.lst

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../../prog/include/tr_tool/
GMMDIR	= ../../resource/message/src/
GRADIR	= ../../resource/trgra/
HASHDIR	= ../../tools/hash/

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
LDIRT_CLEAN = $(TARGETDIR)$(NO_HEADER) $(TARGETDIR)$(TYPE_HEADER) $(TARGETDIR)$(GENDER_HEADER) $(GMMDIR)$(NAME_GMMFILE) $(GMMDIR)$(TYPE_GMMFILE) $(HASHDIR)$(HASH)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += *.s $(NO_HEADER) $(TYPE_HEADER) $(GENDER_HEADER) $(NAME_GMMFILE) $(TYPE_GMMFILE) $(HASH) *.scr out_end
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
	ruby ../../tools/trdata_conv/file_exist.rb $(LSTFILE) ../../../pokemon_wb_doc/trgra/
endif

do-build: $(TARGETDIR)$(NO_HEADER) $(TARGETDIR)$(TYPE_HEADER) $(TARGETDIR)$(GENDER_HEADER) $(GMMDIR)$(NAME_GMMFILE) $(GMMDIR)$(TYPE_GMMFILE) $(GRADIR)$(SCRFILE) $(HASHDIR)$(HASH)

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

$(GRADIR)$(SCRFILE):	$(SCRFILE)
	$(COPY)	$(SCRFILE) $(GRADIR)

$(HASHDIR)$(HASH):	$(HASH)
	$(COPY)	$(HASH) $(HASHDIR)

