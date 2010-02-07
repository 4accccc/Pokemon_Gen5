#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = power

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 


#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

LIST_FILE = list.txt


#------------------------------------------------------------------------------
# Ｃソース→バイナリ変換を可能にするための設定
#------------------------------------------------------------------------------
INCPATH	= -I$(NITROSDK_ROOT)/include
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

ELF2BIN		= $(PROJECT_ROOT)/tools/celf2bin.exe
XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb
DATAFILE	= パレス_パワーデータ.xls
DATATXT = $(DATAFILE:.xls=.csv)

ZONETABLE_DIR		= ../../fldmapdata/zonetable/
ZONEID_H		= zone_id.h

MESSAGE_INC_DIR		= ../../message/dst/
MSG_POWER_H		= msg_power.h
MSG_POWER_EXPLAIN_H		= msg_power_explain.h

FLDMMDL_DIR		= ../../fldmmdl/
FLDMMDL_CODE_H		= fldmmdl_objcode.h

ITEM_DIR		= ../../../prog/include/item/
ITEM_H		= itemsym.h

FIELDCOMM_DIR		= ../../../prog/src/field/field_comm/
MISSION_TYPES_H		= mission_types.h

FIELDINC_DIR		= ../../../prog/include/field/





ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .imd

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
#do-build:	$(ZONETABLE_DIR)$(ZONEID_H) $(MESSAGE_INC_DIR)$(MSG_POWER_H) $(MESSAGE_INC_DIR)$(MSG_POWER_EXPLAIN_H) $(FLDMMDL_DIR)$(FLDMMDL_CODE_H) $(ITEM_DIR)$(ITEM_H) $(FIELDCOMM_DIR)$(MISSION_TYPES_H) subdir $(TARGETDIR)/$(targetname).narc
#do-build:	subdir $(TARGETDIR)/$(targetname).narc $(ZONEID_H) $(MSG_POWER_H) $(MSG_POWER_EXPLAIN_H) $(FLDMMDL_CODE_H) $(ITEM_H) $(MISSION_TYPES_H)
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc gpower_id.h
	$(COPY)	gpower_id.h $(FIELDINC_DIR)gpower_id.h
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(TARGETDIR)/$(targetname).narc $(ZONEID_H) $(MSG_POWER_H) $(MSG_POWER_EXPLAIN_H) $(FLDMMDL_CODE_H) $(ITEM_H) $(MISSION_TYPES_H) makefile power_data.bin power_conv.rb
	-ls *.bin > $(LIST_FILE)
	perl $(PROJECT_ROOT)tools/arc_list_make.pl $(LIST_FILE)
	nnsarc -r -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm $(LIST_FILE)


$(ZONEID_H): $(ZONETABLE_DIR)$(ZONEID_H)
#$(ZONETABLE_DIR)$(ZONEID_H): $(ZONEID_H)
	$(COPY)	$(ZONETABLE_DIR)$(ZONEID_H) $(ZONEID_H)

$(MSG_POWER_H): $(MESSAGE_INC_DIR)$(MSG_POWER_H)
	$(COPY)	$(MESSAGE_INC_DIR)$(MSG_POWER_H) $(MSG_POWER_H)

$(MSG_POWER_EXPLAIN_H): $(MESSAGE_INC_DIR)$(MSG_POWER_EXPLAIN_H)
	$(COPY)	$(MESSAGE_INC_DIR)$(MSG_POWER_EXPLAIN_H) $(MSG_POWER_EXPLAIN_H)

$(FLDMMDL_CODE_H): $(FLDMMDL_DIR)$(FLDMMDL_CODE_H)
	$(COPY)	$(FLDMMDL_DIR)$(FLDMMDL_CODE_H) $(FLDMMDL_CODE_H)

$(ITEM_H): $(ITEM_DIR)$(ITEM_H)
	$(COPY)	$(ITEM_DIR)$(ITEM_H) $(ITEM_H)

$(MISSION_TYPES_H): $(FIELDCOMM_DIR)$(MISSION_TYPES_H)
	$(COPY)	$(FIELDCOMM_DIR)$(MISSION_TYPES_H) $(MISSION_TYPES_H)

$(DATATXT):	$(DATAFILE) $(ZONEID_H) $(MSG_POWER_H) $(MSG_POWER_EXPLAIN_H) $(FLDMMDL_CODE_H) $(ITEM_H) $(MISSION_TYPES_H)
	$(XLS2TAB) -c -n 6 $(DATAFILE) > $(DATATXT)

#%.bin: %.c
#power_data.bin: power_data.cdat
power_data.bin: $(DATATXT)
	@ruby power_conv.rb $(DATATXT) 
	@echo "power data convert"
	@echo "//" > temp.c
	@cat sdk_header.h $(ZONEID_H) $(MSG_POWER_H) $(MSG_POWER_EXPLAIN_H) $(FLDMMDL_CODE_H) $(ITEM_H) $(MISSION_TYPES_H) power_data.cdat |unix2dos > temp.c
	@$(CC) $(INCLUDES) $(INCPATH)  -c temp.c -MD -o temp.o
	@$(MWLD) -dis -o power_data.elf temp.o
	@$(ELF2BIN) power_data.elf
	@-rm temp.o temp.c temp.d
	@-rm power_data.cdat

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(ZONEID_H)
	rm -f $(MSG_POWER_H)
	rm -f $(MSG_POWER_EXPLAIN_H)
	rm -f $(FLDMMDL_CODE_H)
	rm -f $(ITEM_H)
	rm -f $(MISSION_TYPES_H)
	rm -f $(MISSION_TYPES_H)
	rm -f *.csv
	rm -f gpower_id.h
	rm -f power_data.elf
	rm -f power_data.bin
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix

