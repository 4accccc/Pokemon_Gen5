#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= nagihashi_toru ibe_mana ohkubo_tomohiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname			= field_goout_effect
targetname_dat	= field_goout_data
xlsname					= field_goout_effect_data
converter				= field_goout_effect_converter.rb
xls2csv					= ../../tools/exceltool/xls2xml/tab_out_sheet.rb
zonename				= ..\fldmapdata\zonetable\zone_id.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR 			= $(PROJECT_ARCDIR)
TARGETDIR_PROG	= $(PROJECT_ROOT)/prog/src/field

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

LIST_FILE 	= list.txt
OBJ_FILE 	= $(NCLRFILE) $(NCERFILE) $(NSCRFILE) \
			  $(NSBMDFILE) $(NSBCAFILE) $(NSBVAFILE) \
			  $(NSBMAFILE) $(NSBTPFILE) $(NSBTAFILE)



#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	


.SUFFIXES: .ncl .nce .nsc .NCLR .NCER .NSCR .imd .ica .iva .ima .itp .ita .nsbmd .nsbtx .nsbca .nsbma .nsbva .nsbtp .nsbta
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

endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix $(TARGETDIR_PROG)/$(targetname_dat).cdat $(TARGETDIR_PROG)/$(targetname_dat).h $(TARGETDIR)/$(targetname_dat).narc $(TARGETDIR)/$(targetname_dat).naix

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)

$(TARGETDIR)/$(targetname).naix: $(targetname).naix
	$(COPY) $(targetname).naix $(TARGETDIR)

$(TARGETDIR)/$(targetname_dat).narc: $(targetname_dat).narc
	$(COPY) $(targetname_dat).narc $(TARGETDIR)

$(TARGETDIR)/$(targetname_dat).naix: $(targetname_dat).naix
	$(COPY) $(targetname_dat).naix $(TARGETDIR)

$(TARGETDIR_PROG)/$(targetname_dat).cdat:	$(targetname_dat).cdat
	$(COPY) $(targetname_dat).cdat $(TARGETDIR_PROG)

$(TARGETDIR_PROG)/$(targetname_dat).h:	$(targetname_dat).h
	$(COPY) $(targetname_dat).h $(TARGETDIR_PROG)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	

$(targetname_dat).narc $(targetname_dat).naix: $(targetname_dat).dat
	nnsarc -c -l -n -i $(targetname_dat) $^
	$(NAIXCUT) $(targetname_dat).naix

$(targetname_dat).dat $(targetname_dat).cdat $(targetname_dat).h: $(xlsname).xls $(zonename) $(targetname).naix
	ruby $(xls2csv) -n 0 -c $(xlsname).xls > $(xlsname)_data.csv
	ruby $(xls2csv) -n 1 -c $(xlsname).xls > $(xlsname)_def.csv
	ruby $(converter) $(xlsname)_data.csv $(xlsname)_def.csv $(zonename) $(targetname).naix $(targetname_dat)

$(targetname).narc $(targetname).naix: $(OBJ_FILE)
	-ls *.nsb* > $(LIST_FILE)
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
	rm -f *.csv
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif



