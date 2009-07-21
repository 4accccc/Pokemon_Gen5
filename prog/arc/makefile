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

do-build: narc_tree.inc arc_def.h arc_file.h file_tree.inc arc_tool.lst

narc_tree.inc: ../../tools/arc/narc_tree.pl arc_tool.lst $(VER_FILE)
	perl ../../tools/arc/narc_tree.pl arc_tool.lst $(PM_VERSION)

arc_def.h arc_file.h: ../../tools/arc/header.pl arc_tool.lst $(VER_FILE)
	perl ../../tools/arc/header.pl arc_tool.lst $(PM_VERSION) no

file_tree.inc: ../../tools/arc/narc.pl arc_tool.lst $(VER_FILE) $(NARC_TREE)
	perl ../../tools/arc/narc.pl arc_tool.lst $(PM_VERSION) ./ ../filetree/ no

clean:
	rm -f $(LDIRT_CLEAN)
