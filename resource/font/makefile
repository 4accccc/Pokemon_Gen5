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

#DLプレイ用追加
targetname_dl	= font_dl
NARCNAME_DL	= $(targetname_dl).narc
NAIXNAME_DL	= $(targetname_dl).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/font/

#DLプレイ用追加
TARGETDIR_DL	= $(PROJECT_ROOT)/multiboot/arc/font/

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


do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする


font.narc : large.gftr small.gftr small_batt.gftr num.gftr default.nclr talkwin.nclr systemwin.nclr


large.nftr : large.bmp pokemon_wb.xlor
	fontcvtrc -i bmp -if large.bmp -io pokemon_wb.xlor -ib 2 -o nitro -of large.nftr
	xcopy /D /Y large.nftr w:\font

small.nftr : small.bmp pokemon_wb.xlor
	fontcvtrc -i bmp -if small.bmp -io pokemon_wb.xlor -ib 2 -o nitro -of small.nftr
	xcopy /D /Y small.nftr w:\font

small_batt.nftr : small_batt.bmp pokemon_wb.xlor
	fontcvtrc -i bmp -if small_batt.bmp -io pokemon_wb.xlor -ib 2 -o nitro -of small_batt.nftr
	xcopy /D /Y small_batt.nftr w:\font

num.nftr : num.bmp pokemon_wb_num.xlor
	fontcvtrc -i bmp -if num.bmp -io pokemon_wb_num.xlor -ib 2 -o nitro -of num.nftr
	xcopy /D /Y num.nftr w:\font

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

$(NARCNAME): $(FONTFILE) $(GFONTFILE) $(COLFILE) arclist.txt
	nnsarc -c -l -n -i $(NARCNAME) -S arclist.txt > error
	$(NAIXCUT) $(NAIXNAME)

do-build:	$(TARGETDIR_DL)/$(NARCNAME_DL)
#DLプレイ用追加
$(TARGETDIR_DL)/$(NARCNAME_DL):	$(NARCNAME_DL)
	$(COPY)	$(NARCNAME_DL) $(TARGETDIR_DL)
	$(COPY) $(NAIXNAME_DL) $(TARGETDIR_DL)

$(NARCNAME_DL): $(FONTFILE) $(GFONTFILE) $(COLFILE) arclist_dl.txt
	nnsarc -c -l -n -i $(NARCNAME_DL) -S arclist_dl.txt > error_dl
	$(NAIXCUT) $(NAIXNAME_DL)


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f $(TARGETDIR_DL)/$(NARCNAME_DL)
	-rm -f $(TARGETDIR_DL)/$(NAIXNAME_DL)
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

