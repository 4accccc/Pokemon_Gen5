#! make -f
#----------------------------------------------------------------------------
#	@file	makefile
#	@brief	エリア毎の動作モデルリストアーカイブ生成用makefile
#	@date	2005.11.24
#	@author	tamada GAME FREAK inc.
#
#----------------------------------------------------------------------------
USERS = kagaya lee_hyunjung
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

$(ARCLIST): $(RSCTXT) $(FMMDLHEAD)
	@echo エリア別データ生成処理･･･
	@$(TXT2ASM)  $(RSCTXT) $(ARCLIST) $(ARCHEAD)
	@echo 個別アセンブラファイルコンパイル･･･
	@$(MAKE) ASMFILE

test:
	echo $(wildcard mmlid_*.s)
	echo $(ASMFILES:.s=.bin)
endif
