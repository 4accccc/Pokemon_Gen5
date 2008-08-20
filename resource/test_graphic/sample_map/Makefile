#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	watanabe tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= sample_map
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/test_graphic

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=	build


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define


#とりあえず直下の*.imdを依存対象とする
MAPIMDS	:= $(wildcard *.imd)
MAPMDLS = $(subst .imd,.nsbmd,$(MAPIMDS))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.nsbmd .imd

#imdデータからの生成ルール
.imd.nsbmd:
	g3dcvtr $< -emdl
	g3dcvtr $< -etex
endif


.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(NARCNAME)

subdir:
	@$(MAKE_SUBDIR)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(MAPMDLS) $(wildcard *.bin) build/dummy_file
	nnsarc -c -l -n -i $(NARCNAME) -S datalist.scr > error
	$(NAIXCUT) $(NAIXNAME)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	-rm -f *.nsbtx *.nsbmd
endif


