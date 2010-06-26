#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda kawada_koji

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = pokefoot
targetname_ex = pokefoot_ex

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

# 読み込むファイル
input_personal            = $(PROJECT_RSCDIR)/personal/personal_wb.csv

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= graphic graphic_ex

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard .\graphic\*.ncl)
gsources:=$(wildcard .\graphic\*.ncg)
esources:=$(wildcard .\graphic\*.nce)
lsources_ex:=$(wildcard .\graphic_ex\*.ncl)
gsources_ex:=$(wildcard .\graphic_ex\*.ncg)
esources_ex:=$(wildcard .\graphic_ex\*.nce)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES:.spa.narc

endif

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname_ex).narc

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

$(TARGETDIR)/$(targetname_ex).narc: $(targetname_ex).narc
	$(COPY) $(targetname_ex).narc $(TARGETDIR)
	$(COPY) $(targetname_ex).naix $(TARGETDIR) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lsources) $(gsources) $(esources) makefile $(input_personal)
	@$(MAKE_SUBDIR)
	$(NAIXCUT) $(targetname).naix

$(targetname_ex).narc: $(lsources_ex) $(gsources_ex) $(esources_ex) makefile $(input_personal)
	@$(MAKE_SUBDIR)
	$(NAIXCUT) $(targetname_ex).naix
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	@$(MAKE_SUBDIR)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(targetname_ex).narc
	rm -f $(targetname_ex).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(TARGETDIR)/$(targetname_ex).narc
	rm -f $(TARGETDIR)/$(targetname_ex).naix

