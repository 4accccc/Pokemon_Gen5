##########################################################
#	メッセージデータコンバート
##########################################################

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

SOURCE_LIST	= source.list
WORD_DAT_FILE = ..\..\prog\src\system\pms_word.res
INPUT_DAT_FILE = ..\..\prog\src\app\pms_input\pms_input.res

LANG_FLAG	= jp

TARGETDIR = $(PROJECT_ARCDIR)

-include $(SOURCE_LIST)

##########################################################
#	動作部分
##########################################################

no-rule:
	$(MAKE)	$(SOURCE_LIST)
	$(MAKE)	$(WORD_DAT_FILE)



$(WORD_DAT_FILE): $(SOURCE)
	perl pmsconv.pl  $(INPUT_DAT_FILE) $(WORD_DAT_FILE)


# $(INPUT_DAT_FILE): $(SOURCE)


clean:
	-rm $(SOURCE_LIST)
	-rm $(WORD_DAT_FILE)
	-rm $(INPUT_DAT_FILE)


$(SOURCE_LIST):
	perl pmsconv.pl  $(SOURCE_LIST)


