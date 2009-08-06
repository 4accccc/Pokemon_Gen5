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
USERS	=	tamada matsumiya suginaka_katsunori

#----------------------------------------------------------------------------
#※ここに作成するファイル名を書く
#----------------------------------------------------------------------------
FLAGTARGET	= flag_define.h
WORKTARGET	= work_define.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
#TARGETDIR	= $(PROJECT_ARCDIR)fieldmap
TARGETDIR	= ../script


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

do-build: $(TARGETDIR)/$(FLAGTARGET) $(TARGETDIR)/$(WORKTARGET)

$(TARGETDIR)/$(FLAGTARGET):	$(FLAGTARGET)
	$(COPY) $(FLAGTARGET) $(TARGETDIR)/$(FLAGTARGET)

$(TARGETDIR)/$(WORKTARGET):	$(WORKTARGET)
	$(COPY) $(WORKTARGET) $(TARGETDIR)/$(WORKTARGET)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる


$(FLAGTARGET): flags.xls $(FLAGFILES)
	@echo フラグ生成開始...
	ruby scripts/flag.rb flags.xls flag_define.h
	@echo フラグ生成終了！！

$(WORKTARGET):	works.xls $(WORKFILES)
	@echo ワーク生成開始
	ruby scripts/flag.rb works.xls work_define.h
	@echo ワーク生成終了！！
endif

clean:
	@rm -f $(TARGETDIR)/$(FLAGTARGET)
	@rm -f $(TARGETDIR)/$(WORKTARGET)
	@
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	@rm -f $(FLAGTARGET) $(WORKTARGET)
endif


