#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	nishino sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TOKUSEI_HASH = tokusei_hash.rb
TOKUSYU_DEF	 = tokusyu_def.h
TOKUSEI_GMM	 = tokusei.gmm
TOKUSEIINFO_GMM	 = tokuseiinfo.gmm
TOKUSEI_LIST = speabi.txt

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
RUBYDIR	= ../../tools/tokusei/
TOKUSEI_HASH_DIR = ../../tools/hash/
TOKUSYU_DEF_DIR		= ../../prog/include/poke_tool/
TOKUSEI_GMM_DIR		= ../../resource/message/src/
TOKUSEI_LIST_DIR = ../../tools/btl_subway/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build: tokusyu_def.h

tokusyu_def.h: tokusei.csv
	ruby $(RUBYDIR)tokusei_conv.rb tokusei.csv ../../resource/message/template.gmm
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(TOKUSEI_HASH_DIR)$(TOKUSEI_HASH) $(TOKUSYU_DEF_DIR)$(TOKUSYU_DEF) $(TOKUSEI_GMM_DIR)$(TOKUSEI_GMM) $(TOKUSEI_GMM_DIR)$(TOKUSEIINFO_GMM) $(TOKUSEI_LIST_DIR)$(TOKUSEI_LIST)

$(TOKUSEI_HASH_DIR)$(TOKUSEI_HASH): $(TOKUSEI_HASH)
	$(COPY)	$(TOKUSEI_HASH) $(TOKUSEI_HASH_DIR)

$(TOKUSYU_DEF_DIR)$(TOKUSYU_DEF): $(TOKUSYU_DEF)
	$(COPY)	$(TOKUSYU_DEF) $(TOKUSYU_DEF_DIR)

$(TOKUSEI_GMM_DIR)$(TOKUSEI_GMM): $(TOKUSEI_GMM)
	$(COPY)	$(TOKUSEI_GMM) $(TOKUSEI_GMM_DIR)

$(TOKUSEI_GMM_DIR)$(TOKUSEIINFO_GMM): $(TOKUSEIINFO_GMM)
	$(COPY)	$(TOKUSEIINFO_GMM) $(TOKUSEI_GMM_DIR)

$(TOKUSEI_LIST_DIR)$(TOKUSEI_LIST): $(TOKUSEI_LIST)
	$(COPY)	$(TOKUSEI_LIST) $(TOKUSEI_LIST_DIR)

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(TOKUSEI_HASH_DIR)$(TOKUSEI_HASH) $(TOKUSYU_DEF_DIR)$(TOKUSYU_DEF) $(TOKUSEI_GMM_DIR)$(TOKUSEI_GMM) $(TOKUSEI_GMM_DIR)$(TOKUSEIINFO_GMM) $(TOKUSEI_LIST_DIR)$(TOKUSEI_LIST)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(TOKUSEI_HASH) $(TOKUSYU_DEF) $(TOKUSEI_GMM) $(TOKUSEIINFO_GMM) $(TOKUSEI_LIST)
endif

