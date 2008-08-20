
include	$(PROJECT_RSCDIR)/macro_define

TARGET_DIR	= $(PROJECT_ARCDIR)/test_graphic

.PHONY:	do-build clean install clean-arc

do-build:	install

install:
	$(COPY) *.narc $(TARGET_DIR)
	$(COPY) *.naix $(TARGET_DIR)

clean:	clean-arc


clean-arc:
	rm -f $(TARGET_DIR)/*.narc
	rm -f $(TARGET_DIR)/*.naix

