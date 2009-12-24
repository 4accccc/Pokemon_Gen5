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
targetname_gate   = gate
targetname_zone   = zone
targetname_spnews = spnews

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

ICACONV             = ..\..\..\..\tools\ica\ica_converter.rb
XLSCONV             = ../../../../tools/exceltool/xls2xml/tab_out.rb
GRAPHIC_SOURCE      = ./graphic
GRAPHIC_BIN         = ./graphic/bin
SETTING_SOURCE      = ./setting
SETTING_BIN         = ./setting/bin
BIN_ZONE_DATA       = $(SETTING_BIN)/zone_data
BIN_SPNEWS_DATA     = $(SETTING_BIN)/spnews_data
ELBOARD_ZONE_DATA   = $(BIN_ZONE_DATA)/*.bin 
ELBOARD_SPNEWS_DATA = $(BIN_SPNEWS_DATA)/*.bin

CONVERTDATA_GATE = $(GRAPHIC_BIN)/gelboard01.nsbmd \
							$(GRAPHIC_BIN)/gelboard01.nsbtx \
							$(GRAPHIC_BIN)/gelboard01_1.nsbta \
							$(GRAPHIC_BIN)/gelboard01_2.nsbta \
							$(GRAPHIC_BIN)/gelboard01_3.nsbta \
							$(GRAPHIC_BIN)/gelboard01_4.nsbta \
							$(GRAPHIC_BIN)/gelboard01_5.nsbta \
							$(GRAPHIC_BIN)/gelboard01_6.nsbta \
							$(GRAPHIC_BIN)/gelboard01_7.nsbta \
							$(GRAPHIC_BIN)/gelboard01_tv01.nsbtp \
							$(GRAPHIC_BIN)/gelboard01_tv02.nsbtp \
							$(GRAPHIC_BIN)/gelboard01_tv03.nsbtp
CONVERTDATA_ZONE   = $(ELBOARD_ZONE_DATA)
CONVERTDATA_SPNEWS = $(ELBOARD_SPNEWS_DATA)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 
endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname_gate).narc $(TARGETDIR)/$(targetname_zone).narc $(TARGETDIR)/$(targetname_spnews).narc

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
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_tv01.itp -o $(GRAPHIC_BIN)/gelboard01_tv01.nsbtp
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_tv02.itp -o $(GRAPHIC_BIN)/gelboard01_tv02.nsbtp
	g3dcvtr $(GRAPHIC_SOURCE)/gelboard01_tv03.itp -o $(GRAPHIC_BIN)/gelboard01_tv03.nsbtp

$(ELBOARD_ZONE_DATA): $(SETTING_SOURCE)/elboard_zone_data.txt
	ruby $(SETTING_SOURCE)/elboard_zone_data_conv.rb $< $(BIN_ZONE_DATA)

$(SETTING_SOURCE)/elboard_zone_data.txt: $(SETTING_SOURCE)/elboard_zone_data.xls
	ruby $(XLSCONV) $< > $@

$(ELBOARD_SPNEWS_DATA): $(SETTING_SOURCE)/elboard_spnews_data.txt
	ruby $(SETTING_SOURCE)/elboard_spnews_data.rb $< $(BIN_SPNEWS_DATA)

$(SETTING_SOURCE)/elboard_spnews_data.txt: $(SETTING_SOURCE)/elboard_spnews_data.xls
	ruby $(XLSCONV) $< > $@

endif


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_gate).narc: $(targetname_gate).narc
	$(COPY) $(targetname_gate).narc $(TARGETDIR)
	$(COPY) $(targetname_gate).naix $(TARGETDIR) 

$(TARGETDIR)/$(targetname_zone).narc: $(targetname_zone).narc
	$(COPY) $(targetname_zone).narc $(TARGETDIR)
	$(COPY) $(targetname_zone).naix $(TARGETDIR) 

$(TARGETDIR)/$(targetname_spnews).narc: $(targetname_spnews).narc
	$(COPY) $(targetname_spnews).narc $(TARGETDIR)
	$(COPY) $(targetname_spnews).naix $(TARGETDIR) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname_gate).narc: $(CONVERTDATA_GATE)
	nnsarc -r -l -n -i $(targetname_gate) $^
	$(NAIXCUT) $(targetname_gate).naix

$(targetname_zone).narc: $(CONVERTDATA_GATE)
	nnsarc -r -l -n -i $(targetname_zone) $^
	$(NAIXCUT) $(targetname_zone).naix

$(targetname_spnews).narc: $(CONVERTDATA_SPNEWS)
	nnsarc -r -l -n -i $(targetname_spnews) $^
	$(NAIXCUT) $(targetname_spnews).naix

endif



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(CONVERTDATA_GATE)
	rm -f $(CONVERTDATA_SPNEWS)
	rm -f $(targetname_gate).narc
	rm -f $(targetname_gate).naix
	rm -f $(targetname_zone).narc
	rm -f $(targetname_zone).naix
	rm -f $(targetname_spnews).narc
	rm -f $(targetname_spnews).naix
endif
	rm -f $(TARGETDIR)$(targetname_gate).narc
	rm -f $(TARGETDIR)$(targetname_gate).naix
	rm -f $(TARGETDIR)$(targetname_zone).narc
	rm -f $(TARGETDIR)$(targetname_zone).naix
	rm -f $(TARGETDIR)$(targetname_spnews).narc
	rm -f $(TARGETDIR)$(targetname_spnews).naix


