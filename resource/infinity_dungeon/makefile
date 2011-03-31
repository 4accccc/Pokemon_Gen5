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
POKEMON_DATA	= infinity_dungeon_pd.narc
TRAINER_DATA	= infinity_dungeon_td.narc

#------------------------------------------------------------------------------
#※認識文字列を書く（ファイル名に使用します）
#------------------------------------------------------------------------------
HEAD_STR = inf

#------------------------------------------------------------------------------
#※トレーナーデータ名
#------------------------------------------------------------------------------
TDM_FILE = inf_tr.tdm

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

PDFILES:=$(wildcard $(HEAD_STR)pd_*.bin)
TDFILES:=$(wildcard $(HEAD_STR)td_*.bin)

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
LDIRT_CLEAN = $(TARGETDIR)$(POKEMON_DATA) $(TARGETDIR)$(TRAINER_DATA)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(POKEMON_DATA) $(TRAINER_DATA) *.bin
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES: .bin

#パーソナルデータ生成
do-build: binmake narc

binmake:
	ruby ../../tools/btl_subway/bts_trainer_arr.rb ../../tools/btl_subway/trainer.txt > ../../tools/btl_subway/bts_tr_arr.rb
	ruby ../../tools/btl_subway/btl_subway_conv.rb pd_list.txt $(TDM_FILE) ../../resource/message/template.gmm $(HEAD_STR)

narc:
$(POKEMON_DATA): $(PDFILES)
	nnsarc -c -l -n $(POKEMON_DATA) $(HEAD_STR)pd_*.bin

$(TRAINER_DATA): $(TDFILES)
	nnsarc -c -l -n $(TRAINER_DATA) $(HEAD_STR)td_*.bin

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)$(POKEMON_DATA) $(TARGETDIR)$(TRAINER_DATA)

$(TARGETDIR)$(POKEMON_DATA):	$(POKEMON_DATA)
	$(COPY)	$(POKEMON_DATA) $(TARGETDIR)

$(TARGETDIR)$(TRAINER_DATA):	$(TRAINER_DATA)
	$(COPY)	$(TRAINER_DATA) $(TARGETDIR)
