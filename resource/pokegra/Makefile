#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= pokegra_wb
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/pokegra/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#リソースファイルの存在するディレクトリを指定
RES_DIR = ../../../pokemon_wb_doc/pokegra/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

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
	perl ncg.pl $< $(CNV_DIR)

do-build: $(notdir $(NMCFILE:.nmc=.NMCR))
%.NMCR: $(RES_DIR)%.nmc $(RES_DIR)%.nce
	g2dcvtr $< -bmp -cr/0_0_32_16 -o/$(CNV_DIR)
	perl nce.pl $(RES_DIR)$*.nce $(CNV_DIR)

do-build: $(notdir $(NCGCFILE:.ncg=.NCBR))
%.NCBR: $(RES_DIR)%.ncg
	perl ncgc.pl $< $(CNV_DIR)

do-build: $(notdir $(NCLNFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	g2dcvtr $< -o/$(CNV_DIR)

do-build: $(notdir $(NCLRFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	g2dcvtr $< -o/$(CNV_DIR)
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME): $(notdir $(NCGFILE:.ncg=.NCGR)) $(notdir $(NMCFILE:.nmc=.NMCR)) $(notdir $(NCGCFILE:.ncg=.NCBR)) $(notdir $(NCLNFILE:.ncl=.NCLR)) $(notdir $(NCLRFILE:.ncl=.NCLR))
	nnsarc -c -l -n -i $(NARCNAME) -S pokegra_wb.scr > error
	$(NAIXCUT) $(NAIXNAME)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


