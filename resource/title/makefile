#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda watanabe mori toyama #tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_b = title_black
targetname_w = title_white

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

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
nsources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
nobjects = $(subst .nce,.NCER,$(nsources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg  .NSCR .nsc .NCER .nce 

.ncl.NCLR:
	g2dcvtr $< -pcm

.ncg.NCGR:
	g2dcvtr $< -bg
	ntrcomp -l 2 -o $*_lz.NCGR $*.NCGR

.nsc.NSCR:
	g2dcvtr $<
	ntrcomp -l 2 -o $*_lz.NSCR $*.NSCR

.nce.NCER:
	g2dcvtr $<

endif


.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname_b).narc $(TARGETDIR)/$(targetname_w).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_b).narc: $(targetname_b).narc
	$(COPY) $(targetname_b).narc $(TARGETDIR)
	$(COPY) $(targetname_b).naix $(TARGETDIR) 

$(TARGETDIR)/$(targetname_w).narc: $(targetname_w).narc
	$(COPY) $(targetname_w).narc $(TARGETDIR)
	$(COPY) $(targetname_w).naix $(TARGETDIR) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname_b).narc: $(lobjects) $(gobjects) $(sobjects) $(nobjects)
	ntrcomp -l 2 -o blk_obj_lz.NCGR blk_obj.NCGR
	nnsarc -r -l -n -i $(targetname_b) -S title_black_arc.txt
	$(NAIXCUT) $(targetname_b).naix

$(targetname_w).narc: $(lobjects) $(gobjects) $(sobjects) $(nobjects)
	ntrcomp -l 2 -o wht_obj_lz.NCGR wht_obj.NCGR
	nnsarc -r -l -n -i $(targetname_w) -S title_white_arc.txt
	$(NAIXCUT) $(targetname_w).naix
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.NCER
	rm -f *.NANR
	rm -f *.NSCR
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname_w).narc
	rm -f $(targetname_w).naix
	rm -f $(targetname_b).narc
	rm -f $(targetname_b).naix
endif
	rm -f $(TARGETDIR)/$(targetname_w).narc
	rm -f $(TARGETDIR)/$(targetname_w).naix
	rm -f $(TARGETDIR)/$(targetname_b).narc
	rm -f $(TARGETDIR)/$(targetname_b).naix


