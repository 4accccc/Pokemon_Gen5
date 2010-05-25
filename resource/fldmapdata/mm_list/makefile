#! make -f
#----------------------------------------------------------------------------
#	@file	makefile
#	@brief	エリア毎の動作モデルリストアーカイブ生成用makefile
#	@date	2005.11.24
#	@author	tamada GAME FREAK inc.
#
#----------------------------------------------------------------------------
#※USERS に名前を追加する場合はfldmmdlのメイクも通るようにしてください
USERS = kagaya lee_hyunjung takahashi saitou saya_tsuruta fuchino iwasawa
include $(PROJECT_RSCDIR)/macro_define

#----------------------------------------------------------------------------
#	アセンブラ→バイナリ生成に必要な定義ファイルをインクルード
#----------------------------------------------------------------------------
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#include $(DP_ROOT)/modulerules.GF
include $(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
TARGET_DIR = $(PROJECT_ROOT)/prog/arc/fieldmap
RSCFILE	=	movemodel_list.xls
RSCTXT	=	$(RSCFILE:.xls=.txt)
ARCNAME	=	move_model_list.narc
ARCLIST	=	arclist.txt
ARCHEAD	=	mmlist_def.h
ASMFILES	= $(wildcard mmlid_*.s)
FMMDLHEAD = $(PROJECT_ROOT)/resource/fldmmdl/fldmmdl_objcode.h

CSV_LIST = fldmmdl_list.csv
CSV_POKE = fldmmdl_poke_list.csv
CSV_MDL = fldmmdl_mdl_list.csv
MMDL_XLS = fldmmdl_list.xls
PRM_BIN	= ../../fldmmdl/fldmmdl_mdlparam.bin

MMLIST = movemodel_list.txt
ZONEXLS = ../zonetable/zonetable.xls
ZONECSV = zonetable.csv


#Makeで生成される*.bin/*.narcはmake cleanの削除対象にする
ifeq ($(CONVERTUSER),true) #CONVERTUSER
LDIRT_CLEAN = *.s *.bin $(RSCTXT) $(ARCNAME) $(TARGET_DIR)/$(ARCNAME)
else
LDIRT_CLEAN	= $(TARGET_DIR)/$(ARCNAME)
endif

#アセンブラファイルでインクルードするヘッダを探すパス指定
IPATH	=	-I$(PROJECT_ROOT)/resource/fldmmdl/

#ツール定義
ELF2BIN	=	$(PROJECT_ROOT)/tools/elf2bin.exe
XLS2TXT	=	$(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe
TXT2ASM	=	ruby mmodel_file.rb

AREA_MMDL_CHECK	=	ruby area_mdl_check.rb

do-build:  $(TARGET_DIR)/$(ARCNAME)

#----------------------------------------------------------------------------
#	*.s --> *.binへのルール定義
#----------------------------------------------------------------------------
%.bin: %.s
	@echo $<
	@cp $< temp.s
	@$(MWAS) $(IPATH) temp.s -o $*.o
	@$(MWLD) -dis -o $*.elf $*.o
	@$(ELF2BIN) $*.elf > /dev/null
	@rm $*.o
	@rm $*.elf
	@rm temp.s
	@ruby rewrite_bin.rb $*

#----------------------------------------------------------------------------
#	アーカイブ生成
#----------------------------------------------------------------------------
$(TARGET_DIR)/$(ARCNAME): $(ARCNAME)
	$(COPY) $(ARCNAME) $(TARGET_DIR)

ifeq ($(CONVERTUSER),true) #CONVERTUSER
$(ARCNAME): $(ARCLIST) $(ASMFILE:.s=.bin)
	nnsarc -i -c -l -n $(ARCNAME) -S $(ARCLIST) > arc_result.txt
	@echo エリア毎動作モデルリストのアーカイブを生成しました。
endif

#----------------------------------------------------------------------------
#	エリア毎のアセンブラファイル生成
#----------------------------------------------------------------------------
ifeq ($(CONVERTUSER),true) #CONVERTUSER
$(RSCTXT): $(RSCFILE)
	@echo 動作モデル管理表変換処理･･･
	@$(XLS2TXT) $(RSCFILE)				#タブ区切りテキストに変換
	
ASMFILE:	$(ASMFILES:.s=.bin)

#$(CSV_LIST): ../../fldmmdl/$(CSV_LIST)
#	@$(COPY)  ../../fldmmdl/$(CSV_LIST) $(CSV_LIST)
#$(CSV_POKE): ../../fldmmdl/$(CSV_POKE)
#	@$(COPY)  ../../fldmmdl/$(CSV_POKE) $(CSV_POKE)
#$(CSV_MDL): ../../fldmmdl/$(CSV_MDL)
#	@$(COPY)  ../../fldmmdl/$(CSV_MDL) $(CSV_MDL)

$(MMDL_XLS): ../../fldmmdl/$(MMDL_XLS)
	@$(COPY)  ../../fldmmdl/$(MMDL_XLS) $(MMDL_XLS)
	@echo エクセル処理･･･
	@$(XLS2TXT) -o $(CSV_LIST) -p 1 -s csv $(MMDL_XLS)
	@$(XLS2TXT) -o $(CSV_POKE) -p 2 -s csv $(MMDL_XLS)
	@$(XLS2TXT) -o $(CSV_MDL) -p 3 -s csv $(MMDL_XLS)
	@echo エクセル処理･･･終了

$(ARCLIST): $(RSCTXT) $(FMMDLHEAD) $(MMDL_XLS) $(PRM_BIN)
	@echo エリア別データ生成処理･･･
	@$(TXT2ASM)  $(RSCTXT) $(ARCLIST) $(ARCHEAD)
	@echo 個別アセンブラファイルコンパイル･･･
	@$(MAKE) ASMFILE

test:
	echo $(wildcard mmlid_*.s)
	echo $(ASMFILES:.s=.bin)

check:
	#ゾーンテーブルをＣＳＶファイルにする
	@$(XLS2TXT) -o $(ZONECSV) -p 1 -s csv $(ZONEXLS)
	#チェックする
	@$(AREA_MMDL_CHECK) $(MMLIST) $(ZONECSV)

endif
