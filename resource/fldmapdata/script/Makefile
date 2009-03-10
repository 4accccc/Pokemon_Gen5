#! make -f
#----------------------------------------------------------------------------
# Project:  スクリプト
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------
#対象ユーザー
USERS = kagaya
#共通
include $(PROJECT_RSCDIR)/macro_define

SUBDIRS =
DEPENDHEADERS = usescript.h

#SCRSRCS
include	scrfile.lst
#MSGLIST
include msg_list

#MWAS,MWLD
include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs

#target
TARGET_DIR = $(PROJECT_ROOT)/prog/arc/fieldmap
TARGET_NARC = scr_seq.narc
TARGET_NAIX = $(TARGET_NARC:.narc=.naix)

#Makeで生成される*.bin/*.narcはmake cleanの削除対象にする
ifeq ($(CONVERTUSER),true) #CONVERTUSER
LDIRT_CLEAN	= $(SCRSRCS:.ev=.bin) $(TARGET_DIR)/$(TARGET_NARC) $(TARGET_DIR)/$(TARGET_NAIX) $(TARGET_NARC)
else
LDIRT_CLEAN	= $(TARGET_DIR)/$(TARGET_NARC) $(TARGET_DIR)/$(TARGET_NAIX)
endif

#WB NULL
#IPATH	=	-I../eventdata

#----------------------------------------------------------------------------

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

ifeq ($(CONVERTUSER),true) #CONVERTUSER
do-build: $(SCRSRCS:.ev=.bin)

#common_scr.bin:	../../../include/msgdata/msg_common_scr.h

%.bin: %.ev $(DEPENDHEADERS)
	@echo $<
	@cp $< temp_$*.s
	@$(MWAS) $(SUBDIRS) temp_$*.s -o $*.o
	@$(MWLD) -dis -o $*.elf $*.o
	@elf2bin $*.elf
	@rm $*.o $*.elf temp_$*.s
endif

#	@echo $<
#	@cp $< temp_$*.s
#	@$(MWAS) $(IPATH) temp_$*.s -o $*.o
#	@$(MWLD) -dis -o $*.elf $*.o
#	@elf2bin $*.elf
#	@rm $*.o $*.elf temp_$*.s

do-build: $(TARGET_DIR)/$(TARGET_NARC)

$(TARGET_DIR)/$(TARGET_NARC): $(TARGET_NARC)
	$(COPY) $(TARGET_NARC) $(TARGET_DIR)
	$(COPY) $(TARGET_NAIX) $(TARGET_DIR)

ifeq ($(CONVERTUSER),true) #CONVERTUSER
$(TARGET_NARC): $(SCRSRCS:.ev=.bin)
	nnsarc -i -c -l -n $(TARGET_NARC) $(SCRSRCS:.ev=.bin) > arc_result.txt
endif

mklst:
	ls -1 *.ev > temp_$*
	listmk temp_$* scrfile.lst SCRSRCS
	rm temp_$*

mkmsglist:
	cat $(MSGLIST) > msglist.h

diff:
	-cp $(TARGET_NARC) old.narc
	$(MAKE)
	-fc /b $(TARGET_NARC) old.narc
