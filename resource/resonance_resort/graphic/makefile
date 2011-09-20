
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	nagihashi_toru fuchino_hiroki shimoyamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = resonance_resort_graphic

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

#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
ARCLIST = arc_list.txt

#------------------------------------------------------------------------------
#ツール
#------------------------------------------------------------------------------

.PHONY:	do-build clean

IMDFILE = $(wildcard *.imd)
ICAFILE = $(wildcard *.ica)
IVAFILE = $(wildcard *.iva)
IMAFILE = $(wildcard *.ima)
ITPFILE = $(wildcard *.itp)
ITAFILE = $(wildcard *.ita)

NSBMDFILE = $(IMDFILE:.imd=.nsbmd)
NSBCAFILE = $(ICAFILE:.ica=.nsbca)
NSBVAFILE = $(IVAFILE:.iva=.nsbva)
NSBMAFILE = $(IMAFILE:.ima=.nsbma)
NSBTPFILE = $(ITPFILE:.itp=.nsbtp)
NSBTAFILE = $(ITAFILE:.ita=.nsbta)

OBJ_FILE 	= $(NSBMDFILE) $(NSBCAFILE) $(NSBVAFILE) \
			  $(NSBMAFILE) $(NSBTPFILE) $(NSBTAFILE)


do-build: $(TARGETDIR)/$(targetname).narc

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f *.nsb*
endif

#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

#imdデータからの生成ルール
%.nsbmd:	%.imd
	g3dcvtr $<

%.nsbca:	%.ica
	g3dcvtr $<

%.nsbta:	%.ita
	g3dcvtr $<

%.nsbva:	%.iva
	g3dcvtr $<

%.nsbma:	%.ima
	g3dcvtr $<

%.nsbtp:	%.itp
	g3dcvtr $<

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(OBJ_FILE)
	-ls *.nsb* > $(ARCLIST)
	$(ARCLIST_MAKE) $(ARCLIST)
	nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
	$(NAIXCUT) $(targetname).naix
endif
