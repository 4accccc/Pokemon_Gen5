#! make -f
#----------------------------------------------------------------------------
# Project:  BGパレットアニメ リソースコンバート
# File:     Makefile
# Autor:	hosaka genya
# update	ichiraku katsuhiko 2010/11/2
# Copyright 2008 GameFreak.inc  All rights reserved.
#
#---------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include ../user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= batt_bg_planm
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix
HEADNAME	= $(targetname)_def.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)wazaeffect

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define

# datの構造ヘッダのコピー先
BG_HEADER_DIR	= ./../../../prog/src/battle/btlv

#リソースファイルの存在するディレクトリを指定
RES_DIR = ./

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバート対象のファイルツリーを生成
RES_NCE	:= $(wildcard $(RES_DIR)*.nce)
RES_NCL	:= $(wildcard $(RES_DIR)*.ncl)

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build: dat nclr batt_bg_planm.scr $(NARCNAME) $(TARGETDIR)/$(NARCNAME) $(TARGETDIR)/$(NAIXNAME) $(TARGETDIR)/$(HEADNAME) $(BG_HEADER_DIR)/bg_planm_data.h
else
do-build:	$(TARGETDIR)/$(NARCNAME) $(TARGETDIR)/$(NAIXNAME) $(TARGETDIR)/$(HEADNAME)
endif

#---------------------------------------------------------------------------


#---------------------------------------------------------------------------
# ●動作
# 	BgPlttAnimConv に対象ファイルを登録 -> dat作成
# 	g2dcvtr -> NCLR作成
# 	narc作成
# 	pokemon_gs にコピー
#---------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

batt_bg_planm.scr: *.nce *.ncl mkscr.rb
	ruby mkscr.rb batt_bg_planm.scr

dat: $(notdir $(RES_NCE:.nce=.dat))
%.dat: %.nce
	BgPlttAnimConv.exe $<

nclr: $(notdir $(RES_NCL:.ncl=.NCLR))
%.NCLR: %.ncl
	g2dcvtr $< -pcm -o/./

$(NARCNAME): $(notdir $(RES_NCE:.nce=.dat)) $(notdir $(RES_NCL:.ncl=.NCLR)) batt_bg_planm.scr
	nnsarc -c -l -n -i $(NARCNAME) -S batt_bg_planm.scr
	$(NAIXCUT) $(NAIXNAME)
	../../../tools/etd.exe $(NAIXNAME)

endif

$(BG_HEADER_DIR)/bg_planm_data.h: bg_planm_data.h
	$(COPY) bg_planm_data.h $(BG_HEADER_DIR)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)

$(TARGETDIR)/$(NAIXNAME):	$(NAIXNAME)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(TARGETDIR)/$(HEADNAME):	$(HEADNAME)
	$(COPY) $(HEADNAME) $(TARGETDIR)


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.scr
	-rm -f *.NCER
	-rm -f *.NCGR
	-rm -f *.NSCR
	-rm -f *.NCLR
	-rm -f *.dat
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f $(TARGETDIR)/$(HEADNAME)


