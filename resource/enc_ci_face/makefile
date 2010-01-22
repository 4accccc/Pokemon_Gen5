
#------------------------------------------------------------------------------
#
#	フィールドパーティクルコンバート用Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	saitou omura

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= enc_ci_face
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

#ツールパス

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

ARC_TARGET = arc_target_data

include $(ARC_TARGET)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
#ifeq ($(CONVERTUSER),true)
#SUBDIRS	= ../../../../pokemon_wb_doc/particle/battleeffect
#endif

#LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix $(TARGETDIR)/$(targetname2).narc $(TARGETDIR)/$(targetname2).naix
#コンバート対象者のみ、中間ファイル削除
#ifeq	($(CONVERTUSER),true)	
#LDIRT_CLEAN += $(targetname).narc $(targetname).naix arc_result.txt $(targetname2).narc $(targetname2).naix arc_result2.txt
#endif


#コンバート対象のファイルツリーを生成

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(targetname).narc
	
clean:
	@rm -f $(TARGETDIR)/$(targetname).narc
	@rm -f $(TARGETDIR)/$(targetname).naix

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(targetname).narc
	@rm -f $(targetname).naix
	@rm -f data/*.bin
	@rm -f data/*.ncgr
	@rm -f data/*.nclr
	@rm -f data/*.NCLR
endif

#コピー実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(ARC_TARGET) $(ARC_TARGET_DATA)
	nnsarc -cl -l -n -i $(NARCNAME) -S arc_list.txt > arc_result.txt

data/%_lz.bin: data/%.ncgr
	ntrcomp -l 2 $<

#3Dアニメコンバート
data/%.ncgr: data/%.ncg
	@g2dcvtr $< -bg

data/%.nclr: data/%.ncl
	@g2dcvtr $<

endif

