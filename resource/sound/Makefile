#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.09.02	ohno_katsumi@gamefreak.co.jp
#------------------------------------------------------------------------------

SOUNDPLAYERDIR = ../../../pokemon_wb_doc/wb_sound_player
DIFFCOPY = ../../tools/diffcopy


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:
	$(MAKE) -f makefile_gmm
	$(MAKE) -f makefile_sarc


#./bgm_info/bgm_info.xls : $(SOUNDPLAYERDIR)/Pokemon_WB_BGMList.xls
#	cp $(SOUNDPLAYERDIR)/Pokemon_WB_BGMList.xls ./bgm_info/bgm_info.xls

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------

clean:
	$(MAKE) -f makefile_gmm clean
	$(MAKE) -f makefile_sarc clean

