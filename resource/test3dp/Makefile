##------------------------------------------------------------------------------
##
##	サンプルマップコンバート用Makefile
##
##	2008.09.02	ohno_katsumi@gamefreak.co.jp
##------------------------------------------------------------------------------
#
##------------------------------------------------------------------------------
##※コンバート作業が必要なユーザーの名前を記述する
##------------------------------------------------------------------------------
USERS	=	watanabe ohno
#
##------------------------------------------------------------------------------
##※ここに作成するnarc名を書く
##------------------------------------------------------------------------------
#TARGETNAME	= test3dp
#NARCNAME	= $(TARGETNAME).narc
#NAIXNAME	= $(TARGETNAME).naix
#
##------------------------------------------------------------------------------
##※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
##------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/test_graphic
#
#
##共通ルールファイルのインクルード
#include $(PROJECT_RSCDIR)\macro_define
#
#
##依存対象が複数ある
#MAPNSBMDS	:= $(wildcard *.nsbmd)
#MAP3DPPACKS = $(subst .nsbmd,.3dppack,$(MAPNSBMDS))
#
#ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#.SUFFIXES:	 .3dppack .nsbmd
#
##imdデータからの生成ルール
#.nsbmd.3dppack:
#	echo $<
##	$(BINLINKER) $< $<
#endif
#
#
.PHONY:	do-build clean
#
##------------------------------------------------------------------------------
##	make do-build ルール
##------------------------------------------------------------------------------
#do-build:	$(TARGETDIR)/$(NARCNAME)
#
#subdir:
#	@$(MAKE_SUBDIR)
#
#$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
#	$(COPY)	$(NARCNAME) $(TARGETDIR)
#	$(COPY) $(NAIXNAME) $(TARGETDIR)
#
#$(NARCNAME):	$(MAPMDLS) $(wildcard *.bin)  $(wildcard *.3dmd)
#	nnsarc -c -l -n -i $(NARCNAME) -S data.list > error
#	$(NAIXCUT) $(NAIXNAME)
#
##------------------------------------------------------------------------------
##	make cleanルール
##------------------------------------------------------------------------------



#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define


# ただ今仮作成 k.ohno
do-build:
	ruby ../../tools/binlinker.rb map_a1.nsbmd map_a1.nsbtx NONE map_a1.bin map_a1.3dmd map_a1.3dppack
	nnsarc -c -l -n -i test3dp -S data.list
	ruby ../../tools/naixcutstamp.rb test3dp.naix
	$(COPY)	test3dp.naix $(TARGETDIR)
	$(COPY)	test3dp.narc $(TARGETDIR)

clean:
	-rm -f $(TARGETDIR)/test3dp.naix
	-rm -f $(TARGETDIR)/test3dp.narc
	-rm -f test3dp.naix
	-rm -f test3dp.narc
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	-rm -f *.3dppack
endif


