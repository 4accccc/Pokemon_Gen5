##########################################################
#	旧トレーナーグラフィックコピー
##########################################################

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

TARGETDIR = $(PROJECT_ARCDIR)/test_graphic

-include $(SOURCE_LIST)

##########################################################
#	動作部分
##########################################################

no-rule:
	$(COPY) trfgra.narc $(TARGETDIR)
	$(COPY) trfgra.naix $(TARGETDIR)
	$(COPY) trbgra.narc $(TARGETDIR)
	$(COPY) trbgra.naix $(TARGETDIR)

clean:
	@$(MAKE_SUBDIR)
