#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= mori nakatsui nagihashi_toru

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname  = pokewood

EXCEL_CONV   = ..\..\tools\exceltool\excelcommaconv.exe
SENARIO_CONV = ruby senario_conv.rb

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
MESSAGEDIR = ../message/src/pokewood/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.xls)

# リソースが吐き出すファイルを自動でつくる(.xls→.txt)
ltexts = $(subst .xls,.txt,$(lsources))

# リソースが吐き出すファイルを自動でつくる(.txt→.bin)
lbins = $(subst .txt,.bin,$(ltexts))

# リソースが吐き出すファイルを自動でつくる(.xls -> _m.gmm _f.gmm)
gmmfiles = $(subst .xls,_f.gmm,$(lsources))
gmmfiles += $(subst .xls,_m.gmm,$(lsources))
targetgmm = $(subst poke,$(MESSAGEDIR)/poke,$(gmmfiles))

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(targetgmm)

subdir:
	@$(MAKE_SUBDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES:  .bin .xls 

%_f.gmm %_m.gmm %.bin :%.xls
	$(EXCEL_CONV) $<
	$(SENARIO_CONV) $(subst .xls,.txt,$<)

$(targetname).narc: $(lsources) $(lbins) $(ltexts) 
	nnsarc -r -l -n -i $(targetname).narc -S arc.scr
	$(NAIXCUT) $(targetname).naix
endif

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc 
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

#gmmのコピーだけ実行する
$(targetgmm): $(gmmfiles)
	$(COPY) *.gmm $(MESSAGEDIR)

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

	rm *.txt
	rm *.bin
	rm *.gmm
endif
	-rm -f $(TARGETDIR)/$(targetname).narc
	-rm -f $(TARGETDIR)/$(targetname).naix
	-rm -f $(MESSAGEDIR)/*.gmm


