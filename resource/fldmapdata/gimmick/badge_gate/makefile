
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	onoue_masayuki matsuzaki_tsubasa

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = badge_gate_gmk

XLS_FILE 	= data_def.xls
DEF_CSV_FILE = bage_gate_def.csv
DEF_DATA_FILE = bage_gate_def.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
FLDMAP = fieldmap
TARGETDIR = $(PROJECT_ARCDIR)$(FLDMAP)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

include data_list

#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
ARCLIST = arc_list.txt

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include

#------------------------------------------------------------------------------
#ツール
#------------------------------------------------------------------------------
DEF_DATACONV	= def_data_cnv.rb
EXCELCONV = ../../../../tools/exceltool/xls2xml/tab_out_direct_sheet.rb

.PHONY:	do-build clean



do-build: $(TARGETDIR)/$(targetname).narc $(DEF_DATA_FILE)

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f $(DEF_DATA_FILE)
	@rm -f $(DEF_CSV_FILE)
	@rm -f *.nsb*
endif


#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

%.nsbmd:	data/%.imd
	@g3dcvtr $<

%.nsbca:	data/%.ica
	@g3dcvtr $<

%.nsbta:	data/%.ita
	@g3dcvtr $<

$(DEF_CSV_FILE): $(XLS_FILE)
	@ruby $(EXCELCONV) $(XLS_FILE) -s > $(DEF_CSV_FILE)

$(DEF_DATA_FILE): $(DEF_CSV_FILE) $(DEF_DATACONV)
	@ruby $(DEF_DATACONV) $(DEF_CSV_FILE) $(DEF_DATA_FILE)

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
	$(NAIXCUT) $(targetname).naix

endif
