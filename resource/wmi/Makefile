##  通信アイコンnarcを作るmakefile  
##  コピーしてtargetnameを書き換えれば汎用的に使えます
##  2008.08.19

include	$(PROJECT_RSCDIR)\macro_define

##ここに作成するnarc名を書く
##
targetname = wmi


# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))


# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

.PHONY:	do-build	clean

do-build:	$(PROJECT_ARCDIR)/$(targetname).narc

# narc のコピーだけ実行する
$(PROJECT_ARCDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(PROJECT_ARCDIR)
	$(COPY) $(targetname).naix $(PROJECT_ARCDIR) 

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix

# 削除ファイルを書く
clean:
	rm -rf *.NCGR
	rm -rf *.NCLR
	rm -rf *.narc
	rm -rf *.naix
	rm -rf $(PROJECT_ARCDIR)/$(targetname).narc
	rm -rf $(PROJECT_ARCDIR)/$(targetname).naix


