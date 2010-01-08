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
BTL_SW_PD	= btl_subway_pd.narc
BTL_SW_TD	= btl_subway_td.narc

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

PDFILES:=$(wildcard bspd_*.bin)
PDFILES:=$(wildcard bstd_*.bin)

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(TARGETDIR)$(BTL_SW_PD) $(TARGETDIR)$(BTL_SW_TD)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(BTL_SW_PD) $(BTL_SW_TD) *.bin
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES: .bin

#パーソナルデータ生成
do-build: narc

narc:
$(BTL_SW_PD): $(PDFILES)
	nnsarc -c -l -n $(BTL_SW_PD) bspd_*.bin

$(BTL_SW_TD): $(TDFILES)
	nnsarc -c -l -n $(BTL_SW_TD) bstd_*.bin

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)$(BTL_SW_PD) $(TARGETDIR)$(BTL_SW_TD)

$(TARGETDIR)$(BTL_SW_PD):	$(BTL_SW_PD)
	$(COPY)	$(BTL_SW_PD) $(TARGETDIR)

$(TARGETDIR)$(BTL_SW_TD):	$(BTL_SW_TD)
	$(COPY)	$(BTL_SW_TD) $(TARGETDIR)
