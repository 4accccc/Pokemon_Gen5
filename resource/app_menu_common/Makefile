#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ariizumi_nobuhiko toru_nagihashi ibe_mana hiro_nakamura genya_hosaka toyama kawada_koji

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = app_menu_common
targetname_dl = app_menu_common_dl

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
TARGETDIR_DL = $(PROJECT_ROOT)/multiboot/arc

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= ball

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl) $(wildcard ./*/*.ncl)
gsources:=$(wildcard *.ncg) $(wildcard ./*/*.ncg)
ssources:=$(wildcard *.nsc) $(wildcard ./*/*.nsc)
esources:=$(wildcard *.nce) $(wildcard ./*/*.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
cobjects = $(subst .nce,.NCGR,$(esources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce

.ncl.NCLR:
	g2dcvtr $< 

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -ncn

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、サブファルダのmakefileを利用する
	@$(MAKE_SUBDIR)
endif


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(cobjects)
	nnsarc -c -l -n -i $(targetname) -S app_menu_common.scr
	$(NAIXCUT) $(targetname).naix
endif


do-build:	subdir $(TARGETDIR_DL)/$(targetname_dl).narc
#サブフォルダの処理はいらない。

# narc のコピーだけ実行する
$(TARGETDIR_DL)/$(targetname_dl).narc: $(targetname_dl).narc
	$(COPY) $(targetname_dl).narc $(TARGETDIR_DL)
	$(COPY) $(targetname_dl).naix $(TARGETDIR_DL)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname_dl).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(cobjects)
	nnsarc -c -l -n -i $(targetname_dl) -S app_menu_common_dl.scr
	$(NAIXCUT) $(targetname_dl).naix
endif


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、サブファルダのmakefileを利用する
	@$(MAKE_SUBDIR)
endif
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.NCER
	rm -f *.NANR
	rm -f *.NSCR
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(targetname_dl).narc
	rm -f $(targetname_dl).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(TARGETDIR_DL)/$(targetname_dl).narc
	rm -f $(TARGETDIR_DL)/$(targetname_dl).naix


