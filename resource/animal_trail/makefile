#------------------------------------------------------------------------------
#
#	けものみち用Makefile
#
#	2011.05.16	onoue_masayuki
#------------------------------------------------------------------------------

EXCELCONV = ../../tools/exceltool/ExcelSeetConv.exe
ANIMAL_TRAIL_POS_CSV_NAME = animal_trail_pos_data.csv
ANIMAL_TRAIL_CSV_NAME = animal_trail_data_
ANIMAL_TRAIL_POS_XLS = animal_trail_pos_data.xls
ANIMAL_TRAIL_XLS = animal_trail_data.xls
ANIMAL_TRAIL_CDAT = animal_trail_data.cdat
ANIMAL_TRAIL_POS_CDAT = animal_trail_pos_data.cdat

LIST_FILE = arc_list.txt
TMPDIR = ./tmp

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include user.def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
ANIMAL_TRAIL_DATA	= animal_trail_data
NARCNAME	= $(ANIMAL_TRAIL_DATA).narc
NAIXNAME	= $(ANIMAL_TRAIL_DATA).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN = 
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	LDIRT_CLEAN += $(TMPDIR)/* out_end $(ANIMAL_TRAIL_CDAT) item_hash.rb monsno_hash.rb $(NARCNAME) $(NAIXNAME) $(ANIMAL_TRAIL_POS_CDAT)
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

do-build: out_end $(NARCNAME) $(ANIMAL_TRAIL_POS_CDAT)

out_end: $(ANIMAL_TRAIL_XLS) animal_trail_data_cnv.rb
	cp ../itemconv/item_hash.rb item_hash.rb
	cp ../personal/monsno_hash.rb monsno_hash.rb
	@for x in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20;\
	do $(EXCELCONV) -o $(TMPDIR)/$(ANIMAL_TRAIL_CSV_NAME)$$x.csv -p $$x -s csv  $(ANIMAL_TRAIL_XLS) ;\
	done
	@ruby animal_trail_data_cnv.rb $(TMPDIR) $(ANIMAL_TRAIL_CSV_NAME) 20 $(ANIMAL_TRAIL_CDAT)
	@echo end > out_end

$(NARCNAME) : $(ANIMAL_TRAIL_XLS)
	nnsarc -c -l -n -i $(NARCNAME) -S $(LIST_FILE)
	$(NAIXCUT) $(NAIXNAME)

$(ANIMAL_TRAIL_POS_CDAT) : $(ANIMAL_TRAIL_POS_XLS) animal_trail_pos_data_cnv.rb
	$(EXCELCONV) -o $(TMPDIR)/$(ANIMAL_TRAIL_POS_CSV_NAME) -s csv  $(ANIMAL_TRAIL_POS_XLS)
	@ruby animal_trail_pos_data_cnv.rb $(TMPDIR) $(ANIMAL_TRAIL_POS_CSV_NAME) 20 $(ANIMAL_TRAIL_POS_CDAT)

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME) $(TARGETDIR)/$(NAIXNAME) 
$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)

$(TARGETDIR)/$(NAIXNAME):	$(NAIXNAME)
	$(COPY)	$(NAIXNAME) $(TARGETDIR)

