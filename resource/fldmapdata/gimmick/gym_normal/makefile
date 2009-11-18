
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou murakami_naoto ibe_mana

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gym_normal

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
FLDMAP = fieldmap
TARGETDIR = $(PROJECT_ARCDIR)$(FLDMAP)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

DEF_HEADER = gym_normal_local_def.h
EXIT_DEF_HEADER = gym_normal_exit_def.h

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
#LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt *.nsb*
#endif

LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include

#------------------------------------------------------------------------------
#ツール
#------------------------------------------------------------------------------
DEF_MAKE = ../def_write.rb
XLS_CNV = ../../../../tools/exceltool/xls2xml/tab_out.rb

.PHONY:	do-build clean



do-build: $(TARGETDIR)/$(targetname).narc $(DEF_HEADER) $(EXIT_DEF_HEADER)

clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f *.nsb*

ifeq	($(USERNAME),saitou)
	@rm -f $(DEF_HEADER)
	@rm -f $(EXIT_DEF_HEADER)
endif
ifeq	($(USERNAME),murakami_naoto)
	@rm -f $(DEF_HEADER)
	@rm -f $(EXIT_DEF_HEADER)
endif

endif

#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
.SUFFIXES:	.nsbmd .imd
#imdデータからの生成ルール
%.nsbmd:	data/%.imd
#	@echo $<をコンバート
	@g3dcvtr $<

%.nsbca:	data/%.ica
#	@echo $<をコンバート
	@g3dcvtr $<

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(DATA_LIST)
	@nnsarc -i -cl -l -n $(targetname).narc -S $(ARCLIST) > arc_result.txt
#	@-rm -f *.nsb*

ifeq	($(USERNAME),saitou)
$(DEF_HEADER): wall_def_make.xls
	ruby $(XLS_CNV) -c wall_def_make.xls > wall_def_make.csv
	ruby $(DEF_MAKE) wall_def_make.csv $(DEF_HEADER)

$(EXIT_DEF_HEADER): exit_def_make.xls
	ruby $(XLS_CNV) -c exit_def_make.xls > exit_def_make.csv
	ruby $(DEF_MAKE) exit_def_make.csv $(EXIT_DEF_HEADER)
endif

ifeq	($(USERNAME),murakami_naoto)
$(DEF_HEADER): wall_def_make.xls
	ruby $(XLS_CNV) -c wall_def_make.xls > wall_def_make.csv
	ruby $(DEF_MAKE) wall_def_make.csv $(DEF_HEADER)

$(EXIT_DEF_HEADER): exit_def_make.xls
	ruby $(XLS_CNV) -c exit_def_make.xls > exit_def_make.csv
	ruby $(DEF_MAKE) exit_def_make.csv $(EXIT_DEF_HEADER)
endif

endif
