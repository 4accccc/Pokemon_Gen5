#======================================================================
#	others makefile
#	特に条件、分類の無いデータ類をまとめてアーカイブ化
#======================================================================
include	$(PROJECT_RSCDIR)/macro_define

TARGET_COPY_DIR = $(PROJECT_ARCDIR)

TARGET_NARC_NAME = others.narc
TARGET_NAIX_NAME = $(TARGET_NARC_NAME:.narc=.naix)

#アーカイブで纏めるファイル郡
TARGET_FILES = \
	nfont.NCGR \
	nfont.NCLR
	
#---------------------------------------------------------------
#	ターゲット
#---------------------------------------------------------------
.PHONY:	do-build clean

#---------------------------------------------------------------
#	build
#---------------------------------------------------------------
do-build: $(TARGET_COPY_DIR)/$(TARGET_NARC_NAME)
	
$(TARGET_COPY_DIR)/$(TARGET_NARC_NAME): $(TARGET_NARC_NAME)
	$(COPY) $(TARGET_NARC_NAME) $(TARGET_COPY_DIR)
	$(COPY) $(TARGET_NAIX_NAME) $(TARGET_COPY_DIR)
	
$(TARGET_NARC_NAME): $(TARGET_FILES)
	nnsarc -i -c -l -n $(TARGET_NARC_NAME) $(TARGET_FILES) > arc_result.txt
	$(NAIXCUT) $(TARGET_NAIX_NAME)
	
clean:
	rm -f $(TARGET_COPY_DIR)/$(TARGET_NARC_NAME)
	rm -f $(TARGET_COPY_DIR)/$(TARGET_NAIX_NAME)
	rm -f $(TARGET_NARC_NAME)
	rm -f $(TARGET_NAIX_NAME)
