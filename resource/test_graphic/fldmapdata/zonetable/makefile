#! make -f
#----------------------------------------------------------------------------
# Project:  Cソース->バイナリー
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

SUBDIRS		=
TARGET_ARC		=	zonetable.narc

#include	sourcefile.lst

#----------------------------------------------------------------------------
#	Ｃソース→バイナリ変換を可能にするための設定
#----------------------------------------------------------------------------
include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs

#共通変数定義
#include $(DP_ROOT)/commondefs.GF

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

%.bin: %.c
	echo "//" > temp.c
#	echo "#include \"$(NITROSDK_ROOT)/include/nitro/types.h\"" > temp.c
	cat zonetableformat.h $< |unix2dos > temp.c
#	cat dummy.h $< >> temp.c
	$(CC) $(CCFLAGS) $(INCLUDES)  -c temp.c -MD -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	-rm $*.o temp.c temp.d
	-rm $*.elf


#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
OUTPUT	=	zonetable.dat zone_id.h	#eventlist.txt
#OUTPUT	+=	mapname.bin mapname.dat msg_header.h eventarc.txt doorevent.h
#Makeで生成される*.binはmake cleanの削除対象にする
LDIRT_CLEAN	= $(OUTPUT) $(CONVSRCS:.c=.bin) $(CONVSRCS) $(TARGET_ARC) $(TARGET_ARC:.narc=.naix)

LINCLUDES		= $(NITROSDK_ROOT)/include
#LINCLUDES		+= $(DP_ROOT)/src/field

LINCLUDES	+= $(dir $<)

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------


CONVSRCS	= zonetable.c
DATAFILE	= zonetable.xls
EXCEL2TAB	=	$(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe
DP_ROOT		= c:/home/ds_pokemon/pokemon_dp
ELF2BIN		= $(DP_ROOT)/convert/eventdata/celf2bin.exe
#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
do-build: $(TARGET_ARC)

$(TARGET_ARC): $(DATAFILE)
	$(EXCEL2TAB) $(DATAFILE)
	#$(MAKE) convert
	ruby zonetable.rb $(DATAFILE:.xls=.txt)
	cat zonetable.dat > zonetable.c
	$(MAKE) makebin				#Ｃソース→バイナリ変換して
	nnsarc -i -c -l -n $(TARGET_ARC) *.bin > arc_result.txt	#アーカイブ作成



#Ｃソース→バイナリ変換
makebin: $(CONVSRCS:.c=.bin)




#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
test:
	echo $(LDIRT_CLEAN)

#echo $(foreach types,$(FILETYPES),$(wildcard *$(types)*))



