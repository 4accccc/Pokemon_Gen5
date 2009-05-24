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
EVD_H = $(addsuffix evd.h,$(TGTDATA))
#EVD_CDAT = $(addsuffix evd.cdat,$(TGTDATA))
CDAT = $(addsuffix .cdat,$(TGTDATA))

.PHONY:	do-build clean test

do-build:	$(CDAT)


%.cdat: data/%.mev
	ruby make_event.rb $<

clean:
	-rm $(EVD_H) $(EVD_CDAT)

test:
	@echo $(TGTDATA) $(EVD_H) $(EVD_CDAT)
	ruby make_event.rb data\t01_sample.mev

