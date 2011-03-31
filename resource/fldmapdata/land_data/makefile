include $(PROJECT_RSCDIR)\macro_define

MKFILE_LANDLIST = mkfile_landlist
MKFILE_LAND = mkfile_landarc

SUBDIRS	= land_camera

.PHONY: do-build clean

do-build: subdir
	$(MAKE) -j $(NUMBER_OF_PROCESSORS) -f $(MKFILE_LANDLIST)
	$(MAKE) -j $(NUMBER_OF_PROCESSORS) -f $(MKFILE_LAND)

subdir:
	@$(MAKE_SUBDIR)

clean:
	$(MAKE) -f $(MKFILE_LANDLIST) clean
	$(MAKE) -f $(MKFILE_LAND) clean
	@$(MAKE_SUBDIR) clean
