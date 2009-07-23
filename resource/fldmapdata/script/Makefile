#! make -f
#----------------------------------------------------------------------------
# Project:  スクリプト
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------
#対象ユーザー
USERS = kagaya matsumiya tamada
#共通
include $(PROJECT_RSCDIR)/macro_define

SUBDIRS =
DEPENDHEADERS = usescript.h scr_seq_def.h

#マップ依存のスクリプト：定義ファイルは自動生成
include zone_script.list

#マップに依存しないスクリプトはここに追加してください
include uniq_script.list

#SCRSRCS_BIN
#SCRSRCS_BIN = $(SCRSRCS:.ev=.bin)
SCRSRCS_BIN = $(SCRIPTFILES:.ev=.bin)

#MSGLIST
#include msg_list

#MWAS,MWLD
include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs

#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#eventdata WB NULL
#IPATH	=	-I../eventdata

#target
TARGET_DIR = $(PROJECT_ROOT)/prog/arc/fieldmap
TARGET_NARC = script_seq.narc
TARGET_NAIX = $(TARGET_NARC:.narc=.naix)


ADD_MACRO = -I../eventdata/tmp/
#(PM_DEBUG == yes)のときのみデバッグバージョン
ifeq ($(PM_DEBUG),yes)
ADD_MACRO += -DPM_DEBUG
endif

#----------------------------------------------------------------------------
#	clean
#----------------------------------------------------------------------------
#Makeで生成される*.bin/*.narcはmake cleanの削除対象にする
ifeq ($(CONVERTUSER),true) #CONVERTUSER
LDIRT_CLEAN	= $(SCRSRCS_BIN) $(TARGET_DIR)/$(TARGET_NARC) $(TARGET_DIR)/$(TARGET_NAIX) $(TARGET_NARC) $(TARGET_NAIX)
else
LDIRT_CLEAN	= $(TARGET_DIR)/$(TARGET_NARC) $(TARGET_DIR)/$(TARGET_NAIX)
endif

.SUFFIXES:	.bin .ev
#----------------------------------------------------------------------------
#	make
#----------------------------------------------------------------------------
ifeq ($(CONVERTUSER),true) #CONVERTUSER
#*.ev
do-build: $(SCRSRCS_BIN)

#common_scr.bin
#common_scr.bin: ../../../include/msgdata/msg_common_scr.h

#*.bin
%.bin: %.ev $(DEPENDHEADERS)
	@echo $<
	@ruby filter/user.rb $< > temp_$*.s
	@$(MWAS) $(ADD_MACRO) $(SUBDIRS) temp_$*.s -o $*.o
	@$(MWLD) -dis -o $*.elf $*.o
	@elf2bin $*.elf
	@rm $*.o $*.elf temp_$*.s
endif #CONVERTUSER

#narc
do-build: $(TARGET_DIR)/$(TARGET_NARC)

$(TARGET_DIR)/$(TARGET_NARC): $(TARGET_NARC)
	$(COPY) $(TARGET_NARC) $(TARGET_DIR)
	$(COPY) $(TARGET_NAIX) $(TARGET_DIR)

ifeq ($(CONVERTUSER),true) #CONVERTUSER
$(TARGET_NARC): $(SCRSRCS_BIN)
	nnsarc -i -c -l -n $(TARGET_NARC) $(SCRSRCS_BIN) > arc_result.txt
	$(NAIXCUT) $(TARGET_NAIX)
endif

#----------------------------------------------------------------------------
#	mklst
#----------------------------------------------------------------------------
mklst:
	ls -1 *.ev > temp_$*
	listmk temp_$* scrfile.lst SCRSRCS
	rm temp_$*

#----------------------------------------------------------------------------
#	msglist
#----------------------------------------------------------------------------
#mkmsglist:
#	cat $(MSGLIST) > msglist.h

#----------------------------------------------------------------------------
#	diff
#----------------------------------------------------------------------------
diff:
	-cp $(TARGET_NARC) old.narc
	$(MAKE)
	-fc /b $(TARGET_NARC) old.narc
