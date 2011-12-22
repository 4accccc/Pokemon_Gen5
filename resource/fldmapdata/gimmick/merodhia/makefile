
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ichiraku_katsuhiko shiiyama_yukiko saya_tsuruta

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = mero

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
FLDMAP = fieldmap
TARGETDIR = $(PROJECT_ARCDIR)$(FLDMAP)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
ICACONV = ..\..\..\..\tools\ica\ica_converter.rb
DATA_SOURCE = ./data
CONVERTDATA = $(DATA_SOURCE)/mero_flash.nsbmd \
							$(DATA_SOURCE)/mero_flash.nsbca \
							$(DATA_SOURCE)/mero_flash.nsbma \
							$(DATA_SOURCE)/mero_move_ica.bin


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
	g3dcvtr $(DATA_SOURCE)/mero_flash.imd -o $(DATA_SOURCE)/mero_flash.nsbmd
	g3dcvtr $(DATA_SOURCE)/mero_flash.ica -o $(DATA_SOURCE)/mero_flash.nsbca
	g3dcvtr $(DATA_SOURCE)/mero_flash.ima -o $(DATA_SOURCE)/mero_flash.nsbma
	ruby $(ICACONV) -t $(DATA_SOURCE)/mero_move.ica $(DATA_SOURCE)/mero_move_ica.bin
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
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(DATA_SOURCE)/*.nsb*
	rm -f $(DATA_SOURCE)/*.bin
	rm -f $(CONVERTDATA)
endif

