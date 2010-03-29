#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda ohmori nakamura_akira

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = cross_comm

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_PROGDIR)src/gamesystem/
TARGETHEAD = $(PROJECT_PROGDIR)include/field/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 


#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


XLSDIRECT = ruby $(PROJECT_ROOT)tools/exceltool/xls2xml/tab_out_direct.rb
DATAFILE	= ライブ通信設定データ.xls
DATATXT = $(DATAFILE:.xls=.csv)




ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES:
endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).cdat

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).cdat: $(targetname).cdat
	$(COPY) $(targetname).cdat $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).cdat: $(TARGETDIR)/$(targetname).cdat makefile cross_comm_conv.rb $(DATAFILE)
	$(XLSDIRECT) $(DATAFILE) -c > $(DATATXT)
	@ruby cross_comm_conv.rb $(DATATXT)

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(targetname).cdat
	rm -f *.csv
endif
	rm -f $(TARGETDIR)/$(targetname).cdat

