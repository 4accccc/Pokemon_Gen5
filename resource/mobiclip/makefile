#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
include user.def


#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= opening_1 \
					opening_2

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

#出力
DEF_HEADER = mobiclip_local_def.h

# ツール
DEF_MAKE = ./def_write.rb		# gimmickから拝借
XLS_CNV = ../../tools/exceltool/xls2xml/tab_out.rb


.PHONY:	do-build clean

do-build: $(DEF_HEADER)
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(DEF_HEADER)
	@rm -f *.csv
endif

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

$(DEF_HEADER): def_make.xls
	ruby $(XLS_CNV) -c def_make.xls > def_make.csv
	ruby $(DEF_MAKE) def_make.csv $(DEF_HEADER)

endif
