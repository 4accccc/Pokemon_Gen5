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
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define



TMPDIR	= ../tmp
DATADIR	= ../data
SRCDATA = $(wildcard $(DATADIR)/*.mev)
TGTDATA = $(basename $(notdir $(SRCDATA)))
HEADER = $(addprefix $(TMPDIR)/,$(addsuffix .h,$(TGTDATA)))
TMPDATA = $(addprefix $(TMPDIR)/,$(addsuffix .tmp,$(TGTDATA)))

TARGET = total_header.h
TARGET2 = binary_depend_file

.PHONY:	do-build clean test

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGET) $(TARGET2)

$(TARGET):	$(HEADER)
	cat $(TMPDIR)/*.h > $(TARGET)

$(TARGET2):	$(TMPDATA)
	cat $(TMPDIR)/*.tmp > $(TARGET2)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから

$(TMPDIR)/%.h: $(DATADIR)/%.mev
	ruby make_event.rb $<

$(TMPDIR)/%.tmp: $(DATADIR)/%.mev
	ruby make_event.rb $<
endif


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
	@echo clean up eventdata/define ...
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから
	@-rm -f event_*.h event_*.tmp
	@-rm -f $(HEADER)
endif
	@-rm -f $(TARGET) 
	@-rm -f $(TARGET2)
	@-rm -f ../tmp/*.cdat
	@-rm -f ../tmp/*_make_list_depend*

test:
	@echo $(HEADER)

