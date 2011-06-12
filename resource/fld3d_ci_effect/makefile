
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	unno ibe_mana onoue_masayuki

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
SE_DAT_FILE			= ci_se_tbl.cdat
FADE_DAT_FILE	= ci_fade_tbl.cdat

#ツールパス
XLS_CNV = ../../tools/exceltool/xls2xml/tab_out_direct.rb

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

EFFECT_XLS_DATA = fld3d_ci_eff.xls
EFFECT_CSV_DATA = fld3d_ci_eff.csv
EFFECT_ID_HASH 	= effect_id.rb

#コンバート対象のファイルツリーを生成

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(SE_DAT_FILE)

clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(EFFECT_ID_HASH)
	@rm -f $(SE_DAT_FILE)
	@rm -f $(FADE_DAT_FILE)
	@rm -f $(EFFECT_CSV_DATA)
endif

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

$(SE_DAT_FILE): $(EFFECT_CSV_DATA) 
	@ruby ci_effect.rb $(EFFECT_CSV_DATA) $(SE_DAT_FILE) $(FADE_DAT_FILE) $(EFFECT_ID_HASH)

$(EFFECT_CSV_DATA): $(EFFECT_XLS_DATA) ci_effect.rb
	@ruby $(XLS_CNV) $(EFFECT_XLS_DATA) -s >$(EFFECT_CSV_DATA)
endif

