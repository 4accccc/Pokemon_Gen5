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
gmmname           = zkn_height.gmm zkn_weight.gmm zkn_comment_00.gmm zkn_comment_01.gmm zkn_type.gmm zkn_form.gmm
targetname_area_w  = zukan_area_w
targetname_area_b  = zukan_area_b
encout_marshal 	 = $(PROJECT_RSCDIR)/encount/encount_data.marshal

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
GMMDIR    = $(PROJECT_RSCDIR)/message/src

#------------------------------------------------------------------------------
#※gmm->mstxt変換ツール
#------------------------------------------------------------------------------
UPDATEWITHGMM 	= ../../tools/message/UpdateWithGmm.bat
ZNK_COMMENT_00_MSTXT	= src/zkn_comment_00.mstxt
ZNK_COMMENT_01_MSTXT	= src/zkn_comment_01.mstxt
ZNK_HEIGHT_MSTXT	= src/zkn_height.mstxt
ZNK_WEIGHT_MSTXT	= src/zkn_weight.mstxt
ZNK_TYPE_MSTXT	= src/zkn_type.mstxt
ZNK_FORM_MSTXT	= src/zkn_form.mstxt
MSPROJDIR	= ../message/msproj/


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
					$(MSPROJDIR)$(ZNK_COMMENT_00_MSTXT) \
					$(MSPROJDIR)$(ZNK_COMMENT_01_MSTXT) \
					$(MSPROJDIR)$(ZNK_HEIGHT_MSTXT) \
					$(MSPROJDIR)$(ZNK_WEIGHT_MSTXT) \
					$(MSPROJDIR)$(ZNK_TYPE_MSTXT) \
					$(MSPROJDIR)$(ZNK_FORM_MSTXT)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_data).narc: $(targetname_data).narc
	$(COPY) $(targetname_data).narc $(TARGETDIR)
	$(COPY) $(targetname_data).naix $(TARGETDIR)

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

$(gmmname): $(sources_data)
	perl zkn_sort_personal_make.pl $(input_personal)
	perl zkn_hw_make.pl $(input_personal)
	perl zkn_comment_make.pl $(input_personal) $(input_zukan) wb
	nnsarc -r -l -n -i $(targetname_data) -S zkn_data_file_list.lst
	$(NAIXCUT) $(targetname_data).naix

$(targetname_area_w).narc: $(sources_area_w) $(source_townmap_data) zkn_encount_make.pl
	perl zkn_encount_make.pl $(input_encount) $(input_monsno) w
	nnsarc -r -l -n -i $(targetname_area_w) -S $(input_area_list_w)
	$(NAIXCUT) $(targetname_area_w).naix

$(targetname_area_b).narc: $(sources_area_b) $(source_townmap_data)
	perl zkn_encount_make.pl $(input_encount) $(input_monsno) b zkn_encount_make.pl
	nnsarc -r -l -n -i $(targetname_area_b) -S $(input_area_list_b)
	$(NAIXCUT) $(targetname_area_b).naix

$(MSPROJDIR)$(ZNK_COMMENT_00_MSTXT) : zkn_comment_00.gmm
	$(UPDATEWITHGMM) . zkn_comment_00.gmm $(ZNK_COMMENT_00_MSTXT)

$(MSPROJDIR)$(ZNK_COMMENT_01_MSTXT) : zkn_comment_01.gmm
	$(UPDATEWITHGMM) . zkn_comment_01.gmm $(ZNK_COMMENT_01_MSTXT)

$(MSPROJDIR)$(ZNK_HEIGHT_MSTXT) : zkn_height.gmm
	$(UPDATEWITHGMM) . zkn_height.gmm $(ZNK_HEIGHT_MSTXT)

$(MSPROJDIR)$(ZNK_WEIGHT_MSTXT) : zkn_weight.gmm
	$(UPDATEWITHGMM) . zkn_weight.gmm $(ZNK_WEIGHT_MSTXT)

$(MSPROJDIR)$(ZNK_TYPE_MSTXT) : zkn_type.gmm
	$(UPDATEWITHGMM) . zkn_type.gmm $(ZNK_TYPE_MSTXT)

$(MSPROJDIR)$(ZNK_FORM_MSTXT) : zkn_form.gmm
	$(UPDATEWITHGMM) . zkn_form.gmm $(ZNK_FORM_MSTXT)

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
	rm -f $(gmmname)
	rm -f $(targetname_area_w).narc
	rm -f $(targetname_area_w).naix
	rm -f $(targetname_area_b).narc
	rm -f $(targetname_area_b).naix
endif
	rm -f $(TARGETDIR)/$(targetname_data).narc
	rm -f $(TARGETDIR)/$(targetname_data).naix
	rm -f $(TARGETDIR)/$(targetname_area_w).narc
	rm -f $(TARGETDIR)/$(targetname_area_w).naix
	rm -f $(TARGETDIR)/$(targetname_area_b).narc
	rm -f $(TARGETDIR)/$(targetname_area_b).naix

