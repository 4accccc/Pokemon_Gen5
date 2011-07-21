
include $(PROJECT_RSCDIR)\macro_define

#複雑な依存関係があるため、順序を入れ替えないこと。
#zonetable/zoneindexが先頭、
#zonetableが末尾、という順番は変更してはいけない。

SUBDIRS	=	gym_denki \
					gym_normal \
					gym_fly \
					gym_anti \
					gym_insect \
					gym_ground \
					gym_ice \
					gym_dragon \
					h01 \
					h03 \
					gate \
					d06 \
					league_front \
					sppoke \
					d12 \
					zoroa \
					legend \
					r04d03 \
					t01 \
					infinity \
					wbt \
					gym_poison \
					gym_water \
					fourkings_evil \
					fourkings_esp \
					fourkings_ghost \
					fourkings_fight \
					badge_gate \
					hide_ship_engine \
					hide_ship_deck

.PHONY:	do-build clean

do-build:
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
