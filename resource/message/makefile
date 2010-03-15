
SRC_ROOT_DIR	= src
DST_ROOT_DIR	= dst
DST_SCRIPT_DIR	= $(DST_ROOT_DIR)/script
RESOURCE_LIST	= resource.list
DEPEND_LIST	= depend.list
TARGET_LIST	= target.list

BTLSET		= src\battle\btl_set.gmm
BTLSET_SRC	= resource\btl_set_src.gmm


ARCHIVE_NAME_BODY	= message
ARCHIVE_DATA_NAME	= $(ARCHIVE_NAME_BODY).narc
ARCHIVE_HEADER_NAME	= $(ARCHIVE_NAME_BODY).naix
ARCHIVE_DATA_PATH	= $(PROJECT_ARCDIR)$(ARCHIVE_DATA_NAME)
ARCHIVE_HEADER_PATH	= $(PROJECT_ARCDIR)$(ARCHIVE_HEADER_NAME)

SCRIPT_ARCHIVE_NAME_BODY	= script_message
SCRIPT_ARCHIVE_DATA_NAME	= $(SCRIPT_ARCHIVE_NAME_BODY).narc
SCRIPT_ARCHIVE_HEADER_NAME	= $(SCRIPT_ARCHIVE_NAME_BODY).naix
SCRIPT_ARCHIVE_DATA_PATH	= $(PROJECT_ARCDIR)$(SCRIPT_ARCHIVE_DATA_NAME)
SCRIPT_ARCHIVE_HEADER_PATH	= $(PROJECT_ARCDIR)$(SCRIPT_ARCHIVE_HEADER_NAME)

#マルチブート子機(ポケシフター
ARCHIVE_NAME_BODY_DL	= message_dl
ARCHIVE_DATA_NAME_DL	= $(ARCHIVE_NAME_BODY_DL).narc
ARCHIVE_HEADER_NAME_DL	= $(ARCHIVE_NAME_BODY_DL).naix
ARCHIVE_DATA_PATH_DL	= $(PROJECT_ROOT)multiboot/arc/$(ARCHIVE_DATA_NAME_DL)
ARCHIVE_HEADER_PATH_DL	= $(PROJECT_ROOT)multiboot/arc/$(ARCHIVE_HEADER_NAME_DL)

#マルチブート子機(映画
ARCHIVE_NAME_BODY_DL_MOVIE	= message_dl_movie
ARCHIVE_DATA_NAME_DL_MOVIE	= $(ARCHIVE_NAME_BODY_DL_MOVIE).narc
ARCHIVE_HEADER_NAME_DL_MOVIE	= $(ARCHIVE_NAME_BODY_DL_MOVIE).naix
ARCHIVE_DATA_PATH_DL_MOVIE	= $(PROJECT_ROOT)multiboot2/arc/$(ARCHIVE_DATA_NAME_DL_MOVIE)
ARCHIVE_HEADER_PATH_DL_MOVIE	= $(PROJECT_ROOT)multiboot2/arc/$(ARCHIVE_HEADER_NAME_DL_MOVIE)

#=========================================================
#	プロジェクト言語コード -> 変換対象言語指定
#=========================================================
VERSION_FILE_PATH = $(PROJECT_PROGDIR)version
include $(VERSION_FILE_PATH)

CONVERT_TARGET_LANGS = JPN JPN_KANJI


#=========================================================
#	動作部分
#=========================================================
.SUFFIXES: .dat .gmm

no-rule:
	echo $(TARGETS)
	$(MAKE)	$(BTLSET)
	$(MAKE)	depend
	$(MAKE)	datfiles
	$(MAKE)	$(ARCHIVE_DATA_PATH)
	$(MAKE)	$(SCRIPT_ARCHIVE_DATA_PATH)
	$(MAKE)	$(ARCHIVE_DATA_PATH_DL)
	$(MAKE)	$(ARCHIVE_DATA_PATH_DL_MOVIE)

#=========================================================
#	depend & target list を生成
#=========================================================
$(BTLSET): $(BTLSET_SRC)
	perl btlset.pl $(BTLSET_SRC) $(BTLSET)

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
SCRIPT_HEADER_COPY_DIR	= $(HEADER_COPY_DIR)/script/

-include $(TARGET_LIST)
-include $(DEPEND_LIST)

datfiles: $(TARGETS)
	cp -u $(DST_ROOT_DIR)/*.h $(HEADER_COPY_DIR)
	cp -u $(DST_SCRIPT_DIR)/*.h $(SCRIPT_HEADER_COPY_DIR)

#=========================================================
#	全dat -> narc
#=========================================================
$(ARCHIVE_DATA_PATH) : $(wildcard $(DST_ROOT_DIR)/*.dat)
	nnsarc -c -l -n -i $(ARCHIVE_DATA_NAME) $(DST_ROOT_DIR)/*.dat
	cp -u $(ARCHIVE_DATA_NAME) $(ARCHIVE_DATA_PATH)
	perl naixcopy.pl $(ARCHIVE_HEADER_NAME) $(ARCHIVE_HEADER_PATH)

$(SCRIPT_ARCHIVE_DATA_PATH) : $(wildcard $(DST_SCRIPT_DIR)/*.dat)
	nnsarc -c -l -n -i $(SCRIPT_ARCHIVE_DATA_NAME) $(DST_SCRIPT_DIR)/*.dat
	cp -u $(SCRIPT_ARCHIVE_DATA_NAME) $(SCRIPT_ARCHIVE_DATA_PATH)
	perl naixcopy.pl $(SCRIPT_ARCHIVE_HEADER_NAME) $(SCRIPT_ARCHIVE_HEADER_PATH)

$(ARCHIVE_DATA_PATH_DL) : $(wildcard $(DST_ROOT_DIR)/*.dat)
	nnsarc -c -l -n -i $(ARCHIVE_DATA_NAME_DL) -S dlplay_list.dat
	cp -u $(ARCHIVE_DATA_NAME_DL) $(ARCHIVE_DATA_PATH_DL)
	perl naixcopy.pl $(ARCHIVE_HEADER_NAME_DL) $(ARCHIVE_HEADER_PATH_DL)

$(ARCHIVE_DATA_PATH_DL_MOVIE) : $(wildcard $(DST_ROOT_DIR)/*.dat)
	nnsarc -c -l -n -i $(ARCHIVE_DATA_NAME_DL_MOVIE) -S dlplay_movie_list.dat
	cp -u $(ARCHIVE_DATA_NAME_DL_MOVIE) $(ARCHIVE_DATA_PATH_DL_MOVIE)
	perl naixcopy.pl $(ARCHIVE_HEADER_NAME_DL_MOVIE) $(ARCHIVE_HEADER_PATH_DL_MOVIE)


#=========================================================
#	cleanup
#=========================================================
clean:
	-rm $(DST_ROOT_DIR)/*.dat
	-rm $(DST_ROOT_DIR)/*.h
	-rm $(DST_SCRIPT_DIR)/*.dat
	-rm $(DST_SCRIPT_DIR)/*.h
	-rm $(HEADER_COPY_DIR)/*.h
	-rm $(SCRIPT_HEADER_COPY_DIR)/*.h
	-rm $(DEPEND_LIST)
	-rm $(TARGET_LIST)

