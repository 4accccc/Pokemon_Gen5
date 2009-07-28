#------------------------------------------------------------------------------
#
#	トレーナーデータ用Makefile
#
#	2009.07.27	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname1	= trdata
NARCNAME1	= $(targetname1).narc
targetname2	= trpoke
NARCNAME2	= $(targetname2).narc

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define

include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW

#コンバート対象のファイルツリーを生成
DATAFILES:=$(wildcard trdata_*.s)
POKEFILES:=$(wildcard trpoke_*.s)
ELF2BINDIR = ../../tools/

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

.SUFFIXES: .s .bin

do-build: trdata.narc
trdata.narc: $(DATAFILES:.s=.bin)
	nnsarc -c -l -n trdata.narc trdata_*.bin

do-build: trpoke.narc
trpoke.narc: $(POKEFILES:.s=.bin)
	nnsarc -c -l -n trpoke.narc trpoke_*.bin

do-build: $(DATAFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BINDIR)elf2bin $*.elf
	rm $*.o
	rm $*.elf

do-build: $(POKEFILES:.s=.bin)
%.bin:%.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BINDIR)elf2bin $*.elf
	rm $*.o
	rm $*.elf

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME1)	$(TARGETDIR)/$(NARCNAME2)

$(TARGETDIR)/$(NARCNAME1):	$(NARCNAME1)
	$(COPY)	$(NARCNAME1) $(TARGETDIR)

$(TARGETDIR)/$(NARCNAME2):	$(NARCNAME2)
	$(COPY)	$(NARCNAME2) $(TARGETDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.o
	-rm -f *.elf
	-rm -f *.bin
	-rm -f $(NARCNAME1)
	-rm -f $(NARCNAME2)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME1)
	-rm -f $(TARGETDIR)/$(NARCNAME2)


