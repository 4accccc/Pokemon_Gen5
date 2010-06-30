#------------------------------------------------------------------------------
#
#	トレーナーグラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include	user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_f	= trfgra
targetname_b	= trbgra
NARCNAME_F	= $(targetname_f).narc
NAIXNAME_F	= $(targetname_f).naix
NARCNAME_B	= $(targetname_b).narc
NAIXNAME_B	= $(targetname_b).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)trgra

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
SUBDIRS	= ./conv/
else
SUBDIRS	=
endif

#NARCに必要なscrファイル名
SCRFILE_F = trfgra_wb.scr
SCRFILE_B = trbgra_wb.scr
SCRFILE_F2 = trfgra_wb2.scr
SCRFILE_B2 = trbgra_wb2.scr

#リソースファイルの存在するディレクトリを指定
RES_DIR = ../../../pokemon_wb_doc/trgra/

#LINK情報をCNV書き換えたファイルを格納するディレクトリを指定
COPY_DIR = ./conv/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバータが存在するディレクトリを指定
#g2dcvtr.exe
G2D_DIR =	../../lib/NitroSystem/tools/win/bin/
#clact_conv.rb
RUBY_DIR = ../../tools/clact/

#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../tools/trgra/

#コンバート対象のファイルツリーを生成
TRBNCGFILE	:= $(wildcard $(RES_DIR)tbwb_*.ncg)
TRBNCEFILE	:= $(wildcard $(RES_DIR)tbwb_*.nce)
TRBNMCFILE	:= $(wildcard $(RES_DIR)tbwb_*.nmc)
TRBNCLFILE	:= $(wildcard $(RES_DIR)tbwb_*.ncl)

TRFNCGFILE	:= $(wildcard $(RES_DIR)trwb_*.ncg)
TRFNCEFILE	:= $(wildcard $(RES_DIR)trwb_*.nce)
TRFNMCFILE	:= $(wildcard $(RES_DIR)trwb_*.nmc)
TRFNCLFILE	:= $(wildcard $(RES_DIR)trwb_*.ncl)

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.ncg.nce.ncl

do-build:	sub_dir nmc ncg ncgc nce ncl	$(NARCNAME_F) $(NARCNAME_B)

sub_dir:
	@$(MAKE_SUBDIR)

#nmcデータからの生成ルール
ncg: $(notdir $(TRFNCGFILE:.ncg=.NCGR)) $(notdir $(TRBNCGFILE:.ncg=.NCGR))
%.NCGR: $(RES_DIR)%.ncg
	perl $(SCRIPT_DIR)tr_ncg.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

ncgc: $(notdir $(TRFNCGFILE:.ncg=.NCBR)) $(notdir $(TRBNCGFILE:.ncg=.NCBR))
%.NCBR: $(RES_DIR)%.ncg
	perl $(SCRIPT_DIR)tr_ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

nce: $(notdir $(TRFNCEFILE:.nce=.NCEC)) $(notdir $(TRBNCEFILE:.nce=.NCEC))
%.NCEC: $(RES_DIR)%.nce $(SCRIPT_DIR)tr_nce.pl
	perl $(SCRIPT_DIR)tr_nce.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

nmc: $(notdir $(TRFNMCFILE:.nmc=.NMCR)) $(notdir $(TRBNMCFILE:.nmc=.NMCR))
%.NMCR: $(RES_DIR)%.nmc $(RES_DIR)%.nce
	perl $(SCRIPT_DIR)tr_nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

ncl: $(notdir $(TRFNCLFILE:.ncl=.NCLR)) $(notdir $(TRBNCLFILE:.ncl=.NCLR))
%.NCLR: $(RES_DIR)%.ncl
	perl $(SCRIPT_DIR)tr_ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

$(NARCNAME_F): $(notdir $(TRFNCGFILE:.ncg=.NCBR)) $(notdir $(TRFNCEFILE:.nce=.NCER)) $(notdir $(TRFNCLFILE:.ncl=.NCLR)) $(SCRFILE_F)
	ruby replace.rb $(SCRFILE_F) > move.sh
	$(shell move.sh)
	nnsarc -c  -n -i $(NARCNAME_F) -S $(SCRFILE_F2)
	$(NAIXCUT) $(NAIXNAME_F)

$(NARCNAME_B): $(notdir $(TRBNCGFILE:.ncg=.NCBR)) $(notdir $(TRBNCEFILE:.nce=.NCER)) $(notdir $(TRBNCLFILE:.ncl=.NCLR)) $(SCRFILE_B)
	ruby replace.rb $(SCRFILE_B) > move.sh
	$(shell move.sh)
	nnsarc -c  -n -i $(NARCNAME_B) -S $(SCRFILE_B2)
	$(NAIXCUT) $(NAIXNAME_B)
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME_F) $(TARGETDIR)/$(NARCNAME_B)

$(TARGETDIR)/$(NARCNAME_F):	$(NARCNAME_F) $(NAIXNAME_F)
	$(COPY)	$(NARCNAME_F) $(TARGETDIR)
	$(COPY) $(NAIXNAME_F) $(TARGETDIR)

$(TARGETDIR)/$(NARCNAME_B):	$(NARCNAME_B) $(NAIXNAME_B)
	$(COPY)	$(NARCNAME_B) $(TARGETDIR)
	$(COPY) $(NAIXNAME_B) $(TARGETDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.N*R
	-rm -f *.N*C
	-rm -f $(NARCNAME_F)
	-rm -f $(NAIXNAME_F)
	-rm -f $(NARCNAME_B)
	-rm -f $(NAIXNAME_B)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME_F)
	-rm -f $(TARGETDIR)/$(NAIXNAME_F)
	-rm -f $(TARGETDIR)/$(NARCNAME_B)
	-rm -f $(TARGETDIR)/$(NAIXNAME_B)


