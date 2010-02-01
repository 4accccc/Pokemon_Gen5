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
include users.def

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

#XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb
XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -t
ELF2BIN		= $(PROJECT_ROOT)/tools/celf2bin.exe

CONVERTER	= ruby scripts/zonetable.rb

HEADERDIR	= header
WORKDIR		= tmp
RESDIR		= result
DATAFILE	= zonetable.xls
DATATABTXT	= $(WORKDIR)/$(DATAFILE:.xls=.txt)
CONVSRCS	= $(RESDIR)/zonetable.c
DEPFILES	= ../area_data/area_id.h \
						../../sound/wb_sound_data.sadl \
						../script/script_seq.naix \
						../../message/script_message.naix \
						../../encount/encount_data_w.naix \
						tmp/map_matrix.h

MATRIXDATA = ../map_matrix/map_matrix.xls
MATRIXHEADER = $(WORKDIR)/map_matrix.h

#コンバート結果のテキストファイル一覧（比較対象となるもの）
OUTPUT		= $(RESDIR)/zonetable.c

#確認用などのテンポラリファイル一覧
TEMP_FILES	= zonetable.txt arc_result.txt diff.txt

#arcディレクトリにコピーする必要があるファイルの一覧
RESULTS		= $(RESDIR)/$(NARCNAME) $(RESDIR)/$(NAIXNAME)

#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
LDIRT_CLEAN = $(TEMP_FILES) $(addprefix $(TARGETDIR)/, $(RESULTS))
ifeq	($(CONVERTUSER),true)
#コンバート対象者のみ、中間ファイル削除
LDIRT_CLEAN	+= $(OUTPUT) $(RESDIR)/*.bin $(DATATABTXT) $(CONVSRCS) $(RESULTS) \
							 $(MATRIXHEADER)
endif

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

#----------------------------------------------------------------------------
#	Ｃソース→バイナリ変換を可能にするための設定
#----------------------------------------------------------------------------
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

%.bin: %.c
	echo "//" > temp.c
#	echo "#include \"$(NITROSDK_ROOT)/include/nitro/types.h\"" > temp.c
	cat $(HEADERDIR)/dummy_define.h $(HEADERDIR)/zonetableformat.h $< |unix2dos > temp.c
#	cat dummy.h $< >> temp.c
	$(CC) $(INCLUDES)  -c temp.c -MD -o $*.o
	$(MWLD) -dis -o $*.elf $*.o
	$(ELF2BIN) $*.elf
	-rm $*.o temp.c temp.d
	-rm $*.elf


#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
.PHONY:	subdir	do-build clean test dummyfiles checkfiles

do-build: subdir $(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME):	$(RESDIR)/$(NARCNAME)
	$(foreach AFILE,$(RESULTS),$(COPY) $(AFILE) $(TARGETDIR) $(AND)) true

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
#zonetable.xls
#	--(EXCEL2TAB)-->	zonetable.txt
#	--(zonetable.rb)-->	zonetable.c（＋その他）
#	--(コンパイル＆elf2bin)-->	zonetable.bin
#	--(nnsarc)-->	zonedata.narc/.naix
$(RESDIR)/$(NARCNAME): $(DATAFILE) $(DEPFILES) $(MATRIXHEADER)
	@echo マップ管理表更新
	@$(XLS2TAB) $(DATAFILE) > $(DATATABTXT)
	@$(CONVERTER) contents $(DATATABTXT)
	@$(MAKE) makebin				#Ｃソース→バイナリ変換して
	@nnsarc -i -c -l -n $(RESDIR)/$(NARCNAME) $(RESDIR)/*.bin > $(RESDIR)/arc_result.txt	#アーカイブ作成
	@$(NAIXCUT) $(RESDIR)/$(NAIXNAME)


#map_matrix.xlsからヘッダを生成する
$(MATRIXHEADER):	$(MATRIXDATA)
	$(XLS2TAB) $(MATRIXDATA) > $(MATRIXHEADER:.h=.txt)
	ruby scripts/make_matrix_header.rb $(MATRIXHEADER:.h=.txt) > $(MATRIXHEADER)

endif



#Ｃソース→バイナリ変換
makebin: $(CONVSRCS:.c=.bin)



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
	@$(CONVERTER) $(DATATABTXT)
	@-echo "#差分は以下の通り" > diff.txt
	@$(MAKE) diff_output
	@cat diff.txt
	@-rm $(addsuffix .old, $(OUTPUT))


test:	$(DATATABTXT)
	#$(XLS2TAB) ../map_matrix/map_matrix.xls > $(WORKDIR)/map_matrix.txt
	ruby scripts/make_matrix_header.rb $(WORKDIR)/map_matrix.txt > $(WORKDIR)/map_matrix.h

dummyfiles:	$(DATATABTXT)
	ruby scripts/make_dummy.rb $(DATATABTXT) true

checkfiles:	$(DATATABTXT)
	ruby scripts/make_dummy.rb $(DATATABTXT) false

