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
							$(GRAPHIC_BIN)/pl_ele_01.nsbmd \
							$(GRAPHIC_BIN)/pl_lite1.nsbmd \
							$(GRAPHIC_BIN)/pl_lite2.nsbmd \
							$(GRAPHIC_BIN)/pl_lite3.nsbmd \
							$(GRAPHIC_BIN)/pl_lite4.nsbmd \
							$(GRAPHIC_BIN)/pl_ele_01.nsbta \
							$(GRAPHIC_BIN)/pl_ele_00_00.nsbta \
							$(GRAPHIC_BIN)/pl_lite1_on.nsbta \
							$(GRAPHIC_BIN)/pl_lite2_on.nsbta \
							$(GRAPHIC_BIN)/pl_lite3_on.nsbta \
							$(GRAPHIC_BIN)/pl_lite4_on.nsbta \
							$(GRAPHIC_BIN)/pl_lite1_off.nsbta \
							$(GRAPHIC_BIN)/pl_lite2_off.nsbta \
							$(GRAPHIC_BIN)/pl_lite3_off.nsbta \
							$(GRAPHIC_BIN)/pl_lite4_off.nsbta \
							$(GRAPHIC_BIN)/pl_ele_00_ica.bin \
							$(GRAPHIC_BIN)/pl_ele_01_ica.bin

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
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_01.imd -o $(GRAPHIC_BIN)/pl_ele_01.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite1.imd -o $(GRAPHIC_BIN)/pl_lite1.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite2.imd -o $(GRAPHIC_BIN)/pl_lite2.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite3.imd -o $(GRAPHIC_BIN)/pl_lite3.nsbmd
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite4.imd -o $(GRAPHIC_BIN)/pl_lite4.nsbmd 
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_01.ita -o $(GRAPHIC_BIN)/pl_ele_01.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_ele_00_00.ita -o $(GRAPHIC_BIN)/pl_ele_00_00.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite1_off.ita -o $(GRAPHIC_BIN)/pl_lite1_off.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite2_off.ita -o $(GRAPHIC_BIN)/pl_lite2_off.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite3_off.ita -o $(GRAPHIC_BIN)/pl_lite3_off.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite4_off.ita -o $(GRAPHIC_BIN)/pl_lite4_off.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite1_on.ita -o $(GRAPHIC_BIN)/pl_lite1_on.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite2_on.ita -o $(GRAPHIC_BIN)/pl_lite2_on.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite3_on.ita -o $(GRAPHIC_BIN)/pl_lite3_on.nsbta
	g3dcvtr $(GRAPHIC_SOURCE)/pl_lite4_on.ita -o $(GRAPHIC_BIN)/pl_lite4_on.nsbta
	ruby $(ICACONV) -t $(GRAPHIC_SOURCE)/pl_ele_00.ica $(GRAPHIC_BIN)/pl_ele_00_ica.bin
	ruby $(ICACONV) -t $(GRAPHIC_SOURCE)/pl_ele_01.ica $(GRAPHIC_BIN)/pl_ele_01_ica.bin
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
	rm -f $(GRAPHIC_BIN)/*.bin
	rm -f $(GRAPHIC_BIN)/*.nsbca
	rm -f $(GRAPHIC_BIN)/*.nsbmd
	rm -f $(GRAPHIC_BIN)/*.nsbta
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


