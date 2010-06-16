#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= kawada_koji

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_w = manual_image_w
targetname_b = manual_image_b

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
bmp_sources:=$(wildcard *.bmp)

# 入力ファイル、バッチファイル
manual_image_list_w     = manual_image_w.lst
manual_image_conv_bat_w = manual_image_conv_w.bat
manual_image_comp_bat_w = manual_image_comp_w.bat

manual_image_list_b     = manual_image_b.lst
manual_image_conv_bat_b = manual_image_conv_b.bat
manual_image_comp_bat_b = manual_image_comp_b.bat


.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname_w).narc \
          $(TARGETDIR)/$(targetname_b).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_w).narc: $(targetname_w).narc
	$(COPY) $(targetname_w).narc $(TARGETDIR)
	$(COPY) $(targetname_w).naix $(TARGETDIR)

$(TARGETDIR)/$(targetname_b).narc: $(targetname_b).narc
	$(COPY) $(targetname_b).narc $(TARGETDIR)
	$(COPY) $(targetname_b).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する

$(targetname_w).narc: $(bmp_sources) $(manual_image_list_w) $(manual_image_conv_bat_w) $(manual_image_comp_bat_w)
	$(manual_image_conv_bat_w)
	$(manual_image_comp_bat_w)
	nnsarc -r -l -n -i $(targetname_w) -S $(manual_image_list_w)
	$(NAIXCUT) $(targetname_w).naix

$(targetname_b).narc: $(bmp_sources) $(manual_image_list_b) $(manual_image_conv_bat_b) $(manual_image_comp_bat_b)
	$(manual_image_conv_bat_b)
	$(manual_image_comp_bat_b)
	nnsarc -r -l -n -i $(targetname_b) -S $(manual_image_list_b)
	$(NAIXCUT) $(targetname_b).naix

endif


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.NSCR
	rm -f *.NANR
	rm -f *.NCER
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname_w).narc
	rm -f $(targetname_w).naix
	rm -f $(targetname_b).narc
	rm -f $(targetname_b).naix
	rm -f *_lz77.nbfs
	rm -f *.nbfs
endif
	rm -f $(TARGETDIR)/$(targetname_w).narc
	rm -f $(TARGETDIR)/$(targetname_w).naix
	rm -f $(TARGETDIR)/$(targetname_b).narc
	rm -f $(TARGETDIR)/$(targetname_b).naix

