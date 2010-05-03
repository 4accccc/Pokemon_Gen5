#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= hiro_nakamura toyama

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = staff_roll

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

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)

lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))


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
CSVCVTR	= $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する	ハッシュテーブルのコピーもする
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: WBStaffList.xls arclist.txt $(lobjects) $(gobjects) $(sobjects)
	$(CSVCVTR) -o staff_list_jp.csv -p 3 -s csv WBStaffList.xls
	$(CSVCVTR) -o staff_list_eng.csv -p 4 -s csv WBStaffList.xls
	perl staff_roll_gmm.pl staff_list_jp.csv ../message/template.gmm
	perl staff_roll_gmm.pl staff_list_eng.csv ../message/template.gmm
	perl staff_roll_cnv.pl staff_list_jp.csv
	perl staff_roll_cnv.pl staff_list_eng.csv
	nnsarc -c -l -n -i $(targetname) -S arclist.txt
	$(NAIXCUT) $(targetname).naix

	$(COPY) staff_list_jp.gmm ../message/src
	$(COPY) staff_list_eng.gmm ../message/src
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.nclr
	rm -f *.ncgr
	rm -f *.nscr
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


