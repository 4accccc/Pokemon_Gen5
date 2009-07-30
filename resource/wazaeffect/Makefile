#------------------------------------------------------------------------------
#
#	技エフェクト用Makefile
#
#	2009.02.13	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe fujiwara toyama shota_kageyama sounduser

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= waza_eff
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)wazaeffect

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define

include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW

#リソースファイルの存在するディレクトリを指定
RES_DIR = ./esf_file/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバート対象のファイルツリーを生成
WAZAEFFSRCS	:= $(wildcard $(RES_DIR)?e_???.esf)

.PHONY:	do-build clean subdir

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

.SUFFIXES:	.s

do-build: $(notdir $(WAZAEFFSRCS:.esf=.s)) $(notdir $(WAZAEFFSRCS:.esf=.bin))
%.s: $(RES_DIR)%.esf ../../prog/src/battle/btlv/btlv_effvm_def.h ../../prog/include/sound/wb_sound_data.sadl
	ruby eescmk.rb c:/home/pokemon_wb/prog/src/battle/btlv/btlv_effvm_def.h $(RES_DIR)$*.esf c:/home/pokemon_wb/

%.bin: %.s ../../prog/src/battle/btlv/btlv_effvm_def.h
	$(MWAS) $< -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	../../tools/elf2bin $*.elf

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	$(NARCNAME) $(TARGETDIR)/$(NARCNAME)
else
do-build:	$(TARGETDIR)/$(NARCNAME)
endif

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
$(NARCNAME): $(notdir $(WAZAEFFSRCS:.esf=.bin))
	nnsarc -c -l -n -i $(NARCNAME) we_*.bin be_*.bin
	$(NAIXCUT) $(NAIXNAME)
endif

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.s
	-rm -f *.o
	-rm -f *.elf
	-rm -f *.bin
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
endif
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)


