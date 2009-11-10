
include $(PROJECT_RSCDIR)\macro_define

SUBDIRS	=	rail_editor watanabe lobby_trfgra obata nagihashi

TARGET_NAMES = \
	mmodel.naix \
	mmodel.narc \
	
TARGET_DIR = $(PROJECT_ARCDIR)debug

TARGETS = $(addprefix $(TARGET_DIR)/,$(TARGET_NAMES))

.PHONY:	do-build clean install

$(TARGET_DIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGET_DIR)/%.naix:	%.naix
	$(COPY) $^ $@

do-build: install

	@$(MAKE_SUBDIR)

install: $(TARGETS)

clean:
	@$(MAKE_SUBDIR)

	-rm -f $(TARGETS)
