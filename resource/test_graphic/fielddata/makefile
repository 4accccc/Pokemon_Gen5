#------------------------------------------------------------------------------
#
#	フィールドマップ用データMakefile
#	
#------------------------------------------------------------------------------
#現状make対象が無いのでファイルコピーのみ

include	$(PROJECT_RSCDIR)/macro_define

TARGET_NAMES = \
	fieldmap_act.naix \
	fieldmap_act.narc \
	fieldmap_map.naix \
	fieldmap_map.narc \
	menu_gra.naix \
	menu_gra.narc \
	testgridmap_a.narc \
	testgridmap_a.naix \
	fldmmdl_btx.narc \
	fldmmdl_btx.naix \
	mmodel.naix \
	mmodel.narc
	
TARGET_DIR = $(PROJECT_ARCDIR)/test_graphic

TARGETS = $(addprefix $(TARGET_DIR)/,$(TARGET_NAMES))

.PHONY:	do-build clean install

$(TARGET_DIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGET_DIR)/%.naix:	%.naix
	$(COPY) $^ $@

do-build: install

install: $(TARGETS)

clean:
	rm -f $(TARGETS)
