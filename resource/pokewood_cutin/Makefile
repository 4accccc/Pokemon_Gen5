#------------------------------------------------------------------------------
#  コピーしてtargetname_graを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= nagihashi_toru mori nakatsui

#------------------------------------------------------------------------------
#※依存ファイルを書く
#------------------------------------------------------------------------------
dataname 	= pokewood_cutin_data
scrname 	= pokewood_cutin_scr
scrlist 	= scr_list.txt
scrtarget = scr_make.txt
defname 	= pokewood_cutin_def
xlsconv 	= ruby tab_out_direct_ex.rb
defconv 	= ruby pokewood_cutin_define_conv.rb
cmdconv 	= pokewood_cutin_script_conv.rb
arclistmk	= perl ${PROJECT_ROOT}/tools/arc_list_make.pl
scrmk			= pokewood_cutin_script_make.rb
cdatname	= pokewood_cutin_script_search


#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_gra = pokewood_cutin_gra
targetname_scr = pokewood_cutin_scr

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
TARGETDIR_H = $(PROJECT_ROOT)/prog/src/battle/pokewood/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
esources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NCER .nce

.ncl.NCLR:
	g2dcvtr $< -pcm

.ncg.NCGR:
	g2dcvtr $< -bg

.nce.NCER:
	g2dcvtr $< -br

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname_gra).narc $(TARGETDIR)/$(targetname_gra).naix $(TARGETDIR)/$(targetname_scr).narc $(TARGETDIR)/$(targetname_scr).naix $(TARGETDIR_H)/$(defname).h $(TARGETDIR_H)/$(cdatname).cdat

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_gra).narc: $(targetname_gra).narc
	$(COPY) $(targetname_gra).narc $(TARGETDIR)
	$(COPY) $(targetname_gra).naix $(TARGETDIR) 

$(TARGETDIR)/$(targetname_scr).narc: $(targetname_scr).narc
	$(COPY) $(targetname_scr).narc $(TARGETDIR)
	$(COPY) $(targetname_scr).naix $(TARGETDIR) 

$(TARGETDIR_H)/$(defname).h: $(defname).h
	$(COPY) $(defname).h $(TARGETDIR_H)

$(TARGETDIR_H)/$(cdatname).cdat: $(cdatname).cdat
	$(COPY) $(cdatname).cdat $(TARGETDIR_H)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname_scr).narc $(targetname_scr).naix: $(scrlist)
	nnsarc -r -l -n -i $(targetname_scr) -S $(scrlist)
	$(NAIXCUT) $(targetname_scr).naix

$(scrlist) $(cdatname).cdat: $(scrtarget) $(defname).rb $(targetname_gra).naix
	$(scrmk) $(cmdconv) $(targetname_gra).naix $(scrname) $(cdatname)
	ls $(scrname)*.dat > $(scrlist)
	$(arclistmk) $(scrlist)
	rm $(scrtarget)

$(targetname_gra).narc $(targetname_gra).naix: $(lobjects) $(gobjects) $(eobjects) $(aobjects)
	nnsarc -r -l -n -i $(targetname_gra) $^
	$(NAIXCUT) $(targetname_gra).naix

$(defname).h $(defname).rb: $(defname).csv
	$(defconv) $< $(defname)

$(defname).csv $(scrtarget): $(dataname).xls
	-rm *.txt
	$(xlsconv) $< -s $(scrname)
	mv $(scrname)0.csv $(defname).csv
	echo > $(scrtarget)
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
	rm -f *.narc
	rm -f *.naix
	rm -f *.csv
	rm -f *.txt
	rm -f *.dat
	rm -f $(targetname_gra).narc
	rm -f $(targetname_gra).naix
	rm -f $(targetname_scr).narc
	rm -f $(targetname_scr).naix
endif
	rm -f $(TARGETDIR)/$(targetname_gra).narc
	rm -f $(TARGETDIR)/$(targetname_gra).naix
	rm -f $(TARGETDIR)/$(targetname_scr).narc
	rm -f $(TARGETDIR)/$(targetname_scr).naix

