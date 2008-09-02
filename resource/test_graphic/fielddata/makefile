#------------------------------------------------------------------------------
#
#	フィールドマップ用データMakefile
#	
#------------------------------------------------------------------------------
#現状make対象が無いのでファイルコピーのみ
COPYFILES = fieldmap_act.naix fieldmap_act.narc fieldmap_map.naix fieldmap_map.narc
TARGETDIR = $(PROJECT_ARCDIR)/test_graphic

do-build: $(COPYFILES)
	-cp -f $(COPYFILES) $(TARGETDIR)
clean:
