#------------------------------------------------------------------------------
#  zukan_graのnarcを作るmakefile  
#  直下のサブディレクトリ中のファイルに対してアーカイブを行います
#  2009.12.03
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toyama hiro_nakamura

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = dendou_demo_gra

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC = $(PROJECT_ARCDIR)
TARGETDIR_H = $(PROJECT_ROOT)prog/src/demo/dendou_demo

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#------------------------------------------------------------------------------
#※ここでアーカイブしたい直下のサブディレクトリの名前を書く
#------------------------------------------------------------------------------
SUBDIRS_PTCL = ptcl

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)
parsources:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL)))

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
cobjects = $(subst .nsc,.NSCR,$(csources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce .spa .spt .spr

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -br

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_ARC)/$(targetname).narc $(TARGETDIR_H)/ddemo_spa.h Makefile

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR_ARC)
	$(COPY) $(targetname).naix $(TARGETDIR_ARC)

$(TARGETDIR_H)/ddemo_spa.h :ddemo_spa.h
	$(COPY) ddemo_spa.h $(TARGETDIR_H)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(eobjects) $(cobjects) $(aobjects) ddemo.spa
	nnsarc -r -l -n -i -f $(targetname) $^
	$(NAIXCUT) $(targetname).naix

ddemo.spa : $(parsources)
	../../tools/SPRArchiver.exe -o ddemo_spa.h $(SUBDIRS_PTCL) $(SUBDIRS_PTCL) ddemo.spa

# -fオプションを付けて、パス名から作成したIDにする
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除

ifeq ($(aobjects),)
	rm -f $(aobjects)
endif
ifeq ($(eobjects),)
	rm -f $(eobjects)
endif
ifeq ($(cobjects),)
	rm -f $(cobjects)
endif
ifeq ($(gobjects),)
	rm -f $(gobjects)
endif
ifeq ($(lobjects),)
	rm -f $(lobjects)
endif

	rm -f *.N*R
	rm -f *.spa
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR_ARC)/$(targetname).narc
	rm -f $(TARGETDIR_ARC)/$(targetname).naix


