
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou suginaka_katsunori

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= pl_boat_data
DATANAME	= $(targetname).cdat

#ツールパス
XLS_CNV = ../../tools/exceltool/xls2xml/tab_out.rb

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define


XLS_DATA = pl_boat.xls
CSV_DATA = pl_boat.csv

#コンバート対象のファイルツリーを生成

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(DATANAME)
	
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(DATANAME)
endif

$(DATANAME): $(CSV_DATA)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる	
	ruby pl_boat.rb $(CSV_DATA) $(DATANAME)
endif	
	
$(CSV_DATA): $(XLS_DATA)
#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる	
	@echo __CVS CONVERT
	ruby $(XLS_CNV) -c $(XLS_DATA) > $(CSV_DATA)
endif

