#------------------------------------------------------------------------------
#
#	ポケモンパーソナル用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe niishino

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
PERSONAL	= personal.narc
WOTBL			= wotbl.narc
EVOTBL		= evo.narc
GROWTBL		= growtbl.narc

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)personal/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

PERFILES:=$(wildcard per_*.s)
WOTFILES:=$(wildcard wot_*.s)
EVOFILES:=$(wildcard evo_*.s)

ELF2BIN = ../../tools/elf2bin.exe

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES: .s .bin

#パーソナルデータ生成
do-build: personal wot evo narc

personal: $(PERFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	rm $*.o
	rm $*.elf

wot: $(WOTFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	rm $*.o
	rm $*.elf

evo: $(EVOFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	rm $*.o
	rm $*.elf

narc:
personal.narc: $(PERFILES:.s=.bin)
	nnsarc -c -l -n personal.narc per_*.bin
wotbl.narc: $(WOTFILES:.s=.bin)
	nnsarc -c -l -n wotbl.narc wot_*.bin
evo.narc: $(EVOFILES:.s=.bin)
	nnsarc -c -l -n evo.narc evo_*.bin

#進化テーブルデータ生成
#do-build: evo.narc
#evo.narc: $(EVOFILES:.s=.bin)
#	nnsarc -c -l -n evo.narc evo_*.bin
#	../../../../pmgs_conv/personalcnv/pmseed

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)$(PERSONAL) $(TARGETDIR)$(WOTBL) $(TARGETDIR)$(EVOTBL) $(TARGETDIR)$(GROWTBL)

$(TARGETDIR)$(PERSONAL):	$(PERSONAL)
	$(COPY)	$(PERSONAL) $(TARGETDIR)

$(TARGETDIR)$(WOTBL):	$(WOTBL)
	$(COPY)	$(WOTBL) $(TARGETDIR)

$(TARGETDIR)$(EVOTBL):	$(EVOTBL)
	$(COPY)	$(EVOTBL) $(TARGETDIR)

$(TARGETDIR)$(GROWTBL):	$(GROWTBL)
	$(COPY)	$(GROWTBL) $(TARGETDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f $(PERSONAL)
	-rm -f $(WOTBL)
	-rm -f $(EVOTBL)
	-rm -f *.bin
endif
	-rm -f $(TARGETDIR)$(PERSONAL)
	-rm -f $(TARGETDIR)$(WOTBL)
	-rm -f $(TARGETDIR)$(EVOTBL)

