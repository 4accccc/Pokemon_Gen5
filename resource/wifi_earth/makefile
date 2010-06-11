#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda mori hozumi_yukiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname  = wifi_earth
targetname2 = wifi_earth_place

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

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)
psources:=$(wildcard *.spa)
dsources:=$(wildcard *.dat)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
eobjects = $(subst .nce,.NCER,$(esources))



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname2).narc

subdir:
	@$(MAKE_SUBDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .dat
$(targetname2).narc: $(dsources) countrylist.xls
	$(COPY) ../message/template.gmm ./
	conv.bat
	conv2.bat
	ruby sort_listmake.rb aiueo.txt gts_country_list.cdat
	nnsarc -r -l -n -i $(targetname2).narc -S placename.script
endif

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc 
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

$(TARGETDIR)/$(targetname2).narc: $(targetname2).narc
	$(COPY) $(targetname2).narc $(TARGETDIR)
	$(COPY) $(targetname2).naix $(TARGETDIR) 
	$(COPY) gts_country_list.cdat ../..\prog/src/net_app/wifi_worldtrade
	$(COPY) sort_list.cdat ../..\prog/src/net_app/wifi_earth

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(TARGETDIR)/$(targetname2).narc
	rm -f $(TARGETDIR)/$(targetname2).naix


