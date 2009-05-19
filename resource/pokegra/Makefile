#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ibe_mana tanoue_reiko yoshida

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= pokegra_wb
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)pokegra

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
SUBDIRS	= ./conv/
endif

.PHONY:	do-build clean sub_dir


#NARCに必要なscrファイル名
SCRFILE = pokegra_wb.scr

#リソースファイルの存在するディレクトリを指定
RES_DIR = ../../../pokemon_wb_doc/pokegra/

#LINK情報をCNV書き換えたファイルを格納するディレクトリを指定
COPY_DIR = ./conv/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../tools/pokegra/

#コンバート対象のファイルツリーを生成
NCGFILE	:= $(wildcard $(RES_DIR)????_???_?.ncg)
NCGCFILE	:= $(wildcard $(RES_DIR)????_???c_?.ncg)
NMCFILE	:= $(wildcard $(RES_DIR)*.nmc)
NCEFILE	:= $(wildcard $(RES_DIR)*.nce)
NCLNFILE	:= $(wildcard $(RES_DIR)*_n.ncl)
NCLRFILE	:= $(wildcard $(RES_DIR)*_r.ncl)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	sub_dir ncg ncgc nce nmc ncln nclr $(TARGETDIR)/$(NARCNAME)
else
do-build:	sub_dir $(TARGETDIR)/$(NARCNAME)
endif

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME) $(NAIXNAME) pokemon_viewer_def.h
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)
	$(COPY) pokemon_viewer_def.h $(PROJECT_PROGDIR)src/test/soga/

sub_dir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#nmcデータからの生成ルール
ncg: $(notdir $(NCGFILE:.ncg=.NCGR))
%.NCGR: $(RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

ncgc: $(notdir $(NCGCFILE:.ncg=.NCBR))
%.NCBR: $(RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

nce: $(notdir $(NMCFILE:.nmc=.NCEC))
%.NCEC: $(RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(COPY_DIR)$*.nce $(CNV_DIR)

nmc: $(notdir $(NMCFILE:.nmc=.NMCR))
%.NMCR: $(RES_DIR)%.nmc $(RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

ncln: $(notdir $(NCLNFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

nclr: $(notdir $(NCLRFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)
endif

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	$(SCRFILE) $(NARCNAME)

$(SCRFILE): $(SCRIPT_DIR)mkscr.pl $(NCGFILE) $(NCGCFILE) $(NMCFILE) $(NCLNFILE) $(NCLRFILE)
	ls -1 pfwb*.NMCR > nmcr.lst
	perl $(SCRIPT_DIR)mkscr.pl nmcr.lst

$(NARCNAME): $(notdir $(NCGFILE:.ncg=.NCGR)) $(notdir $(NMCFILE:.nmc=.NMCR)) $(notdir $(NMCFILE:.nmc=.NCEC)) $(notdir $(NCGCFILE:.ncg=.NCBR)) $(notdir $(NCLNFILE:.ncl=.NCLR)) $(notdir $(NCLRFILE:.ncl=.NCLR)) $(SCRFILE)
	nnsarc -c -l -n -i $(NARCNAME) -S $(SCRFILE)
	$(NAIXCUT) $(NAIXNAME)

endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	@$(MAKE_SUBDIR)
	-rm -f *.N*R
	-rm -f *.N*C
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


