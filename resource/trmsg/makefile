#------------------------------------------------------------------------------
#
#	トレーナーデータ用Makefile
#
#	2009.07.27	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	matsumiya

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname1	= trmsgtbl
NARCNAME1	= $(targetname1).narc
targetname2	= trmsgtblofs
NARCNAME2	= $(targetname2).narc
GMMFILE = trmsg.gmm

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)
GMMDIR	= ../message/src/
TOOLDIR = ../../tools/trmsg/

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

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

.SUFFIXES: .s .bin

do-build: out_end trmsgtbl.bin trmsgtbl.narc
trmsgtbl.narc: trmsgtbl.bin
	$(TOOLDIR)trtblmk trmsgtbl.bin trmsgtblofs.bin
	nnsarc -c -l -n -i trmsgtbl.narc trmsgtbl.bin
	nnsarc -c -l -n -i trmsgtblofs.narc trmsgtblofs.bin

trmsgtbl.bin: trmsgtbl.s
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	elf2bin $*.elf
	rm $*.o
	rm $*.elf

out_end: trmsg.csv
	ruby $(TOOLDIR)trmsgconv.rb $< ../message/template.gmm

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME1)	$(TARGETDIR)/$(NARCNAME2)

$(TARGETDIR)/$(NARCNAME1):	$(NARCNAME1)
	$(COPY)	$(NARCNAME1) $(TARGETDIR)

$(TARGETDIR)/$(NARCNAME2):	$(NARCNAME2)
	$(COPY)	$(NARCNAME2) $(TARGETDIR)

$(GMMDIR)$(GMMFILE):	$(GMMFILE)
	$(COPY)	$(GMMFILE) $(GMMDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f out_end
	-rm -f *.o
	-rm -f *.elf
	-rm -f *.bin
	-rm -f $(NARCNAME1)
	-rm -f $(NARCNAME2)
	-rm -f $(GMMFILE)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME1)
	-rm -f $(TARGETDIR)/$(NARCNAME2)
	-rm -f $(GMMDIR)$(GMMFILE)

