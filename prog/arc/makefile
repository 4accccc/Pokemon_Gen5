#! make -f
#----------------------------------------------------------------------------
# Project:  narcÉäÉXÉgê∂ê¨
# File:     Makefile
#
# Copyright 2008 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

#MakeÇ≈ê∂ê¨Ç≥ÇÍÇÈarc_def.h arc_file.hÇÕmake cleanÇÃçÌèúëŒè€Ç…Ç∑ÇÈ
LDIRT_CLEAN	= arc_def.h arc_file.h narc_tree.inc file_tree.inc ../file_tree.inc

#----------------------------------------------------------------------------
VER_FILE	= ../version

include $(VER_FILE)
-include narc_tree.inc

.PHONY:	do-build clean

do-build: narc_tree.inc file_tree.inc

narc_tree.inc: arc_tool.lst
	perl narc_tree.pl arc_tool.lst $(PM_VERSION)

file_tree.inc: narc.pl arc_tool.lst $(VER_FILE) $(NARC_TREE)
	perl narc.pl arc_tool.lst $(PM_VERSION) ./ ../filetree/
	cp file_tree.inc ../

clean:
	rm -f $(LDIRT_CLEAN)
