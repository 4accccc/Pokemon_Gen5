#----------------------------------------------------------------------------
# Project:  Cソース->バイナリー
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#	2008.11.10	tamada	WB環境に移植開始
#
#----------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada nakatsui

#----------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#----------------------------------------------------------------------------
targetname	= zonedata
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS		=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)macro_define

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------

EXCEL2TAB	= $(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe
ELF2BIN		= $(PROJECT_ROOT)/tools/celf2bin.exe

CONVERTER	= ruby zonetable.rb

CONVSRCS	= zonetable.c
DATAFILE	= zonetable.xls

#コンバート結果のテキストファイル一覧（比較対象となるもの）
OUTPUT		= zonetable.c zone_id.h #eventlist.txt
#OUTPUT	+=	mapname.bin mapname.dat msg_header.h eventarc.txt doorevent.h

#確認用などのテンポラリファイル一覧
TEMP_FILES	= zonetable.txt arc_result.txt diff.txt

#arcディレクトリにコピーする必要があるファイルの一覧
RESULTS		= $(NARCNAME) $(NAIXNAME) zone_id.h

#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
LDIRT_CLEAN = $(TEMP_FILES) $(addprefix $(TARGETDIR)/, $(RESULTS))
ifeq	($(CONVERTUSER),true)
#コンバート対象者のみ、中間ファイル削除
LDIRT_CLEAN	+= $(OUTPUT) $(CONVSRCS:.c=.bin) $(CONVSRCS) $(RESULTS)
endif
#Makeで生成される*.binはmake cleanの削除対象にする

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

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
	cat dummy_define.h zonetableformat.h $< |unix2dos > temp.c
#	cat dummy.h $< >> temp.c
	$(CC) $(CCFLAGS) $(INCLUDES)  -c temp.c -MD -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	-rm $*.o temp.c temp.d
	-rm $*.elf


#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
.PHONY:	subdir	do-build clean

do-build: subdir $(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(foreach AFILE,$(RESULTS),$(COPY) $(AFILE) $(TARGETDIR) $(AND)) true

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
#zonetable.xls
#	--(EXCEL2TAB)-->	zonetable.txt
#	--(zonetable.rb)-->	zonetable.c（＋その他）
#	--(コンパイル＆elf2bin)-->	zonetable.bin
#	--(nnsarc)-->	zonedata.narc/.naix
$(NARCNAME): $(DATAFILE)
	$(EXCEL2TAB) $(DATAFILE)
	#$(MAKE) convert
	$(CONVERTER) $(DATAFILE:.xls=.txt)
	$(MAKE) makebin				#Ｃソース→バイナリ変換して
	nnsarc -i -c -l -n $(NARCNAME) *.bin > arc_result.txt	#アーカイブ作成
	$(NAIXCUT) $(NAIXNAME)
endif



#Ｃソース→バイナリ変換
makebin: $(CONVSRCS:.c=.bin)




#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
test:
	echo $(LDIRT_CLEAN)

#echo $(foreach types,$(FILETYPES),$(wildcard *$(types)*))


#----------------------------------------------------------------------------
#	差分表示
#----------------------------------------------------------------------------
cp_output:
	-$(foreach AFILE,$(OUTPUT),cp $(AFILE) $(AFILE).old $(AND)) true

diff_output:
	-$(foreach AFILE,$(OUTPUT),diff $(AFILE) $(AFILE).old >> diff.txt $(AND)) true

diff:
	@$(MAKE) cp_output
	@echo コンバート
	@$(EXCEL2TAB) $(DATAFILE)
	@$(CONVERTER) $(DATAFILE:.xls=.txt)
	@-echo "#差分は以下の通り" > diff.txt
	@$(MAKE) diff_output
	@cat diff.txt
	@-rm $(addsuffix .old, $(OUTPUT))



