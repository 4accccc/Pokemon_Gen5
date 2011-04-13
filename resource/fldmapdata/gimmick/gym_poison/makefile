
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ichiraku_katsuhiko ohtani_mayo

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gym_poison

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

ICA_CNV = ../../../../tools/ica/ica_converter.rb
DEF_MAKE = ../def_write.rb
XLS_CNV = ../../../../tools/exceltool/xls2xml/tab_out.rb

XLSDIRECT = ruby $(PROJECT_ROOT)tools/exceltool/xls2xml/tab_out_direct.rb
DATAFILE = map_array.xls
DATATXT = $(DATAFILE:.xls=.csv)
OUTPUT = map_array.cdat

.PHONY:	do-build clean



do-build: $(TARGETDIR)/$(targetname).narc $(OUTPUT)

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f *.nsb*
	@rm -f *.bin
	@rm -f $(OUTPUT)
	@rm -f *.csv
endif


#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
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

%.nsbtp:	data/%.itp
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbta:	data/%.ita
#	@echo $<をコンバート
	@g3dcvtr $<

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
	@echo naixカット
	$(NAIXCUT) $(targetname).naix
#	@-rm -f *.nsb*
#	@-rm -f *.bin

$(OUTPUT): $(DATAFILE)
	$(XLSDIRECT) $(DATAFILE) -c > $(DATATXT)
	@ruby map_array_conv.rb $(DATATXT)

endif
