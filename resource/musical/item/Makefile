#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ariizumi_nobuhiko saya_tsuruta

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = musical_item

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#管理表
#
XLS2TAB		= ruby $(PROJECT_ROOT)tools/exceltool/xls2xml/tab_out.rb
ITEMDATA_XLS = item_data.xls

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 


NITRO_DIR = nitro_data
IMD_DIR = imd_out

VPATH = $(IMD_DIR):$(NITRO_DIR)

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
nclres:=$(wildcard $(NITRO_DIR)/*.ncl)
ncgres:=$(wildcard $(NITRO_DIR)/*.ncg)
#imdres:=$(wildcard $(IMD_DIR)/*.imd)

# リソースが吐き出すファイルを自動でつくる
imdres = $(subst $(NITRO_DIR)/,$(IMD_DIR)/,$(subst .ncg,.imd,$(ncgres)) )
imdobj = $(subst $(NITRO_DIR)/,,$(subst .ncg,.nsbtx,$(ncgres)) )

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .nsbtx .imd .ncg .ncl

.ncg.imd:
	ruby ncg2imd.rb $< $(IMD_DIR)/

.imd.nsbtx:
	g3dcvtr $(IMD_DIR)/$< -etex

endif


.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

#$(imdres): $(ncgres)
#	ruby ncg2imd.rb $< $(IMD_DIR)/

#$(imdobj): $(imdres)
#	g3dcvtr $< -etex

$(ITEMDATA_XLS:.xls=.bin): $(ITEMDATA_XLS)
	$(XLS2TAB) $(ITEMDATA_XLS) > $(ITEMDATA_XLS:.xls=.txt)
	ruby item_conv.rb $(ITEMDATA_XLS:.xls=.txt) $(ITEMDATA_XLS:.xls=.bin)

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(imdobj) $(ITEMDATA_XLS:.xls=.bin)
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
	rm -f *.nsbtx
	rm -f *.narc
	rm -f *.naix
	rm -f *.bin
	rm -f $(IMD_DIR)/*.imd
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


