#------------------------------------------------------------------------------
#
#	技エフェクト用Makefile
#
#	2009.02.13	sogabe@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	sogabe fujiwara toyama kageyama_shota ichinose adachi_minako sounduser

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname1	= waza_eff
NARCNAME1	= $(targetname1).narc
NAIXNAME1	= $(targetname1).naix
targetname2	= battle_eff
NARCNAME2	= $(targetname2).narc
NAIXNAME2	= $(targetname2).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)wazaeffect

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= graphic

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define
#MWAS,MWLD
#include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#リソースファイルの存在するディレクトリを指定
RES_DIR = ./esf_file/

#コンバートしたファイルを格納するディレクトリを指定
CNV_DIR = ./

#コンバート対象のファイルツリーを生成
WAZAEFFSRCS	:= $(wildcard $(RES_DIR)?e_???.esf)

.PHONY:	do-build clean graphic

.SUFFIXES:	.s

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
do-build:	graphic esf $(NARCNAME1) $(TARGETDIR)/$(NARCNAME1) $(NARCNAME2) $(TARGETDIR)/$(NARCNAME2)
else
do-build:	graphic $(TARGETDIR)/$(NARCNAME1) $(TARGETDIR)/$(NARCNAME2)
endif

graphic:
	@$(MAKE_SUBDIR)

$(TARGETDIR)/$(NARCNAME1):	$(NARCNAME1)
	$(COPY)	$(NARCNAME1) $(TARGETDIR)
	$(COPY) $(NAIXNAME1) $(TARGETDIR)

$(TARGETDIR)/$(NARCNAME2):	$(NARCNAME2)
	$(COPY)	$(NARCNAME2) $(TARGETDIR)
	$(COPY) $(NAIXNAME2) $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

esf: $(notdir $(WAZAEFFSRCS:.esf=.s)) $(notdir $(WAZAEFFSRCS:.esf=.bin))
%.s: $(RES_DIR)%.esf ../../prog/src/battle/btlv/btlv_effvm_def.h ../../prog/include/sound/wb_sound_data.sadl ../../../pokemon_wb_doc/particle/wazaeffect/mkparticle_be.mk ../../../pokemon_wb_doc/particle/wazaeffect/mkparticle_we.mk
	ruby eescmk.rb c:/home/pokemon_wb/prog/src/battle/btlv/btlv_effvm_def.h $(RES_DIR)$*.esf c:/home/pokemon_wb/

%.bin: %.s ../../prog/src/battle/btlv/btlv_effvm_def.h
#	$(MWAS) $< -o $*.o
#	$(MWLD) -dis -o $*.elf $*.o
	/cygdrive/c/Program\ Files/Freescale/CW\ for\ NINTENDO\ DSi\ V1.1/ARM_Tools/Command_Line_Tools/mwasmarm.exe $< -o $*.o
	/cygdrive/c/Program\ Files/Freescale/CW\ for\ NINTENDO\ DSi\ V1.1/ARM_Tools/Command_Line_Tools/mwldarm.exe -dis -o $*.elf $*.o
	../../tools/elf2bin $*.elf

$(NARCNAME1): $(notdir $(WAZAEFFSRCS:.esf=.bin))
	nnsarc -c -l -n -i $(NARCNAME1) we_*.bin
	$(NAIXCUT) $(NAIXNAME1)

$(NARCNAME2): $(notdir $(WAZAEFFSRCS:.esf=.bin))
	nnsarc -c -l -n -i $(NARCNAME2) be_*.bin se_*.bin
	$(NAIXCUT) $(NAIXNAME2)

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
	-rm -f $(NARCNAME1)
	-rm -f $(NAIXNAME1)
	-rm -f $(NARCNAME2)
	-rm -f $(NAIXNAME2)
endif
	@$(MAKE_SUBDIR) clean
	-rm -f $(TARGETDIR)/$(NARCNAME1)
	-rm -f $(TARGETDIR)/$(NAIXNAME1)
	-rm -f $(TARGETDIR)/$(NARCNAME2)
	-rm -f $(TARGETDIR)/$(NAIXNAME2)


