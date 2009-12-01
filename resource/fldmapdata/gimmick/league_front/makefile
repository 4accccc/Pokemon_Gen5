#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro fujiwara_maiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = league_front

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

ICACONV = ..\..\..\..\tools\ica\ica_converter.rb
XLSCONV = ../../../../tools/exceltool/xls2xml/tab_out.rb
GRAPHIC_SOURCE = ./graphic
GRAPHIC_BIN = ./graphic/bin

CONVERTDATA = $(GRAPHIC_BIN)/pl_ele_00.nsbmd \
							$(GRAPHIC_BIN)/pl_ele_00.nsbca \
							$(GRAPHIC_BIN)/pl_ele_00.nsbta \
							$(GRAPHIC_BIN)/pl_ele_01.nsbmd \
							$(GRAPHIC_BIN)/pl_ele_01.nsbca \
							$(GRAPHIC_BIN)/pl_ele_01.nsbta \

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
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_00.imd -o $(GRAPHIC_BIN)/pl_ele_00.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_00.ica -o $(GRAPHIC_BIN)/pl_ele_00.nsbca
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_00.ita -o $(GRAPHIC_BIN)/pl_ele_00.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_01.imd -o $(GRAPHIC_BIN)/pl_ele_01.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_01.ica -o $(GRAPHIC_BIN)/pl_ele_01.nsbca
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_01.ita -o $(GRAPHIC_BIN)/pl_ele_01.nsbta 
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


