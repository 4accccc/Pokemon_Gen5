#------------------------------------------------------------------------------
#  図鑑に必要なデータを生成するMakefile  
#  2010.01.07
#  Koji Kawada
#  personalを使用しているので、personalより後にmakeすること
#  gmmを生成しているので、messageより先にmakeすること
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
include ../townmap/users_def

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_data 	= zukan_data
habitat_data_w   	= habitat_data_w
habitat_data_b   	= habitat_data_b
gmmname           = zkn_height.gmm zkn_weight.gmm zkn_comment_00.gmm zkn_comment_01.gmm zkn_type.gmm zkn_form.gmm
targetname_area_w  = zukan_area_w
targetname_area_b  = zukan_area_b
habitat_data_index = habitat_data_index.rb
habitat_data_list_w = habitat_data_file_list_w.list
habitat_data_list_b = habitat_data_file_list_b.list
encout_marshal 	 = $(PROJECT_RSCDIR)/encount/encount_data.marshal
habitat_data				= habitat_list_data.cdat

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
GMMDIR    = $(PROJECT_RSCDIR)/message/src

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS =

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define


# 読み込むファイル
input_personal            = $(PROJECT_RSCDIR)/personal/personal_wb.csv
input_zukan               = zukan.csv
input_encount             = $(PROJECT_RSCDIR)/encount/encount_wb.csv
input_monsno              = $(PROJECT_ROOT)tools/hash/monsno_hash.rb
input_area_list_w         = zkn_area_w_file_list.lst
input_area_list_b         = zkn_area_b_file_list.lst
input_data_sort_aiueo_idx = zkn_sort_aiueo_idx.h
input_townmap             = $(PROJECT_RSCDIR)/townmap/data/townmap_data.xls
input_zonetable           = $(PROJECT_RSCDIR)/fldmapdata/zonetable/zonetable.xls
input_zoned               = $(PROJECT_RSCDIR)/fldmapdata/zonetable/zone_id.h
habitat_marshal           = $(PROJECT_RSCDIR)/encount/zone_id.h
input_habitat_data        = habitat_list_data.xls
input_habitat_data_csv    = habitat_list_data.csv

EXCELCONV = ../../tools/exceltool/xls2xml/tab_out_direct_sheet.rb

# 依存するファイル
sources_data        = $(input_personal) \
                      $(input_zukan)
sources_area_w      = $(input_encount) \
                      $(input_monsno)
sources_area_b      = $(input_encount) \
                      $(input_monsno)

source_townmap_data = ../townmap/data/townmap_data.xls

.PHONY: do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: subdir \
          $(TARGETDIR)/$(targetname_data).narc \
          $(TARGETDIR)/$(targetname_area_w).narc \
          $(TARGETDIR)/$(targetname_area_b).narc \
          $(TARGETDIR)/$(habitat_data_b).narc \
          $(TARGETDIR)/$(habitat_data_w).narc \
					$(habitat_data) \
          $(foreach eachname, $(gmmname), $(GMMDIR)/$(eachname))

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_data).narc: $(targetname_data).narc
	$(COPY) $(targetname_data).narc $(TARGETDIR)
	$(COPY) $(targetname_data).naix $(TARGETDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(habitat_data_w).narc: $(habitat_data_w).narc
	$(COPY) $(habitat_data_w).narc $(TARGETDIR)
	$(COPY) $(habitat_data_w).naix $(TARGETDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(habitat_data_b).narc: $(habitat_data_b).narc
	$(COPY) $(habitat_data_b).narc $(TARGETDIR)
	$(COPY) $(habitat_data_b).naix $(TARGETDIR)

$(GMMDIR)/zkn_height.gmm: zkn_height.gmm
	$(COPY) zkn_height.gmm $(GMMDIR)

$(GMMDIR)/zkn_weight.gmm: zkn_weight.gmm
	$(COPY) zkn_weight.gmm $(GMMDIR)

$(GMMDIR)/zkn_comment_00.gmm: zkn_comment_00.gmm
	$(COPY) zkn_comment_00.gmm $(GMMDIR)

$(GMMDIR)/zkn_comment_01.gmm: zkn_comment_01.gmm
	$(COPY) zkn_comment_01.gmm $(GMMDIR)

$(GMMDIR)/zkn_type.gmm: zkn_type.gmm
	$(COPY) zkn_type.gmm $(GMMDIR)

$(GMMDIR)/zkn_form.gmm: zkn_form.gmm
	$(COPY) zkn_form.gmm $(GMMDIR)

$(TARGETDIR)/$(targetname_area_w).narc: $(targetname_area_w).narc
	$(COPY) $(targetname_area_w).narc $(TARGETDIR)
	$(COPY) $(targetname_area_w).naix $(TARGETDIR)

$(TARGETDIR)/$(targetname_area_b).narc: $(targetname_area_b).narc
	$(COPY) $(targetname_area_b).narc $(TARGETDIR)
	$(COPY) $(targetname_area_b).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する

$(targetname_data).narc: $(sources_data)
	perl zkn_sort_personal_make.pl $(input_personal)
	perl zkn_hw_make.pl $(input_personal)
	perl zkn_comment_make.pl $(input_personal) $(input_zukan) wb
	nnsarc -r -l -n -i $(targetname_data) -S zkn_data_file_list.lst
	$(NAIXCUT) $(targetname_data).naix
	$(COPY) $(gmmname) $(GMMDIR)

$(habitat_data_b).narc: $(sources_data) $(encout_marshal) zkn_habitat_make.rb
	ruby zkn_habitat_make.rb $(encout_marshal) $(habitat_data_index) $(habitat_data_list_w) $(habitat_data_list_b)
	nnsarc -r -l -n -i $(habitat_data_b) -S $(habitat_data_list_b)
	$(NAIXCUT) $(habitat_data_b).naix

$(habitat_data_w).narc: $(sources_data) $(encout_marshal) zkn_habitat_make.rb
	nnsarc -r -l -n -i $(habitat_data_w) -S $(habitat_data_list_w)
	$(NAIXCUT) $(habitat_data_w).naix

$(gmmname): $(sources_data)
	perl zkn_sort_personal_make.pl $(input_personal)
	perl zkn_hw_make.pl $(input_personal)
	perl zkn_comment_make.pl $(input_personal) $(input_zukan) wb
	nnsarc -r -l -n -i $(targetname_data) -S zkn_data_file_list.lst
	$(NAIXCUT) $(targetname_data).naix
	$(COPY) $(gmmname) $(GMMDIR)

$(targetname_area_w).narc: $(sources_area_w) $(source_townmap_data)
	perl zkn_encount_make.pl $(input_encount) $(input_monsno) w
	nnsarc -r -l -n -i $(targetname_area_w) -S $(input_area_list_w)
	$(NAIXCUT) $(targetname_area_w).naix

$(targetname_area_b).narc: $(sources_area_b) $(source_townmap_data)
	perl zkn_encount_make.pl $(input_encount) $(input_monsno) b
	nnsarc -r -l -n -i $(targetname_area_b) -S $(input_area_list_b)
	$(NAIXCUT) $(targetname_area_b).naix

$(input_habitat_data_csv) : $(input_habitat_data) habitat_data_cnv.rb
	@ruby $(EXCELCONV) $(input_habitat_data) -n 0 -s > $(input_habitat_data_csv)

$(habitat_data) : $(input_habitat_data_csv) habitat_data_cnv.rb
	ruby habitat_data_cnv.rb $(habitat_data) $(input_habitat_data_csv)

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.dat
	rm -f *.cdat
	rm -f $(input_data_sort_aiueo_idx)
	rm -f $(input_area_list_w)
	rm -f $(input_area_list_b)
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname_data).narc
	rm -f $(targetname_data).naix
	rm -f $(habitat_data_w).narc
	rm -f $(habitat_data_w).naix
	rm -f $(habitat_data_b).narc
	rm -f $(habitat_data_b).naix
	rm -f $(gmmname)
	rm -f $(targetname_area_w).narc
	rm -f $(targetname_area_w).naix
	rm -f $(targetname_area_b).narc
	rm -f $(targetname_area_b).naix
	rm -f hb_w*.bin
	rm -f hb_b*.bin
	rm -f $(habitat_data_list_w)
	rm -f $(habitat_data_list_b)
	rm -f $(habitat_data)
	rm -f $(habitat_data_index)
endif
	rm -f $(TARGETDIR)/$(targetname_data).narc
	rm -f $(TARGETDIR)/$(targetname_data).naix
	rm -f $(TARGETDIR)/$(habitat_data_w).narc
	rm -f $(TARGETDIR)/$(habitat_data_w).naix
	rm -f $(TARGETDIR)/$(habitat_data_b).narc
	rm -f $(TARGETDIR)/$(habitat_data_b).naix
	rm -f $(foreach eachname, $(gmmname), $(GMMDIR)/$(eachname))
	rm -f $(TARGETDIR)/$(targetname_area_w).narc
	rm -f $(TARGETDIR)/$(targetname_area_w).naix
	rm -f $(TARGETDIR)/$(targetname_area_b).narc
	rm -f $(TARGETDIR)/$(targetname_area_b).naix

