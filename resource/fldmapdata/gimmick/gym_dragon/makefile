
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou tanoue_reiko unno fuchino onoue_masayuki nakamura_akira ohkubo_tomohiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_w = gym_dragon_w
targetname_b = gym_dragon_b

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
FLDMAP = fieldmap
TARGETDIR = $(PROJECT_ARCDIR)$(FLDMAP)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#DEF_HEADER = gym_dragon_local_def.h

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

include data_list

#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
ARCLIST_W = arc_list_w.txt
ARCLIST_B = arc_list_b.txt

#------------------------------------------------------------------------------
#コンバートファイル
#------------------------------------------------------------------------------
XLS_FILE 	= gym_data.xls
HEIGHT_XLS_FILE 	= ex_height_data.xls
DATACONV	= data_cnv.rb
HEIGHT_DATACONV	= height_data_cnv.rb
DEF_DATACONV	= def_data_cnv.rb
DATA_CSV_FILE = gym_data.csv
DEF_CSV_FILE = gym_data_def.csv
HEIGHT_DATA_CSV_FILE = height_data.csv
DATA_CDAT_FILE = gym_dragon_trace_data.cdat
HEIGHT_DATA_CDAT_FILE = gym_dragon_height_data.cdat
ID_HEADER_FILE = gym_dragon_trace_data_id.h
DEF_DATA_FILE = gym_dragon_gym_data_def.h

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include

#------------------------------------------------------------------------------
#ツール
#------------------------------------------------------------------------------

DEF_MAKE = ../def_write.rb
ICA_CNV = ../../../../tools/ica/ica_converter.rb
EXCELCONV = ../../../../tools/exceltool/xls2xml/tab_out_direct_sheet.rb

.PHONY:	do-build clean



do-build: $(TARGETDIR)/$(targetname_b).narc $(TARGETDIR)/$(targetname_w).narc $(DATA_CDAT_FILE) $(HEIGHT_DATA_CDAT_FILE) $(DEF_DATA_FILE)

clean:
	@rm -f $(TARGETDIR)/$(targetname_w).narc
	@rm -f $(TARGETDIR)/$(targetname_w).naix
	@rm -f $(TARGETDIR)/$(targetname_b).narc
	@rm -f $(TARGETDIR)/$(targetname_b).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname_w).narc
	@rm -f $(targetname_w).naix
	@rm -f $(targetname_b).narc
	@rm -f $(targetname_b).naix
	@rm -f *.nsb*
	@rm -f *.bin
	@rm -f $(DATA_CSV_FILE)
	@rm -f $(DEF_CSV_FILE)
	@rm -f $(DATA_CDAT_FILE)
	@rm -f $(ID_HEADER_FILE)
	@rm -f $(HEIGHT_DATA_CSV_FILE)
	@rm -f $(HEIGHT_DATA_CDAT_FILE)
	@rm -f $(DEF_CSV_FILE)
	@rm -f $(DEF_DATA_FILE)
endif


#コピー実行する
$(TARGETDIR)/$(targetname_w).narc:  $(targetname_w).narc 
	@echo copying $(targetname_w).narc
	@$(COPY) $(targetname_w).narc $(TARGETDIR)
	@$(COPY) $(targetname_w).naix $(TARGETDIR)

#コピー実行する
$(TARGETDIR)/$(targetname_b).narc:  $(targetname_b).narc 
	@echo copying $(targetname_b).narc
	@$(COPY) $(targetname_b).narc $(TARGETDIR)
	@$(COPY) $(targetname_b).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

$(DATA_CSV_FILE): $(XLS_FILE) $(DATACONV)
	@ruby $(EXCELCONV) $(XLS_FILE) -n 0 -s > $(DATA_CSV_FILE)

$(DEF_CSV_FILE): $(XLS_FILE) $(DATACONV)
	@ruby $(EXCELCONV) $(XLS_FILE) -n 1 -s > $(DEF_CSV_FILE)

$(HEIGHT_DATA_CSV_FILE): $(HEIGHT_XLS_FILE) $(HEIGHT_DATACONV)
	@ruby $(EXCELCONV) $(HEIGHT_XLS_FILE) -n 0 -s > $(HEIGHT_DATA_CSV_FILE)

$(DATA_CDAT_FILE): $(DATA_CSV_FILE) $(DATACONV)
	@ruby $(DATACONV) $(DATA_CSV_FILE) $(DATA_CDAT_FILE) $(ID_HEADER_FILE)

$(HEIGHT_DATA_CDAT_FILE): $(HEIGHT_DATA_CSV_FILE)
	@ruby $(HEIGHT_DATACONV) $(HEIGHT_DATA_CSV_FILE) $(HEIGHT_DATA_CDAT_FILE)

$(DEF_DATA_FILE): $(DEF_CSV_FILE) $(DEF_DATACONV)
	@ruby $(DEF_DATACONV) $(DEF_CSV_FILE) $(DEF_DATA_FILE)

#.SUFFIXES:	.nsbmd .imd
#imdデータからの生成ルール
%.nsbmd:	data/%.imd
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbca:	data/%.ica
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbma:	data/%.ima
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbta:	data/%.ita
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbtp:	data/%.itp
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbva:	data/%.iva
#	@echo $<をコンバート
	@g3dcvtr $<

%.bin:	data/null/%.ica
	@ruby $(ICA_CNV) -srt $< $@

# コピー先のnarcが古い場合実行する
$(targetname_b).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname_b).narc -S $(ARCLIST_B) > arc_result.txt
	$(NAIXCUT) $(targetname_b).naix

# コピー先のnarcが古い場合実行する
$(targetname_w).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname_w).narc -S $(ARCLIST_W) > arc_result.txt
	$(NAIXCUT) $(targetname_w).naix

endif
