#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてdemo3d_narcを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#ユーザー定義をインクルード
#------------------------------------------------------------------------------
include	make_users

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

ifneq	(,$(filter $(USERNAME),$(CMD_USERS)))
CMDONLY_USER	= true
else
CMDONLY_USER = false
endif

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= res res2d

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
DATDIR = data/
TMPDIR = tmp/
BINDIR = bin/

FLDMMDL_DIR = ../fldmmdl/mmdl_res/
FLDMMDL_RES_PREFIX = res/fldmmdl/fmdl_

DEMO_INC_DIR = $(PROJECT_PROGDIR)include/demo/
DEMO3D_DIR	= $(PROJECT_PROGDIR)src/demo/demo3d/
DEMO3D_DAT_DIR = $(DEMO3D_DIR)data/

demoid_def_h = $(DATDIR)demo3d_demoid.h
target_demoid_def_h = $(DEMO_INC_DIR)$(notdir $(demoid_def_h))

demo_res_dat = $(DATDIR)demo3d_resdata.cdat
target_demo_res_dat = $(DEMO3D_DAT_DIR)$(notdir $(demo_res_dat))

demo3d_narc = $(DATDIR)demo3d.narc
demo3d_naix = $(demo3d_narc:.narc=.naix)
target_narc = $(TARGETDIR)$(notdir $(demo3d_narc))
target_naix = $(TARGETDIR)$(notdir $(demo3d_naix))

#プレイアブル向けに書換え
#arc_list = $(TMPDIR)arc_list.txt
arc_list = playable_arc_list.txt

#コンバート対象にする管理表リストをインクルード
include demo_list.txt	#$(DEMO_LIST)定義
DEMO_LIST_DAT = $(addprefix $(DATDIR),$(DEMO_LIST:.xls=.cdat))
DEMO_LIST_TMP = $(addprefix $(TMPDIR),$(DEMO_LIST:.xls=.txt))
TARGET_DEMO_LIST_DAT = $(addprefix $(DEMO3D_DAT_DIR),$(notdir $(DEMO_LIST_DAT)))

#コンバート対称にするfldmmdlのリストをインクルード
include fldmmdl_list.txt	#$(FLDMMDL_LIST)定義
FLDMMDL_IMD = $(addprefix $(FLDMMDL_RES_PREFIX),$(FLDMMDL_LIST))
FLDMMDL_SRC = $(addprefix $(FLDMMDL_DIR),$(FLDMMDL_LIST))

# NARCの依存ターゲットを集める
obj_nsx:=$(wildcard ./bin/*.ns*)
obj_bin:=$(wildcard ./bin/*.bin)
demo_rule_xls:=$(wildcard ./*.xls)

.PHONY:	do-build clean subdir

#自動削除して欲しくない中間ファイル
.PRECIOUS: $(DEMO_LIST_TMP)

#ツールパス定義
#XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb
XLS2TAB = $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
COMMON_TARGET = make_res2d $(target_narc) $(target_naix) $(target_demoid_def_h) $(target_demo_res_dat) $(TARGET_DEMO_LIST_DAT)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

ifeq	($(CMDONLY_USER),true)
do-build:	make_res_dat $(COMMON_TARGET)
else
do-build:	$(FLDMMDL_IMD) subdir make_res_dat $(COMMON_TARGET)
endif

else
do-build:	$(COMMON_TARGET)
endif

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

######################################
#demo_idコンバート
$(demoid_def_h): demo_list.txt
	$(MAKE) make_demoid

$(demo_res_dat): demo_list.txt
	$(MAKE) make_demoid

make_demoid:
	@echo	デモID/Resourceテーブルコンバート開始
#	ruby conv/make_demoid.rb $(demoid_def_h) $(demo_res_dat) $(DEMO_LIST)
#	unix2dos -D $(demoid_def_h)
#	unix2dos -D $(demo_res_dat)

######################################
#fldmmdlコピー
$(FLDMMDL_IMD): $(FLDMMDL_SRC)
	@echo	fldmmdl resコピー開始
	$(COPY) $(addprefix $(FLDMMDL_DIR),$(notdir $(subst fmdl_,,$@))) $@

######################################
#シーンデータコンバート
make_res_dat: $(DEMO_LIST_DAT)
	@echo $(DEMO_LIST_DAT)

$(TMPDIR)%.txt: %.xls
	@echo $@ $<
	$(XLS2TAB) -o $@ -p 1 -s tab $<
#	$(XLS2TAB) $< > $@

$(DATDIR)%.cdat: $(TMPDIR)%.txt 
	ruby conv/make_cdat.rb $< $@
	unix2dos -D $@
endif

######################################
#3dリソースデータコンバート
# リソースバイナリが古い場合実行する
$(demo3d_narc): $(obj_nsx) $(obj_bin)
	$(MAKE) make_narc

$(demo3d_naix): $(obj_nsx) $(obj_bin)
	$(MAKE) make_narc

make_narc: $(arc_list)
#	-ls bin/*_camera.bin bin/*.nsb* > $(arc_list)
#	perl $(PROJECT_ROOT)tools/arc_list_make.pl $(arc_list)
	nnsarc -r -l -n -i $(demo3d_narc) -S $(arc_list) > $(TMPDIR)arc_result.txt
	@echo naixカット
	$(NAIXCUT) $(demo3d_naix)

make_res2d:
	$(MAKE) -C res2d

# narc のコピーだけ実行する
$(target_narc): $(demo3d_narc)
	$(COPY) $(demo3d_narc) $(target_narc)
$(target_naix): $(demo3d_naix)
	$(COPY) $(demo3d_naix) $(target_naix)

# ヘッダ のコピー
$(target_demoid_def_h): $(demoid_def_h) 
	$(COPY) $(demoid_def_h) $(target_demoid_def_h)

# リソースのコピー
$(target_demo_res_dat): $(demo_res_dat) 
	$(COPY) $(demo_res_dat) $(target_demo_res_dat)

$(DEMO3D_DAT_DIR)%.cdat: $(DATDIR)%.cdat
	$(COPY) -f $< $@


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除

ifeq	($(CMDONLY_USER),false)
	@$(MAKE_SUBDIR)
	rm -f $(FLDMMDL_IMD)
endif
	rm -f $(TMPDIR)*.*
	rm -f $(DATDIR)n_legend_meat_*.cdat
#	rm -f $(demoid_def_h)
#	rm -f $(demo_res_dat)

endif
	make clean -C res2d
	@echo $(DEMO3D_DAT_DIR)
	rm -f $(target_demoid_def_h)
	rm -f $(target_demo_res_dat)
	rm -f $(DEMO3D_DAT_DIR)*.cdat
	rm -f $(demo3d_narc)
	rm -f $(demo3d_naix)
	rm -f $(target_narc)
	rm -f $(target_naix)

