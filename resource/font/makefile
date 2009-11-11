#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	taya

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= font
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/font/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

IMGFILE  := $(wildcard *.bmp)
FONTFILE := $(IMGFILE:.bmp=.nftr)
GFONTFILE := $(FONTFILE:.nftr=.gftr)
COLSRCFILE  := $(wildcard *.ncl)
COLFILE  := $(COLSRCFILE:.ncl=.nclr)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする


#bmpデータからの生成ルール
%.nftr: %.bmp
	fontcvtrc -i bmp -if $< -io pokemon_wb.xlor -ib 2 -o nitro -of $@
	xcopy /D /Y $@ w:\font


%.gftr: %.nftr
	fontex $< $@


%.nclr: %.ncl
	g2dcvtr $< -pcm




endif

.SUFFIXES: .bmp .nftr .gftr .ncl .nclr
.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME): $(FONTFILE) $(GFONTFILE) $(COLFILE) arclist.txt
	nnsarc -c -l -n -i $(NARCNAME) -S arclist.txt > error
	$(NAIXCUT) $(NAIXNAME)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f *.narc
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.nftr
	-rm -f *.gftr
	-rm -f *.NCLR
endif

#------------------------------------------------------------------------------
#	make sub ルール（使用する文字セットが変わった時にbmpを作り直し）
#------------------------------------------------------------------------------
sub:	convlist.bin

convlist.bin: pokemon_wb.xlor
	perl makesub.pl JIS_X0201_X0208_01.xlor pokemon_wb.xlor
	makesubfont largefull.bmpd convlist.bin large.bmp
	makesubfont smallfull.bmpd convlist.bin small.bmp

