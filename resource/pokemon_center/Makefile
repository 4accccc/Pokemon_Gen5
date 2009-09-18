
include $(PROJECT_RSCDIR)\macro_define

SUBDIRS	=	recovery_anime

do-build: install

	@$(MAKE_SUBDIR)

install: $(TARGETS)

clean:
	@$(MAKE_SUBDIR)

	-rm -f $(TARGETS)
