#----------------------------------------------------------------------------
# Project:  Cソース->バイナリー
# File:     Makefile
#
# Copyright 2005 GameFreak.inc  All rights reserved.
#
#	2008.11.10	tamada	WB環境に移植開始
#	2009.07.25	tamada	ヘッダ生成部とデータ生成部を分離
#
#----------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	masafumi_saitou mizuguchi_mai mai_ando tamada kagaya tomoya_takahashi

#----------------------------------------------------------------------------
#※ここに作成するファイル名を書く
#----------------------------------------------------------------------------
FLAGTARGET	= flag_define.h
WORKTARGET	= work_define.h

DEBUG_FLAG_LIST = wkdir/debug_flag.list
DEBUG_WORK_LIST = wkdir/debug_work.list
DEBUG_NARC_LIST = wkdir/debug_narc.list
DEBUG_NARC	= debug_symbol.narc
DEBUG_NAIX	= debug_symbol.naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap


#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)macro_define


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
FLAGFILES	= $(wildcard *_flag.xls)
WORKFILES	= $(wildcard *_work.xls)

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------

XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb

#----------------------------------------------------------------------------
#
#	ルール定義
#
#----------------------------------------------------------------------------
.PHONY:	do-build clean 

ifeq	($(PM_DEBUG),yes)
do-build: $(FLAGTARGET) $(WORKTARGET) $(TARGETDIR)/$(DEBUG_NARC)
else
do-build: $(FLAGTARGET) $(WORKTARGET)
endif


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

$(FLAGTARGET) $(DEBUG_FLAG_LIST) : flags.xls $(FLAGFILES)
	@echo フラグ生成開始...
	ruby scripts/flag.rb flags.xls $(FLAGTARGET) $(DEBUG_FLAG_LIST)
	@echo フラグ生成終了！！

$(WORKTARGET) $(DEBUG_WORK_LIST) :	works.xls $(WORKFILES)
	@echo ワーク生成開始
	ruby scripts/flag.rb works.xls $(WORKTARGET) $(DEBUG_WORK_LIST)
	@echo ワーク生成終了！！

endif	#CONVERTUSER

ifeq	($(PM_DEBUG),yes)
#デバッグ用アーカイブ生成
$(DEBUG_NARC): $(DEBUG_WORK_LIST) $(DEBUG_FLAG_LIST)
	@echo デバッグアーカイブ生成
	@cat $(DEBUG_WORK_LIST) $(DEBUG_FLAG_LIST) > $(DEBUG_NARC_LIST)
	@nnsarc -l -i -c -n -S $(DEBUG_NARC_LIST) $(DEBUG_NARC)
	@$(NAIXCUT) $(DEBUG_NAIX)

$(TARGETDIR)/$(DEBUG_NARC):	$(DEBUG_NARC)
	@echo デバッグアーカイブ　コピー
	@$(COPY) $(DEBUG_NARC) $(TARGETDIR)
	@$(COPY) $(DEBUG_NAIX) $(TARGETDIR)
endif	#PM_DEBUG

clean:
ifeq	($(PM_DEBUG),yes)
	rm -f $(TARGETDIR)/$(DEBUG_NARC)
	rm -f $(TARGETDIR)/$(DEBUG_NAIX)
endif
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	rm -f $(DEBUG_NARC) $(DEBUG_NAIX)
	rm -f wkdir/*.bin
	rm -f wkdir/*.list
	@rm -f $(FLAGTARGET) $(WORKTARGET)
endif


test:
	@echo $(DEBUG_WORKBINS)
