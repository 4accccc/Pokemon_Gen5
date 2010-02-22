##########################################################
#	メッセージデータコンバート
##########################################################
#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= genya_hosaka kawada_koji

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

SOURCE_LIST	= source.list

WORD_DAT_DIR	= ..\..\prog\src\system
INPUT_DAT_DIR	= ..\..\prog\src\app\pms_input

WORD_DAT_FILE = pms_word.res
INPUT_DAT_FILE = pms_input.res
GMM_HEADER_FILE = pms_abc_gmm_def.h

LANG_FLAG	= jp


-include $(SOURCE_LIST)

##########################################################
#	動作部分
##########################################################
do-build:	$(WORD_DAT_DIR)/$(WORD_DAT_FILE) $(INPUT_DAT_DIR)/$(INPUT_DAT_FILE)

$(WORD_DAT_DIR)/$(WORD_DAT_FILE): $(WORD_DAT_FILE)
	$(COPY) $(WORD_DAT_FILE) $(WORD_DAT_DIR)

$(INPUT_DAT_DIR)/$(INPUT_DAT_FILE): $(INPUT_DAT_FILE)
	$(COPY) $(INPUT_DAT_FILE) $(INPUT_DAT_DIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートする
$(WORD_DAT_FILE): $(SOURCE)
	perl pmsconv.pl  $(INPUT_DAT_FILE) $(WORD_DAT_FILE)
	pms_gmm_conv.rb
	$(COPY) $(GMM_HEADER_FILE) $(INPUT_DAT_DIR)

$(SOURCE_LIST):
	perl pmsconv.pl  $(SOURCE_LIST)

endif

clean:
	-rm $(SOURCE_LIST)
	-rm $(WORD_DAT_DIR)\$(WORD_DAT_FILE)
	-rm $(INPUT_DAT_DIR)\$(INPUT_DAT_FILE)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、フォルダ内の中間ファイルを削除
	-rm $(WORD_DAT_FILE)
	-rm $(INPUT_DAT_FILE)
endif




