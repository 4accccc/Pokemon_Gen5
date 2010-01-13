#---------------------------------------------------------------
# エンカウントデータコンバート環境
#---------------------------------------------------------------

include encount_common.def

DQ		="
#" #↑subst構文で"を使うとVimのコメント自動認識に狂いがでるので定義している

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	nishino iwasawa

#---------------------------------------------------------------
#	共通ルールファイルのインクルード
#---------------------------------------------------------------
include $(PROJECT_RSCDIR)/macro_define

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
NARCNAME_W	=	$(subst $(DQ),,$(TARGET_BIN_FILE_W))
NAIXNAME_W	=	$(NARCNAME_W:.narc=.naix)
NARCNAME_B	=	$(subst $(DQ),,$(TARGET_BIN_FILE_B))
NAIXNAME_B	=	$(NARCNAME_B:.narc=.naix)

GENERATE_DAT = generate_enc.cdat
GENERATE_SRC = $(GENERATE_DAT:.cdat=.csv)
GENERATE_HEADER = $(subst .cdat,_def.h,$(GENERATE_DAT))

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)
GENERATE_DIR = $(PROJECT_ARCDIR)../src/field

#中間ファイル出力先
BIN_DIR_W	 = $(subst $(DQ),,$(WFBIN_DIR))
BIN_DIR_B	 = $(subst $(DQ),,$(BFBIN_DIR))

ARCW_LIST_FILE = $(subst $(DQ),,$(ARCW_LIST_FILENAME))
ARCB_LIST_FILE = $(subst $(DQ),,$(ARCB_LIST_FILENAME))

#リソース
SRC_TEXT = $(subst $(DQ),,$(SRC_TXT_FILENAME))

#アーカイブ依存ファイル
MONSNO_HASH = $(PROJECT_ROOT)tools/hash/monsno_hash.rb
NNSARC_DEPEND_FILES = $(SRC_TEXT) $(MONSNO_HASH) encount_common.def 

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS		=

#Makeで生成される*.bin,*.datはmake cleanの削除対象にする
CLEANFILES	= $(BIN_DIR_W)/*.* $(BIN_DIR_B)/*.* $(GENERATE_DAT) $(GENERATE_HEADER)

vpath %bin $(BIN_DIR_W) $(BIN_DIR_B)
vpath %dat $(BIN_DIR_W) $(BIN_DIR_B)

.PHONY: do-build clean

#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(NARCNAME_W) $(TARGETDIR)/$(NARCNAME_B) \
					$(GENERATE_DIR)/$(GENERATE_DAT) $(GENERATE_DIR)/$(GENERATE_HEADER)

$(TARGETDIR)/$(NARCNAME_W): $(NARCNAME_W)
	$(COPY) $(NARCNAME_W) $(TARGETDIR)
$(TARGETDIR)/$(NARCNAME_B): $(NARCNAME_B)
	$(COPY) $(NARCNAME_B) $(TARGETDIR)
$(GENERATE_DIR)/$(GENERATE_DAT): $(GENERATE_DAT)
	$(COPY) $(GENERATE_DAT) $(GENERATE_DIR)
$(GENERATE_DIR)/$(GENERATE_HEADER): $(GENERATE_HEADER)
	$(COPY) $(GENERATE_HEADER) $(GENERATE_DIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成

$(ARCW_LIST_FILE):$(NNSARC_DEPEND_FILES)
	ruby encount_conv.rb
$(ARCB_LIST_FILE):$(NNSARC_DEPEND_FILES)
	ruby encount_conv.rb

$(NARCNAME_W): $(ARCW_LIST_FILE) 
	nnsarc -i -c -l -n $(NARCNAME_W) -S $(ARCW_LIST_FILE)  > arc_result_w.txt
	$(NAIXCUT) $(NAIXNAME_W)

#blackはnaixを作らない
$(NARCNAME_B): $(ARCB_LIST_FILE)
	nnsarc -c -l -n $(NARCNAME_B) -S $(ARCB_LIST_FILE)  > arc_result_b.txt

$(GENERATE_DAT): $(GENERATE_SRC)
	ruby generate_conv.rb $(GENERATE_SRC) $(GENERATE_DAT)
$(GENERATE_HEADER): $(GENERATE_SRC)
	ruby generate_conv.rb $(GENERATE_SRC) $(GENERATE_DAT)

endif

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
clean:
	@-rm $(TARGETDIR)/$(NARCNAME_W) $(TARGETDIR)/$(NARCNAME_B) $(GENERATE_DIR)/$(GENERATE_DAT) $(GENERATE_DIR)/$(GENERATE_HEADER)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、バイナリ生成
	@-rm $(NARCNAME_W) $(ARCW_LIST_FILE) $(NAIXNAME_W)
	@-rm $(NARCNAME_B) $(ARCB_LIST_FILE)
	@-rm $(CLEANFILES)
endif

