
SRC_ROOT_DIR	= src
DST_ROOT_DIR	= dst
RESOURCE_LIST	= resource.list
DEPEND_LIST	= depend.list
TARGET_LIST	= target.list

ARCHIVE_NAME_BODY	= message
ARCHIVE_DATA_NAME	= $(ARCHIVE_NAME_BODY).narc
ARCHIVE_HEADER_NAME	= $(ARCHIVE_NAME_BODY).naix
ARCHIVE_DATA_PATH	= $(PROJECT_ARCDIR)$(ARCHIVE_DATA_NAME)
ARCHIVE_HEADER_PATH	= $(PROJECT_ARCDIR)$(ARCHIVE_HEADER_NAME)

ARCHIVE_NAME_BODY_DL	= message_dl
ARCHIVE_DATA_NAME_DL	= $(ARCHIVE_NAME_BODY_DL).narc
ARCHIVE_HEADER_NAME_DL	= $(ARCHIVE_NAME_BODY_DL).naix
ARCHIVE_DATA_PATH_DL	= $(PROJECT_ROOT)multiboot/arc/$(ARCHIVE_DATA_NAME_DL)
ARCHIVE_HEADER_PATH_DL	= $(PROJECT_ROOT)multiboot/arc/$(ARCHIVE_HEADER_NAME_DL)

#=========================================================
#	プロジェクト言語コード -> 変換対象言語指定
#=========================================================
VERSION_FILE_PATH = $(PROJECT_PROGDIR)version
include $(VERSION_FILE_PATH)

CONVERT_TARGET_LANGS = JPN JPN_KANJI

# この辺りで PM_LANG を参照しつつ CONVERT_TARGET_LANGS を書き換えることを想定…

#=========================================================
#	動作部分
#=========================================================
.SUFFIXES: .dat .gmm

no-rule:
	echo $(TARGETS)
	$(MAKE)	depend
	$(MAKE)	datfiles
	$(MAKE)	$(ARCHIVE_DATA_PATH)
	$(MAKE)	$(ARCHIVE_DATA_PATH_DL)


#=========================================================
#	depend & target list を生成
#=========================================================
depend:
	$(shell find src|grep gmm$$ > $(RESOURCE_LIST)) 
	perl  makedepend.pl $(SRC_ROOT_DIR) $(DST_ROOT_DIR) $(RESOURCE_LIST) $(DEPEND_LIST) $(TARGET_LIST) $(CONVERT_TARGET_LANGS)
	-rm $(RESOURCE_LIST)


#=========================================================
#	gmm -> dat
#=========================================================
HEADER_COPY_DIR	= $(PROJECT_PROGDIR)/include/msg/

-include $(TARGET_LIST)
-include $(DEPEND_LIST)

datfiles: $(TARGETS)
	cp -u $(DST_ROOT_DIR)/*.h $(HEADER_COPY_DIR)


#=========================================================
#	全dat -> narc
#=========================================================
$(ARCHIVE_DATA_PATH) : $(wildcard $(DST_ROOT_DIR)/*.dat)
	nnsarc -c -l -n -i $(ARCHIVE_DATA_NAME) $(DST_ROOT_DIR)/*.dat
	cp -u $(ARCHIVE_DATA_NAME) $(ARCHIVE_DATA_PATH)
	perl naixcopy.pl $(ARCHIVE_HEADER_NAME) $(ARCHIVE_HEADER_PATH)

$(ARCHIVE_DATA_PATH_DL) : $(DST_ROOT_DIR)/d_dlplay.dat
	nnsarc -c -l -n -i $(ARCHIVE_DATA_NAME_DL) $(DST_ROOT_DIR)/d_dlplay.dat
	cp -u $(ARCHIVE_DATA_NAME_DL) $(ARCHIVE_DATA_PATH_DL)
	perl naixcopy.pl $(ARCHIVE_HEADER_NAME_DL) $(ARCHIVE_HEADER_PATH_DL)


#=========================================================
#	cleanup
#=========================================================
clean:
	-rm $(DST_ROOT_DIR)/*.dat
	-rm $(DST_ROOT_DIR)/*.h
	-rm $(DEPEND_LIST)
	-rm $(TARGET_LIST)

