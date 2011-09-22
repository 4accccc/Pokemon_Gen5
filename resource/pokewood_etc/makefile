#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ichiraku_katsuhiko nakatsui tamada

#------------------------------------------------------------------------------
# ごほうび：作成するデータ
#------------------------------------------------------------------------------
BONUS_TARGET	= pokewood_bonus
BONUS_CDAT	 	= $(BONUS_TARGET).cdat

#コンバータ
BONUS_CONV		= bonus_conv.rb

# 元データ
BONUS_XLS_DATA		= $(BONUS_TARGET).xls
BONUS_CSV_DATA		= $(BONUS_TARGET).csv

#------------------------------------------------------------------------------
#	TRTYPE→MMDL変換：作成するデータ
#------------------------------------------------------------------------------
TR2MMDL_TARGET	= pokewood_tr2mmdl
TR2MMDL_CDAT		= $(TR2MMDL_TARGET).cdat

#コンバータ
TR2MMDL_CONV		= tr2mmdl_conv.rb

# 元データ
TR2MMDL_XLS_DATA		= $(TR2MMDL_TARGET).xls
TR2MMDL_CSV_DATA		= $(TR2MMDL_TARGET).csv

#------------------------------------------------------------------------------
#	共通部分
#------------------------------------------------------------------------------
# ツール
XLS_CNV = ../../tools/exceltool/xls2xml/tab_out.rb

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include

#xls --> csvのルール定義
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる	
%.csv:%.xls
		@echo	__CSV CONVERT
		ruby $(XLS_CNV) -c $< > $@
endif

#最終ターゲット
TARGETS	= $(BONUS_CDAT) $(TR2MMDL_CDAT)


.PHONY:	do-build clean

do-build: $(TARGETS)

clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(TARGETS)
	@rm -f *.csv
endif

#------------------------------------------------------------------------------
# ごほうび：作成するデータ
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる	
$(BONUS_CDAT): $(BONUS_CSV_DATA) $(BONUS_CONV)
	ruby $(BONUS_CONV) $(BONUS_CSV_DATA) $(BONUS_CDAT)
endif	

#------------------------------------------------------------------------------
#	TRTYPE→MMDL変換：作成するデータ
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)
$(TR2MMDL_CDAT):	$(TR2MMDL_CSV_DATA) $(TR2MMDL_CONV)
	ruby $(TR2MMDL_CONV) $(TR2MMDL_CSV_DATA) $(TR2MMDL_CDAT)
endif


