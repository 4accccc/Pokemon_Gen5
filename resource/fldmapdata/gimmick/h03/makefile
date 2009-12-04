#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro fujiwara_maiko kageyama_shota ichinose satou

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = h03

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

CONVERTDATA = h03_train1.nsbmd \
							train1_3ds_unit_data.bin \
							train2_3ds_unit_data.bin \
							train1_ica_data.bin \
							train2_ica_data.bin \
							train1_wait_data.bin \
							train2_wait_data.bin

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
	g3dcvtr h03_train1.imd
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

train1_ica_data.bin: h03_train1.ica
	ruby $(ICACONV) -t h03_train1.ica train1_ica_data.bin

train2_ica_data.bin: h03_train2.ica
	ruby $(ICACONV) -t h03_train2.ica train2_ica_data.bin

train1_3ds_unit_data.bin: h03.txt
	ruby h03_conv.rb

train2_3ds_unit_data.bin: h03.txt
	ruby h03_conv.rb

train1_wait_data.bin: h03.txt
	ruby h03_conv.rb

train2_wait_data.bin: h03.txt
	ruby h03_conv.rb

h03.txt: h03.xls
	ruby $(XLSCONV) h03.xls > h03.txt

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
	rm -f h03.txt
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


