#------------------------------------------------------------------------------
#		本来、ダウンロードサーバーに置いたり、赤外線配信のソフトで使うため
#		ROM上に乗らないデータだが、デバッグ用にROMに載せることになりました
#  2010.04.2	nagihashi
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ohno toru_nagihashi shimoyamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = debug_regulation_card

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

NAIXADDMAX	= ruby $(PROJECT_ROOT)/tools/naixaddmax.rb

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

CONVERTDATA=*.dat

#コンバートメイン
MAKE_REG = makefile_reg
MAKE_CRC = makefile_crc


do-build: $(TARGETDIR)/$(targetname).narc


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
$(targetname).narc: $(CONVERTDATA)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix
	$(NAIXADDMAX) $(targetname).naix

$(CONVERTDATA): regulation_card.xls
	$(MAKE) -f $(MAKE_REG)
	$(MAKE) -j 3 -f $(MAKE_CRC)

endif



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	$(MAKE) -f $(MAKE_REG) clean
	$(MAKE) -f $(MAKE_CRC) clean
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
