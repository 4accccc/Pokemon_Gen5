#------------------------------------------------------------------------------
#  3Dリソースのコンバート > １つ下の階層にコピー
#  2009.12.03
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

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

#ICAコンバータ
ICACONV = ..\..\..\tools\ica\ica_converter.rb

# 自動でリソースを集める
src_imd:=$(wildcard *.imd)
src_ica:=$(wildcard *.ica)
src_ita:=$(wildcard *.ita)
src_ima:=$(wildcard *.ima)
src_itp:=$(wildcard *.itp)
src_iva:=$(wildcard *.iva)

# リソースが吐き出すファイルを自動でつくる
obj_imd = $(subst .imd,.nsbmd,$(src_imd))
obj_ica = $(subst .ica,.nsbca,$(src_ica))
obj_ita = $(subst .ita,.nsbta,$(src_ita))
obj_ima = $(subst .ima,.nsbma,$(src_ima))
obj_itp = $(subst .itp,.nsbtp,$(src_itp))
obj_iva = $(subst .iva,.nsbva,$(src_iva))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .nsbmd .imd .nsbca .ica .nsbta .ita .nsbma .ima .nsbtp .itp .nsbva .iva

.imd.nsbmd:
	g3dcvtr $<
#	CP $*.nsbmd ../

.ica.nsbca:
	g3dcvtr $< 
#	CP $*.nsbca ../
	# ICAカメラ とりあえず全て生成
	ruby $(ICACONV) -rt $< $*.bin
#	CP $*.bin ../

.ita.nsbta:
	g3dcvtr $<
#	CP $*.nsbta ../

.ima.nsbma:
	g3dcvtr $<
#	CP $*.nsbma ../

.itp.nsbtp:
	g3dcvtr $<
#	CP $*.nsbtp ../

.iva.nsbva:
	g3dcvtr $<
#	CP $*.nsbva ../

endif

.PHONY:	do-build clean subdir


#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバート処理

do-build:	subdir $(obj_imd) $(obj_ica) $(obj_ita) $(obj_ima) $(obj_itp) $(obj_iva)

else

do-build:	subdir

endif

subdir:
	@$(MAKE_SUBDIR)



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.nsb*
	rm -f *.bin
endif



