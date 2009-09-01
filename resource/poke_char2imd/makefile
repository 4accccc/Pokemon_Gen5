include imd_list
#include make_path		<<ＷＢ移行でコメントアウト

TSURE_DATA_32_DIR	= tsure_32x32
TSURE_DATA_64_DIR	= tsure_64x64

TARGET_32_LIST	= $(TSURE_DATA_32_DIR)/pair_poke32x32
TARGET_64_LIST	= $(TSURE_DATA_64_DIR)/pair_poke64x64

XLS_DATA = pair_poke.xls
TEXT_DATA = $(XLS_DATA:.xls=.txt)
PAIR_POKE_INFO_ARC	= pair_poke_info.narc
ARCLIST = pair_poke_info_list

CSVCVTR = ExcelSeetConv.exe

CSVFILE = pair_poke.csv

do-build: rev_info $(TARGET_32_LIST) $(TARGET_64_LIST) $(IMD_DATA) $(PAIR_POKE_INFO_ARC)

$(TEXT_DATA):$(XLS_DATA)
	$(CSVCVTR) -s csv $(XLS_DATA)

$(TARGET_32_LIST) $(TARGET_64_LIST) : $(TEXT_DATA)
#	$(CSVCVTR) -s csv $(XLS_DATA)
	ruby make_list.rb $(TEXT_DATA)

$(IMD_DATA): ncg2imd32 ncg2imd64
	
ncg2imd32:
	$(MAKE) -C $(TSURE_DATA_32_DIR)
ncg2imd64:
	$(MAKE) -C $(TSURE_DATA_64_DIR)


#pmgs_convディレクトリとpokemon_gsディレクトリが同じ階層にあることを前提とした処理です
#imd_copy:
#	xcopy tex_imd $(DST_PATH)\src\data\mmodel\mmdl_tex_poke /d /y	<<ＷＢ移行でコメントアウト

$(PAIR_POKE_INFO_ARC):$(TEXT_DATA)
	ruby pair_poke_info.rb $(TEXT_DATA)
	nnsarc -i -c -l -n $(PAIR_POKE_INFO_ARC) -S $(ARCLIST)  > arc_result.txt
	#環境に依存しますコピー処理なので消して、手動でコピーしても問題ないです
#	cp $(PAIR_POKE_INFO_ARC) $(DST_PATH)\src\arc		<<ＷＢ移行でコメントアウト

rev_info: $(CSVFILE)
	ruby make_rev_info.rb $(CSVFILE)

$(CSVFILE):$(XLS_DATA)
	$(CSVCVTR) -s csv -o $(CSVFILE) $(XLS_DATA)

poke_dat:
	ruby ppoke_no_make.rb $(TEXT_DATA)
