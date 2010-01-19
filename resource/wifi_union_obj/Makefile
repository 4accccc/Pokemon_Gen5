#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ohno iwasawa

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = wifi_unionobj

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
csources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
cobjects = $(subst .nsc,.NSCR,$(csources))
eobjects = $(subst .nce,.NCER,$(esources))

PAL_LST = wf_match_top_trainer.ncl

CHR_LST = wf_match_top_trainer_boy1.ncg \
					wf_match_top_trainer_boy3.ncg \
					wf_match_top_trainer_man3.ncg \
					wf_match_top_trainer_badman.ncg \
					wf_match_top_trainer_explore.ncg \
					wf_match_top_trainer_fighter.ncg \
					wf_match_top_trainer_gorggeousm.ncg \
					wf_match_top_trainer_mystery.ncg \
					wf_match_top_trainer_girl1.ncg \
					wf_match_top_trainer_girl2.ncg \
					wf_match_top_trainer_woman2.ncg \
					wf_match_top_trainer_woman3.ncg \
					wf_match_top_trainer_idol.ncg \
					wf_match_top_trainer_lady.ncg \
					wf_match_top_trainer_cowgirl.ncg \
					wf_match_top_trainer_gorggeousw.ncg

NORMAL_LST = normal00.nce \
						 normal01.nce \
						 normal02.nce \
						 normal03.nce \
						 normal04.nce \
						 normal05.nce \
						 normal06.nce \
						 normal07.nce \
						 normal08.nce \
						 normal09.nce \
						 normal10.nce \
						 normal11.nce \
						 normal12.nce \
						 normal13.nce \
						 normal14.nce \
						 normal15.nce

FRONT_LST = $(subst normal,front,$(NORMAL_LST))

DEP_LIST = $(PAL_LST) $(CHR_LST) $(NORMA_LST) $(FRONT_LST) 

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
	g2dcvtr $< -br

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
$(targetname).narc: $(DEP_LIST)
	$(MAKE) make_bin	
	nnsarc -r -l -n -i $(targetname) -S conv_list.txt
	$(NAIXCUT) $(targetname).naix

#エクセルコンバート
make_bin:
	convert.bat

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
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


