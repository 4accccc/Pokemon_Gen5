
include $(PROJECT_RSCDIR)\macro_define

SUBDIRS	=	rail_editor easy_pokelist

TARGET_NAMES = \
	mmodel.naix \
	mmodel.narc
	
TARGET_DIR = $(PROJECT_ARCDIR)/debug

.PHONY:	do-build clean

$(TARGET_DIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGET_DIR)/%.naix:	%.naix
	$(COPY) $^ $@

do-build:
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
