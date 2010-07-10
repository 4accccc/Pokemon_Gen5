
#------------------------------------------------------------------------------
#  デバッグ用：友達手帳用登録リスト生成
#  2010.07.10	tamada
#------------------------------------------------------------------------------

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

XLS2CSV		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -c

XLSFILE	= 友達コード.xls
CSVFILE = friendlist.csv
TARGET	= gamefreak_list.cdat

.PHONY:	do-build clean

do-build:	$(TARGET)

$(CSVFILE):	$(XLSFILE)
	$(XLS2CSV) $(XLSFILE) > $(CSVFILE)

$(TARGET):	$(CSVFILE)
	ruby make_cdat.rb $(CSVFILE) > $(TARGET)


clean:
	rm -f $(TARGET) $(CSVFILE)

