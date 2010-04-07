



#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

XLS2TAB	= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb

SRCXLS	= "シンボルエンカウント100405.xls"

do-build:
	$(XLS2TAB)	-c -n 1 $(SRCXLS) > symbol_map.csv	
	ruby symbol_map_pos.rb symbol_map.csv > symbol_map_pos.cdat

