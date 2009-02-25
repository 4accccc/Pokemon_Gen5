#------------------------------------------------------------------------------
#
#	ノーグリッドマップコンバート用Makefile
#
#	2009.02.23	tamada@gamefreak.co.jp
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada fuchino lee_hyunjung

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGETNAME	= nogrid_mapblock
NARCNAME	= $(TARGETNAME).narc
NAIXNAME	= $(TARGETNAME).naix

EXTNAME		= 3dppack
OUTDIR		= output
#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

HEIGHTCNV =	$(PROJECT_ROOT)tools/heightcnv/imdcnv01
HEIGHTRETFILE = $(PROJECT_ROOT)tools/heightcnv/return.txt


.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	 .$(EXTNAME) .nsbmd .nsbtx .nsbtp .3dmd .bhc .imd .nsbmd

-include $(OUTDIR)/dependfile

#ARCDEPENDSはdependfile内で定義
MAP3DPPACKS	= $(ARCDEPENDS)


$(OUTDIR)/dependfile:	list
	ruby scripts/makedepend.rb list
endif


$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(MAP3DPPACKS)  
	nnsarc -c -l -n -i $(NARCNAME) -S $(OUTDIR)/arcfilelist
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
	-rm -f $(OUTDIR)/arcfilelist $(OUTDIR)/dependfile heightlist.lst
	-rm -f $(OUTDIR)/*.bhc
	-rm -f $(OUTDIR)/*.nsbmd
	-rm -f *.$(EXTNAME)
endif


