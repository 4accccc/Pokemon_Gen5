#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi fujiwara_maiko #ohno #tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = fourkings_scene

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

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
ICAFILE = c09_01_mf.ica c09_02_mf.ica c09_03_mf.ica c09_04_mf.ica
CAMERAICAFILE = c09_01_camera.ica c09_02_camera.ica c09_03_camera.ica c09_04_camera.ica
SEFILE = c09_01_se.ica c09_02_se.ica c09_03_se.ica c09_04_se.ica
IVAFILE = $(wildcard *.iva)
IMAFILE = $(wildcard *.ima)
ITPFILE = $(wildcard *.itp)
ITAFILE = $(wildcard *.ita)

TXTFILE = $(wildcard *.txt)
XLSFILE = $(wildcard *.xls)

##########################################
#ターゲットファイル
##########################################
#2D
NCLRFILE = $(NCLFILE:.ncl=.NCLR)
NCERFILE = $(NCEFILE:.nce=.NCER)
NSCRFILE = $(NSCFILE:.nsc=.NSCR)
	
#3D
NSBMDFILE = $(IMDFILE:.imd=.nsbmd)
NSBCAFILE = $(ICAFILE:.ica=.nsbca)
NSBVAFILE = $(IVAFILE:.iva=.nsbva)
NSBMAFILE = $(IMAFILE:.ima=.nsbma)
NSBTPFILE = $(ITPFILE:.itp=.nsbtp)
NSBTAFILE = $(ITAFILE:.ita=.nsbta)

BINFILE = $(CAMERAICAFILE:.ica=.bin)

SEBIN_FILE = $(SEFILE:.ica=.se)

#DAT
DATFILE = $(XLSFILE:.xls=.dat)

LIST_FILE 	= list.txt
OBJ_FILE 	= $(NCLRFILE) $(NCERFILE) $(NSCRFILE) \
			  $(NSBMDFILE) $(NSBCAFILE) $(NSBVAFILE) \
			  $(NSBMAFILE) $(NSBTPFILE) $(NSBTAFILE) \
			  $(DATFILE) $(BINFILE) $(SEBIN_FILE)



#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	


.SUFFIXES: .ncl .nce .nsc .NCLR .NCER .NSCR .imd .ica .iva .ima .itp .ita .nsbmd .nsbtx .nsbca .nsbma .nsbva .nsbtp .nsbta .xls .dat .bin .se

#コンバート環境
#2d
.ncl.NCLR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -br

.nsc.NSCR:
	g2dcvtr $<

#3d
.imd.nsbmd:
	g3dcvtr $<

.ica.nsbca:
	g3dcvtr $<

.iva.nsbva:
	g3dcvtr $<

.ima.nsbma:
	g3dcvtr $<

.itp.nsbtp:
	g3dcvtr $<

.ita.nsbta:
	g3dcvtr $<

.ica.bin:
	ruby $(PROJECT_ROOT)tools/ica/ica_converter.rb -srt $< $@

.ica.se:
	ruby $(PROJECT_ROOT)tools/ica/ica_converter.rb -srt $< $@


#light
.xls.dat:
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $*.csv -p 0 -s csv $<
	perl ../field_light/conv.pl $*.csv $@

endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(OBJ_FILE) $(TXTFILE)
	-ls *.txt > $(LIST_FILE)
	-ls *.dat > $(LIST_FILE)
	-ls *.N*R >> $(LIST_FILE)
	-ls *.nsb* >> $(LIST_FILE)
	-ls *.bin >> $(LIST_FILE)
	-ls *.se >> $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.N*R
	rm -f *.nsb*
	rm -f *.dat
	rm -f *.bin
	rm -f *.se
	rm -f *.csv
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif



