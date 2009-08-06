
include	$(PROJECT_RSCDIR)/macro_define

TARGET_DIR	= $(PROJECT_ARCDIR)

TARGET_NAMES	= \
	config_gra.narc \
	config_gra.naix

TARGETS	= $(addprefix $(TARGET_DIR)/,$(TARGET_NAMES))

.PHONY:	do-build clean install clean-arc test

$(TARGET_DIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGET_DIR)/%.naix:	%.naix
	$(COPY) $^ $@

$(TARGET_DIR)/%.h:	%.h
	$(COPY) $^ $@

do-build:	install

install:	$(TARGETS)

clean:	clean-arc


clean-arc:
	rm -f $(TARGETS)

test:
	@echo $(TARGETS)
