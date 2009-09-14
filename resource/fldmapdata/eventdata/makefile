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
USERS	=	 tamada matsumiya suginaka_katsunori nakatsui masafumi_saitou suginaka_katsunori mai_ando murakami_naoto hozumi_yukiko kagaya iwasawa obata_toshihiro

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= anime_files

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define



TMPDIR	= tmp
SRCDATA = $(wildcard data/*.mev)
TGTDATA = $(basename $(notdir $(SRCDATA)))
CDAT = $(addprefix $(TMPDIR)/,$(addsuffix .cdat,$(TGTDATA)))
HEADER = $(CDAT:.cdat=.h)

TARGET = eventdata_table.cdat

.PHONY:	do-build clean test

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGET)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから
$(TARGET):	$(CDAT)
	ruby make_table.rb $(CDAT) > $(TARGET)

$(TMPDIR)/%.cdat: data/%.mev
	ruby make_event.rb $<
endif


#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作、ここから
	-rm event_*.cdat event_*.h
	-rm $(CDAT) $(HEADER) $(TARGET)
endif

test:
	@echo $(CDAT)

