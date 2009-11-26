#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro yoshida

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = season_display

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

GRAPHIC_SOURCE = ./graphic
GRAPHIC_BIN    = ./graphic/bin

CONVERTDATA = $(GRAPHIC_BIN)/spring.nclr \
							$(GRAPHIC_BIN)/spring.ncgr \
							$(GRAPHIC_BIN)/spring.nscr \
							$(GRAPHIC_BIN)/summer.nclr \
							$(GRAPHIC_BIN)/summer.ncgr \
							$(GRAPHIC_BIN)/summer.nscr \
							$(GRAPHIC_BIN)/autumn.nclr \
							$(GRAPHIC_BIN)/autumn.ncgr \
							$(GRAPHIC_BIN)/autumn.nscr \
							$(GRAPHIC_BIN)/winter.nclr \
							$(GRAPHIC_BIN)/winter.ncgr \
							$(GRAPHIC_BIN)/winter.nscr

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
	g2dcvtr $(GRAPHIC_SOURCE)/spring.ncl -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/spring.ncg -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/spring.nsc -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/summer.ncl -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/summer.ncg -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/summer.nsc -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/autumn.ncl -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/autumn.ncg -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/autumn.nsc -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/winter.ncl -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/winter.ncg -o/$(GRAPHIC_BIN) -bg
	g2dcvtr $(GRAPHIC_SOURCE)/winter.nsc -o/$(GRAPHIC_BIN) -bg
endif


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(CONVERTDATA)
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
	rm -f $(CONVERTDATA)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


