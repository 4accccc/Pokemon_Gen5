#----------------------------------------------------------------------------
# Project:  スクリプト
# File:     Makefile
# Copyright 2005 GameFreak.inc  All rights reserved.
#----------------------------------------------------------------------------
#対象ユーザー
USERS = kagaya matsumiya tamada nakatsui masafumi_saitou suginaka_katsunori mai_ando murakami_naoto hozumi_yukiko iwasawa obata_toshihiro ariizumi_nobuhiko tomoya_takahashi saitou

#共通マクロ
include $(PROJECT_RSCDIR)/macro_define

#SUBDIR
SUBDIRS =

#対象ヘッダーファイル
DEPENDHEADERS = usescript.h scr_seq_def.h $(PROJECT_PROGDIR)src/field/scrcmd_table.cdat

#マップ依存のスクリプト：定義ファイルは自動生成
SCRIPTFILES	=
include zone_script.list

#マップ非依存スクリプトはここに追加してください
#UNIQ_SCRIPTFILES =
include uniq_script.list

#COMPILE_TARGETSとARCHIVE_TARGETSに含まれる内容は一緒だが並びを代えている

#非マップスクリプトから先にコンパイルされる（通常スクリプトから呼び出す際にIDを参照するため）
COMPILE_TARGETS	= $(UNIQ_SCRIPTFILES:.ev=.bin) $(SCRIPTFILES:.ev=.bin)

#アーカイブにするときはマップスクリプトからの並びにする
#（非マップスクリプトを追加してもアーカイブIDがずれないように）
ARCHIVE_TARGETS = $(SCRIPTFILES:.ev=.bin) $(UNIQ_SCRIPTFILES:.ev=.bin)

#MWAS,MWLD
include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs

#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#ev_def
EV_DEF = ruby ./scr_tools/ev_def.rb

#script_offset_id.h
#SCR_OFFS_ID_H = ../../../prog/src/field/script_offset_id.h
SCR_OFFS_ID_H	= scrid_offset/scr_offset_id.h

#eventdata WB NULL
#IPATH	=	-I../eventdata

#target
TARGET_DIR = $(PROJECT_ARCDIR)fieldmap
TARGET_NARC = script_seq.narc
TARGET_NAIX = $(TARGET_NARC:.narc=.naix)

#eventdata/tmp参照
ADD_MACRO = -I../eventdata/tmp/ -I../../message/dst/script/

#(PM_DEBUG == yes)のときのみデバッグバージョン
ifeq ($(PM_DEBUG),yes)
ADD_MACRO += -DPM_DEBUG
endif

#----------------------------------------------------------------------------
#	clean
# Makeで生成される*.narcはmake cleanの削除対象にする
#----------------------------------------------------------------------------
ifeq ($(CONVERTUSER),true) #CONVERTUSER
LDIRT_CLEAN	= $(TARGET_DIR)/$(TARGET_NARC) $(TARGET_DIR)/$(TARGET_NAIX) $(COMPILE_TARGETS)
else
LDIRT_CLEAN	= $(TARGET_DIR)/$(TARGET_NARC) $(TARGET_DIR)/$(TARGET_NAIX)
endif

#----------------------------------------------------------------------------
#	SUFFIXES
#----------------------------------------------------------------------------
.SUFFIXES: .bin .ev

#----------------------------------------------------------------------------
#	make
#----------------------------------------------------------------------------
#narc
do-build: $(TARGET_DIR)/$(TARGET_NARC)

$(TARGET_DIR)/$(TARGET_NARC): $(TARGET_NARC)
	$(COPY) $(TARGET_NARC) $(TARGET_DIR)
	$(COPY) $(TARGET_NAIX) $(TARGET_DIR)

ifeq ($(CONVERTUSER),true) #CONVERTUSER
#*.bin
%.bin: %.ev $(DEPENDHEADERS)
	@echo $<
	@ruby filter/user.rb $< > temp_$*.s
	@#$(MWAS) -E $(ADD_MACRO)  temp_$*.s -o $*.dis
	@$(MWAS) $(ADD_MACRO)  temp_$*.s -o $*.o
	@$(MWLD) -dis -o $*.elf $*.o
	@elf2bin $*.elf
	@rm $*.o $*.elf
	@rm temp_$*.s
	@$(EV_DEF) $< $(SCR_OFFS_ID_H)
endif #CONVERTUSER


#script.narc
$(TARGET_NARC): $(COMPILE_TARGETS)
	@echo "スクリプトデータをアーカイブします"
	@nnsarc -i -c -l -n $(TARGET_NARC) $(ARCHIVE_TARGETS) > arc_result.txt
	@$(NAIXCUT) $(TARGET_NAIX)

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
make-verinfo:
	#do nothing!!

#----------------------------------------------------------------------------
#	diff
#----------------------------------------------------------------------------
diff:
	-cp $(TARGET_NARC) old.narc
	$(MAKE)
	-fc /b $(TARGET_NARC) old.narc

#----------------------------------------------------------------------------
#	test
#----------------------------------------------------------------------------
test:
	@echo $(SCRIPTFILES)

