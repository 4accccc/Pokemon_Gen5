
include	$(PROJECT_RSCDIR)/macro_define

TARGET_DIR	= $(PROJECT_ARCDIR)/test_graphic

TARGET_NAMES	= \
	d_taya.naix \
	d_taya.narc \
	fatal_error.naix \
	fatal_error.narc \
	fld_act.naix \
	fld_act.narc \
	spa.narc \
	titledemo.naix \
	titledemo.narc \
	map_tex_set.narc \
	land_data.narc \
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
