#------------------------------------------------------------------------------
#  3Dリソースのコンバート > １つ下の階層にコピー
#  2009.12.03
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#ユーザー定義をインクルード
#------------------------------------------------------------------------------
include ..\make_users

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = intro

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

#ICAコンバータ
ICACONV = ..\..\..\tools\ica\ica_converter.rb

#リソースファイルの存在するディレクトリを指定
COPY_DIR = ./
#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./
#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../../tools/trgra/

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
esources:=$(wildcard *.nce)
mcsources:=$(wildcard *.nmc)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCBR,$(gsources))
eobjects = $(subst .nce,.NCEC,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))
maobjects = $(subst .nmc,.NMCR,$(mcsources))
mcobjects = $(subst .nmc,.NMAR,$(mcsources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NMCR .nmc .NCLR .ncl .NCBR .ncg .NCEC .nce

.nmc.NMCR:
	perl $(SCRIPT_DIR)tr_nmc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

.ncl.NCLR:
	perl $(SCRIPT_DIR)tr_ncl.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

.ncg.NCBR:
	perl $(SCRIPT_DIR)tr_ncgc.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)

.nce.NCEC:
	perl $(SCRIPT_DIR)tr_nce.pl $(COPY_DIR)$(notdir $<) $(CNV_DIR)
	g2dcvtr $<

endif


.PHONY:	do-build clean subdir


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバート処理

do-build:	subdir $(maobjects) $(mcobjects) $(lobjects) $(gobjects) $(eobjects) $(aobjects)

else

do-build:	subdir

endif

subdir:
	@$(MAKE_SUBDIR)



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.N*
endif



