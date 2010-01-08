#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ariizumi_nobuhiko tanoue_reiko iwao_kazumasa

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = stage_gra

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= script

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)
imdres:=$(wildcard *.imd)
icares:=$(wildcard *.ica)
itares:=$(wildcard *.ita)
itpres:=$(wildcard *.itp)
spares:=$(wildcard *.spa)
esfres:=$(wildcard script/*.esf)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))
imdobj = $(subst .imd,.nsbtx,$(imdres))
imdobj2 = $(subst .imd,.nsbmd,$(imdres))
icaobj = $(subst .ica,.nsbca,$(icares))
itaobj = $(subst .ita,.nsbta,$(itares))
itpobj = $(subst .itp,.nsbtp,$(itpres))
scriptobj = $(subst script/, ,$(subst .esf,.bin,$(esfres)))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce .nsbtx .nsbmd .imd .nsbca .ica .nsbta .ita .nsbtp .itp .esf .s .bin

.ncl.NCLR:
	g2dcvtr $< -pcm

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $<

.ica.nsbca:
	g3dcvtr $<

.ita.nsbta:
	g3dcvtr $<

.itp.nsbtp:
	g3dcvtr $<

.imd.nsbtx:
	g3dcvtr $< -etex
	g3dcvtr $<

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(imdobj) $(imdobj2) $(itaobj) $(itpobj) $(icaobj) $(spares) $(scriptobj)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix
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
	rm -f *.nsbmd
	rm -f *.nsbtx
	rm -f *.nsbta
	rm -f *.nsbtp
	rm -f *.nsbca
	rm -f *.bin
	rm -f *.elf
	rm -f *.o
	rm -f *.s
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


