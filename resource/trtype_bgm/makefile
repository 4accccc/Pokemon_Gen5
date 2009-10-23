#----------------------------------------------------------------------------
# Project:  トレーナータイプ別の視線BGM対応表からデータ作成
# File:     makefile
#----------------------------------------------------------------------------
#---------------------------------------------------------------
#	共通ルールファイルのインクルード
#---------------------------------------------------------------
USERS = kagaya nishino 
include $(PROJECT_RSCDIR)/macro_define

#---------------------------------------------------------------
#	各要素
#---------------------------------------------------------------
TARGET = trtype_bgm.cdat
HEADER = ../../prog/include/tr_tool/trtype_def.h

XLS_TRBGM = wbtr_bgm.xls
CSV_TRBGM = wbtr_bgm.csv

CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

#---------------------------------------------------------------
#	PHONY
#---------------------------------------------------------------
.PHONY: do-build clean

do-build: $(TARGET)

ifeq	($(CONVERTUSER),true) #CONVERTUSER
$(CSV_TRBGM): $(XLS_TRBGM)
	@$(CSVCVTR) -o $(CSV_TRBGM) -p 1 -s csv $(XLS_TRBGM)

$(TARGET): $(CSV_TRBGM) $(HEADER)
	@ruby trtype_bgm.rb $(CSV_TRBGM) $(TARGET) $(HEADER) 1
else
$(TARGET): $(CSV_TRBGM) $(HEADER)
	@ruby trtype_bgm.rb $(CSV_TRBGM) $(TARGET) $(HEADER) 0
endif

clean:
	-rm -f $(TARGET)
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	-rm -f $(CSV_TRBGM)
endif
