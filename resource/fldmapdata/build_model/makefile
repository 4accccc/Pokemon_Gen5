#------------------------------------------------------------------------------
#
#	配置モデルデータコンバート用Makefile
#
#	2009.04.26	tamada	エリア別リストデータの生成まで完了
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	 lee_hyunjung fuchino tamada nakatsui ibe_mana tanoue_reiko \
				 fujiwara_maiko ohkubo_tomohiko unno james_turner tomoya_takahashi iwasawa \
				 obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
#targetname	= all_build_model
TARGET_OUTDOOR	= buildmodel_outdoor
NARC_OUTDOOR	= $(TARGET_OUTDOOR).narc
NAIX_OUTDOOR	= $(TARGET_OUTDOOR).naix

TARGET_INDOOR		= buildmodel_indoor
NARC_INDOOR	= $(TARGET_INDOOR).narc
NAIX_INDOOR	= $(TARGET_INDOOR).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= anime_files


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

XLS2TAB	= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb
TEXMAG	= $(PROJECT_ROOT)/tools/tex_mag/tex_mag.exe


WKDIR			= wkdir
RESDIR		= nsbmdfiles
SRCDIR		= imdfiles
INDEXDIR	= indexfiles
TEXSETDIR	= texset
TEXSET_TMPDIR	= $(TEXSETDIR)/tmp
OUTPUT		= output

XLS_DATA_OUTDOOR	= $(TARGET_OUTDOOR).xls
TXT_DATA_OUTDOOR	= $(WKDIR)/$(XLS_DATA_OUTDOOR:.xls=.txt)
DEP_LIST_OUTDOOR	= $(WKDIR)/depend_outdoor.txt
ARC_LIST_OUTDOOR	= $(WKDIR)/archive_outdoor.txt
INDEX_OUTDOOR			= bmodel_index_outdoor.narc
BIN_DATA_OUTDOOR	= $(WKDIR)/$(XLS_DATA_OUTDOOR:.xls=.bin)
TEXSET_OUTDOOR		= bmodel_texset_outdoor.narc
TEXSET_DEP_OUTDOOR		= $(WKDIR)/depend_texset_outdoor.txt
TEXSET_ARC_OUTDOOR		= $(WKDIR)/archive_texset_outdoor.txt

XLS_DATA_INDOOR	= $(TARGET_INDOOR).xls
TXT_DATA_INDOOR	= $(WKDIR)/$(XLS_DATA_INDOOR:.xls=.txt)
DEP_LIST_INDOOR	= $(WKDIR)/depend_indoor.txt
ARC_LIST_INDOOR	= $(WKDIR)/archive_indoor.txt
INDEX_INDOOR		= bmodel_index_indoor.narc
BIN_DATA_INDOOR	= $(WKDIR)/$(XLS_DATA_INDOOR:.xls=.bin)
TEXSET_INDOOR		= bmodel_texset_indoor.narc
TEXSET_DEP_INDOOR		= $(WKDIR)/depend_texset_indoor.txt
TEXSET_ARC_INDOOR		= $(WKDIR)/archive_texset_indoor.txt

#アニメデータ関連
XLS_DATA_ANIME	= buildmodel_anime.xls
TXT_DATA_ANIME	= $(WKDIR)/$(XLS_DATA_ANIME:.xls=.txt)
BIN_DATA_ANIME	= $(WKDIR)/$(XLS_DATA_ANIME:.xls=.bin)

NARC_BIN_DATA		= buildmodel_info.narc
NAIX_BIN_DATA		= buildmodel_info.naix

.PHONY:	do-build lists clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir bindata
#	$(MAKE) bindata
	$(MAKE) -j 3 arc
	$(MAKE) -j 3 index
	$(MAKE) -j 3 texset

-include $(DEP_LIST_OUTDOOR)
-include $(DEP_LIST_INDOOR)
-include $(TEXSET_DEP_OUTDOOR)
-include $(TEXSET_DEP_INDOOR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	.nsbmd .nsbtx .imd
vpath %imd $(SRCDIR)
vpath %imd $(TEXSETDIR)
vpath %nsbtx $(TEXSETDIR)

#imdデータからの生成ルール
%.nsbmd:	../$(SRCDIR)/%.imd
	@echo $<をコンバート
	@g3dcvtr $< -o $@ -emdl

#テクスチャセットimdデータからの生成ルール
%.nsbtx:	%.imd
	@echo	$<TextureSet imdをコンバート
	@g3dcvtr $< -o $@ -etex

endif	#コンバート対象者のみのルール終了

subdir:
	@$(MAKE_SUBDIR)

#モデルデータアーカイブ
arc:	$(TARGETDIR)/$(NARC_OUTDOOR) $(TARGETDIR)/$(NARC_INDOOR)

#エリア別インデックスアーカイブ
index: $(TARGETDIR)/$(INDEX_OUTDOOR) $(TARGETDIR)/$(INDEX_INDOOR)

#エリア別テクスチャセットアーカイブ
texset: $(TARGETDIR)/$(TEXSET_OUTDOOR) $(TARGETDIR)/$(TEXSET_INDOOR)

bindata:	$(TARGETDIR)/$(NARC_BIN_DATA)

$(TARGETDIR)/$(NARC_BIN_DATA): $(NARC_BIN_DATA)
	@echo $(TARGETDIR)に$(NARC_BIN_DATA)をコピー
	@$(COPY)	$(NARC_BIN_DATA) $(TARGETDIR)
	@$(COPY) $(NAIX_BIN_DATA) $(TARGETDIR)

$(TARGETDIR)/$(NARC_OUTDOOR):	$(OUTPUT)/$(NARC_OUTDOOR)
	@echo $(TARGETDIR)に$(OUTPUT)/$(NARC_OUTDOOR)をコピー
	@$(COPY)	$(OUTPUT)/$(NARC_OUTDOOR) $(TARGETDIR)
	@$(COPY) $(OUTPUT)/$(NAIX_OUTDOOR) $(TARGETDIR)

$(TARGETDIR)/$(NARC_INDOOR):	$(OUTPUT)/$(NARC_INDOOR)
	@echo $(TARGETDIR)に$(OUTPUT)/$(NARC_INDOOR)をコピー
	@$(COPY)	$(OUTPUT)/$(NARC_INDOOR) $(TARGETDIR)
	@$(COPY) $(OUTPUT)/$(NAIX_INDOOR) $(TARGETDIR)

$(OUTPUT)/$(NARC_OUTDOOR):	$(DEP_OUTDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(OUTPUT)/$(NARC_OUTDOOR) -S $(ARC_LIST_OUTDOOR) > $(WKDIR)/arc_result.txt
	@$(NAIXCUT) $(OUTPUT)/$(NAIX_OUTDOOR)

$(OUTPUT)/$(NARC_INDOOR):	$(DEP_INDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(OUTPUT)/$(NARC_INDOOR) -S $(ARC_LIST_INDOOR) > $(WKDIR)/arc_result.txt
	@$(NAIXCUT) $(OUTPUT)/$(NAIX_INDOOR)

#エリア別インデックスコピー
$(TARGETDIR)/$(INDEX_OUTDOOR):	$(INDEXDIR)/$(INDEX_OUTDOOR)
	@echo $(TARGETDIR)に$<をコピー
	@$(COPY) $(INDEXDIR)/$(INDEX_OUTDOOR) $(TARGETDIR)

$(TARGETDIR)/$(INDEX_INDOOR):	$(INDEXDIR)/$(INDEX_INDOOR)
	@echo $(TARGETDIR)に$<をコピー
	@$(COPY) $(INDEXDIR)/$(INDEX_INDOOR) $(TARGETDIR)

#エリア別テクスチャセットアーカイブコピー
$(TARGETDIR)/$(TEXSET_OUTDOOR):	$(OUTPUT)/$(TEXSET_OUTDOOR)
	@echo $(TARGETDIR)に$<をコピー
	@$(COPY) $(OUTPUT)/$(TEXSET_OUTDOOR) $(TARGETDIR)

$(TARGETDIR)/$(TEXSET_INDOOR):	$(OUTPUT)/$(TEXSET_INDOOR)
	@echo $(TARGETDIR)に$<をコピー
	@$(COPY) $(OUTPUT)/$(TEXSET_INDOOR) $(TARGETDIR)

#エリア別テクスチャセットアーカイブ生成
$(OUTPUT)/$(TEXSET_OUTDOOR): $(TEXSET_DEP_OUTDOOR) $(DEP_TEXSET_OUTDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $@ -S $(TEXSET_ARC_OUTDOOR) > $(WKDIR)/arc_texset_result.txt

$(OUTPUT)/$(TEXSET_INDOOR):	$(TEXSET_DEP_INDOOR) $(DEP_TEXSET_INDOOR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $@ -S $(TEXSET_ARC_INDOOR) > $(WKDIR)/arc_texset_result.txt

#------------------------------------------------------------------------------
#	以下はコンバート対象者のみ動作する
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから

$(NARC_BIN_DATA):	$(BIN_DATA_ANIME) $(BIN_DATA_OUTDOOR) $(BIN_DATA_INDOOR)
	@echo \"$(BIN_DATA_ANIME)\" > $(WKDIR)/bindata.list
	@echo \"$(BIN_DATA_OUTDOOR)\" >> $(WKDIR)/bindata.list
	@echo \"$(BIN_DATA_INDOOR)\" >> $(WKDIR)/bindata.list
	nnsarc -c -l -n -i $(NARC_BIN_DATA) -S $(WKDIR)/bindata.list > $(WKDIR)/arc_result.txt
	$(NAIXCUT) $(NAIX_BIN_DATA)

#
$(TXT_DATA_OUTDOOR):	$(XLS_DATA_OUTDOOR)
	@echo $<からリスト生成中
	@$(XLS2TAB) $(XLS_DATA_OUTDOOR) > $(TXT_DATA_OUTDOOR)

$(TXT_DATA_INDOOR):	$(XLS_DATA_INDOOR)
	@echo $<からリスト生成中
	@$(XLS2TAB) $(XLS_DATA_INDOOR) > $(TXT_DATA_INDOOR)

#
$(BIN_DATA_OUTDOOR):	$(TXT_DATA_OUTDOOR)
	@echo コンバート対象配置モデルのリストアップ中
	@ruby makelist.rb $(TXT_DATA_OUTDOOR) \
		DEP_OUTDOOR $(DEP_LIST_OUTDOOR) $(ARC_LIST_OUTDOOR) $(BIN_DATA_OUTDOOR)

$(BIN_DATA_INDOOR):	$(TXT_DATA_INDOOR)
	@echo コンバート対象配置モデルのリストアップ中
	@ruby makelist.rb $(TXT_DATA_INDOOR) \
		DEP_INDOOR $(DEP_LIST_INDOOR) $(ARC_LIST_INDOOR) $(BIN_DATA_INDOOR)

#インデックス生成
$(INDEXDIR)/$(INDEX_OUTDOOR):		$(TXT_DATA_OUTDOOR)
	@echo 屋外エリア別配置モデルリスト（$@）生成中
	@ruby makeindex.rb	$(TXT_DATA_OUTDOOR) out $(INDEXDIR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(INDEXDIR)/$(INDEX_OUTDOOR) -S $(INDEXDIR)/out.list > $(WKDIR)/arc_result.txt

$(INDEXDIR)/$(INDEX_INDOOR):		$(TXT_DATA_INDOOR)
	@echo 屋内エリア別配置モデルリスト（$@）生成中
	@ruby makeindex.rb	$(TXT_DATA_INDOOR) in $(INDEXDIR)
	@echo $@をアーカイブしています
	@nnsarc -c -l -n -i $(INDEXDIR)/$(INDEX_INDOOR) -S $(INDEXDIR)/in.list > $(WKDIR)/arc_result.txt

#エリア別配置モデルテクスチャセット生成
$(TEXSET_DEP_OUTDOOR):		$(TXT_DATA_OUTDOOR)
	@echo 屋外エリア別配置モデルTexture Set生成リスト（$@）生成中
	@ruby maketexset.rb	$(TXT_DATA_OUTDOOR) outdoor $(TEXSETDIR) $(TEXSET_TMPDIR) \
		$(TEXSET_DEP_OUTDOOR) $(TEXSET_ARC_OUTDOOR)

$(TEXSET_DEP_INDOOR):		$(TXT_DATA_INDOOR)
	@echo 屋内エリア別配置モデルTexture Set生成リスト（$@）生成中
	@ruby maketexset.rb	$(TXT_DATA_INDOOR) indoor $(TEXSETDIR) \
		$(TEXSET_TMPDIR) $(TEXSET_DEP_INDOOR) $(TEXSET_ARC_INDOOR)

$(BIN_DATA_ANIME):	$(XLS_DATA_ANIME) anime_files/buildmodel_anime.naix
	@echo 配置モデルアニメ情報（$(BIN_DATA_ANIME)）生成中
	@$(XLS2TAB) -c $(XLS_DATA_ANIME) > $(TXT_DATA_ANIME)
	@ruby bmanime_info.rb $(TXT_DATA_ANIME) $(BIN_DATA_ANIME)

endif		#コンバート対象者のみ動作、ここまで

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	@echo cleanup build_model directory
	@-rm -f $(TARGETDIR)/$(NARC_OUTDOOR) $(TARGETDIR)/$(NAIX_OUTDOOR)
	@-rm -f $(TARGETDIR)/$(NARC_INDOOR) $(TARGETDIR)/$(NAIX_INDOOR)
	@-rm -f $(OUTPUT)/$(NARC_OUTDOOR) $(OUTPUT)/$(NAIX_OUTDOOR)
	@-rm -f $(OUTPUT)/$(NARC_INDOOR) $(OUTPUT)/$(NAIX_INDOOR)
	@-rm -f $(TARGETDIR)/$(INDEX_OUTDOOR) $(TARGETDIR)/$(INDEX_INDOOR)
	@-rm -f $(TARGETDIR)/$(TEXSET_OUTDOOR) $(TARGETDIR)/$(TEXSET_INDOOR)
	@-rm -f $(OUTPUT)/$(TEXSET_OUTDOOR) $(OUTPUT)/$(TEXSET_OUTDOOR:.narc=.naix)
	@-rm -f $(OUTPUT)/$(TEXSET_INDOOR) $(OUTPUT)/$(TEXSET_INDOOR:.narc=.naix)
	@-rm -f $(TARGETDIR)/$(NARC_BIN_DATA)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	@-rm -f $(DEP_OUTDOOR) $(DEP_INDOOR)
	@-rm -f $(WKDIR)/*.txt *.txt
	@-rm -f $(INDEXDIR)/*
	@-rm -f $(TEXSETDIR)/*.nsbtx $(TEXSETDIR)/*.imd
	@-rm -f $(TEXSET_TMPDIR)/*
	@-rm -f $(WKDIR)/*.bin
	@-rm -f $(NARC_BIN_DATA)
endif

texture_clean:
	@-rm -f $(TEXSETDIR)/*.nsbtx $(TEXSETDIR)/*.imd
	@-rm -f $(TEXSET_TMPDIR)/*
	@-rm -f $(TEXSET_DEP_OUTDOOR) $(TEXSET_ARC_OUTDOOR)
	@-rm -f $(TEXSET_DEP_INDOOR) $(TEXSET_ARC_INDOOR)
	@-rm -f $(OUTPUT)/$(TEXSET_OUTDOOR) $(OUTPUT)/$(TEXSET_OUTDOOR:.narc=.naix)
	@-rm -f $(OUTPUT)/$(TEXSET_INDOOR) $(OUTPUT)/$(TEXSET_INDOOR:.narc=.naix)

texture_test:
	ruby check_texture.rb

