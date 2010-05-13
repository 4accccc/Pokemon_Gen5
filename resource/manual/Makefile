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
targetname = manual

# 出力ファイル
target_gmm              = manual_text.gmm
target_image_comp_bat   = manual_image_comp.bat
target_image_conv_bat   = manual_image_conv.bat
target_image_lst        = manual_image.lst

# プログラムで使用するファイル
prog_header_file        = manual_data_def.h manual_open_flag.h

# 使用しないファイル
temp_image_src_lst      = manual_image_src.lst
temp_debug_file         = debug_manual_data_cate.csv debug_manual_data_cate_ref.csv debug_manual_data_title.csv debug_manual_data_title_ref.csv


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
binary_manual_data  = manual_data_cate.dat manual_data_cate_ref.dat manual_data_title.dat manual_data_title_ref.dat

# perlから生成されるファイル全て
perl_file = $(binary_manual_data) $(target_gmm) $(target_image_conv_bat) $(target_image_comp_bat) $(target_image_lst) $(prog_header_file)


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
do-build:	subdir $(TARGETDIR)/$(targetname).narc \
          $(GMMDIR)/$(target_gmm) \
          $(IMAGEDIR)/$(target_image_comp_bat) \
          $(IMAGEDIR)/$(target_image_conv_bat) \
          $(IMAGEDIR)/$(target_image_lst)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

# gmmやバッチファイル、リストのコピーだけ実行する
$(GMMDIR)/$(target_gmm): $(target_gmm)
	$(COPY) $(target_gmm) $(GMMDIR)

$(IMAGEDIR)/$(target_image_comp_bat): $(target_image_comp_bat)
	$(COPY) $(target_image_comp_bat) $(IMAGEDIR)

$(IMAGEDIR)/$(target_image_conv_bat): $(target_image_conv_bat)
	$(COPY) $(target_image_conv_bat) $(IMAGEDIR)

$(IMAGEDIR)/$(target_image_lst): $(target_image_lst)
	$(COPY) $(target_image_lst) $(IMAGEDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する

$(perl_file): $(source_manual_xls)
	perl manual_text_make.pl $(source_manual_xls)
	chmod 777 $(target_image_conv_bat)
	chmod 777 $(target_image_comp_bat)

$(targetname).narc: $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(binary_manual_data)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix

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
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(target_gmm)
	rm -f $(target_image_comp_bat)
	rm -f $(target_image_conv_bat)
	rm -f $(target_image_lst)
	rm -f $(binary_manual_data)
	rm -f $(prog_header_file)
	rm -f $(temp_image_src_lst)
	rm -f $(temp_debug_file)
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix
	rm -f $(GMMDIR)/$(target_gmm)
	rm -f $(IMAGEDIR)/$(target_image_comp_bat)
	rm -f $(IMAGEDIR)/$(target_image_conv_bat)
	rm -f $(IMAGEDIR)/$(target_image_lst)

