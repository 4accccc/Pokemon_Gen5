#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.09.02	ohno_katsumi@gamefreak.co.jp
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ohno tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGETNAME	= loopbridge
NARCNAME	= $(TARGETNAME).narc
NAIXNAME	= $(TARGETNAME).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/test_graphic


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define


#依存対象が複数ある
MAPNSBMDS	:= $(wildcard *.nsbmd)
MAPNSBTXS	:= $(wildcard *.nsbtx)
#MAPNSBTPS	:= $(wildcard *.nsbtp)
MAP3DMDS	:= $(wildcard *.3dmd)
MAPBHCS		:= $(wildcard *.bhc)

MAP3DPPACKS = $(subst .nsbmd,.3dppack,$(MAPNSBMDS))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	 .3dppack .nsbmd .nsbtx .nsbtp .3dmd .bhc .imd .nsbmd

.imd.nsbmd:
	g3dcvtr $< -emdl
	g3dcvtr $< -etex

#データからの生成ルール
.nsbmd.3dppack:
.nsbtx.3dppack:
.3dmd.3dppack:
.bhc.3dppack:
	$(BINLINKER) $*.nsbmd $*.nsbtx $*.bhc $*.3dmd $*.3dppack BR

endif


.PHONY:	do-build clean height

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)


$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(MAP3DPPACKS)  $(MAP3DMDS) $(MAPNSBMDS) $(MAPNSBTXS) $(MAPBHCS) #$(MAPNSBTPS)
	nnsarc -c -l -n -i $(NARCNAME) -S data.list
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
	-rm -f *.3dppack
endif


height:
	ls *.imd |sed -e 's/\.imd//' > height_data.lst
	echo \#END >> height_data.lst
	heightcnv/imdcnv01 height_data.lst
