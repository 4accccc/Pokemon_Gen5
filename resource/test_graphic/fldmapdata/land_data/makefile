#----------------------------------------------------------------------------
# Project:  地形関連データ->バイナリー>アーカイブ
# File:     Makefile
# Copyright 2005 GameFreak.inc  All rights reserved.
#----------------------------------------------------------------------------

#---------------------------------------------------------------
#	共通ルールファイルのインクルード
#---------------------------------------------------------------
include $(PROJECT_RSCDIR)/macro_define

#---------------------------------------------------------------
#	ツールへのパス指定
#---------------------------------------------------------------
LAND_LIST		= land_list
LAND_ARCLIST	= land_arc_list
TARGET_ARC		= map_land_data.narc
TARGET_ARCIDX	= map_land_data.naix
TARGET_DIR		= $(PROJECT_ARCDIR)/test_graphic
XLS_DATA		= map_list.xls
FLDCSV			= field_list.csv

DIR_G3DCVTR		= land_g3dcvtr/
DIR_3DPPACK		:= land_3dppack/
DIR_LANDRES		:= ../land_res/
DIR_MATRIX		:= ../map_matrix

#共通変数定義
include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#include land_list
include land_model_list
G3D_NSBMD = $(G3D_IMD:.imd=.nsbmd)
G3D_NSBTX = $(G3D_IMD:.imd=.nsbtx)
G3D_TARGET	= $(subst $(DIR_LANDRES)/,$(DIR_G3DCVTR),$(G3D_NSBMD)) $(subst $(DIR_LANDRES)/,$(DIR_G3DCVTR),$(G3D_NSBTX))
G3D_3DPP    = $(G3D_IMD:.imd=.3dppack)
G3D_3DPPACK = $(subst $(DIR_LANDRES)/,$(DIR_3DPPACK),$(G3D_3DPP))

#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
#アーカイブファイルをmake clean対象に設定
LDIRT_CLEAN	= $(LAND_LIST) 
LDIRT_CLEAN	= $(LAND_ARCLIST) $(TARGET_ARC) $(TARGET_ARC:.narc=.naix) $(DIR_G3DCVTR)/*.nsbmd $(DIR_G3DCVTR)/*.nsbtx $(DIR_3DPPACK)/*.3dppack $(TARGET_DIR)/$(TARGET_ARC) $(TARGET_DIR)/$(TARGET_ARCIDX)

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
G3DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g3dcvtr.exe
CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe
MK_GROUNDTOOL = mkmapdmy.rb

#----------------------------------------------------------------------------
#	生成ルール
#----------------------------------------------------------------------------
#nsbmd
%.nsbmd: ../$(DIR_LANDRES)/%.imd
	@echo create nsbmd $@
	$(G3DCVTR) $(subst $(DIR_G3DCVTR),$(DIR_LANDRES)/,$(@:.nsbmd=.imd)) -o $@

%.nsbtx: ../$(DIR_LANDRES)/%.imd
	@echo create nsbtx $@
	$(G3DCVTR) $(subst $(DIR_G3DCVTR),$(DIR_LANDRES)/,$(@:.nsbtx=.imd)) -o $@ -etex

#3dppack
%.3dppack: $(subst $(DIR_3DPPACK),$(DIR_LANDRES)/,$*.*)
	@echo create 3dppack $@
	$(BINLINKER) $(subst $(DIR_3DPPACK),$(DIR_G3DCVTR)/,$*.nsbmd) $(subst $(DIR_3DPPACK),$(DIR_G3DCVTR)/,$*.nsbtx) $(subst $(DIR_3DPPACK),$(DIR_G3DCVTR)/,$*.nsbtp) $(subst $(DIR_3DPPACK),$(DIR_LANDRES)/,$*.bin) $(subst $(DIR_3DPPACK),$(DIR_LANDRES)/,$*.3dmd) $*.3dppack 3D

#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
do-build: $(TARGET_DIR)/$(TARGET_ARC)

#リスト作成
$(LAND_ARCLIST): $(XLS_DATA)
	$(CSVCVTR) -o $(FLDCSV) -p 1 -s csv $(XLS_DATA)	
	ruby land_list.rb $(FLDCSV) 

#IMDをMSBNDに変換
imdconv: $(G3D_TARGET)

#データ結合
m3dppack: $(G3D_3DPPACK)

#アーカイブ
$(TARGET_DIR)/$(TARGET_ARC): $(CONCAT_TARGET) $(LAND_ARCLIST)
	$(MAKE) imdconv
	$(MAKE) m3dppack
	$(MAKE) makearc
	$(COPY) $(TARGET_ARC) $(TARGET_DIR)
	$(COPY) $(TARGET_ARCIDX) $(TARGET_DIR)

#アーカイブ作成
makearc:
	nnsarc -i -c -l -n $(TARGET_ARC) -S $(LAND_ARCLIST) > arc_result.txt
	
