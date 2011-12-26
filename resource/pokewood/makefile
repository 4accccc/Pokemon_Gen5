#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	 = mori nakatsui ito_hiroto sato_hitomi shimoyamada
CALCUSER = mori
#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname  = pokewood

EXCEL_CONV    = ..\..\tools\exceltool\excelcommaconv.exe
UPDATEWITHGMM = ../../tools/message/UpdateWithGmm.bat
SENARIO_CONV  = ruby -Ks senario_conv.rb
BGM_HASH_CONV = ruby make_bgmhash.rb
BRIEFING_CONV = ruby briefing_conv.rb
BRIEFING_MODEL_CDAT = pkwd_briefing_model.cdat
PROFIT_CALC_CONV = ruby senario_calc_tool.rb

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR  = $(PROJECT_ARCDIR)
MESSAGEDIR = ../message/src/pokewood/
MSPROJDIR  = ../message/msproj/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard pokewood_*.xls)

# リソースが吐き出すファイルを自動でつくる(.xls→.txt)
ltexts = $(subst .xls,.txt,$(lsources))

# リソースが吐き出すファイルを自動でつくる(.txt→.bin)
lbins = $(subst .txt,.bin,$(ltexts))

# リソースが吐き出すファイルを自動でつくる(.xls -> _m.gmm _f.gmm)
gmmfiles = $(subst .xls,_f.gmm,$(lsources))
gmmfiles += $(subst .xls,_m.gmm,$(lsources))
targetgmm   = $(subst poke,$(MESSAGEDIR)/poke,$(gmmfiles))
mstxtfiles  = $(subst .gmm,.mstxt, $(gmmfiles))
targetmstxt = $(subst poke,src/pokewood/poke, $(mstxtfiles))
bgm_hashfile = bgm_hash.rb

#手で作成する録画データファイル
recdata = pokewood_rec01.bin

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(bgm_hashfile) $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES:  .bin .xls

$(bgm_hashfile): ../sound/swan_sound_data.sadl
	$(BGM_HASH_CONV) $(bgm_hashfile)

%_f.gmm %_m.gmm %.bin :%.xls
	$(EXCEL_CONV) $<
	$(SENARIO_CONV) $(subst .xls,.txt,$<)
	ruby pw_gmm2mstxt.rb $(subst .xls,_f.gmm, $<) $(subst .xls,_m.gmm, $<)

$(targetname).narc: $(lsources) $(lbins) $(ltexts) $(recdata)
	nnsarc -r -l -n -i $(targetname).narc -S arc.scr
	$(NAIXCUT) $(targetname).naix
	$(BRIEFING_CONV) $(BRIEFING_MODEL_CDAT)
ifeq ($(CALCUSER),nakatsui)
	$(PROFIT_CALC_CONV) pkwd_calc_templete.xls ウッド結果算出.xls
endif

endif

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc 
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 
#gmmのコピーだけ実行する
#$(targetgmm): $(gmmfiles)
#	$(COPY) *.gmm $(MESSAGEDIR)

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

	-rm -f *.txt
	-rm -f pokewood_senario*.bin
	-rm -f *_f.gmm
	-rm -f *_m.gmm
	-rm -f bgm_hash.rb
	-rm -f pkwd_briefing_model.cdat
endif
	-rm -f $(TARGETDIR)/$(targetname).narc
	-rm -f $(TARGETDIR)/$(targetname).naix
	-rm -f $(MESSAGEDIR)/*_f.gmm
	-rm -f $(MESSAGEDIR)/*_m.gmm


