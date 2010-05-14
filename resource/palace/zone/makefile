#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda ohmori obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = palace_zone_setting
targetname_tbl = palace_zone_id_tbl

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_PROGDIR)src/field/field_comm/
TARGETHEAD = $(PROJECT_PROGDIR)include/field/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 


#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


XLSDIRECT = ruby $(PROJECT_ROOT)tools/exceltool/xls2xml/tab_out_direct.rb
DATAFILE	= パレスZONE設定.xls
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
	$(COPY) $(targetname_tbl).cdat $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).cdat: $(TARGETDIR)/$(targetname).cdat makefile palace_zone_conv.rb $(DATAFILE)
	$(XLSDIRECT) $(DATAFILE) -c > $(DATATXT)
	@ruby palace_zone_conv.rb $(DATATXT)

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
	rm -f $(TARGETDIR)/$(targetname_tbl).cdat

