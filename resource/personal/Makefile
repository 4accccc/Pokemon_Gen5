#------------------------------------------------------------------------------
#
#	ポケモンパーソナル用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
PERSONAL	= personal.narc
GROWTBL		= growtbl.narc
WOTBL		= wotbl.narc

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
PMSFILES:=$(wildcard pms_*.bin)

ELF2BIN = ../../tools/elf2bin.exe

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES: .s .bin

#成長曲線テーブル生成
#do-build: growtbl.narc
#growtbl.narc: grow_*.bin
#	nnsarc -c -l -n growtbl.narc grow_*.bin

#パーソナルデータ生成
do-build: personal.narc
personal.narc: $(PERFILES:.s=.bin)
	nnsarc -c -l -n personal.narc per_*.bin

#技覚えテーブルデータ生成
do-build: wotbl.narc
wotbl.narc: $(WOTFILES:.s=.bin)
	nnsarc -c -l -n wotbl.narc wot_*.bin

#進化テーブルデータ生成
#do-build: evo.narc
#evo.narc: $(EVOFILES:.s=.bin)
#	nnsarc -c -l -n evo.narc evo_*.bin
#	../../../../pmgs_conv/personalcnv/pmseed

do-build: $(PERFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	rm $*.o
	rm $*.elf

do-build: $(WOTFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	rm $*.o
	rm $*.elf

#do-build: $(EVOFILES:.s=.bin)
#%.bin:%.s
#	$(MWAS) $< -o $*.o
#	$(MWLD) -dis -o $*.elf $*.o
#	$(ELF2BIN) $*.elf
#	rm $*.o
#	rm $*.elf
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)$(PERSONAL) $(TARGETDIR)$(GROWTBL) $(TARGETDIR)$(WOTBL)

$(TARGETDIR)$(PERSONAL):	$(PERSONAL)
	$(COPY)	$(PERSONAL) $(TARGETDIR)

$(TARGETDIR)$(GROWTBL):	$(GROWTBL)
	$(COPY)	$(GROWTBL) $(TARGETDIR)

$(TARGETDIR)$(WOTBL):	$(WOTBL)
	$(COPY)	$(WOTBL) $(TARGETDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f $(PERSONAL)
#	-rm -f $(GROWTBL)
	-rm -f $(WOTBL)
endif
	-rm -f $(TARGETDIR)$(PERSONAL)
	-rm -f $(TARGETDIR)$(GROWTBL)
	-rm -f $(TARGETDIR)$(WOTBL)

