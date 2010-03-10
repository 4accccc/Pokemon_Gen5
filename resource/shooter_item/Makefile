#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi

#------------------------------------------------------------------------------
#※ここに作成するtarget名を書く
#------------------------------------------------------------------------------
targetname = shooter_item_data

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_H			= $(PROJECT_ROOT)/prog/include/item
TARGETDIR_CDAT	= $(PROJECT_ROOT)/prog/src/item

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
XLSFILES:=$(wildcard *.xls)

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_H)/$(targetname).h $(TARGETDIR_CDAT)/$(targetname).cdat

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_H)/$(targetname).h: $(targetname).h
	$(COPY) $(targetname).h $(TARGETDIR_H)

$(TARGETDIR_CDAT)/$(targetname).cdat: $(targetname).cdat
	$(COPY) $(targetname).cdat $(TARGETDIR_CDAT) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
$(targetname).cdat $(targetname).h: $(XLSFILES)
	ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb	-c $^ > $*.csv
	ruby shooter_item_conv.rb $*.csv ..\..\prog\include\item\itemsym.h
	rm -f *.csv

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.cdat
	rm -f *.h
endif



