#---------------------------------------------------------------
# Project:  ゾーンマトリクス作成
# File:     Makefile
# Copyright 2005 GameFreak.inc  All rights reserved.
#---------------------------------------------------------------
#マトリクスのヘッダーつくれ
#---------------------------------------------------------------
#	定義
#---------------------------------------------------------------
#コンバート対象ユーザー
#
USERS = kagaya tamada nakatsui tomoya_takahashi obata_toshihiro

#共通ルールファイルのインクルード
#
include $(PROJECT_RSCDIR)/macro_define

#Ｃソース→バイナリ変換を可能にするための設定
#
#include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
#include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
#include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules
#LINCLUDES	= $(NITROSDK_ROOT)/include
#LINCLUDES	+= $(dir $<)

#コピー先
#
TARGET_DIR		= $(PROJECT_ARCDIR)/fieldmap

#アーカイブデータ
#
TARGET_ARC = map_matrix.narc
TARGET_ARCIDX = map_matrix.naix

#マトリクス表
#
MAPMATRIX_XLS = map_matrix.xls
MAPMATRIX_TXT = map_matrix.txt
MAPBLOCKNUM_TXT = blocknum.txt

#アーカイブリスト
#
ARCLIST = arclist

#*.xls
#
XLS = *.xls

#*.bin
#
BINDATA = *.bin

#land data
#
LAND_DATA_XLS = ../land_data/map_list.xls
LAND_DATA_LIST_CSV	= field_list_for_mat.csv
LAND_NAIX_PATH  = $(PROJECT_RSCDIR)fldmapdata/land_data/map_land_data.naix

#テンポラリ
#
TMPDIR_PATH = $(PROJECT_RSCDIR)fldmapdata/map_matrix/tmp

#ゾーンID.h
#
ZONEID_H = ../zonetable/zone_id.h

#listmk.rb
#
ARCLIST_MK		= ruby listmk.rb

#mapmatcv.rb
#
MAPMATCV	= ruby mapmatcv.rb

#エクセルコンバータ
#
CSV_RUBY	= ruby $(PROJECT_ROOT)tools/exceltool/xls2xml/tab_out_sheet.rb

#PHONY
#
.PHONY:	do-build clean

#---------------------------------------------------------------
#	ルール定義
#---------------------------------------------------------------
do-build: $(TARGET_DIR)/$(TARGET_ARC)

$(TARGET_DIR)/$(TARGET_ARC): $(TARGET_ARC)
	$(COPY) $(TARGET_ARC) $(TARGET_DIR)
	$(COPY) $(TARGET_ARCIDX) $(TARGET_DIR)

ifeq	($(CONVERTUSER),true) #CONVERTUSER
$(TARGET_ARC): $(MAPMATRIX_XLS) $(ZONEID_H) $(LAND_DATA_XLS)
	$(MAKE) mklandlst			#ランドリスト作成
	$(MAKE) convxls				#エクセルコンバート
	$(MAKE) mklst				#一覧を作成
	$(MAKE) convert				#マトリクスコンバート
	$(MAKE) makearc				#アーカイブ
	$(NAIXCUT) $(TARGET_ARCIDX)
	
#ランドリスト作成
#
mklandlst:
	$(CSV_RUBY) --comma -n 0 $(LAND_DATA_XLS) > $(LAND_DATA_LIST_CSV)
	ruby make_land_list.rb $(LAND_DATA_LIST_CSV)

#エクセルコンバート
#
convxls:
	$(CSV_RUBY) -n 0 -tab map_matrix.xls > map_matrix.txt
	$(CSV_RUBY) -n 1 -tab map_matrix.xls > blocknum.txt
	$(CSV_RUBY) -n 2 -tab map_matrix.xls > field_land.txt
	$(CSV_RUBY) -n 3 -tab map_matrix.xls > field_zone.txt
	$(CSV_RUBY) -n 4 -tab map_matrix.xls > palace_zone.txt

#対象ソース一覧作成
#
mklst:
	$(ARCLIST_MK) $(MAPMATRIX_TXT)

#コンバート
#
convert:
	$(MAPMATCV) $(MAPMATRIX_TXT) $(MAPBLOCKNUM_TXT) $(LAND_NAIX_PATH) $(TMPDIR_PATH) $(ZONEID_H)

#アーカイブ作成
#
makearc:
	nnsarc -i -c -l -n $(TARGET_ARC) -S $(ARCLIST)  > arc_result.txt
endif #CONVERTUSER


#clean
#
clean:
	rm -f $(TARGET_DIR)/$(TARGET_ARC)
	rm -f $(TARGET_DIR)/$(TARGET_ARCIDX)
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	rm -f $(TARGET_ARC)
	rm -f $(TARGET_ARCIDX)
	rm -f $(TMPDIR_PATH)/$(BINDATA)
endif
