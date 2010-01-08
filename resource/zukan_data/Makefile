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
USERS = kawada_koji

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_data    = zukan_data
gmmname            = zkn_height.gmm zkn_weight.gmm zkn_comment_00.gmm zkn_comment_01.gmm zkn_type.gmm
targetname_area    = zukan_area

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
input_area_list           = zkn_area_file_list.lst
input_data_sort_aiueo_idx = zkn_sort_aiueo_idx.h

# 依存するファイル
sources_data        = $(input_personal) \
                      $(input_zukan)
sources_area        = $(input_encount) \
                      $(input_monsno)


.PHONY: do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: subdir \
          $(TARGETDIR)/$(targetname_data).narc \
          $(TARGETDIR)/$(targetname_area).narc \
          $(foreach eachname, $(gmmname), $(GMMDIR)/$(eachname))

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_data).narc: $(targetname_data).narc
	$(COPY) $(targetname_data).narc $(TARGETDIR)
	$(COPY) $(targetname_data).naix $(TARGETDIR)

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

$(TARGETDIR)/$(targetname_area).narc: $(targetname_area).narc
	$(COPY) $(targetname_area).narc $(TARGETDIR)
	$(COPY) $(targetname_area).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する

$(targetname_data).narc: $(sources_data)
	perl zkn_sort_personal_make.pl $(input_personal)
	perl zkn_hw_make.pl $(input_personal)
	perl zkn_comment_make.pl $(input_personal) $(input_zukan) wb
	nnsarc -r -l -n -i $(targetname_data) -S zkn_data_file_list.lst
	$(NAIXCUT) $(targetname_data).naix
	$(COPY) $(gmmname) $(GMMDIR)

$(gmmname): $(sources_data)
	perl zkn_sort_personal_make.pl $(input_personal)
	perl zkn_hw_make.pl $(input_personal)
	perl zkn_comment_make.pl $(input_personal) $(input_zukan) wb
	nnsarc -r -l -n -i $(targetname_data) -S zkn_data_file_list.lst
	$(NAIXCUT) $(targetname_data).naix
	$(COPY) $(gmmname) $(GMMDIR)

$(targetname_area).narc: $(sources_area)
	perl zkn_encount_make.pl $(input_encount) $(input_monsno)
	nnsarc -r -l -n -i $(targetname_area) -S $(input_area_list)
	$(NAIXCUT) $(targetname_area).naix

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.dat
	rm -f $(input_data_sort_aiueo_idx)
	rm -f $(input_area_list)
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname_data).narc
	rm -f $(targetname_data).naix
	rm -f $(gmmname)
	rm -f $(targetname_area).narc
	rm -f $(targetname_area).naix
endif
	rm -f $(TARGETDIR)/$(targetname_data).narc
	rm -f $(TARGETDIR)/$(targetname_data).naix
	rm -f $(foreach eachname, $(gmmname), $(GMMDIR)/$(eachname))
	rm -f $(TARGETDIR)/$(targetname_area).narc
	rm -f $(TARGETDIR)/$(targetname_area).naix
