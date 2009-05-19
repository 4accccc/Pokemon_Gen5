#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ibe_mana tanoue_reiko yoshida sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= pokegra_wb
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)pokegra

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#NARCに必要なscrファイル名
SCRFILE = pokegra_wb.scr

#リソースファイルの存在するディレクトリを指定
RES_DIR = ../../../../pokemon_wb_doc/pokegra/

#LINK情報をCNV書き換えたファイルを格納するディレクトリを指定
COPY_DIR = ./

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../../tools/pokegra/

#コンバート対象のファイルツリーを生成
NCGFILE	:= $(wildcard $(RES_DIR)????_???_?.ncg)
NCGCFILE	:= $(wildcard $(RES_DIR)????_???c_?.ncg)
NMCFILE	:= $(wildcard $(RES_DIR)*.nmc)
NCEFILE	:= $(wildcard $(RES_DIR)*.nce)
NCLNFILE	:= $(wildcard $(RES_DIR)*_n.ncl)
NCLRFILE	:= $(wildcard $(RES_DIR)*_r.ncl)

.PHONY:	do-build clean subdir

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build: $(COPY_DIR)$(notdir $(NCGFILE))
%.ncg: $(RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(NCGCFILE))
%.ncg: $(RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(NCEFILE))
%.nce: $(RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(NMCFILE))
%.nmc: $(RES_DIR)%.nmc $(RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(NCLNFILE))
%.ncl: $(RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(NCLRFILE))
%.ncl: $(RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)
endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm *.*
endif


