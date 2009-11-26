
include $(PROJECT_RSCDIR)\macro_define

#複雑な依存関係があるため、順序を入れ替えないこと。
#zonetable/zoneindexが先頭、
#zonetableが末尾、という順番は変更してはいけない。

SUBDIRS	=	zonetable/zoneindex \
					flagwork \
					area_data \
					land_data \
					camera_data \
					gimmick \
					map_matrix \
					build_model \
					mm_list \
					eventdata/groupdata \
					eventdata/define \
					script \
					effect \
					rail_data \
					zonefog_table \
					zonelight_table \
					fld_gimmick_assign \
					gimmick \
					camera_scroll \
					pushrock \
					eventdata/binary \
					zonetable/fieldskill_mapeffect \
					zonetable
					

.PHONY:	do-build clean

do-build:
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
