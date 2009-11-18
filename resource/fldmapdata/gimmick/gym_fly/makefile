
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou ohmori ohkubo_tomohiko

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gym_fly

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
FLDMAP = fieldmap
TARGETDIR = $(PROJECT_ARCDIR)$(FLDMAP)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

DEF_HEADER = gym_fly_local_def.h

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

.PHONY:	do-build clean



do-build: $(TARGETDIR)/$(targetname).narc $(DEF_HEADER)

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f *.nsb*
	@rm -f *.bin
	@rm -f $(DEF_HEADER)
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
%.nsbmd:	data/cannon/%.imd
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbca:	data/cannon/%.ica
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbma:	data/cannon/%.ima
#	@echo $<をコンバート
	@g3dcvtr $<

%.bin:	data/null/%.ica
	@ruby $(ICA_CNV) -t $< $@

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
#	@-rm -f *.nsb*
#	@-rm -f *.bin

$(DEF_HEADER): def_make.xls
	ruby $(XLS_CNV) -c def_make.xls > def_make.csv
	ruby $(DEF_MAKE) def_make.csv $(DEF_HEADER)
endif
