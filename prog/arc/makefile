#! make -f
#----------------------------------------------------------------------------
# Project:  narcÉäÉXÉgê∂ê¨
# File:     Makefile
#
# Copyright 2008 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

SUBDIRS		=

include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs

#MakeÇ≈ê∂ê¨Ç≥ÇÍÇÈarc_def.h arc_file.hÇÕmake cleanÇÃçÌèúëŒè€Ç…Ç∑ÇÈ

LDIRT_CLEAN	= arc_def.h arc_file.h file_tree.inc

#----------------------------------------------------------------------------
VER_FILE	= ../version

include $(VER_FILE)
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

.SUFFIXES: .h

do-build: file_tree.inc
file_tree.inc: narc.pl arc_tool.lst $(VER_FILE)
	perl narc.pl arc_tool.lst $(PM_VERSION) ./ ../filetree/
	cp file_tree.inc ../
