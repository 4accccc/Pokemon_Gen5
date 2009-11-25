#------------------------------------------------------------------------------
#
#	戦闘背景テーブルデータコンバート用Makefile
#
#	2009.11.25	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGET	= batt_bg_tbl
TARGET_NARC	= $(TARGET).narc
TARGET_NAIX	= $(TARGET).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

ELF2BIN = ../../tools/elf2bin.exe

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(TARGETDIR)$(TARGET_NARC) $(TARGETDIR)$(TARGET_NAIX)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(TARGET_NARC) $(TARGET_NAIX) *.s *.bin
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES: .s .bin

#パーソナルデータ生成
do-build: batt_bg_tbl.narc

%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	rm $*.o
	rm $*.elf

batt_bg_tbl.narc: zone_spec_table.bin batt_bg.bin batt_stage.bin
	nnsarc -c -l -n -i batt_bg_tbl.narc zone_spec_table.bin batt_bg.bin batt_stage.bin

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)$(TARGET_NARC) $(TARGETDIR)$(TARGET_NAIX)

$(TARGETDIR)$(TARGET_NARC):	$(TARGET_NARC)
	$(COPY)	$(TARGET_NARC) $(TARGETDIR)

$(TARGETDIR)$(TARGET_NAIX):	$(TARGET_NAIX)
	$(COPY)	$(TARGET_NAIX) $(TARGETDIR)

