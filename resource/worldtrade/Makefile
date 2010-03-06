#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi saya_tsuruta

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = worldtrade

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce

#なぜサフィックスの大文字を小文字にしているかというと、
#HGSSから移植してきたGTSがそうだったから
.ncl.NCLR:
	g2dcvtr $<
	rename	$*.NCLR $*.nclr

.ncg.NCGR:
	g2dcvtr $< -bg
	ntrcomp -l 2 -o $*_lz.ncgr $*.NCGR

.nsc.NSCR:
	g2dcvtr $<
	ntrcomp -l 2 -o $*_lz.nscr $*.NSCR

.nce.NCER:
	g2dcvtr $<
	ntrcomp -l 2 -o $*_lz.nanr $*.NANR
	ntrcomp -l 2 -o $*_lz.ncer $*.NCER
	ntrcomp -l 2 -o $*_lz.ncgr $*.NCGR
	rename	$*.NCLR $*.nclr

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_ARC)/$(targetname).narc $(TARGETDIR_ARC)/$(targetname).naix

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetname).naix: $(targetname).naix
	$(COPY) $(targetname).naix $(TARGETDIR_ARC) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects)
	nnsarc -c -i -n $(targetname) -S worldtrade.script
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.NANR
	rm -f *.NCER
	rm -f *.NSCR
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.nanr
	rm -f *.ncer
	rm -f *.nscr
	rm -f *.ncgr
	rm -f *.nclr
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR_ARC)/$(targetname).narc
	rm -f $(TARGETDIR_ARC)/$(targetname).naix


