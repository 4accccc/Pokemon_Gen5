#----------------------------------------------------------------------------
# Project:  かいりきで押す岩が参照、保存する座標データを作成
# File:     makefile
#----------------------------------------------------------------------------
#---------------------------------------------------------------
#	共通ルールファイルのインクルード
#---------------------------------------------------------------
include $(PROJECT_RSCDIR)/macro_define

#---------------------------------------------------------------
#	各要素
#---------------------------------------------------------------
TARGET = rockpos.cdat

XLS_ROCKPOS = rockpos.xls
CSV_ROCKPOS = rockpos.csv

ZONEID_HEADER = $(PROJECT_RSCDIR)/fldmapdata/zonetable/zone_id.h
OBJID_HEADER = $(PROJECT_RSCDIR)/fldmapdata/eventdata/total_header.h

CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

#---------------------------------------------------------------
#	PHONY
#---------------------------------------------------------------
.PHONY: do-build clean

#---------------------------------------------------------------
#	make
#---------------------------------------------------------------
do-build: $(TARGET)

$(TARGET): $(XLS_ROCKPOS) $(ZONEID_HEADER) $(OBJID_HEADER)
	@$(CSVCVTR) -o $(CSV_ROCKPOS) -p 1 -s csv $(XLS_ROCKPOS)
	@ruby rockpos.rb $(CSV_ROCKPOS) $(TARGET)
	
clean:
	-rm -f $(TARGET)
	-rm -f $(CSV_ROCKPOS)
