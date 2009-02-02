#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.09.02	ohno_katsumi@gamefreak.co.jp
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada lee_hyunjung

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGETNAME	= test_c3
NARCNAME	= $(TARGETNAME).narc
NAIXNAME	= $(TARGETNAME).naix

EXTNAME		= 3dppack
#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/test_graphic


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

HEIGHTCNV =	$(PROJECT_ROOT)tools/heightcnv/imdcnv01
HEIGHTRETFILE = $(PROJECT_ROOT)tools/heightcnv/return.txt

#依存対象が複数ある
MAPNSBMDS	:= $(wildcard *.nsbmd)
MAPNSBTXS	:= $(wildcard *.nsbtx)
#MAPNSBTPS	:= $(wildcard *.nsbtp)
MAP3DMDS	:= $(wildcard *.3dmd)
MAPBHCS		:= $(wildcard *.bhc)

#MAP3DPPACKS = $(subst .nsbmd,.$(EXTNAME),$(MAPNSBMDS))

.imd.nsbmd:
	g3dcvtr $< -emdl
	g3dcvtr $< -etex

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	 .$(EXTNAME) .nsbmd .nsbtx .nsbtp .3dmd .bhc .imd .nsbmd

-include dependfile

MAP3DPPACKS	= $(ARCDEPENDS)

.imd.bhc:
	echo $< | sed -e 's/\.imd//' > heightlist.lst
	echo \#END >> heightlist.lst
	-$(HEIGHTCNV) heightlist.lst < $(HEIGHTRETFILE)

dependfile:	list
	ruby makedepend.rb list
endif


$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(MAP3DPPACKS)  
	nnsarc -c -l -n -i $(NARCNAME) -S arcfilelist
	$(NAIXCUT) $(NAIXNAME)


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------

clean:
	-rm -f $(TARGETDIR)/$(TARGETNAME).naix
	-rm -f $(TARGETDIR)/$(TARGETNAME).narc
	-rm -f $(TARGETNAME).naix
	-rm -f $(TARGETNAME).narc
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	-rm -f arcfilelist dependfile heightlist.lst
	-rm -f *.nsbtx *.nsbmd
	-rm -f *h.bhc
	-rm -f *.$(EXTNAME)
endif


height:
	ls *h.imd |sed -e 's/\.imd//' > heightlist.lst
	echo \#END >> height_data.lst
	#heightcnv/imdcnv01 height_data.lst
	-$(HEIGHTCNV) heightlist.lst < $(HEIGHTRETFILE)
	@echo finish height data convert

