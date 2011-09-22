#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ichiraku_katsuhiko nakatsui

#------------------------------------------------------------------------------
# 作成するデータ
#------------------------------------------------------------------------------
targetname	= pokewood_bonus
DATANAME	 	= $(targetname).cdat

CONV				= bonus_conv.rb

# 元データ
XLS_DATA		= $(targetname).xls
CSV_DATA		= $(targetname).csv

# ツール
XLS_CNV = ../../tools/exceltool/xls2xml/tab_out.rb

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include


.PHONY:	do-build clean

do-build: $(DATANAME)

clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(DATANAME)
	@rm -f *.csv
endif

$(DATANAME): $(CSV_DATA)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる	
	ruby bonus_conv.rb $(CSV_DATA) $(DATANAME)
endif	
	
$(CSV_DATA): $(XLS_DATA) $(CONV)
#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる	
	@echo __CVS CONVERT
	ruby $(XLS_CNV) -c $(XLS_DATA) > $(CSV_DATA)
endif

