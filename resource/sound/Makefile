#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.09.02	ohno_katsumi@gamefreak.co.jp
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	ohno kageyama_shota yasumoto_shingo sato_hitomi ichinose


#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../message/src
TARGETFILENAME  = snd_test_name.gmm

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define


#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= bgm_info

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(TARGETFILENAME)

subdir:
	@$(MAKE_SUBDIR)

$(TARGETDIR)/$(TARGETFILENAME):	$(TARGETFILENAME)
	$(COPY) $(TARGETFILENAME) $(TARGETDIR)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

$(TARGETFILENAME) : wb_sound_data.sadl gmm_make.rb
	ruby gmm_make.rb ../message/template.gmm wb_sound_data.sadl $(TARGETFILENAME)

endif


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------

clean:
	rm -f $(TARGETDIR)/$(TARGETFILENAME)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(TARGETFILENAME)
endif

