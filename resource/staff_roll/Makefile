#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= hiro_nakamura toyama ohkubo_tomohiko ichiraku_katsuhiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = staff_roll

# GMMファイル
STAFF_LIST_GMMFILE_JP = staff_list_jp.gmm
STAFF_LIST_GMMFILE_ENG = staff_list_eng.gmm
# MSTXTファイル
STAFF_LIST_MSTXTFILE_JP = src/staff_list_jp.mstxt
STAFF_LIST_MSTXTFILE_ENG = src/staff_list_eng.mstxt

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

GMMDIR	= ../../resource/message/src/

#------------------------------------------------------------------------------
#※gmm->mstxt変換ツール
#------------------------------------------------------------------------------
UPDATEWITHGMM 	= ../../tools/message/UpdateWithGmm.bat
TYPE_MSTXTFILE = src/trtype.mstxt
MSPROJDIR	= ../message/msproj/


#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=


#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

#ICAコンバータ
ICACONV = ..\..\..\tools\ica\ica_converter.rb

#------------------------------------------------------------------------------
# グラフィックコンバータ
#------------------------------------------------------------------------------
SUBDIRS_TOP = .
# 1枚絵カット
SUBDIRS_CUT_01 = 2d_cut/cut01

SUBDIRS_ARC = $(SUBDIRS_TOP) $(SUBDIRS_CUT_01)

# 自動でリソースを集める
lsources:=$(wildcard $(addsuffix /*.ncl,$(SUBDIRS_ARC)))
gsources:=$(wildcard $(addsuffix /*.ncg,$(SUBDIRS_ARC)))
csources:=$(wildcard $(addsuffix /*.nsc,$(SUBDIRS_ARC)))

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(csources))

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .xls 

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

endif

.PHONY:	do-build clean subdir

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_direct_sheet.rb


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(MSPROJDIR)$(STAFF_LIST_MSTXTFILE_JP) $(MSPROJDIR)$(STAFF_LIST_MSTXTFILE_ENG)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する	ハッシュテーブルのコピーもする
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: WBStaffList.xls arclist.txt $(lobjects) $(gobjects) $(sobjects)
	ruby $(CSVCVTR) WBStaffList.xls -s -n 1 > staff_list_jp.csv
	ruby $(CSVCVTR) WBStaffList.xls -s -n 2 > staff_list_eng.csv
	perl staff_roll_gmm.pl staff_list_jp.csv ../message/template.gmm
	perl staff_roll_gmm.pl staff_list_eng.csv ../message/template.gmm
	perl staff_roll_cnv.pl staff_list_jp.csv
	perl staff_roll_cnv.pl staff_list_eng.csv
	nnsarc -c -l -n -i -f $(targetname) -S arclist.txt
	$(NAIXCUT) $(targetname).naix

# gmm -> mstxt 変換
$(MSPROJDIR)$(STAFF_LIST_MSTXTFILE_JP) : $(STAFF_LIST_GMMFILE_JP)
	$(UPDATEWITHGMM) . $(STAFF_LIST_GMMFILE_JP) $(STAFF_LIST_MSTXTFILE_JP)

$(MSPROJDIR)$(STAFF_LIST_MSTXTFILE_ENG) : $(STAFF_LIST_GMMFILE_ENG)
	$(UPDATEWITHGMM) . $(STAFF_LIST_GMMFILE_ENG) $(STAFF_LIST_MSTXTFILE_ENG)

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除

ifeq ($(sobjects),)
	rm -f $(sobjects)
endif
ifeq ($(gobjects),)
	rm -f $(gobjects)
endif
ifeq ($(lobjects),)
	rm -f $(lobjects)
endif

	rm -f *.nclr
	rm -f *.ncgr
	rm -f *.nscr
	rm -f *.narc
	rm -f *.naix
	rm -f $(SUBDIRS_TOP)/*.N*R
	rm -f $(SUBDIRS_CUT_01)/*.N*R
	rm -f *.dat
	rm -f *.csv
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


