include $(PROJECT_RSCDIR)\macro_define

MKFILE_LANDLIST = mkfile_landlist
MKFILE_LAND = mkfile_landarc

.PHONY: do-build clean

do-build:
	$(MAKE) -f $(MKFILE_LANDLIST)
	$(MAKE) -f $(MKFILE_LAND)

clean:
	$(MAKE) -f $(MKFILE_LANDLIST) clean
	$(MAKE) -f $(MKFILE_LAND) clean
