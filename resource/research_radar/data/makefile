#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = 

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
GMM_COPY_DIR = $(PROJECT_RSCDIR)message/src

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

XLSCONV           = excel_tab_out.rb
CONV_ROOT_DIR     = .
CONVERTER         = research_radar.rb
XLS_DATA          = research_radar.xls
ANSWER_DATA       = answers.txt
QUESTION_DATA     = questions.txt
TOPIC_DATA        = topics.txt
HOBBY_DATA        = hobbies.txt
DATA_SOURCE       = .
DATA_BIN          = bin
DATA_DEBUG        = debug
GMM_QUESTIONNAIRE = questionnaire.gmm
GMM_HOBBY         = hobby.gmm


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 
endif

.PHONY:	do-build clean subdir


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(GMM_COPY_DIR)/$(GMM_QUESTIONNAIRE) $(GMM_COPY_DIR)/$(GMM_HOBBY)

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# コンバート
	ruby $(XLSCONV) $(XLS_DATA) 1 $(TOPIC_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 2 $(QUESTION_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 3 $(ANSWER_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 4 $(HOBBY_DATA)
	ruby $(CONVERTER) $(CONV_ROOT_DIR) $(ANSWER_DATA) $(QUESTION_DATA) $(TOPIC_DATA) $(HOBBY_DATA) $(DATA_SOURCE) $(DATA_DEBUG)
endif

# gmm コピー
$(GMM_COPY_DIR)/$(GMM_QUESTIONNAIRE): $(GMM_QUESTIONNAIRE)
	$(COPY) $(GMM_QUESTIONNAIRE) $(GMM_COPY_DIR)

# gmm コピー
$(GMM_COPY_DIR)/$(GMM_HOBBY): $(GMM_HOBBY)
	$(COPY) $(GMM_HOBBY) $(GMM_COPY_DIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
endif


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
endif
	rm -f $(GMM_COPY_DIR)/$(GMM_QUESTIONNAIRE)
	rm -f $(GMM_COPY_DIR)/$(GMM_HOBBY) 
