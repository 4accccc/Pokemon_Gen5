#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ibe_mana tanoue_reiko yoshida sogabe

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
KANTO_RES_DIR = ../../../pokemon_wb_doc/pokegra/kanto/
JYOTO_RES_DIR = ../../../pokemon_wb_doc/pokegra/jyoto/
HOUEN_RES_DIR = ../../../pokemon_wb_doc/pokegra/houen/
SHINOU_RES_DIR = ../../../pokemon_wb_doc/pokegra/shinou/
ISSYU_RES_DIR = ../../../pokemon_wb_doc/pokegra/issyu/

#LINK情報をCNV書き換えたファイルを格納するディレクトリを指定
COPY_DIR = ./conv/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../tools/pokegra/

#コンバート対象のファイルツリーを生成
KANTO_NCGFILE		:= $(wildcard $(KANTO_RES_DIR)????_???_?.ncg)
KANTO_NCGCFILE	:= $(wildcard $(KANTO_RES_DIR)????_???c_?.ncg)
KANTO_NMCFILE		:= $(wildcard $(KANTO_RES_DIR)*.nmc)
KANTO_NCEFILE		:= $(wildcard $(KANTO_RES_DIR)*.nce)
KANTO_NCLNFILE	:= $(wildcard $(KANTO_RES_DIR)*_n.ncl)
KANTO_NCLRFILE	:= $(wildcard $(KANTO_RES_DIR)*_r.ncl)

JYOTO_NCGFILE		:= $(wildcard $(JYOTO_RES_DIR)????_???_?.ncg)
JYOTO_NCGCFILE	:= $(wildcard $(JYOTO_RES_DIR)????_???c_?.ncg)
JYOTO_NMCFILE		:= $(wildcard $(JYOTO_RES_DIR)*.nmc)
JYOTO_NCEFILE		:= $(wildcard $(JYOTO_RES_DIR)*.nce)
JYOTO_NCLNFILE	:= $(wildcard $(JYOTO_RES_DIR)*_n.ncl)
JYOTO_NCLRFILE	:= $(wildcard $(JYOTO_RES_DIR)*_r.ncl)

HOUEN_NCGFILE		:= $(wildcard $(HOUEN_RES_DIR)????_???_?.ncg)
HOUEN_NCGCFILE	:= $(wildcard $(HOUEN_RES_DIR)????_???c_?.ncg)
HOUEN_NMCFILE		:= $(wildcard $(HOUEN_RES_DIR)*.nmc)
HOUEN_NCEFILE		:= $(wildcard $(HOUEN_RES_DIR)*.nce)
HOUEN_NCLNFILE	:= $(wildcard $(HOUEN_RES_DIR)*_n.ncl)
HOUEN_NCLRFILE	:= $(wildcard $(HOUEN_RES_DIR)*_r.ncl)

SHINOU_NCGFILE	:= $(wildcard $(SHINOU_RES_DIR)????_???_?.ncg)
SHINOU_NCGCFILE	:= $(wildcard $(SHINOU_RES_DIR)????_???c_?.ncg)
SHINOU_NMCFILE	:= $(wildcard $(SHINOU_RES_DIR)*.nmc)
SHINOU_NCEFILE	:= $(wildcard $(SHINOU_RES_DIR)*.nce)
SHINOU_NCLNFILE	:= $(wildcard $(SHINOU_RES_DIR)*_n.ncl)
SHINOU_NCLRFILE	:= $(wildcard $(SHINOU_RES_DIR)*_r.ncl)

ISSYU_NCGFILE		:= $(wildcard $(ISSYU_RES_DIR)????_???_?.ncg)
ISSYU_NCGCFILE	:= $(wildcard $(ISSYU_RES_DIR)????_???c_?.ncg)
ISSYU_NMCFILE		:= $(wildcard $(ISSYU_RES_DIR)*.nmc)
ISSYU_NCEFILE		:= $(wildcard $(ISSYU_RES_DIR)*.nce)
ISSYU_NCLNFILE	:= $(wildcard $(ISSYU_RES_DIR)*_n.ncl)
ISSYU_NCLRFILE	:= $(wildcard $(ISSYU_RES_DIR)*_r.ncl)

NCGFILE		:= $(KANTO_NCGFILE)+$(JYOTO_NCGFILE)+$(HOUEN_NCGFILE)+$(SHINOU_NCGFILE)+$(ISSYU_NCGFILE)
NCGCFILE	:= $(KANTO_NCGCFILE)+$(JYOTO_NCGCFILE)+$(HOUEN_NCGCFILE)+$(SHINOU_NCGCFILE)+$(ISSYU_NCGCFILE)
NMCFILE		:= $(KANTO_NMCFILE)+$(JYOTO_NMCFILE)+$(HOUEN_NMCFILE)+$(SHINOU_NMCFILE)+$(ISSYU_NMCFILE)
NCEFILE		:= $(KANTO_NCEFILE)+$(JYOTO_NCEFILE)+$(HOUEN_NCEFILE)+$(SHINOU_NCEFILE)+$(ISSYU_NCEFILE)
NCLNFILE	:= $(KANTO_NCLNFILE)+$(JYOTO_NCLNFILE)+$(HOUEN_NCLNFILE)+$(SHINOU_NCLNFILE)+$(ISSYU_NCLNFILE)
NCLRFILE	:= $(KANTO_NCLRFILE)+$(JYOTO_NCLRFILE)+$(HOUEN_NCLRFILE)+$(SHINOU_NCLRFILE)+$(ISSYU_NCLRFILE)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	sub_dir kanto_ncg kanto_ncgc kanto_nce kanto_nmc kanto_ncln kanto_nclr jyoto_ncg jyoto_ncgc jyoto_nce jyoto_nmc jyoto_ncln jyoto_nclr houen_ncg houen_ncgc houen_nce houen_nmc houen_ncln houen_nclr shinou_ncg shinou_ncgc shinou_nce shinou_nmc shinou_ncln shinou_nclr issyu_ncg issyu_ncgc issyu_nce issyu_nmc issyu_ncln issyu_nclr $(TARGETDIR)/$(NARCNAME)
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
kanto_ncg: $(notdir $(KANTO_NCGFILE:.ncg=.NCGR))
%.NCGR: $(KANTO_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

kanto_ncgc: $(notdir $(KANTO_NCGCFILE:.ncg=.NCBR))
%.NCBR: $(KANTO_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

kanto_nce: $(notdir $(KANTO_NMCFILE:.nmc=.NCEC))
%.NCEC: $(KANTO_RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(COPY_DIR)$*.nce $(CNV_DIR)

kanto_nmc: $(notdir $(KANTO_NMCFILE:.nmc=.NMCR))
%.NMCR: $(KANTO_RES_DIR)%.nmc $(KANTO_RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

kanto_ncln: $(notdir $(KANTO_NCLNFILE:.ncl=.NCLR))
%.NCLR: $(KANTO_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

kanto_nclr: $(notdir $(KANTO_NCLRFILE:.ncl=.NCLR))
%.NCLR: $(KANTO_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

jyoto_ncg: $(notdir $(JYOTO_NCGFILE:.ncg=.NCGR))
%.NCGR: $(JYOTO_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

jyoto_ncgc: $(notdir $(JYOTO_NCGCFILE:.ncg=.NCBR))
%.NCBR: $(JYOTO_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

jyoto_nce: $(notdir $(JYOTO_NMCFILE:.nmc=.NCEC))
%.NCEC: $(JYOTO_RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(COPY_DIR)$*.nce $(CNV_DIR)

jyoto_nmc: $(notdir $(JYOTO_NMCFILE:.nmc=.NMCR))
%.NMCR: $(JYOTO_RES_DIR)%.nmc $(JYOTO_RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

jyoto_ncln: $(notdir $(JYOTO_NCLNFILE:.ncl=.NCLR))
%.NCLR: $(JYOTO_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

jyoto_nclr: $(notdir $(JYOTO_NCLRFILE:.ncl=.NCLR))
%.NCLR: $(JYOTO_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

houen_ncg: $(notdir $(HOUEN_NCGFILE:.ncg=.NCGR))
%.NCGR: $(HOUEN_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

houen_ncgc: $(notdir $(HOUEN_NCGCFILE:.ncg=.NCBR))
%.NCBR: $(HOUEN_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

houen_nce: $(notdir $(HOUEN_NMCFILE:.nmc=.NCEC))
%.NCEC: $(HOUEN_RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(COPY_DIR)$*.nce $(CNV_DIR)

houen_nmc: $(notdir $(HOUEN_NMCFILE:.nmc=.NMCR))
%.NMCR: $(HOUEN_RES_DIR)%.nmc $(HOUEN_RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

houen_ncln: $(notdir $(HOUEN_NCLNFILE:.ncl=.NCLR))
%.NCLR: $(HOUEN_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

houen_nclr: $(notdir $(HOUEN_NCLRFILE:.ncl=.NCLR))
%.NCLR: $(HOUEN_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

shinou_ncg: $(notdir $(SHINOU_NCGFILE:.ncg=.NCGR))
%.NCGR: $(SHINOU_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

shinou_ncgc: $(notdir $(SHINOU_NCGCFILE:.ncg=.NCBR))
%.NCBR: $(SHINOU_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

shinou_nce: $(notdir $(SHINOU_NMCFILE:.nmc=.NCEC))
%.NCEC: $(SHINOU_RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(COPY_DIR)$*.nce $(CNV_DIR)

shinou_nmc: $(notdir $(SHINOU_NMCFILE:.nmc=.NMCR))
%.NMCR: $(SHINOU_RES_DIR)%.nmc $(SHINOU_RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

shinou_ncln: $(notdir $(SHINOU_NCLNFILE:.ncl=.NCLR))
%.NCLR: $(SHINOU_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

shinou_nclr: $(notdir $(SHINOU_NCLRFILE:.ncl=.NCLR))
%.NCLR: $(SHINOU_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

issyu_ncg: $(notdir $(ISSYU_NCGFILE:.ncg=.NCGR))
%.NCGR: $(ISSYU_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

issyu_ncgc: $(notdir $(ISSYU_NCGCFILE:.ncg=.NCBR))
%.NCBR: $(ISSYU_RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

issyu_nce: $(notdir $(ISSYU_NMCFILE:.nmc=.NCEC))
%.NCEC: $(ISSYU_RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(COPY_DIR)$*.nce $(CNV_DIR)

issyu_nmc: $(notdir $(ISSYU_NMCFILE:.nmc=.NMCR))
%.NMCR: $(ISSYU_RES_DIR)%.nmc $(ISSYU_RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

issyu_ncln: $(notdir $(ISSYU_NCLNFILE:.ncl=.NCLR))
%.NCLR: $(ISSYU_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

issyu_nclr: $(notdir $(ISSYU_NCLRFILE:.ncl=.NCLR))
%.NCLR: $(ISSYU_RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

#do-build:	$(SCRFILE) $(NARCNAME)
do-build:	$(NARCNAME)

#$(SCRFILE): $(SCRIPT_DIR)mkscr.pl $(NCGFILE) $(NCGCFILE) $(NMCFILE) $(NCLNFILE) $(NCLRFILE)
#	ls -1 pfwb*.NMCR > nmcr.lst
#	perl $(SCRIPT_DIR)mkscr.pl nmcr.lst

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


