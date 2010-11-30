
include $(PROJECT_RSCDIR)\macro_define

SUBDIRS	=	rail_editor watanabe obata nagihashi hitch_check cgearp

TARGET_DIR = $(PROJECT_ARCDIR)debug

.PHONY:	do-build clean

$(TARGET_DIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGET_DIR)/%.naix:	%.naix
	$(COPY) $^ $@

do-build:
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
