#------------------------------------------------------------------------------
#  zukan_graのnarcを作るmakefile  
#  直下のサブディレクトリ中のファイルに対してアーカイブを行います
#  2009.12.03
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= kawada_koji toyama hiro_nakamura

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = zukan_gra

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#------------------------------------------------------------------------------
#※ここでアーカイブしたい直下のサブディレクトリの名前を書く
#------------------------------------------------------------------------------
SUBDIRS_INFO = info
SUBDIRS_LIST = list
SUBDIRS_SEARCH = search
SUBDIRS_TOP = top

SUBDIRS_ARC = $(SUBDIRS_INFO) $(SUBDIRS_LIST) $(SUBDIRS_SEARCH) $(SUBDIRS_TOP)

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard $(addsuffix /*.ncl,$(SUBDIRS_ARC)))
gsources:=$(wildcard $(addsuffix /*.ncg,$(SUBDIRS_ARC)))
csources:=$(wildcard $(addsuffix /*.nsc,$(SUBDIRS_ARC)))
esources:=$(wildcard $(addsuffix /*.nce,$(SUBDIRS_ARC)))

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
cobjects = $(subst .nsc,.NSCR,$(csources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce

.ncl.NCLR:
	g2dcvtr $< -pcm

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -br -lbl

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc Makefile

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(eobjects) $(cobjects) $(aobjects)
	nnsarc -r -l -n -i -f $(targetname) $^
	$(NAIXCUT) $(targetname).naix
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

	rm -f $(SUBDIRS_INFO)/*.N*R
	rm -f $(SUBDIRS_LIST)/*.N*R
	rm -f $(SUBDIRS_SEARCH)/*.N*R
	rm -f $(SUBDIRS_TOP)/*.N*R
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


