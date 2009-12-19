#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ariizumi_nobuhiko iwao_kazumasa

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= pokegra_mus
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#リソースファイルの存在するディレクトリを指定
RES_DIR = ../../../../pokemon_wb_doc/pokegra_mus/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#perlスクリプトが存在するディレクトリを指定
PERL_DIR = ../../../tools/pokegra/

#コンバート対象のファイルツリーを生成
NCGFILE	:= $(wildcard $(RES_DIR)????_???_?.ncg)
NCGCFILE	:= $(wildcard $(RES_DIR)????_???c_?.ncg)
NMCFILE	:= $(wildcard $(RES_DIR)*.nmc)
NCLNFILE	:= $(wildcard $(RES_DIR)*_n.ncl)
NCLRFILE	:= $(wildcard $(RES_DIR)*_r.ncl)

.PHONY:	do-build clean subdir

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.nmc.nmcr.ncl.nclr

#nmcデータからの生成ルール
do-build: $(notdir $(NCGFILE:.ncg=.NCGR))
%.NCGR: $(RES_DIR)%.ncg
	perl $(PERL_DIR)ncg.pl $< $(CNV_DIR)

do-build: $(notdir $(NMCFILE:.nmc=.NMCR))
%.NMCR: $(RES_DIR)%.nmc $(RES_DIR)%.nce
	perl $(PERL_DIR)nmc.pl $< $(CNV_DIR)

do-build: $(notdir $(NMCFILE:.nmc=.NCEC))
%.NCEC: $(RES_DIR)%.nce $(PERL_DIR)nce.pl
	perl $(PERL_DIR)nce_mus.pl $(RES_DIR)$*.nce $(CNV_DIR)

do-build: $(notdir $(NCGCFILE:.ncg=.NCBR))
%.NCBR: $(RES_DIR)%.ncg
	perl $(PERL_DIR)ncgc.pl $< $(CNV_DIR)

do-build: $(notdir $(NCLNFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	perl $(PERL_DIR)ncl.pl $< $(CNV_DIR)

do-build: $(notdir $(NCLRFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	perl $(PERL_DIR)ncl.pl $< $(CNV_DIR)
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
$(NARCNAME): $(notdir $(NCGFILE:.ncg=.NCGR)) $(notdir $(NMCFILE:.nmc=.NMCR)) $(notdir $(NMCFILE:.nmc=.NCEC)) $(notdir $(NCGCFILE:.ncg=.NCBR)) $(notdir $(NCLNFILE:.ncl=.NCLR)) $(notdir $(NCLRFILE:.ncl=.NCLR))
	nnsarc -c -l -n -i $(NARCNAME) -S pokegra_wb.scr
	$(NAIXCUT) $(NAIXNAME)
endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#	-rm -f *.N*R
#	-rm -f *.N*C
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


