#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= kawada_koji toyama


#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname_w = manual_w
targetname_b = manual_b


# 出力ファイル
target_gmm_w             = manual_text_w.gmm
target_image_comp_bat_w  = manual_image_comp_w.bat
target_image_conv_bat_w  = manual_image_conv_w.bat
target_image_lst_w       = manual_image_w.lst

target_gmm_b             = manual_text_b.gmm
target_image_comp_bat_b  = manual_image_comp_b.bat
target_image_conv_bat_b  = manual_image_conv_b.bat
target_image_lst_b       = manual_image_b.lst


# プログラムで使用するファイル
prog_header_file_w       = manual_data_def_w.h manual_open_flag.h
prog_header_file_b       = manual_data_def_b.h manual_open_flag.h


# 使用しないファイル
temp_image_src_lst_w = manual_image_src_w.lst
temp_debug_file_w    = debug_manual_data_cate_w.csv debug_manual_data_cate_ref_w.csv debug_manual_data_title_w.csv debug_manual_data_title_ref_w.csv

temp_image_src_lst_b = manual_image_src_b.lst
temp_debug_file_b    = debug_manual_data_cate_b.csv debug_manual_data_cate_ref_b.csv debug_manual_data_title_b.csv debug_manual_data_title_ref_b.csv


#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

GMMDIR    = $(PROJECT_RSCDIR)/message/src
IMAGEDIR  = $(PROJECT_RSCDIR)/manual_image


#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)/macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))


# 読み込むファイル
source_manual_xls   = manual.xls

# 生成されるファイル
binary_manual_data_w  = manual_data_cate_w.dat manual_data_cate_ref_w.dat manual_data_title_w.dat manual_data_title_ref_w.dat
binary_manual_data_b  = manual_data_cate_b.dat manual_data_cate_ref_b.dat manual_data_title_b.dat manual_data_title_ref_b.dat

# perlから生成されるファイル全て
perl_file_w = $(binary_manual_data_w) $(target_gmm_w) $(target_image_conv_bat_w) $(target_image_comp_bat_w) $(target_image_lst_w) $(prog_header_file_w)
perl_file_b = $(binary_manual_data_b) $(target_gmm_b) $(target_image_conv_bat_b) $(target_image_comp_bat_b) $(target_image_lst_b) $(prog_header_file_b)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $<

endif

.PHONY:	do-build clean subdir


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname_w).narc \
          $(GMMDIR)/$(target_gmm_w) \
          $(IMAGEDIR)/$(target_image_comp_bat_w) \
          $(IMAGEDIR)/$(target_image_conv_bat_w) \
          $(IMAGEDIR)/$(target_image_lst_w) \
          $(TARGETDIR)/$(targetname_b).narc \
          $(GMMDIR)/$(target_gmm_b) \
          $(IMAGEDIR)/$(target_image_comp_bat_b) \
          $(IMAGEDIR)/$(target_image_conv_bat_b) \
          $(IMAGEDIR)/$(target_image_lst_b)

subdir:
	@$(MAKE_SUBDIR)


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname_w).narc: $(targetname_w).narc
	$(COPY) $(targetname_w).narc $(TARGETDIR)
	$(COPY) $(targetname_w).naix $(TARGETDIR)

$(TARGETDIR)/$(targetname_b).narc: $(targetname_b).narc
	$(COPY) $(targetname_b).narc $(TARGETDIR)
	$(COPY) $(targetname_b).naix $(TARGETDIR)


# gmmやバッチファイル、リストのコピーだけ実行する
$(GMMDIR)/$(target_gmm_w): $(target_gmm_w)
	rm -f $(GMMDIR)/manual_text.gmm
	$(COPY) $(target_gmm_w) $(GMMDIR)

$(IMAGEDIR)/$(target_image_comp_bat_w): $(target_image_comp_bat_w)
	$(COPY) $(target_image_comp_bat_w) $(IMAGEDIR)

$(IMAGEDIR)/$(target_image_conv_bat_w): $(target_image_conv_bat_w)
	$(COPY) $(target_image_conv_bat_w) $(IMAGEDIR)

$(IMAGEDIR)/$(target_image_lst_w): $(target_image_lst_w)
	$(COPY) $(target_image_lst_w) $(IMAGEDIR)


$(GMMDIR)/$(target_gmm_b): $(target_gmm_b)
	rm -f $(GMMDIR)/manual_text.gmm
	$(COPY) $(target_gmm_b) $(GMMDIR)

$(IMAGEDIR)/$(target_image_comp_bat_b): $(target_image_comp_bat_b)
	$(COPY) $(target_image_comp_bat_b) $(IMAGEDIR)

$(IMAGEDIR)/$(target_image_conv_bat_b): $(target_image_conv_bat_b)
	$(COPY) $(target_image_conv_bat_b) $(IMAGEDIR)

$(IMAGEDIR)/$(target_image_lst_b): $(target_image_lst_b)
	$(COPY) $(target_image_lst_b) $(IMAGEDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する

$(perl_file_w): $(source_manual_xls)
	perl manual_text_make.pl $(source_manual_xls) w
	chmod 777 $(target_image_conv_bat_w)
	chmod 777 $(target_image_comp_bat_w)

$(perl_file_b): $(source_manual_xls)
	perl manual_text_make.pl $(source_manual_xls) b
	chmod 777 $(target_image_conv_bat_b)
	chmod 777 $(target_image_comp_bat_b)

$(targetname_w).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(binary_manual_data_w)
	nnsarc -r -l -n -i $(targetname_w) $^
	$(NAIXCUT) $(targetname_w).naix

$(targetname_b).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(binary_manual_data_b)
	nnsarc -r -l -n -i $(targetname_b) $^
	$(NAIXCUT) $(targetname_b).naix

endif


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.NSCR
	rm -f *.NANR
	rm -f *.NCER
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname_w).narc
	rm -f $(targetname_w).naix
	rm -f $(targetname_b).narc
	rm -f $(targetname_b).naix
	rm -f $(target_gmm_w)
	rm -f $(target_image_comp_bat_w)
	rm -f $(target_image_conv_bat_w)
	rm -f $(target_image_lst_w)
	rm -f $(target_gmm_b)
	rm -f $(target_image_comp_bat_b)
	rm -f $(target_image_conv_bat_b)
	rm -f $(target_image_lst_b)
	rm -f $(binary_manual_data_w)
	rm -f $(binary_manual_data_b)
	rm -f $(prog_header_file_w)
	rm -f $(prog_header_file_b)
	rm -f $(temp_image_src_lst_w)
	rm -f $(temp_image_src_lst_b)
	rm -f $(temp_debug_file_w)
	rm -f $(temp_debug_file_b)
endif
	rm -f $(TARGETDIR)/$(targetname_w).narc
	rm -f $(TARGETDIR)/$(targetname_w).naix
	rm -f $(TARGETDIR)/$(targetname_b).narc
	rm -f $(TARGETDIR)/$(targetname_b).naix
	rm -f $(GMMDIR)/$(target_gmm_w)
	rm -f $(IMAGEDIR)/$(target_image_comp_bat_w)
	rm -f $(IMAGEDIR)/$(target_image_conv_bat_w)
	rm -f $(IMAGEDIR)/$(target_image_lst_w)
	rm -f $(GMMDIR)/$(target_gmm_b)
	rm -f $(IMAGEDIR)/$(target_image_comp_bat_b)
	rm -f $(IMAGEDIR)/$(target_image_conv_bat_b)
	rm -f $(IMAGEDIR)/$(target_image_lst_b)
	rm -f $(GMMDIR)/manual_text.gmm


