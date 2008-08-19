#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#----------------------------------------------------------------------------


RESOURCE_DIR = ./resource/
PROG_DIR = ./prog/
ARC_DIR = ./prog/arc


#  makeした場合にすべて作られるように記述してください
#--------------------------------------- ----------------------------
do-build:
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR)
	$(MAKE) -C $(PROG_DIR)


# cleanした場合にすべて消えるように記述してください
#--------------------------------------------------------------------
.PHONY : clean	prog resource arc

clean:
	$(MAKE) -C $(RESOURCE_DIR) clean
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	rm -r ./prog/filetree/a

#--------------------------------------------------------------------
prog:
	$(MAKE) -C $(PROG_DIR)

resource:
	$(MAKE) -C $(RESOURCE_DIR)

arc:
	$(MAKE) -C $(ARC_DIR)

