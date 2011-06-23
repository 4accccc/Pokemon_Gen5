
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou tanoue_reiko unno fuchino onoue_masayuki nakamura_akira

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gym_dragon

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
ARCLIST = arc_list.txt

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

#LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix
#コンバート対象者のみ、中間ファイル削除
#ifeq	($(CONVERTUSER),true)	
#LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt *.nsb* *.bin
#endif

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



do-build: $(TARGETDIR)/$(targetname).narc $(DATA_CDAT_FILE) $(HEIGHT_DATA_CDAT_FILE) $(DEF_DATA_FILE)

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
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
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

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
$(targetname).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
	$(NAIXCUT) $(targetname).naix

endif
