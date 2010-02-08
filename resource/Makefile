#------------------------------------------------------------------------------
#
#
#
#------------------------------------------------------------------------------
RESOURCE_MACRO	= $(PROJECT_RSCDIR)\macro_define

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	環境が違うところでMakeできないようにするためのおまじない
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#現在のディレクトリ（cygwinのパスから変換する）
NOWDIR	=	$(shell pwd|tr [A-Z] [a-z] |sed -e 's/\/cygdrive\///')/
#環境変数が想定しているディレクトリから、：を取り除いたもの
TGTDIR	= $(shell echo $(PROJECT_RSCDIR)|tr [A-Z] [a-z]|sed -e 's/://')
ifneq ($(NOWDIR),$(TGTDIR))
$(warning コンパイル環境を確認してください)
$(error $(NOWDIR) != $(TGTDIR) )
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


SUBDIRS	=	wmi \
			tokusei\
			waza_tbl\
			itemconv\
			personal\
			trdata \
			trmsg \
			sound \
			bag \
			test_layout \
			test_graphic\
			place_name \
			field_weather \
			field_weather_light \
			field_light \
			zukan_data \
			message\
			fldmmdl \
			encount \
			fld3d_ci \
			batt_bg_tbl\
			fldmapdata\
			pokegra\
			battle\
			net_err\
			font\
			itemicon\
			title\
			snd_strm\
			mystery\
			startmenu \
			ircbattle \
			wificlub \
			wifi_login \
			wifi_union_obj \
			wifi_2dchar \
			wifi_leading_obj \
			trainer_case \
			yn_touch \
			mysign \
			pms \
			pms_gra \
			compatible \
			musical \
			infowin \
			c_gear \
			cg_comm \
			gsync \
			gtsnego \
			bmpoam \
			shop \
			code_input \
			wazatype_icon \
			wifi_note \
			record \
			selbox \
			particle/wazaeffect \
			others \
			wazaeffect \
			field_menu \
			pokelist \
			wifi_earth \
			p_status \
			app_menu_common \
			irc_ranking \
			iss \
			townmap \
			unionroom \
			winframe \
			trgra \
			irc_compatible \
			worldtrade \
			calender \
			pokefoot \
			debug \
			config \
			narc_cpy \
			pokeicon \
			zukan_data_old \
            b_bag \
            b_plist \
		  regulation \
			mictest \
			pokemon_center \
			namein \
            box \
			battle_championship \
			trade \
			waza_oshie \
			sodateya \
			shopdata \
			fld_shortcut_menu \
			wifi_match \
			trtype_bgm \
			local_tvt \
			palace \
            op_demo \
			battle_recorder \
			monolith \
			fourkings_scene \
			multiboot \
			field_wfbc_data/after \
			mail \
			season_display \
			zukan_gra \
			demo3d \
			fld_trade \
			intro \
			regulation_card \
			comm_tvt \
			guru2 \
			guru2_2d \
			oekaki \
			beacon_status \
			tr_ai \
			enceff_imd \
			pl_boat \
			enc_ci_face \
			comm_btl_demo \
			shinka_demo \
			face_up \
			batt_rec_gra \
			sea_temple \
			egg_demo \
			research_radar \
			kokuren \
			pokevoice

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


