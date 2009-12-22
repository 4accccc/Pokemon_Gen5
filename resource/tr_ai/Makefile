#------------------------------------------------------------------------------
#
#	トレーナーAI用Makefile
#
#	2009.12.15	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= tr_ai_seq
NARCNAME	= $(targetname).narc

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
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = $(TARGETDIR)$(NARCNAME)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += *.o *.elf *.bin $(NARCNAME)
endif

#コンバート対象のファイルツリーを生成
SRC	=	tr_ai_basic.s
#		tr_ai_damage.s \
#		tr_ai_double.s \
#		tr_ai_expert.s \
#		tr_ai_first.s \
#		tr_ai_gamble.s \
#		tr_ai_harassing.s \
#		tr_ai_other.s \
#		tr_ai_relay.s \
#		tr_ai_strong.s \
#		tr_ai_weather.s 

ELF2BINDIR = ../../tools/

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

.SUFFIXES: .s .bin

do-build: tr_ai_seq.narc
tr_ai_seq.narc: $(SRC:.s=.bin)
	nnsarc -c -l -n -i tr_ai_seq.narc -S tr_ai_seq.scr

do-build: $(SRC:.s=.bin)
%.bin: %.s *.h
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BINDIR)elf2bin $*.elf

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)

