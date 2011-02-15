
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	nagihashi_toru fuchino_hiroki

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gym_elec

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
FLDMAP = fieldmap
TARGETDIR = $(PROJECT_ARCDIR)$(FLDMAP)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


DEF_HEADER = gym_elec_local_def.h

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
ARCLIST = arc_list.txt

DATADIR	= data

#LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix
#コンバート対象者のみ、中間ファイル削除
#ifeq	($(CONVERTUSER),true)	
#LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt *.nsb* *.bin
#endif

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include

#------------------------------------------------------------------------------
#ツール
#------------------------------------------------------------------------------

ICA_CNV = ../../../../tools/ica/ica_converter.rb
DEF_MAKE = ../def_write.rb
XLS_CNV = ../../../../tools/exceltool/xls2xml/tab_out.rb

.PHONY:	do-build clean

IMDFILE = $(wildcard $(DATADIR)/*.imd)
ICAFILE = $(wildcard $(DATADIR)/*.ica)
IVAFILE = $(wildcard $(DATADIR)/*.iva)
IMAFILE = $(wildcard $(DATADIR)/*.ima)
ITPFILE = $(wildcard $(DATADIR)/*.itp)
ITAFILE = $(wildcard $(DATADIR)/*.ita)

NSBMDFILE = $(IMDFILE:.imd=.nsbmd)
NSBCAFILE = $(ICAFILE:.ica=.nsbca)
NSBVAFILE = $(IVAFILE:.iva=.nsbva)
NSBMAFILE = $(IMAFILE:.ima=.nsbma)
NSBTPFILE = $(ITPFILE:.itp=.nsbtp)
NSBTAFILE = $(ITAFILE:.ita=.nsbta)

OBJ_FILE 	= $(NSBMDFILE) $(NSBCAFILE) $(NSBVAFILE) \
			  $(NSBMAFILE) $(NSBTPFILE) $(NSBTAFILE)


do-build: $(TARGETDIR)/$(targetname).narc $(DEF_HEADER)

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f *.nsb*
	@rm -f $(DATADIR)/*.nsb*
	@rm -f *.bin
	@rm -f $(DEF_HEADER)
endif

#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

#imdデータからの生成ルール
$(DATADIR)/%.nsbmd:	$(DATADIR)/%.imd
	g3dcvtr $<
	cp $*.nsbmd $(DATADIR)

$(DATADIR)/%.nsbca:	$(DATADIR)/%.ica
	g3dcvtr $<
	cp $*.nsbca $(DATADIR)

$(DATADIR)/%.nsbta:	$(DATADIR)/%.ita
	g3dcvtr $<
	cp $*.nsbta $(DATADIR)

$(DATADIR)/%.nsbva:	$(DATADIR)/%.iva
	g3dcvtr $<
	cp $*.nsbva $(DATADIR)

$(DATADIR)/%.nsbma:	$(DATADIR)/%.ima
	g3dcvtr $<
	cp $*.nsbma $(DATADIR)

$(DATADIR)/%.nsbtp:	$(DATADIR)/%.itp
	g3dcvtr $<
	cp $*.nsbtp $(DATADIR)

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(OBJ_FILE)
	-ls *.nsb* > $(ARCLIST)
	$(ARCLIST_MAKE) $(ARCLIST)
	nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
	$(NAIXCUT) $(targetname).naix
#	@-rm -f *.nsb*
#	@-rm -f *.bin

$(DEF_HEADER): def_make.xls
	ruby $(XLS_CNV) -c def_make.xls > def_make.csv
	ruby $(DEF_MAKE) def_make.csv $(DEF_HEADER)
endif
