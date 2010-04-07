#------------------------------------------------------------------------------
#
#	戦闘背景テーブルデータコンバート用Makefile
#
#	2009.11.25	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

HEADER	= batt_bg_tbl.h
HASH		= zone_spec.rb
SCR			= battgra_3d_wb.scr

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
HEADER_DIR	= ../../prog/include/battle/
HASH_DIR		= ../../tools/hash/
RES_DIR			= ./batt_bg_tbl/

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

.PHONY:	do-build

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(HEADER_DIR)$(HEADER) $(HASH_DIR)$(HASH) $(SCR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(RES_DIR)$(HEADER) $(RES_DIR)$(HASH) $(RES_DIR)$(SCR) $(RES_DIR)out_end
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成

do-build: $(RES_DIR)out_end

$(RES_DIR)out_end: $(RES_DIR)wb_battle_bg.xls ../../tools/batt_bg_tbl/batt_bg_tbl_cnv.rb
	cd $(RES_DIR);ruby ../../../tools/exceltool/xls2xml/tab_out_direct.rb wb_battle_bg.xls -s > wb_battle_bg.csv
	cd $(RES_DIR);ruby ../../../tools/batt_bg_tbl/batt_bg_tbl_cnv.rb wb_battle_bg.csv ./
endif

do-build: $(HEADER_DIR)$(HEADER) $(HASH_DIR)$(HASH) $(SCR)

$(HEADER_DIR)$(HEADER):	$(RES_DIR)$(HEADER)
	$(COPY)	$(RES_DIR)$(HEADER) $(HEADER_DIR)

$(HASH_DIR)$(HASH):	$(RES_DIR)$(HASH)
	$(COPY)	$(RES_DIR)$(HASH) $(HASH_DIR)

$(SCR):	$(RES_DIR)$(SCR)
	$(COPY)	$(RES_DIR)$(SCR) ./

