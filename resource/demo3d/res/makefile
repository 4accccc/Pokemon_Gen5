#------------------------------------------------------------------------------
#  3Dリソースのコンバート 
#  2010.03.08
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#ユーザー定義をインクルード
#------------------------------------------------------------------------------
include ..\make_users

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = demo3d

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
TMPDIR = ../bin/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

#ICAコンバータ
ICACONV = ..\..\..\tools\ica\ica_converter.rb

# 自動でリソースを集める
src_imd1:=$(wildcard *.imd)
src_ica1:=$(wildcard *.ica)
src_ita1:=$(wildcard *.ita)
src_ima1:=$(wildcard *.ima)
src_itp1:=$(wildcard *.itp)
src_iva1:=$(wildcard *.iva)
src_camera1:=$(wildcard *_camera.ica)

src_imd2:=$(wildcard ./*/*.imd)
src_ica2:=$(wildcard ./*/*.ica)
src_ita2:=$(wildcard ./*/*.ita)
src_ima2:=$(wildcard ./*/*.ima)
src_itp2:=$(wildcard ./*/*.itp)
src_iva2:=$(wildcard ./*/*.iva)
src_camera2:=$(wildcard ./*/*_camera.ica)

#icaのリストから*_camera.icaを抜く
src_ica3 = $(src_ica1) $(src_ica2)
src_ica = $(patsubst %_camera.ica,,$(src_ica3))

src_imd = $(src_imd1) $(src_imd2)
src_ita = $(src_ita1) $(src_ita2)
src_ima = $(src_ima1) $(src_ima2)
src_itp = $(src_itp1) $(src_itp2)
src_iva = $(src_iva1) $(src_iva2)
src_camera = $(src_camera1) $(src_camera2)

# リソースが吐き出すファイルを自動でつくる
obj_imd = $(addprefix $(TMPDIR),$(notdir $(src_imd:.imd=.nsbmd)))
obj_ica = $(addprefix $(TMPDIR),$(notdir $(src_ica:.ica=.nsbca)))
obj_ita = $(addprefix $(TMPDIR),$(notdir $(src_ita:.ita=.nsbta)))
obj_ima = $(addprefix $(TMPDIR),$(notdir $(src_ima:.ima=.nsbma)))
obj_itp = $(addprefix $(TMPDIR),$(notdir $(src_itp:.itp=.nsbtp)))
obj_iva = $(addprefix $(TMPDIR),$(notdir $(src_iva:.iva=.nsbva)))
obj_bin = $(addprefix $(TMPDIR),$(notdir $(src_camera:.ica=.bin)))

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバート処理

do-build:	make_vpath subdir $(obj_imd) $(obj_ica) $(obj_ita) $(obj_ima) $(obj_itp) $(obj_iva) $(obj_bin)

make_vpath:
	@echo $(src_ica1)
	ruby ../conv/make_vpath.rb

#vpath設定
-include make_vpath

# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .nsbmd .imd .nsbca .ica .nsbta .ita .nsbma .ima .nsbtp .itp .nsbva .iva .bin

$(TMPDIR)%.nsbmd:%.imd
	@echo Model Data Convert: $< → $(addprefix $(TMPDIR),$(notdir $@))
	g3dcvtr $< -o $(addprefix $(TMPDIR),$(notdir $@))

$(TMPDIR)%.nsbca:%.ica
	g3dcvtr $< -o $(addprefix $(TMPDIR),$(notdir $@))

$(TMPDIR)%.nsbta:%.ita
	g3dcvtr $< -o $(addprefix $(TMPDIR),$(notdir $@))

$(TMPDIR)%.nsbma:%.ima
	g3dcvtr $< -o $(addprefix $(TMPDIR),$(notdir $@))

$(TMPDIR)%.nsbtp:%.itp
	g3dcvtr $< -o $(addprefix $(TMPDIR),$(notdir $@))

$(TMPDIR)%.nsbva:%.iva
	g3dcvtr $< -o $(addprefix $(TMPDIR),$(notdir $@))

$(TMPDIR)%.bin:%.ica
	ruby $(ICACONV) -rt $< $(addprefix $(TMPDIR),$(notdir $*.bin))

else

do-build:	subdir

endif

subdir:
#	@echo $(obj_imd)
	@$(MAKE_SUBDIR)



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f make_vpath
	rm -f $(TMPDIR)/*.nsb*
	rm -f $(TMPDIR)/*.bin
endif



