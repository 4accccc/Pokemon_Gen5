#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ohno #tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = wmi


#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg
endif

.PHONY:	do-build	clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(PROJECT_ARCDIR)/$(targetname).narc

# narc のコピーだけ実行する
$(PROJECT_ARCDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(PROJECT_ARCDIR)
	$(COPY) $(targetname).naix $(PROJECT_ARCDIR) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(PROJECT_ARCDIR)/$(targetname).narc
	rm -f $(PROJECT_ARCDIR)/$(targetname).naix


