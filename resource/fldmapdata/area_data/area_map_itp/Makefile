#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi fuchino ohkubo_tomohiko lee_hyunjung fujiwara_maiko ibe_mana tanoue_reiko unno james_turner

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname 	= area_map_itp
targetname2 = area_map_itp_tex
targetname3 = area_map_itp_tbl

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

##########################################
#コンバートファイル
##########################################
NCLFILE = $(wildcard *.ncl)
NCEFILE = $(wildcard *.nce)
NSCFILE = $(wildcard *.nsc)
IMDFILE = $(wildcard *.imd)
ICAFILE = $(wildcard *.ica)
IVAFILE = $(wildcard *.iva)
IMAFILE = $(wildcard *.ima)
ITPFILE = $(wildcard *.itp)
ITAFILE = $(wildcard *.ita)

XLSFILE = $(wildcard *.xls)

##########################################
#ターゲットファイル
##########################################
#2D
NCLRFILE = $(NCLFILE:.ncl=.NCLR)
NCERFILE = $(NCEFILE:.nce=.NCER)
NSCRFILE = $(NSCFILE:.nsc=.NSCR)
	
#3D
NSBTXFILE = $(IMDFILE:.imd=.nsbtx)
NSBCAFILE = $(ICAFILE:.ica=.nsbca)
NSBVAFILE = $(IVAFILE:.iva=.nsbva)
NSBMAFILE = $(IMAFILE:.ima=.nsbma)
NSBTPFILE = $(ITPFILE:.itp=.itpdat)
NSBTAFILE = $(ITAFILE:.ita=.nsbta)

#エクセルファイル
TBLFILE = $(XLSFILE:.xls=.itptbl)

LIST_FILE 	= list.txt
OBJ_FILE 	=  $(NSBTPFILE)
OBJ_FILE2 = $(NSBTXFILE)
OBJ_FILE3 = $(TBLFILE)


#ITPコンバータ
ITP_CONV = $(PROJECT_ROOT)tools/imdtools/itp_conv/itp_conv.exe

#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	


.SUFFIXES: .ncl .nce .nsc .NCLR .NCER .NSCR .imd .ica .iva .ima .itp .ita .nsbmd .nsbtx .nsbca .nsbma .nsbva .itpdat .nsbta .xls .itptbl

#コンバート環境
#2d
.ncl.NCLR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -br

.nsc.NSCR:
	g2dcvtr $<

#3d
.imd.nsbtx:
	g3dcvtr $< -etex

.ica.nsbca:
	g3dcvtr $<

.iva.nsbva:
	g3dcvtr $<

.ima.nsbma:
	g3dcvtr $<

.itp.itpdat:
	$(ITP_CONV) $< $@
	
.ita.nsbta:
	g3dcvtr $<

.xls.itptbl:
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $*.csv -p 0 -s csv $<
	perl conv.pl $*.csv $@ area_map_itp.naix area_map_itp_tex.naix


endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname2).narc $(TARGETDIR)/$(targetname3).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname2).narc: $(targetname2).narc
	$(COPY) $(targetname2).narc $(TARGETDIR)
	$(COPY) $(targetname2).naix $(TARGETDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname3).narc: $(targetname3).narc
	$(COPY) $(targetname3).narc $(TARGETDIR)
	$(COPY) $(targetname3).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(OBJ_FILE)
	-ls *.itpdat > $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)

# コピー先のnarcが古い場合実行する
$(targetname2).narc: $(OBJ_FILE2)
	-ls *.nsb* > $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname2) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname2).naix
	-rm	$(LIST_FILE)

# コピー先のnarcが古い場合実行する
$(targetname3).narc: $(OBJ_FILE3)
	-ls *.itptbl > $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname3) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname3).naix
	-rm	$(LIST_FILE)

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(TARGETDIR)/$(targetname2).narc
	rm -f $(TARGETDIR)/$(targetname2).naix
	rm -f $(TARGETDIR)/$(targetname3).narc
	rm -f $(TARGETDIR)/$(targetname3).naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(targetname2).narc
	rm -f $(targetname2).naix
	rm -f $(targetname3).narc
	rm -f $(targetname3).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.itpdat
	rm -f *.nsb*
	rm -f *.itptbl
	rm -f *.narc
	rm -f *.naix
endif



