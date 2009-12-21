#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro fuchino kageyama_shota ichinose saito

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
							$(BIN)/trailer1_head_3dsu_data.bin \
							$(BIN)/trailer1_head_wait_data.bin \
							$(BIN)/trailer1_head_ica_data.bin \
							$(BIN)/trailer1_tail_3dsu_data.bin \
							$(BIN)/trailer1_tail_wait_data.bin \
							$(BIN)/trailer1_tail_ica_data.bin \
							$(BIN)/trailer2_head_3dsu_data.bin \
							$(BIN)/trailer2_head_wait_data.bin \
							$(BIN)/trailer2_head_ica_data.bin \
							$(BIN)/trailer2_tail_3dsu_data.bin \
							$(BIN)/trailer2_tail_wait_data.bin \
							$(BIN)/trailer2_tail_ica_data.bin \
							$(BIN)/ship_ica_data.bin \
							$(BIN)/ship_3dsu_data.bin \
							$(BIN)/ship_wait_data.bin \
							$(BIN)/trailer_01a.nsbmd \
							$(BIN)/trailer_01b.nsbmd \
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
	g3dcvtr $(GRAPHIC)/trailer_01a.imd -o $(BIN)/trailer_01a.nsbmd
	g3dcvtr $(GRAPHIC)/trailer_01b.imd -o $(BIN)/trailer_01b.nsbmd
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
	ruby $(GRAPHIC)/ica_trans_data_converter.rb $(GRAPHIC)/h01_ship.ica $(GRAPHIC)/ica_conv/h01_ship_conv.ica 2048 0 3072
	ruby $(ICACONV) -rt $(GRAPHIC)/ica_conv/h01_ship_conv.ica $(BIN)/ship_ica_data.bin

$(BIN)/trailer1_head_ica_data.bin: $(GRAPHIC)/trailer_01a_up.ica
	ruby $(GRAPHIC)/ica_trans_data_converter.rb $(GRAPHIC)/trailer_01a_up.ica $(GRAPHIC)/ica_conv/trailer_01a_up_conv.ica 2048 0 3072
	ruby $(ICACONV) -rt $(GRAPHIC)/ica_conv/trailer_01a_up_conv.ica $(BIN)/trailer1_head_ica_data.bin

$(BIN)/trailer1_tail_ica_data.bin: $(GRAPHIC)/trailer_01b_up.ica
	ruby $(GRAPHIC)/ica_trans_data_converter.rb $(GRAPHIC)/trailer_01b_up.ica $(GRAPHIC)/ica_conv/trailer_01b_up_conv.ica 2048 0 3072
	ruby $(ICACONV) -rt $(GRAPHIC)/ica_conv/trailer_01b_up_conv.ica $(BIN)/trailer1_tail_ica_data.bin

$(BIN)/trailer2_head_ica_data.bin: $(GRAPHIC)/trailer_01a_down.ica
	ruby $(GRAPHIC)/ica_trans_data_converter.rb $(GRAPHIC)/trailer_01a_down.ica $(GRAPHIC)/ica_conv/trailer_01a_down_conv.ica 2048 0 3072
	ruby $(ICACONV) -rt $(GRAPHIC)/ica_conv/trailer_01a_down_conv.ica $(BIN)/trailer2_head_ica_data.bin

$(BIN)/trailer2_tail_ica_data.bin: $(GRAPHIC)/trailer_01b_down.ica
	ruby $(GRAPHIC)/ica_trans_data_converter.rb $(GRAPHIC)/trailer_01b_down.ica $(GRAPHIC)/ica_conv/trailer_01b_down_conv.ica 2048 0 3072
	ruby $(ICACONV) -rt $(GRAPHIC)/ica_conv/trailer_01b_down_conv.ica $(BIN)/trailer2_tail_ica_data.bin

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
else
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
endif


