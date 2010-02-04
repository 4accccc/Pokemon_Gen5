#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	taya sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname		= waza_tbl
NARCNAME			= $(targetname).narc
NAIXNAME			= $(targetname).naix
WAZANAME_GMM	= wazaname.gmm
WAZAINFO_GMM	= wazainfo.gmm
ATKMSG_GMM		= btl_attack.gmm
WAZANAME_HASH = wazano_hash.rb
WAZANO_DEF		= wazano_def.h
WAZALIST			= wazalist.txt

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/waza_tbl/
RUBYDIR	= ../../tools/waza_tbl/
WAZANAME_GMM_DIR	= ../../resource/message/src/
WAZAINFO_GMM_DIR	= ../../resource/message/src/
ATKMSG_GMM_DIR		= ../../resource/message/src/battle/
WAZANAME_HASH_DIR = ../../tools/hash/
WAZANO_DEF_DIR		= ../../prog/include/waza_tool/
WAZALIST_DIR			= ../../tools/btl_subway/

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
do-build: wazano_def.h

wazano_def.h: waza.tab
	ruby $(RUBYDIR)waza_tbl_conv.rb waza.tab tame_waza.csv ../../resource/message/template.gmm

$(NARCNAME): waza.tab wazaconv.pl
	perl wazaconv.pl waza.tab
	nnsarc -c -l -n -i $(NARCNAME) bin/*.bin > error
	$(NAIXCUT) $(NAIXNAME)

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(WAZANAME_GMM_DIR)$(WAZANAME_GMM) $(WAZAINFO_GMM_DIR)$(WAZAINFO_GMM) $(ATKMSG_GMM_DIR)$(ATKMSG_GMM) $(WAZANAME_HASH_DIR)$(WAZANAME_HASH) $(WAZANO_DEF_DIR)$(WAZANO_DEF) $(TARGETDIR)/$(NARCNAME) $(WAZALIST_DIR)$(WAZALIST)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)

$(WAZANAME_GMM_DIR)$(WAZANAME_GMM): $(WAZANAME_GMM)
	$(COPY)	$(WAZANAME_GMM) $(WAZANAME_GMM_DIR)

$(WAZAINFO_GMM_DIR)$(WAZAINFO_GMM): $(WAZAINFO_GMM)
	$(COPY)	$(WAZAINFO_GMM) $(WAZAINFO_GMM_DIR)

$(ATKMSG_GMM_DIR)$(ATKMSG_GMM): $(ATKMSG_GMM)
	$(COPY)	$(ATKMSG_GMM) $(ATKMSG_GMM_DIR)

$(WAZANAME_HASH_DIR)$(WAZANAME_HASH): $(WAZANAME_HASH)
	$(COPY)	$(WAZANAME_HASH) $(WAZANAME_HASH_DIR)

$(WAZANO_DEF_DIR)$(WAZANO_DEF): $(WAZANO_DEF)
	$(COPY)	$(WAZANO_DEF) $(WAZANO_DEF_DIR)

$(WAZALIST_DIR)$(WAZALIST): $(WAZALIST)
	$(COPY)	$(WAZALIST) $(WAZALIST_DIR)

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(TARGETDIR)/$(NARCNAME) $(WAZANAME_GMM_DIR)$(WAZANAME_GMM) $(WAZAINFO_GMM_DIR)$(WAZAINFO_GMM) $(ATKMSG_GMM_DIR)$(ATKMSG_GMM) $(WAZANAME_HASH_DIR)$(WAZANAME_HASH) $(WAZANO_DEF_DIR)$(WAZANO_DEF) $(WAZALIST_DIR)$(WAZALIST)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#LDIRT_CLEAN += $(NARCNAME) $(WAZANAME_GMM) $(WAZAINFO_GMM) $(ATKMSG_GMM) $(WAZANAME_HASH) $(WAZANO_DEF)
#NARCは現状生成していないのでCLEAN対象からは外しておく
LDIRT_CLEAN += $(WAZANAME_GMM) $(WAZAINFO_GMM) $(ATKMSG_GMM) $(WAZANAME_HASH) $(WAZANO_DEF) $(WAZALIST)
endif

