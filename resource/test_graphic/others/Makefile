
include	$(PROJECT_RSCDIR)/macro_define

TARGET_DIR	= $(PROJECT_ARCDIR)/test_graphic

TARGET_NAMES	= \
	build_model.narc \
	build_model_id.h \
	d_taya.naix \
	d_taya.narc \
	fatal_error.naix \
	fatal_error.narc \
	fld_act.naix \
	fld_act.narc \
	fld_map.naix \
	fld_map.narc \
	mapobj.naix \
	mapobj.narc \
	spa.narc \
	spaEffect.naix \
	spaEffect.narc \
	test3d.naix \
	test3d.narc \
	titledemo.naix \
	titledemo.narc \
	map_tex_set.narc \
	land_data.narc \
	bm_field.narc \
	bm_field.narc \
	bm_info_out.narc \
	bm_anime.narc \
	area_build.narc \
	areabm_texset.narc \
	d_goto.naix \
	d_goto.narc \
	config_gra.narc \
	config_gra.naix \
	pl_keycursor.narc \
	pl_keycursor.naix

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
