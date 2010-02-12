#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = research_radar_graphic

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

XLSCONV        = ../../../../tools/exceltool/xls2xml/tab_out_sheet.rb
GRAPHIC_SOURCE = .
GRAPHIC_BIN    = bin

GRAPHIC_DATA = $(GRAPHIC_BIN)/bgd.NCLR \
							 $(GRAPHIC_BIN)/bgd.NCGR \
							 $(GRAPHIC_BIN)/bgd_base.NSCR \
							 $(GRAPHIC_BIN)/bgd_topbtn.NSCR \
							 $(GRAPHIC_BIN)/bgu.NCLR \
							 $(GRAPHIC_BIN)/bgu.NCGR \
							 $(GRAPHIC_BIN)/bgu_base.NSCR \
							 $(GRAPHIC_BIN)/bgu_win1.NSCR \
							 $(GRAPHIC_BIN)/obj.NCER \
							 $(GRAPHIC_BIN)/obj.NCGR \
							 $(GRAPHIC_BIN)/obj.NCLR \
							 $(GRAPHIC_BIN)/obj.NANR



ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 
endif

.PHONY:	do-build clean subdir


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# コンバート
	g2dcvtr $(GRAPHIC_SOURCE)/bgd.ncl -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgd.ncg -bg -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgd_base.nsc -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgd_topbtn.nsc -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgu.ncl -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgu.ncg -bg -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgu_base.nsc -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/bgu_win1.nsc -o/$(GRAPHIC_BIN)
	g2dcvtr $(GRAPHIC_SOURCE)/obj.nce -lbl -o/$(GRAPHIC_BIN)
endif


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(GRAPHIC_DATA)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix
endif


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(GRAPHIC_DATA)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)$(targetname).narc
	rm -f $(TARGETDIR)$(targetname).naix


