#------------------------------------------------------------------------------
#
#
#
#------------------------------------------------------------------------------
RESOURCE_MACRO	= $(PROJECT_RSCDIR)\macro_define


#SUBDIRS	= wmi test_graphic message fldmapdata test3dp test3dp2 pokegra battle personal waza_tbl net_err font itemconv itemicon title snd_strm wifi_lobby_minigame mystery startmenu
SUBDIRS	=	wmi \
			test_graphic\
			message\
			fldmapdata\
			test3dp\
			test3dp2\
			pokegra\
			battle\
			personal\
			waza_tbl\
			net_err\
			font\
			itemconv\
			itemicon\
			title\
			snd_strm\
			wifi_lobby_minigame\
			mystery\
			startmenu \
			palace

include $(RESOURCE_MACRO)

.PHONY:	do-build	clean
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
do-build:
	@$(MAKE_SUBDIR)
	@$(ECHO_CURDIR)


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	@$(ECHO_CURDIR)


