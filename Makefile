#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#----------------------------------------------------------------------------


PROG_DIR = ./prog/


#  makeした場合にすべて作られるように記述してください
#--------------------------------------- ----------------------------
do-build:
	$(MAKE) -C $(PROG_DIR)


# cleanした場合にすべて消えるように記述してください
#--------------------------------------------------------------------
.PHONY : clean
clean:
	$(MAKE) -C $(PROG_DIR) clean

