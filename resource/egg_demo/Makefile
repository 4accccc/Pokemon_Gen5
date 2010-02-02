
include $(PROJECT_RSCDIR)\macro_define

TARGETDIR = $(PROJECT_ARCDIR)


NARCFILE		= $(wildcard *.narc)
NAIXFILE		= $(wildcard *.naix)
NARCTARGET 	= $(addprefix $(TARGETDIR)/,$(NARCFILE))
NAIXTARGET 	= $(addprefix $(TARGETDIR)/,$(NAIXFILE))

.PHONY:	do-build clean

do-build:	$(NARCTARGET) $(NAIXTARGET)


$(TARGETDIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGETDIR)/%.naix:	%.naix
	$(COPY) $^ $@

clean:
