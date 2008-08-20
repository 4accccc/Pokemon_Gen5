#! make -f
#----------------------------------------------------------------------------
# Project:  narcÉäÉXÉgê∂ê¨
# File:     Makefile
#
# Copyright 2008 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

#MakeÇ≈ê∂ê¨Ç≥ÇÍÇÈarc_def.h arc_file.hÇÕmake cleanÇÃçÌèúëŒè€Ç…Ç∑ÇÈ
LDIRT_CLEAN	= arc_def.h arc_file.h file_tree.inc ../file_tree.inc

#----------------------------------------------------------------------------
VER_FILE	= ../version

include $(VER_FILE)


.PHONY:	do-build clean

do-build: file_tree.inc

file_tree.inc: narc.pl arc_tool.lst $(VER_FILE)
	perl narc.pl arc_tool.lst $(PM_VERSION) ./ ../filetree/
	cp file_tree.inc ../

clean:
	rm -f $(LDIRT_CLEAN)
