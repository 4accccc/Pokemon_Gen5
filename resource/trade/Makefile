#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ohno ibe_mana saya_tsuruta

#------------------------------------------------------------------------------
#※テクスチャとモデルを分離してコンバートするかを指定する
#------------------------------------------------------------------------------
IMDTEX_SEPARATE =			#分離しない
#IMDTEX_SEPARATE = on		#分離する

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = trade

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= demo hedtable ir

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

BTNTBL = ruby $(PROJECT_ROOT)/tools/btndata/gfl_hittbl.rb

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
csources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)

#3D
IMDFILE = $(wildcard *.imd)
ICAFILE = $(wildcard *.ica)
IVAFILE = $(wildcard *.iva)
IMAFILE = $(wildcard *.ima)
ITPFILE = $(wildcard *.itp)
ITAFILE = $(wildcard *.ita)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
cobjects = $(subst .nsc,.NSCR,$(csources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))

#3D
NSBMDFILE = $(IMDFILE:.imd=.nsbmd)
#NSBTXFILE = $(IMDFILE:.imd=.nsbtx)	#テクスチャなしimdだとエラーがでる
NSBCAFILE = $(ICAFILE:.ica=.nsbca)
NSBVAFILE = $(IVAFILE:.iva=.nsbva)
NSBMAFILE = $(IMAFILE:.ima=.nsbma)
NSBTPFILE = $(ITPFILE:.itp=.nsbtp)

OBJ_FILE 	= $(lobjects) $(gobjects) $(eobjects) $(cobjects) $(aobjects) $(gobjects) \
						$(NSBMDFILE) $(NSBTXFILE) $(NSBCAFILE) $(NSBVAFILE) $(NSBMAFILE) $(NSBTPFILE)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .ncg .NCGR .NSCR .nsc .NCER .nce .imd .ica .iva .ima .itp .ita .nsbmd .nsbtx .nsbca .nsbma .nsbva .nsbtp


.ncl.NCLR:
	g2dcvtr $< 

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -br -lbl

#3d
.imd.nsbmd:
ifdef IMDTEX_SEPARATE
	g3dcvtr $< -emdl
	g3dcvtr $< -etex
else
	g3dcvtr $<
endif

ifdef IMDTEX_SEPARATE
.imd.nsbtx:
	g3dcvtr $< -etex
endif

.ica.nsbca:
	g3dcvtr $<

.iva.nsbva:
	g3dcvtr $<

.ima.nsbma:
	g3dcvtr $<

.itp.nsbtp:
	g3dcvtr $<

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

LIST_FILE = list.txt

$(targetname).narc:  $(OBJ_FILE) Makefile
	-ls *.N*R > $(LIST_FILE)
	-ls *.nsb* >> $(LIST_FILE)
	perl $(PROJECT_ROOT)tools/arc_list_make.pl $(LIST_FILE)
	nnsarc -r -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm $(LIST_FILE)
#	nnsarc -r -l -n -i $(targetname) $^
#	$(NAIXCUT) $(targetname).naix
	$(DIFFCOPY) wb_trade_obj01_NANR_LBLDEFS.h  $(TARGETDIR)../src/net_app/poke_trade
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -rf *.NANR
	rm -rf *.NCER
	rm -rf *.NSCR
	rm -rf *.NCGR
	rm -rf *.NCLR
	rm -rf *.narc
	rm -rf *.naix
	rm -rf *.h
	rm -rf $(targetname).narc
	rm -rf $(targetname).naix
endif
	rm -rf $(TARGETDIR)/$(targetname).narc
	rm -rf $(TARGETDIR)/$(targetname).naix


