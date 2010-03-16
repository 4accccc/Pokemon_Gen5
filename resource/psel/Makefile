#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= unno kawada_koji

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = psel

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
# ツール
#------------------------------------------------------------------------------
ICACONV = ..\..\tools\ica\ica_converter.rb

#------------------------------------------------------------------------------
# 3Dファイル
#------------------------------------------------------------------------------
# 3Dリソースファイル
SOURCES_3D = \
    psel_s01.imd psel_s01.ica psel_s01.ima psel_s01.ita \
    psel_s01_cam.ica \
    psel_s02.imd psel_s02.ica \
    psel_s02_cam.ica \
    psel_mb.imd psel_mb.ima

# 3Dコンバート後のファイル
OBJECTS_3D = \
    psel_s01.nsbmd psel_s01.nsbca psel_s01.nsbma psel_s01.nsbta \
    psel_s01_cam.bin \
    psel_s02.nsbmd psel_s02.nsbca \
    psel_s02_cam.bin \
    psel_mb.nsbmd psel_mb.nsbma

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

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $<

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

# 3Dコンバート
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
$(OBJECTS_3D): $(SOURCES_3D)
	g3dcvtr psel_s01.imd
	g3dcvtr psel_s01.ica
	g3dcvtr psel_s01.ima
	g3dcvtr psel_s01.ita
	ruby $(ICACONV) -rt psel_s01_cam.ica psel_s01_cam.bin
	g3dcvtr psel_s02.imd
	g3dcvtr psel_s02.ica
	ruby $(ICACONV) -rt psel_s02_cam.ica psel_s02_cam.bin
	g3dcvtr psel_mb.imd
	g3dcvtr psel_mb.ima
endif

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(OBJECTS_3D)
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
	rm -f *.NSCR
	rm -f *.NANR
	rm -f *.NCER
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(OBJECTS_3D)
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


