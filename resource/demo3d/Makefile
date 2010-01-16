#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#ユーザー定義をインクルード
#------------------------------------------------------------------------------
include	make_users

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = demo3d

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
#SUBDIRS	= c_cruiser n_castle
include make_dir

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# NARCの依存ターゲットを集める
obj_imd:=$(wildcard ./*/*.imd)
obj_ica:=$(wildcard ./*/*.ica)
obj_ita:=$(wildcard ./*/*.ita)
obj_ima:=$(wildcard ./*/*.ima)
obj_itp:=$(wildcard ./*/*.itp)
obj_iva:=$(wildcard ./*/*.iva)
obj_bin:=$(wildcard ./*/*.bin)

obj_init:=$(wildcard ./*/init.txt)

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	makedir subdir $(TARGETDIR)/$(targetname).narc

makedir:
	ruby make_dir.rb

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(obj_imd) $(obj_ica) $(obj_ita) $(obj_ima) $(obj_itp) $(obj_iva) $(obj_bin) $(obj_init)
	-ls */*.nsb* */*.bin > arclist.txt
	perl $(PROJECT_ROOT)tools/arc_list_make.pl arclist.txt
	nnsarc -r -l -n -i $(targetname) -S arclist.txt

	$(NAIXCUT) $(targetname).naix 
	ruby make_demoid.rb
	ruby make_header.rb
	$(COPY) demo3d_demoid.h $(PROJECT_PROGDIR)include/demo
	$(COPY) demo3d_resdata.cdat $(PROJECT_PROGDIR)src/demo/demo3d

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.*bin*
	rm -f *.*nsb*
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)$(targetname).narc
	rm -f $(TARGETDIR)$(targetname).naix


