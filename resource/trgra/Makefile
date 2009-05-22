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
targetname	= trgra_wb
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)trgra

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#NARCに必要なscrファイル名
SCRFILE = trgra_wb.scr

#リソースファイルの存在するディレクトリを指定
RES_DIR = ../../../pokemon_wb_doc/trgra/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバータが存在するディレクトリを指定
#g2dcvtr.exe
G2D_DIR =	../../lib/NitroSystem/tools/win/bin/
#clact_conv.rb
RUBY_DIR = ../../tools/clact/

#コンバート対象のファイルツリーを生成
NCGFILE	:= $(wildcard $(RES_DIR)*.ncg)
NCEFILE	:= $(wildcard $(RES_DIR)*.nce)
NCLFILE	:= $(wildcard $(RES_DIR)*.ncl)

.PHONY:	do-build clean subdir

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.ncg.nce.ncl

do-build: $(notdir $(NCEFILE:.nce=.NCER))
%.NCER: $(RES_DIR)%.nce
	ruby $(RUBY_DIR)clact_conv.rb $(RES_DIR)$*.nce $(RES_DIR)$*.ncg $(RES_DIR)$*.ncl $(G2D_DIR) $(CNV_DIR)

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME) $(NAIXNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

do-build:	$(SCRFILE) $(NARCNAME)

#本来、scrファイルはトレーナーデータから生成されるがそれまでの暫定処置
$(SCRFILE): $(RUBY_DIR)mkscr.rb
	ls -1 tbwb*.NCER > ncer.lst
	ruby $(RUBY_DIR)mkscr.rb ncer.lst $(SCRFILE)

$(NARCNAME): $(notdir $(NCGFILE:.ncg=.NCGR)) $(notdir $(NCEFILE:.nce=.NCER)) $(notdir $(NCLFILE:.ncl=.NCLR)) $(SCRFILE)
	nnsarc -c -l -n -i $(NARCNAME) -S $(SCRFILE)
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


