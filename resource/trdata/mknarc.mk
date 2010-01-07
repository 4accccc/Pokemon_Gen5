#------------------------------------------------------------------------------
#
#	トレーナーデータ用Makefile
#
#	2009.07.27	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname1	= trdata
NARCNAME1	= $(targetname1).narc
targetname2	= trpoke
NARCNAME2	= $(targetname2).narc

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

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
LDIRT_CLEAN = $(TARGETDIR)/$(NARCNAME1) $(TARGETDIR)/$(NARCNAME2)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += *.bin $(NARCNAME1) $(NARCNAME2)
endif

#コンバート対象のファイルツリーを生成
DATAFILES:=$(wildcard trdata_*.bin)
POKEFILES:=$(wildcard trpoke_*.bin)

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

.SUFFIXES: .bin

do-build: trdata.narc
trdata.narc: $(DATAFILES)
	nnsarc -c -l -n -i trdata.narc trdata_*.bin

do-build: trpoke.narc
trpoke.narc: $(POKEFILES)
	nnsarc -c -l -n -i trpoke.narc trpoke_*.bin

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME1)	$(TARGETDIR)/$(NARCNAME2)

$(TARGETDIR)/$(NARCNAME1):	$(NARCNAME1)
	$(COPY)	$(NARCNAME1) $(TARGETDIR)

$(TARGETDIR)/$(NARCNAME2):	$(NARCNAME2)
	$(COPY)	$(NARCNAME2) $(TARGETDIR)

