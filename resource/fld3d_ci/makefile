
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= fld3d_ci
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix
HEADNAME	= $(targetname)_def.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
#ifeq ($(CONVERTUSER),true)
#SUBDIRS	= ../../../../pokemon_wb_doc/particle/battleeffect
#endif

LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix
#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt
endif


#コンバート対象のファイルツリーを生成

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(targetname).narc
	

#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
# コピー先のnarcが古い場合実行する
$(targetname).narc:
	@g3dcvtr test_cart.imd
	nnsarc -cl -l -n -i $(NARCNAME) -S fld_spa.lst > arc_rsult.txt
endif



#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------


