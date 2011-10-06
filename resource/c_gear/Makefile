#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= iwasawa toyama

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = c_gear_sw

src_narc_h1 = c_gear_obj_NANR_LBLDEFS.h
src_narc_h2 = c_gear_btn_m1_NANR_LBLDEFS.h
narc_h1 = c_gear_sw_nanr_common.h
narc_h2 = c_gear_sw_nanr_panel.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
TARGET_PROG = $(PROJECT_PROGDIR)src/field/c_gear/

target_narc_h1 = $(TARGET_PROG)$(narc_h1)
target_narc_h2 = $(TARGET_PROG)$(narc_h2)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
csources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)
#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
csources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
cobjects = $(subst .nsc,.NSCR,$(csources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce .NANR .nce .imd

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(target_narc_h1) $(target_narc_h2)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

$(target_narc_h1):$(src_narc_h1)
	$(COPY) $(src_narc_h1) $(target_narc_h1)
$(target_narc_h2):$(src_narc_h2)
	$(COPY) $(src_narc_h2) $(target_narc_h2)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lsources) $(gsources) $(esources) $(csources) $(asources) cnv.bat Makefile arc_list.txt
	cnv.bat
	nnsarc -c -l -n -i $(targetname) -S arc_list.txt > arc_result.txt
	$(NAIXCUT) $(targetname).naix
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.NCER
	rm -f *.NANR
	rm -f *.NSCR
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
	rm -f *.bin
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(target_narc_h1)
	rm -f $(target_narc_h2)


