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
targetname = h01

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

BIN     = ./bin
GRAPHIC = ./graphic
ICACONV = ..\..\..\..\tools\ica\ica_converter.rb
XLSCONV = ../../../../tools/exceltool/xls2xml/tab_out.rb

CONVERTDATA = $(BIN)/wind_data.bin \
							$(BIN)/trailer1_3dsu_data.bin \
							$(BIN)/trailer1_wait_data.bin \
							$(BIN)/trailer1_ica_data.bin \
							$(BIN)/trailer2_3dsu_data.bin \
							$(BIN)/trailer2_wait_data.bin \
							$(BIN)/trailer2_ica_data.bin \
							$(BIN)/ship_ica_data.bin \
							$(BIN)/ship_3dsu_data.bin \
							$(BIN)/ship_wait_data.bin \
							$(BIN)/h01_trailer.nsbmd \
							$(BIN)/h01_ship.nsbmd

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
	g3dcvtr $(GRAPHIC)/h01_trailer.imd -o $(BIN)/h01_trailer.nsbmd
	g3dcvtr $(GRAPHIC)/h01_ship.imd -o $(BIN)/h01_ship.nsbmd
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

$(BIN)/ship_ica_data.bin: $(GRAPHIC)/h01_ship.ica
	ruby $(ICACONV) -tr $(GRAPHIC)/h01_ship.ica $(BIN)/ship_ica_data.bin

$(BIN)/trailer1_ica_data.bin: $(GRAPHIC)/h01_trailer1.ica
	ruby $(ICACONV) -tr $(GRAPHIC)/h01_trailer1.ica $(BIN)/trailer1_ica_data.bin

$(BIN)/trailer2_ica_data.bin: $(GRAPHIC)/h01_trailer2.ica
	ruby $(ICACONV) -tr $(GRAPHIC)/h01_trailer2.ica $(BIN)/trailer2_ica_data.bin

$(BIN)/trailer1_3dsu_data.bin: h01.txt
	ruby h01.rb h01.txt ./bin

$(BIN)/trailer2_3dsu_data.bin: h01.txt
	ruby h01.rb h01.txt ./bin

$(BIN)/wind_data.bin: h01.txt
	ruby h01.rb h01.txt ./bin

$(BIN)/ship_3dsu_data.bin: h01.txt
	ruby h01.rb h01.txt ./bin

h01.txt: h01.xls
	ruby $(XLSCONV) h01.xls > h01.txt

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
	rm -f h01.txt
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


