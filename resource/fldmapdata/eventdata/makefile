#============================================================================
#
#		イベントデータコンバート用Makefile
#
#
#		2009.05.25	tamada GAME FREAK inc.
#
#
#
#============================================================================



SRCDATA = $(wildcard data/*.mev)
TGTDATA = $(basename $(notdir $(SRCDATA)))
CDAT = $(addprefix event_,$(addsuffix .cdat,$(TGTDATA)))
HEADER = $(CDAT:.cdat=.h)

TARGET = eventdata_table.cdat

.PHONY:	do-build clean test

do-build:	$(TARGET)

$(TARGET):	$(CDAT)
	ruby make_table.rb $(CDAT) > $(TARGET)

event_%.cdat: data/%.mev
	ruby make_event.rb $<

clean:
	-rm $(CDAT) $(HEADER) $(TARGET)

test:
	@echo $(CDAT)

