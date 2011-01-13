#---------------------------------------------------------------
# 特殊エンカウント(Nポケ)データコンバート環境
#---------------------------------------------------------------

DQ		="
#" #↑subst構文で"を使うとVimのコメント自動認識に狂いがでるので定義している

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	= nakatsui iwasawa

#---------------------------------------------------------------
#	共通ルールファイルのインクルード
#---------------------------------------------------------------
include $(PROJECT_RSCDIR)/macro_define

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)../src/field

#リソース
ENC_SRC = encount_data_ex.xls
ENC_CSV = $(ENC_SRC:.xls=.csv)
ENC_DAT = encount_data_ex.cdat
ENC_H = encount_data_ex_def.h 

#アーカイブ依存ファイル
MONSNO_HASH = $(PROJECT_ROOT)tools/hash/monsno_hash.rb
SEIKAKU_HASH = $(PROJECT_ROOT)tools/hash/seikaku_hash.rb
NNSARC_DEPEND_FILES = $(ENC_SRC) $(MONSNO_HASH) $(SEIKAKU_HASH) 

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS		=

.PHONY: do-build clean

XLS2CSV = ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -c

#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(ENC_DAT) $(TARGETDIR)/$(ENC_H)

$(TARGETDIR)/$(ENC_DAT): $(ENC_DAT)
	$(COPY) $(ENC_DAT) $(TARGETDIR)
$(TARGETDIR)/$(ENC_H): $(ENC_H)
	$(COPY) $(ENC_H) $(TARGETDIR)

ifeq ($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成

$(ENC_CSV): $(NNSARC_DEPEND_FILES)
	@$(XLS2CSV) $(ENC_SRC) > $(ENC_CSV)

$(ENC_DAT): $(ENC_CSV)
	ruby encount_ex_conv.rb $(ENC_CSV) $(ENC_DAT)
$(ENC_H): $(ENC_CSV)
	ruby encount_ex_conv.rb $(ENC_CSV) $(ENC_DAT)
endif

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
clean:
	@-rm $(TARGETDIR)/$(ENC_DAT) $(TARGETDIR)/$(ENC_H)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
	@-rm $(ENC_DAT) $(ENC_H) $(ENC_CSV)
endif

