
include $(PROJECT_RSCDIR)\macro_define

#複雑な依存関係があるため、順序を入れ替えないこと。
#zonetable/zoneindexが先頭、
#zonetableが末尾、という順番は変更してはいけない。

SUBDIRS	=	zonetable/zoneindex \
					flagwork \
					area_data \
					land_data \
					camera_data \
					map_matrix \
					build_model \
					land_data_ex \
					mm_list \
					eventdata \
					script \
					effect \
					rail_data \
					scenearea \
					zonefog_table \
					zonelight_table \
					zonetable 

.PHONY:	do-build clean

do-build:
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
