
include $(PROJECT_RSCDIR)\macro_define

TARGETDIR = $(PROJECT_ARCDIR)


NARCFILE		= $(wildcard *.narc)
NAIXFILE		= $(wildcard *.naix)
HFILE				= $(wildcard *.h)
NARCTARGET 	= $(addprefix $(TARGETDIR)/,$(NARCFILE))
NAIXTARGET 	= $(addprefix $(TARGETDIR)/,$(NAIXFILE))
HTARGET			= $(addprefix $(TARGETDIR)/,$(HFILE))

.PHONY:	do-build clean

do-build:	$(NARCTARGET) $(NAIXTARGET) $(HTARGET)


$(TARGETDIR)/%.narc:	%.narc
	$(COPY) $^ $@

$(TARGETDIR)/%.naix:	%.naix
	$(COPY) $^ $@

$(TARGETDIR)/%.h:	%.h
	$(COPY) $^ $@

clean:
