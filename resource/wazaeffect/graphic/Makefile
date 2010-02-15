#------------------------------------------------------------------------------
#
#	技エフェクト用Makefile
#
#	2009.02.13	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include ../user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= waza_eff_gra
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix
HEADNAME	= $(targetname)_def.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)wazaeffect

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define

#リソースファイルの存在するディレクトリを指定
RES_DIR = ./

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバート対象のファイルツリーを生成
WAZAEFFNCE	:= $(wildcard $(RES_DIR)*.nce)
WAZAEFFNCG	:= $(wildcard $(RES_DIR)*.ncg)
WAZAEFFNSC	:= $(wildcard $(RES_DIR)*.nsc)
WAZAEFFNCL	:= $(wildcard $(RES_DIR)*.ncl)

.PHONY:	do-build clean waza_eff_gra.scr ncgr nscr nclr

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	waza_eff_gra.scr ncer ncgr nscr nclr $(NARCNAME) $(TARGETDIR)/$(NARCNAME)
else
do-build:	$(TARGETDIR)/$(NARCNAME)
endif

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

waza_eff_gra.scr:
	ruby mkscr.rb waza_eff_gra.scr

ncer: $(notdir $(WAZAEFFNCE:.nce=.NCER))
%.NCER: %.nce
	g2dcvtr $< -pcm -o/./

ncgr: $(notdir $(WAZAEFFNCG:.ncg=.NCGR))
%.NCGR: %.ncg
	g2dcvtr $< -bg -pcm -o/./

nscr: $(notdir $(WAZAEFFNSC:.nsc=.NSCR))
%.NSCR: %.nsc
	g2dcvtr $< -bg -pcm -o/./

nclr: $(notdir $(WAZAEFFNCL:.ncl=.NCLR))
%.NCLR: %.ncl
	g2dcvtr $< -pcm -o/./

$(NARCNAME): $(notdir $(WAZAEFFNCE:.nce=.NCER)) $(notdir $(WAZAEFFNCG:.ncg=.NCGR)) $(notdir $(WAZAEFFNSC:.nsc=.NSCR)) $(notdir $(WAZAEFFNCL:.ncl=.NCLR))
	nnsarc -c -l -n -i $(NARCNAME) -S waza_eff_gra.scr
	$(NAIXCUT) $(NAIXNAME)
	../../../tools/etd.exe $(NAIXNAME)
endif

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)
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
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
	-rm -f $(HEADNAME)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f $(TARGETDIR)/$(HEADNAME)


