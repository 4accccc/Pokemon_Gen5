#------------------------------------------------------------------------------
#
#	技エフェクト用Makefile
#
#	2009.02.13	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ariizumi_nobuhiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= musical_script
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

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

include	$(COMMONDEFS_CCTYPE_CW)

#リソースファイルの存在するディレクトリを指定
RES_DIR = ./

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバート対象のファイルツリーを生成
WAZAEFFSRCS	:= $(wildcard $(RES_DIR)*.s)

.PHONY:	do-build clean subdir

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

.SUFFIXES:	.s

do-build: $(notdir $(WAZAEFFSRCS:.s=.bin))
%.bin: %.s ../../prog/src/musical/stage/script/musical_script_cmmand.h
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	../../tools/elf2bin $*.elf

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
$(NARCNAME): $(notdir $(WAZAEFFSRCS:.s=.bin))
	nnsarc -c -l -n -i $(NARCNAME) *.bin
	$(NAIXCUT) $(NAIXNAME)
endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#	-rm -f *.N*R
#	-rm -f *.N*C
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


