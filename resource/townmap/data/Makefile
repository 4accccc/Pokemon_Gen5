#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi tetsuji_ohta obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = townmap_data

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC	= $(PROJECT_ARCDIR)#アーカイブ用
TARGETDIR_H		= $(PROJECT_ROOT)/prog/src/app/townmap#ヘッダ用

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
xsources:=$(wildcard *.xls)

# リソースが吐き出すファイルを自動でつくる
aobjects = $(subst .xls,.dat,$(xsources))
hobjects = $(subst .xls,.h,$(xsources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .xls .dat

.xls.dat:
	perl townmap_data_conv.pl $<

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_ARC)/$(targetname).narc $(TARGETDIR_ARC)/$(targetname).naix $(TARGETDIR_H)/$(targetname).h

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetname).naix: $(targetname).naix
	$(COPY) $(targetname).naix $(TARGETDIR_ARC) 

$(TARGETDIR_H)/$(targetname).h: $(targetname).h
	$(COPY) $(targetname).h		 $(TARGETDIR_H) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(aobjects)
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
	rm -f *.dat
	rm -f *.h
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR_ARC)/$(targetname).narc
	rm -f $(TARGETDIR_ARC)/$(targetname).naix


