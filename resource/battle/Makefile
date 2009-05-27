#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe taya

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= battgra_wb
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/battle/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#=========================
#	リソースディレクトリ
#=========================
RES_BGDIR = battle_bg/
RES_OBJDIR = battle_obj/
RES_3DDIR = battle_3d/

#コンバータ
NITRO_COMP = ../../tools/nitro_comp

#とりあえず直下の*.imdを依存対象とする
IMDFILE	:= $(wildcard $(RES_3DDIR)*.imd)
ICAFILE	:= $(wildcard $(RES_3DDIR)*.ica)
BGNCGFILE := $(wildcard $(RES_BGDIR)*.ncg)
BGNCLFILE := $(wildcard $(RES_BGDIR)*.ncl)

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#do-build: pl_batt_obj.narc $(TARGETDIR)/$(NARCNAME)
do-build: nsbmd nsbca ncgr nclr $(NARCNAME) $(TARGETDIR)/$(NARCNAME)
else
do-build: $(TARGETDIR)/$(NARCNAME)
endif

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#pl_batt_obj.narc:
#	$(NITRO_COMP) $(NITROSDK_ROOT) batt_obj.lst pl_batt_obj.narc pl_batt_obj.naix
#	$(COPY) pl_batt_obj.narc platinum_data/
#	$(COPY) pl_batt_obj_*.h platinum_data/

nsbmd: $(notdir $(IMDFILE:.imd=.nsbmd))
%.nsbmd: $(RES_3DDIR)%.imd
	g3dcvtr $<

nsbca: $(notdir $(ICAFILE:.ica=.nsbca))
%.nsbca: $(RES_3DDIR)%.ica
	g3dcvtr $<

ncgr: $(notdir $(BGNCGFILE:.ncg=.NCGR))
%.NCGR: %.ncg
	g2dcvtr -bg $< -o/./

nclr: $(notdir $(BGNCLFILE:.ncl=.NCLR))
%.NCLR: %.ncl
	g2dcvtr $< -o/./

$(NARCNAME):	$(notdir $(IMDFILE:.imd=.nsbmd)) $(notdir $(ICAFILE:.ica=.nsbca)) $(notdir $(BGNCGFILE:.ncg=.NCGR)) $(notdir $(BGNCLFILE:.ncl=.NCLR)) battgra_wb.scr
	nnsarc -c -l -n -i $(NARCNAME) -S battgra_wb.scr > error
	$(NAIXCUT) $(NAIXNAME)

endif

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME) platinum_data/*
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)
	$(COPY) platinum_data/* $(TARGETDIR)	#プラチナデータはそのままコピーしておく

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	-rm -f *.N*R
	-rm -f *.nsbmd
	-rm -f *.nsbca
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


