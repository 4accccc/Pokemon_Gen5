#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro ohmori

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = 

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
GMM_COPY_DIR = $(PROJECT_RSCDIR)message/src
DEF_COPY_DIR = $(PROJECT_PROGDIR)src/app/research_radar

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

XLSCONV             = excel_tab_out.rb
CONV_ROOT_DIR       = .
CONVERTER           = research_radar.rb
XLS_DATA            = research_radar.xls
ANSWER_DATA         = answers.txt
QUESTION_DATA       = questions.txt
TOPIC_DATA          = topics.txt
HOBBY_DATA          = hobbies.txt
JOB_DATA            = job.txt
DATA_SOURCE         = .
DATA_BIN            = bin
DATA_DEBUG          = debug
GMM_QUESTIONNAIRE   = questionnaire.gmm
GMM_HOBBY           = hobby.gmm
GMM_JOB             = job.gmm
TOPIC_ID_DEF        = topic_id.h
QUESTION_ID_DEF     = question_id.h
ANSWER_ID_DEF       = answer_id.h
STRID_ANSWER        = string_id_answer.cdat
STRID_QUESTION      = string_id_question.cdat
STRID_QUESTION_CAPTION = caption_string_id_question.cdat
STRID_TOPIC_C       = string_id_topic_caption.cdat
STRID_TOPIC_T       = string_id_topic_title.cdat
ANSWER_NUM_QUESTION = answer_num_question.cdat
ANSWER_ID_QUESTION  = answer_id_question.cdat
QUESTION_ID_TOPIC   = question_id_topic.cdat
COLOR_R_ANSWER      = color_r_answer.cdat
COLOR_G_ANSWER      = color_g_answer.cdat
COLOR_B_ANSWER      = color_b_answer.cdat


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 
endif

.PHONY:	do-build clean subdir


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir \
	        $(GMM_COPY_DIR)/$(GMM_QUESTIONNAIRE) \
					$(GMM_COPY_DIR)/$(GMM_HOBBY) \
					$(GMM_COPY_DIR)/$(GMM_JOB) \
					$(DEF_COPY_DIR)/$(TOPIC_ID_DEF) \
					$(DEF_COPY_DIR)/$(QUESTION_ID_DEF) \
					$(DEF_COPY_DIR)/$(ANSWER_ID_DEF) \
					$(DEF_COPY_DIR)/$(STRID_ANSWER) \
					$(DEF_COPY_DIR)/$(STRID_QUESTION) \
					$(DEF_COPY_DIR)/$(STRID_QUESTION_CAPTION) \
					$(DEF_COPY_DIR)/$(STRID_TOPIC_C) \
					$(DEF_COPY_DIR)/$(STRID_TOPIC_T) \
					$(DEF_COPY_DIR)/$(ANSWER_NUM_QUESTION) \
					$(DEF_COPY_DIR)/$(ANSWER_ID_QUESTION) \
					$(DEF_COPY_DIR)/$(QUESTION_ID_TOPIC) \
					$(DEF_COPY_DIR)/$(COLOR_R_ANSWER) \
					$(DEF_COPY_DIR)/$(COLOR_G_ANSWER) \
					$(DEF_COPY_DIR)/$(COLOR_B_ANSWER)

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
  # コンバート
	ruby $(XLSCONV) $(XLS_DATA) 1 $(TOPIC_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 2 $(QUESTION_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 3 $(ANSWER_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 4 $(HOBBY_DATA)
	ruby $(XLSCONV) $(XLS_DATA) 5 $(JOB_DATA)
	ruby $(CONVERTER) $(CONV_ROOT_DIR) $(ANSWER_DATA) $(QUESTION_DATA) $(TOPIC_DATA) $(HOBBY_DATA) $(JOB_DATA) $(DATA_SOURCE) $(DATA_DEBUG)
endif

# gmm コピー
$(GMM_COPY_DIR)/$(GMM_QUESTIONNAIRE): $(GMM_QUESTIONNAIRE)
	$(COPY) $(GMM_QUESTIONNAIRE) $(GMM_COPY_DIR)

# gmm コピー
$(GMM_COPY_DIR)/$(GMM_HOBBY): $(GMM_HOBBY)
	$(COPY) $(GMM_HOBBY) $(GMM_COPY_DIR)

$(GMM_COPY_DIR)/$(GMM_JOB): $(GMM_JOB)
	$(COPY) $(GMM_JOB) $(GMM_COPY_DIR)

# def コピー
$(DEF_COPY_DIR)/$(TOPIC_ID_DEF): $(TOPIC_ID_DEF)
	$(COPY) $(TOPIC_ID_DEF) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(QUESTION_ID_DEF): $(QUESTION_ID_DEF)
	$(COPY) $(QUESTION_ID_DEF) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(ANSWER_ID_DEF): $(ANSWER_ID_DEF)
	$(COPY) $(ANSWER_ID_DEF) $(DEF_COPY_DIR)

# strid コピー
$(DEF_COPY_DIR)/$(STRID_ANSWER): $(STRID_ANSWER)
	$(COPY) $(STRID_ANSWER) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(STRID_QUESTION): $(STRID_QUESTION)
	$(COPY) $(STRID_QUESTION) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(STRID_QUESTION_CAPTION): $(STRID_QUESTION_CAPTION)
	$(COPY) $(STRID_QUESTION_CAPTION) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(STRID_TOPIC_C): $(STRID_TOPIC_C)
	$(COPY) $(STRID_TOPIC_C) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(STRID_TOPIC_T): $(STRID_TOPIC_T)
	$(COPY) $(STRID_TOPIC_T) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(ANSWER_ID_QUESTION): $(ANSWER_ID_QUESTION)
	$(COPY) $(ANSWER_ID_QUESTION) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(ANSWER_NUM_QUESTION): $(ANSWER_NUM_QUESTION)
	$(COPY) $(ANSWER_NUM_QUESTION) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(QUESTION_ID_TOPIC): $(QUESTION_ID_TOPIC)
	$(COPY) $(QUESTION_ID_TOPIC) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(COLOR_R_ANSWER): $(COLOR_R_ANSWER)
	$(COPY) $(COLOR_R_ANSWER) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(COLOR_G_ANSWER): $(COLOR_G_ANSWER)
	$(COPY) $(COLOR_G_ANSWER) $(DEF_COPY_DIR)

$(DEF_COPY_DIR)/$(COLOR_B_ANSWER): $(COLOR_B_ANSWER)
	$(COPY) $(COLOR_B_ANSWER) $(DEF_COPY_DIR)

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
