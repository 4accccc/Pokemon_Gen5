#! make -f
#----------------------------------------------------------------------------
# Project:  narcリスト生成
# File:     Makefile
#
# Copyright 2008 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

#Makeで生成されるarc_def.h arc_file.hはmake cleanの削除対象にする
LDIRT_CLEAN	= arc_def.h arc_file.h narc_tree.inc file_tree.inc ../file_tree.inc

#----------------------------------------------------------------------------
VER_FILE	= ../../prog/version

include $(VER_FILE)
-include narc_tree.inc

.PHONY:	do-build clean

#成長曲線テーブル生成
do-build: pl_growtbl.narc
pl_growtbl.narc: grow_*.bin
	nnsarc -c -l -n pl_growtbl.narc grow_*.bin
do-build: narc_tree.inc arc_def.h arc_file.h file_tree.inc

narc_tree.inc: ../../tools/arc/narc_tree.pl arc_tool.lst $(VER_FILE)
	perl ../../tools/arc/narc_tree.pl arc_tool.lst $(PM_VERSION) $(PM_DEBUG)

arc_def.h arc_file.h: ../../tools/arc/header.pl arc_tool.lst $(VER_FILE)
	perl ../../tools/arc/header.pl arc_tool.lst $(PM_VERSION) $(PM_DEBUG) yes

file_tree.inc: ../../tools/arc/narc.pl arc_tool.lst $(VER_FILE) $(NARC_TREE)
	perl ../../tools/arc/narc.pl arc_tool.lst $(PM_VERSION) $(PM_DEBUG) ./ ../filetree/ yes

clean:
	rm -f $(LDIRT_CLEAN)
