#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro murakami_naoto

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gate

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
SETTING_SOURCE = ./setting
SETTING_BIN = ./setting/bin

# 電光掲示板バイナリデータリストのマクロ定義をインクルード
include ./setting/bin/elboard_zone_data.list

CONVERTDATA = $(GRAPHIC_BIN)/gelboard01.nsbmd \
							$(GRAPHIC_BIN)/gelboard01.nsbtx \
							$(GRAPHIC_BIN)/gelboard01_1.nsbta \
							$(GRAPHIC_BIN)/gelboard01_2.nsbta \
							$(GRAPHIC_BIN)/gelboard01_3.nsbta \
							$(GRAPHIC_BIN)/gelboard01_4.nsbta \
							$(GRAPHIC_BIN)/gelboard01_5.nsbta \
							$(GRAPHIC_BIN)/gelboard01_6.nsbta \
							$(GRAPHIC_BIN)/gelboard01_7.nsbta \
							$(ELBOARD_ZONE_DATA)

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
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01.imd -emdl -o $(GRAPHIC_BIN)/gelboard01.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01.imd -etex -o $(GRAPHIC_BIN)/gelboard01.nsbtx
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_1.ita -o $(GRAPHIC_BIN)/gelboard01_1.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_2.ita -o $(GRAPHIC_BIN)/gelboard01_2.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_3.ita -o $(GRAPHIC_BIN)/gelboard01_3.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_4.ita -o $(GRAPHIC_BIN)/gelboard01_4.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_5.ita -o $(GRAPHIC_BIN)/gelboard01_5.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_6.ita -o $(GRAPHIC_BIN)/gelboard01_6.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_7.ita -o $(GRAPHIC_BIN)/gelboard01_7.nsbta

$(ELBOARD_ZONE_DATA): $(SETTING_SOURCE)/elboard_zone_data.txt
	ruby $(SETTING_SOURCE)/elboard_zone_data_conv.rb $(SETTING_SOURCE)/elboard_zone_data.txt $(SETTING_BIN)

$(SETTING_SOURCE)/elboard_zone_data.txt: $(SETTING_SOURCE)/elboard_zone_data.xls
	ruby $(XLSCONV) $(SETTING_SOURCE)/elboard_zone_data.xls > $(SETTING_SOURCE)/elboard_zone_data.txt

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


