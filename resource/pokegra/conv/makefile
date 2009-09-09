#------------------------------------------------------------------------------
#
#	ポケモングラフィックコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	yoshida sogabe

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
KANTO_RES_DIR = ../../../../pokemon_wb_doc/pokegra/kanto/
JYOTO_RES_DIR = ../../../../pokemon_wb_doc/pokegra/jyoto/
HOUEN_RES_DIR = ../../../../pokemon_wb_doc/pokegra/houen/
SHINOU_RES_DIR = ../../../../pokemon_wb_doc/pokegra/shinou/
ISSYU_RES_DIR = ../../../../pokemon_wb_doc/pokegra/issyu/

#LINK情報をCNV書き換えたファイルを格納するディレクトリを指定
COPY_DIR = ./

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#スクリプトが存在するディレクトリを指定
SCRIPT_DIR = ../../../tools/pokegra/

#コンバート対象のファイルツリーを生成
KANTO_NCGFILE		:= $(wildcard $(KANTO_RES_DIR)????_???_?.ncg)
KANTO_NCGCFILE	:= $(wildcard $(KANTO_RES_DIR)????_???c_?.ncg)
KANTO_NMCFILE		:= $(wildcard $(KANTO_RES_DIR)*.nmc)
KANTO_NCEFILE		:= $(wildcard $(KANTO_RES_DIR)*.nce)
KANTO_NCLNFILE	:= $(wildcard $(KANTO_RES_DIR)*_n.ncl)
KANTO_NCLRFILE	:= $(wildcard $(KANTO_RES_DIR)*_r.ncl)

JYOTO_NCGFILE		:= $(wildcard $(JYOTO_RES_DIR)????_???_?.ncg)
JYOTO_NCGCFILE	:= $(wildcard $(JYOTO_RES_DIR)????_???c_?.ncg)
JYOTO_NMCFILE		:= $(wildcard $(JYOTO_RES_DIR)*.nmc)
JYOTO_NCEFILE		:= $(wildcard $(JYOTO_RES_DIR)*.nce)
JYOTO_NCLNFILE	:= $(wildcard $(JYOTO_RES_DIR)*_n.ncl)
JYOTO_NCLRFILE	:= $(wildcard $(JYOTO_RES_DIR)*_r.ncl)

HOUEN_NCGFILE		:= $(wildcard $(HOUEN_RES_DIR)????_???_?.ncg)
HOUEN_NCGCFILE	:= $(wildcard $(HOUEN_RES_DIR)????_???c_?.ncg)
HOUEN_NMCFILE		:= $(wildcard $(HOUEN_RES_DIR)*.nmc)
HOUEN_NCEFILE		:= $(wildcard $(HOUEN_RES_DIR)*.nce)
HOUEN_NCLNFILE	:= $(wildcard $(HOUEN_RES_DIR)*_n.ncl)
HOUEN_NCLRFILE	:= $(wildcard $(HOUEN_RES_DIR)*_r.ncl)

SHINOU_NCGFILE	:= $(wildcard $(SHINOU_RES_DIR)????_???_?.ncg)
SHINOU_NCGCFILE	:= $(wildcard $(SHINOU_RES_DIR)????_???c_?.ncg)
SHINOU_NMCFILE	:= $(wildcard $(SHINOU_RES_DIR)*.nmc)
SHINOU_NCEFILE	:= $(wildcard $(SHINOU_RES_DIR)*.nce)
SHINOU_NCLNFILE	:= $(wildcard $(SHINOU_RES_DIR)*_n.ncl)
SHINOU_NCLRFILE	:= $(wildcard $(SHINOU_RES_DIR)*_r.ncl)

ISSYU_NCGFILE		:= $(wildcard $(ISSYU_RES_DIR)????_???_?.ncg)
ISSYU_NCGCFILE	:= $(wildcard $(ISSYU_RES_DIR)????_???c_?.ncg)
ISSYU_NMCFILE		:= $(wildcard $(ISSYU_RES_DIR)*.nmc)
ISSYU_NCEFILE		:= $(wildcard $(ISSYU_RES_DIR)*.nce)
ISSYU_NCLNFILE	:= $(wildcard $(ISSYU_RES_DIR)*_n.ncl)
ISSYU_NCLRFILE	:= $(wildcard $(ISSYU_RES_DIR)*_r.ncl)

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

do-build: $(COPY_DIR)$(notdir $(KANTO_NCGFILE))
%.ncg: $(KANTO_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(KANTO_NCGCFILE))
%.ncg: $(KANTO_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(KANTO_NCEFILE))
%.nce: $(KANTO_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(KANTO_NMCFILE))
%.nmc: $(KANTO_RES_DIR)%.nmc $(KANTO_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(KANTO_NCLNFILE))
%.ncl: $(KANTO_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(KANTO_NCLRFILE))
%.ncl: $(KANTO_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(JYOTO_NCGFILE))
%.ncg: $(JYOTO_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(JYOTO_NCGCFILE))
%.ncg: $(JYOTO_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(JYOTO_NCEFILE))
%.nce: $(JYOTO_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(JYOTO_NMCFILE))
%.nmc: $(JYOTO_RES_DIR)%.nmc $(JYOTO_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(JYOTO_NCLNFILE))
%.ncl: $(JYOTO_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(JYOTO_NCLRFILE))
%.ncl: $(JYOTO_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(HOUEN_NCGFILE))
%.ncg: $(HOUEN_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(HOUEN_NCGCFILE))
%.ncg: $(HOUEN_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(HOUEN_NCEFILE))
%.nce: $(HOUEN_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(HOUEN_NMCFILE))
%.nmc: $(HOUEN_RES_DIR)%.nmc $(HOUEN_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(HOUEN_NCLNFILE))
%.ncl: $(HOUEN_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(HOUEN_NCLRFILE))
%.ncl: $(HOUEN_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(SHINOU_NCGFILE))
%.ncg: $(SHINOU_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(SHINOU_NCGCFILE))
%.ncg: $(SHINOU_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(SHINOU_NCEFILE))
%.nce: $(SHINOU_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(SHINOU_NMCFILE))
%.nmc: $(SHINOU_RES_DIR)%.nmc $(SHINOU_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(SHINOU_NCLNFILE))
%.ncl: $(SHINOU_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(SHINOU_NCLRFILE))
%.ncl: $(SHINOU_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(ISSYU_NCGFILE))
%.ncg: $(ISSYU_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(ISSYU_NCGCFILE))
%.ncg: $(ISSYU_RES_DIR)%.ncg
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(ISSYU_NCEFILE))
%.nce: $(ISSYU_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(ISSYU_NMCFILE))
%.nmc: $(ISSYU_RES_DIR)%.nmc $(ISSYU_RES_DIR)%.nce
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(ISSYU_NCLNFILE))
%.ncl: $(ISSYU_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

do-build: $(COPY_DIR)$(notdir $(ISSYU_NCLRFILE))
%.ncl: $(ISSYU_RES_DIR)%.ncl
	ruby $(SCRIPT_DIR)nclinkrw.rb $< $(COPY_DIR)

endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm *.*
endif


