##########################################################
#	旧トレーナーグラフィックコピー
##########################################################

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

TARGETDIR = $(PROJECT_ARCDIR)/test_graphic

-include $(SOURCE_LIST)

NARCFILE		= $(wildcard *.narc)
NAIXFILE		= $(wildcard *.naix)
NARCTARGET 	= $(addprefix $(TARGETDIR)/,$(NARCFILE))
NAIXTARGET 	= $(addprefix $(TARGETDIR)/,$(NAIXFILE))

.PHONY:	do-build clean

##########################################################
#	動作部分
##########################################################

do-build:	$(NARCTARGET) $(NAIXTARGET)


$(TARGETDIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGETDIR)/%.naix:	%.naix
	$(COPY) $^ $@


clean:
	@$(MAKE_SUBDIR)
