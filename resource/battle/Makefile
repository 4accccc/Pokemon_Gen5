#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe taya yoshida

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
IMAFILE	:= $(wildcard $(RES_3DDIR)*.ima)
ITAFILE	:= $(wildcard $(RES_3DDIR)*.ita)
BGNCGFILE := $(wildcard $(RES_BGDIR)*.ncg)
BGNSCFILE := $(wildcard $(RES_BGDIR)*.nsc)
BGNCLFILE := $(wildcard $(RES_BGDIR)*.ncl)
OBJNCGFILE := $(wildcard $(RES_OBJDIR)*.ncg)
OBJNCEFILE := $(wildcard $(RES_OBJDIR)*.nce)
OBJNCLFILE := $(wildcard $(RES_OBJDIR)*.ncl)

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build: nsbmd nsbca nsbma nsbta ncgr ncer bg_nclr obj_nclr $(NARCNAME) $(TARGETDIR)/$(NARCNAME)
else
do-build: $(TARGETDIR)/$(NARCNAME)
endif

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

nsbmd: $(notdir $(IMDFILE:.imd=.nsbmd))
%.nsbmd: $(RES_3DDIR)%.imd
	g3dcvtr $<

nsbca: $(notdir $(ICAFILE:.ica=.nsbca))
%.nsbca: $(RES_3DDIR)%.ica
	g3dcvtr $<

nsbma: $(notdir $(IMAFILE:.ima=.nsbma))
%.nsbma: $(RES_3DDIR)%.ima
	g3dcvtr $<

nsbta: $(notdir $(ITAFILE:.ita=.nsbta))
%.nsbta: $(RES_3DDIR)%.ita
	g3dcvtr $<

ncgr: $(notdir $(OBJNCGFILE:.ncg=.NCGR))
%.NCGR: $(RES_OBJDIR)%.ncg
	g2dcvtr $< -bg -o/./

ncer: $(notdir $(OBJNCEFILE:.nce=.NCER))
%.NCER: $(RES_OBJDIR)%.nce
	g2dcvtr $< -o/./

obj_nclr: $(notdir $(OBJNCLFILE:.ncl=.NCLR))
%.NCLR: $(RES_OBJDIR)%.ncl
	g2dcvtr $< -o/./

nscr: $(notdir $(BGNSCFILE:.nsc=.NSCR))
%.NSCR: $(RES_BGDIR)%.nsc
	g2dcvtr $< -bg -o/./

ncgr: $(notdir $(BGNCGFILE:.ncg=.NCGR))
%.NCGR: $(RES_BGDIR)%.ncg
	g2dcvtr $< -bg -o/./

bg_nclr: $(notdir $(BGNCLFILE:.ncl=.NCLR))
%.NCLR: $(RES_BGDIR)%.ncl
	g2dcvtr $< -o/./

$(NARCNAME): $(notdir $(IMDFILE:.imd=.nsbmd)) $(notdir $(ICAFILE:.ica=.nsbca)) $(notdir $(IMAFILE:.ima=.nsbma)) $(notdir $(ITAFILE:.ita=.nsbta)) $(notdir $(BGNSCFILE:.nsc=.NSCR)) $(notdir $(BGNCGFILE:.ncg=.NCGR)) $(notdir $(BGNCLFILE:.ncl=.NCLR)) $(notdir $(OBJNCEFILE:.nce=.NCER)) $(notdir $(OBJNCLFILE:.ncl=.NCLR)) battgra_wb.scr
	nnsarc -c -l -n -i $(NARCNAME) -S battgra_wb.scr > error
	$(NAIXCUT) $(NAIXNAME)
	../../tools/etd.exe $(NAIXNAME)
endif

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	-rm -f *.N*R
	-rm -f *.nsbmd
	-rm -f *.nsbca
	-rm -f *.nsbma
	-rm -f *.nsbta
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


