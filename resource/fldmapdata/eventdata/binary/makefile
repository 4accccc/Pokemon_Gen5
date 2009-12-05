#============================================================================
#
#		イベントデータコンバート用Makefile
#
#
#
#
#		2009.05.25	tamada GAME FREAK inc.
#
#============================================================================

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include ../../event_script_users_def

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS =

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = eventdata

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)/fieldmap/

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define


ARCFIELDDIR = $(PROJECT_ARCDIR)/fieldmap/
SRCFIELDDIR = $(PROJECT_PROGDIR)/src/field/
HEADERFIELDDIR = $(PROJECT_PROGDIR)/include/field/
SCRIPTDIR = ../../script/
FLAGWORKDIR = ../../flagwork/
TOTALHEADER = ../define/total_header.h


TMPDIR	= ../tmp
DATADIR	= ../data
SCRIPTDIR	= ../../script
SRCDATA = $(wildcard $(DATADIR)/*.mev)
TGTDATA = $(basename $(notdir $(SRCDATA)))
BINDAT = $(addprefix $(TMPDIR)/,$(addsuffix .bin,$(TGTDATA)))

#UNIQ_SCRIPT
include  ../../script/uniq_script.list
#SCRBASE = $(basename $(notdir $(UNIQ_SCRIPTFILES)))
SCRBASE = $(basename $(notdir $(UNIQ_TOP_EVENT_SCRFILES)))
UNIQ_SCRIPTHEADER = $(addprefix $(SCRIPTDIR)/,$(addsuffix _def.h,$(SCRBASE)))

LIST_FILE 	= list.txt


.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

# コピー先のnarcが古い場合実行する
$(targetname).narc: $(BINDAT)
	$(COPY) $(TMPDIR)/*.bin ./
	ls *.bin > $(LIST_FILE)
	echo dummy_data >> $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i -S $(LIST_FILE) $(targetname) > arc_result.txt
	-rm *.bin
	-rm $(LIST_FILE)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから

#binary化の依存関係ファイルを取得
-include ../define/binary_depend_file

endif


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	@echo clean up eventdata/define ...
	-rm -f $(TARGET_DIR)/$(targetname).narc
	-rm -f $(TARGET_DIR)/$(targetname).naix
	-rm -f $(targetname).narc
	-rm -f $(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから
	@-rm -f $(TMPDIR)/*.bin
endif

