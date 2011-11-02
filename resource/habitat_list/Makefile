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
USERS = suginaka_katsunori onoue_masayuki nishino

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
habitat_data_w   	= habitat_data_w
habitat_data_b   	= habitat_data_b
habitat_data_index = habitat_data_index.rb
habitat_data_list_w = habitat_data_file_list_w.list
habitat_data_list_b = habitat_data_file_list_b.list
encout_marshal 	 = $(PROJECT_RSCDIR)/encount/encount_data.marshal
encout_form_marshal 	 = $(PROJECT_RSCDIR)/encount/encount_form_data.marshal
habitat_data				= habitat_list_data.cdat
habitat_data_h			= habitat_list_data.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS =

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define


# 読み込むファイル
input_townmap             = $(PROJECT_RSCDIR)/townmap/data/townmap_data.xls
input_zonetable           = $(PROJECT_RSCDIR)/fldmapdata/zonetable/zonetable.xls
habitat_marshal           = $(PROJECT_RSCDIR)/encount/zone_id.h
input_habitat_data        = habitat_list_data.xls
input_habitat_data_csv    = habitat_list_data.csv

EXCELCONV = ../../tools/exceltool/xls2xml/tab_out_direct_sheet.rb
source_townmap_data = ../townmap/data/townmap_data.xls

.PHONY: do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: subdir \
          $(TARGETDIR)/$(habitat_data_b).narc \
          $(TARGETDIR)/$(habitat_data_w).narc \
					$(habitat_data) \

# narc のコピーだけ実行する
$(TARGETDIR)/$(habitat_data_w).narc: $(habitat_data_w).narc
	$(COPY) $(habitat_data_w).narc $(TARGETDIR)
	$(COPY) $(habitat_data_w).naix $(TARGETDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(habitat_data_b).narc: $(habitat_data_b).narc
	$(COPY) $(habitat_data_b).narc $(TARGETDIR)
	$(COPY) $(habitat_data_b).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する

$(habitat_data_b).narc: $(sources_data) $(encout_marshal)  $(input_habitat_data_csv) zkn_habitat_make.rb  ../encount/enc_hash_data.rb $(input_townmap)
	ruby zkn_habitat_make.rb $(encout_marshal) $(habitat_data_index) $(habitat_data_list_w) $(habitat_data_list_b) $(encout_form_marshal) $(input_habitat_data_csv)
	nnsarc -r -l -n -i $(habitat_data_b) -S $(habitat_data_list_b)
	$(NAIXCUT) $(habitat_data_b).naix

$(habitat_data_w).narc: $(sources_data) $(encout_marshal) zkn_habitat_make.rb  ../encount/enc_hash_data.rb
	nnsarc -r -l -n -i $(habitat_data_w) -S $(habitat_data_list_w)
	$(NAIXCUT) $(habitat_data_w).naix

$(input_habitat_data_csv) : $(input_habitat_data) $(encout_marshal)
	@ruby $(EXCELCONV) $(input_habitat_data) -n 0 -s > $(input_habitat_data_csv)

$(habitat_data) : $(input_habitat_data_csv) habitat_data_cnv.rb
	ruby habitat_data_cnv.rb $(habitat_data) $(habitat_data_h) $(input_habitat_data_csv)

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.cdat
	rm -f *.narc
	rm -f *.naix
	rm -f hb_w*.bin
	rm -f hb_b*.bin
	rm -f $(habitat_data_list_w)
	rm -f $(habitat_data_list_b)
	rm -f $(habitat_data_index)
	rm -f $(habitat_data)
endif
	rm -f $(TARGETDIR)/$(targetname_data).narc
	rm -f $(TARGETDIR)/$(targetname_data).naix
	rm -f $(TARGETDIR)/$(habitat_data_w).narc
	rm -f $(TARGETDIR)/$(habitat_data_w).naix
	rm -f $(TARGETDIR)/$(habitat_data_b).narc
	rm -f $(TARGETDIR)/$(habitat_data_b).naix


