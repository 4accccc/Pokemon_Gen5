#======================================================================
# makefile	
# フェスミッションデータコンバート環境
#======================================================================

#コンバート対象ユーザー定義
USERS = iwasawa masafumi_saitou onoue_masayuki

include $(PROJECT_RSCDIR)/macro_define

TARGETDIR = $(PROJECT_ARCDIR)
TOOLDIR = tool/
DATDIR = ./
TMPDIR = ./tmp/
GSYS_H_DIR = $(PROJECT_PROGDIR)include/gamesystem/
GSYS_S_DIR = $(PROJECT_PROGDIR)src/gamesystem/
FIELD_S_DIR = $(PROJECT_PROGDIR)src/field/

target_name = fmission_data
data_xls = $(target_name).xls
data_bin = $(target_name).bin
data_common_bin = $(target_name)_common.bin
def_header = $(target_name)_def.h
mission_id_h = $(target_name)_id.h
data_item_cdat = fes_gimmick_item.cdat
data_item_hdat = fes_gimmick_item_dat.h
data_npc_cdat = fes_gimmick_npc.cdat
data_npc_hdat = fes_gimmick_npc_dat.h
data_trail_cdat = fes_gimmick_trail.cdat
data_trail_hdat = fes_gimmick_trail.h
msg_data = $(PROJECT_RSCDIR)/message/msproj/src/fest_mission.mstxt
monsno_def = $(PROJECT_RSCDIR)/personal/monsno_def.h
quiz_pokelist = fmission_quiz_pokelist

data_narc = $(target_name).narc
data_naix = $(target_name).naix

data_csv = $(TMPDIR)$(target_name).csv
data_def_csv = $(TMPDIR)$(target_name)_def.csv
data_item_csv = $(TMPDIR)$(target_name)_item.csv
data_trail_csv = $(TMPDIR)$(target_name)_trail.csv

evflag_h = $(PROJECT_RSCDIR)fldmapdata/flagwork/flag_define.h
msg_id_h = $(PROJECT_PROGDIR)include/msg/msg_fest_mission.h
poke_name_hash = $(PROJECT_ROOT)/tools/hash/monsno_hash.rb
item_name_hash = $(PROJECT_ROOT)/tools/hash/item_hash.rb
special_mons_tbl = $(PROJECT_ROOT)/tools/hash/special_mons_tbl.rb
monstype_hash = $(PROJECT_ROOT)/tools/hash/monstype_hash.rb
chihou_zukanno_hash = $(PROJECT_ROOT)/tools/hash/chihou_zukanno_hash.rb

target_narc = $(addprefix $(TARGETDIR),$(notdir $(data_narc)))
target_naix = $(addprefix $(TARGETDIR),$(notdir $(data_naix)))
target_mission_id_h = $(addprefix $(GSYS_H_DIR),$(notdir $(mission_id_h)))
target_def_h = $(addprefix $(GSYS_H_DIR),$(notdir $(def_header)))
target_item_cdat = $(addprefix $(FIELD_S_DIR),$(notdir $(data_item_cdat)))
target_item_hdat = $(addprefix $(FIELD_S_DIR),$(notdir $(data_item_hdat)))
target_npc_cdat = $(addprefix $(FIELD_S_DIR),$(notdir $(data_npc_cdat)))
target_npc_hdat = $(addprefix $(FIELD_S_DIR),$(notdir $(data_npc_hdat)))
trail_mons_cdat = $(addprefix $(FIELD_S_DIR),$(notdir $(data_trail_cdat)))
trail_mons_hdat = $(addprefix $(FIELD_S_DIR),$(notdir $(data_trail_hdat)))

BIN_DEPEND_LIST	= $(data_csv) $(data_def_csv) $(evflag_h) $(msg_id_h) $(poke_name_hash) $(item_name_hash) $(msg_data)
ITEM_DEPEND_LIST	= $(data_item_csv) $(data_trail_csv) $(item_name_hash)
QUIZ_DEPEND_LIST = $(poke_name_hash) $(special_mons_tbl) $(monstype_hash) $(chihou_zukanno_hash)

#自動削除して欲しくない中間ファイル
.PRECIOUS: $(data_csv) $(mission_id_h)

#---------------------------------------------------------------
# ツールパス	
#---------------------------------------------------------------
RUBY_DAT_RES = $(TOOLDIR)fmission_dat_conv.rb
RUBY_ITEM_RES = $(TOOLDIR)fmission_item_conv.rb
RUBY_TRAIL_RES = $(TOOLDIR)fmission_trail_conv.rb
RUBY_QUIZ_RES = $(TOOLDIR)fmission_quiz_conv.rb
EXCELCONV = ../../tools/exceltool/ExcelSeetConv.exe
NCGIMDCONV = ../../tools/fld_char2imd/ncg2imd.rb
CSVCONV = ../../tools/exceltool/xls2xml/tab_out_direct.rb


#---------------------------------------------------------------
#	ターゲット
#---------------------------------------------------------------
.PHONY: do-build clean

#---------------------------------------------------------------
#	build
#---------------------------------------------------------------
#do-build: $(data_csv) $(data_def_csv) $(target_narc)
do-build: $(target_narc) $(target_mission_id_h) $(target_def_h) $(target_item_cdat) $(target_item_hdat) $(target_npc_cdat) $(target_npc_hdat) $(trail_mons_cdat) $(trail_mons_hdat)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

######################################
#エクセルデータコンバート
$(data_csv): $(data_xls)
	@echo $@ $<
	$(EXCELCONV) -o $@ -p 1 -s csv $<

$(data_def_csv): $(data_xls)
	@echo $@ $<
	$(EXCELCONV) -o $@ -p 2 -s csv $<

$(data_item_csv): $(data_xls)
	@echo $@ $<
	$(EXCELCONV) -o $@ -p 3 -s csv $<

$(data_trail_csv): $(data_xls)
	@echo $@ $<
	$(EXCELCONV) -o $@ -p 4 -s csv $<

$(data_bin): $(BIN_DEPEND_LIST)
	$(RUBY) $(RUBY_DAT_RES) $(data_bin) $(data_common_bin) $(def_header) $(mission_id_h) $(data_csv) $(data_def_csv) $(evflag_h) $(msg_id_h)

$(data_common_bin): $(BIN_DEPEND_LIST)
	$(RUBY) $(RUBY_DAT_RES) $(data_bin) $(data_common_bin) $(def_header) $(mission_id_h) $(data_csv) $(data_def_csv) $(evflag_h) $(msg_id_h)

$(data_item_cdat): $(ITEM_DEPEND_LIST)
	$(RUBY) $(RUBY_ITEM_RES) $(data_item_cdat) $(data_item_hdat) $(data_npc_cdat) $(data_npc_hdat) $(data_item_csv)

$(data_item_hdat): $(ITEM_DEPEND_LIST)
	$(RUBY) $(RUBY_ITEM_RES) $(data_item_cdat) $(data_item_hdat) $(data_npc_cdat) $(data_npc_hdat) $(data_item_csv)

$(data_npc_cdat): $(ITEM_DEPEND_LIST)
	$(RUBY) $(RUBY_ITEM_RES) $(data_item_cdat) $(data_item_hdat) $(data_npc_cdat) $(data_npc_hdat) $(data_item_csv)

$(data_npc_hdat): $(ITEM_DEPEND_LIST)
	$(RUBY) $(RUBY_ITEM_RES) $(data_item_cdat) $(data_item_hdat) $(data_npc_cdat) $(data_npc_hdat) $(data_item_csv)

$(data_trail_cdat): $(ITEM_DEPEND_LIST) $(monsno_def) $(def_header) $(RUBY_TRAIL_RES)
	$(RUBY) $(RUBY_TRAIL_RES) $(data_trail_cdat) $(data_trail_hdat) $(def_header) $(monsno_def) $(data_trail_csv)

$(mission_id_h): $(BIN_DEPEND_LIST)
	@echo $@ $<
	ruby $(RUBY_DAT_RES) $< $(DATDIR)
	unix2dos -D $@

$(def_header): $(BIN_DEPEND_LIST)
	@echo $@ $<
	ruby $(RUBY_DAT_DEF) $< $(DATDIR)
	unix2dos -D $@

$(data_narc): $(data_bin) $(data_common_bin) $(QUIZ_DEPEND_LIST)
	ruby $(RUBY_QUIZ_RES) $(quiz_pokelist)
	nnsarc -c -l -n -i $(data_narc) $(data_common_bin) $(data_bin) $(quiz_pokelist)*.bin >arc_result.txt
	$(NAIXCUT) $(data_naix)

endif

#リソースとヘッダのコピー
$(target_mission_id_h): $(mission_id_h)
	$(COPY) -f $< $@
$(target_def_h): $(def_header)
	$(COPY) -f $< $@
$(target_item_cdat): $(data_item_cdat)
	$(COPY) -f $< $@
$(target_item_hdat): $(data_item_hdat)
	$(COPY) -f $< $@
$(target_npc_cdat): $(data_npc_cdat)
	$(COPY) -f $< $@
$(target_npc_hdat): $(data_npc_hdat)
	$(COPY) -f $< $@
$(trail_mons_cdat): $(data_trail_cdat)
	$(COPY) -f $< $@
$(trail_mons_hdat): $(data_trail_hdat)
	$(COPY) -f $< $@

# narc のコピーだけ実行する
$(target_narc): $(data_narc)
	$(COPY) $(data_narc) $(TARGETDIR)
	$(COPY) $(data_naix) $(TARGETDIR) 

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.bin *.cdat *.h *log.txt
	rm -f $(TMPDIR)*.*
	rm -f $(data_narc) $(data_naix)
endif
	rm -f $(target_narc) $(target_naix)
	rm -f $(target_mission_id_h)
	rm -f $(target_def_h)
	rm -f $(target_item_cdat) $(target_item_hdat)
	rm -f $(target_npc_cdat) $(target_npc_hdat)
	rm -f $(trail_mons_cdat) $(trail_mons_hdat)

