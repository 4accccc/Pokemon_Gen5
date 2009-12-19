#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

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
SUBDIRS	=

.PHONY:	do-build clean


#NARCに必要なscrファイル名
SCRFILE = pokegra_wb.scr

#リソースファイルの存在するディレクトリを指定
RES_DIR = ./conv/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../tools/pokegra/

#コンバート対象のファイルツリーを生成
NCGFILE		:= $(wildcard $(RES_DIR)????_???_*.ncg)
NCGCFILE	:= $(wildcard $(RES_DIR)????_???c_*.ncg)
NMCFILE		:= $(wildcard $(RES_DIR)*.nmc)
NCEFILE		:= $(wildcard $(RES_DIR)*.nce)
NCLFILE	:= $(wildcard $(RES_DIR)*.ncl)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	ncg ncgc nce nmc ncl $(NARCNAME) $(TARGETDIR)/$(NARCNAME)
else
do-build:	$(TARGETDIR)/$(NARCNAME)
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
	perl $(SCRIPT_DIR)ncg.pl $< $(CNV_DIR)

ncgc: $(notdir $(NCGCFILE:.ncg=.NCBR))
%.NCBR: $(RES_DIR)%.ncg
	perl $(SCRIPT_DIR)ncgc.pl $< $(CNV_DIR)

nce: $(notdir $(NMCFILE:.nmc=.NCEC))
%.NCEC: $(RES_DIR)%.nce $(SCRIPT_DIR)nce.pl
	perl $(SCRIPT_DIR)nce.pl $(RES_DIR)$*.nce $(CNV_DIR)

nmc: $(notdir $(NMCFILE:.nmc=.NMCR))
%.NMCR: $(RES_DIR)%.nmc $(RES_DIR)%.nce
	perl $(SCRIPT_DIR)nmc.pl $< $(CNV_DIR)

ncl: $(notdir $(NCLFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	perl $(SCRIPT_DIR)ncl.pl $< $(CNV_DIR)

$(NARCNAME): $(notdir $(NCGFILE:.ncg=.NCGR)) $(notdir $(NMCFILE:.nmc=.NMCR)) $(notdir $(NMCFILE:.nmc=.NCEC)) $(notdir $(NCGCFILE:.ncg=.NCBR)) $(notdir $(NCLFILE:.ncl=.NCLR)) $(SCRFILE)
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


