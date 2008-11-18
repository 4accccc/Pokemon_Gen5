//============================================================================================
/**
 * @file	event_mapchange.c
 * @brief
 * @date	2008.11.04
 * @author	tamada GAME FREAK inc.
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/zonedata.h"
#include "field/fieldmap.h"
#include "field_data.h"

#include "event_mapchange.h"

//============================================================================================
//
//	イベント：ゲーム開始
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	GAMEDATA * gamedata;
	GAME_INIT_WORK * game_init_work;
}FIRST_MAPIN_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FirstMapIn(GMEVENT * event, int *seq, void *work)
{
	FIRST_MAPIN_WORK * fmw = work;
	GAMESYS_WORK * gsys = fmw->gsys;
	GAME_INIT_WORK * game_init_work = fmw->game_init_work;
	switch (*seq) {
	case 0:
		{
			PLAYER_WORK * mywork = GAMEDATA_GetMyPlayerWork(fmw->gamedata);
			ZONEID start_id;
			VecFx32 start_pos;
			start_id = game_init_work->mapid;
			PLAYERWORK_setZoneID(mywork, start_id);
			ZONEDATA_DEBUG_GetStartPos(start_id, &start_pos);
			PLAYERWORK_setPosition(mywork, &start_pos);
			PLAYERWORK_setDirection(mywork, 0);
		}
		(*seq)++;
		break;
	case 1:
#if 1		/* 暫定的にプロセス登録 */
		GAMESYSTEM_CallFieldProc(gsys);
#endif
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_SetFirstMapIn(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work)
{
	FIRST_MAPIN_WORK * fmw;
	GMEVENT * event;
	//event = GAMESYSTEM_EVENT_Set(gsys, EVENT_FirstMapIn, sizeof(FIRST_MAPIN_WORK));
	event = GMEVENT_Create(gsys, NULL, EVENT_FirstMapIn, sizeof(FIRST_MAPIN_WORK));
	fmw = GMEVENT_GetEventWork(event);
	fmw->gsys = gsys;
	fmw->gamedata = GAMESYSTEM_GetGameData(gsys);
	fmw->game_init_work = game_init_work;
	return event;
}

//============================================================================================
//
//	イベント：デバッグ用マップ切り替え
//	※マップIDをインクリメントしている。最大値になったら先頭に戻る
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	GAMEDATA * gamedata;
	FIELD_MAIN_WORK * fieldmap;
	ZONEID next_map;
}MAPCHANGE_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChange(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK * mcw = work;
	GAMESYS_WORK  * gsys = mcw->gsys;
	GAMEDATA * gamedata = mcw->gamedata;
	switch (*seq) {
	case 0:
		FIELDMAP_Close(mcw->fieldmap);
		(*seq)++;
		break;
	case 1:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		(*seq)++;
		break;
	case 2:
		{
			PLAYER_WORK * mywork = GAMEDATA_GetMyPlayerWork(mcw->gamedata);
			VecFx32 start_pos;
			PLAYERWORK_setZoneID(mywork, mcw->next_map);
			ZONEDATA_DEBUG_GetStartPos(mcw->next_map, &start_pos);
			PLAYERWORK_setPosition(mywork, &start_pos);
			PLAYERWORK_setDirection(mywork, 0);
		}
		GAMESYSTEM_CallFieldProc(gsys);
		(*seq)++;
		break;
	case 3:
		if (!GAMESYSTEM_IsProcExists(gsys)) break;

		return GMEVENT_RES_FINISH;

	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap)
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;
	//event = GAMESYSTEM_EVENT_Set(gsys, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
	mcw->gsys = gsys;
	mcw->fieldmap = fieldmap;
	mcw->gamedata = GAMESYSTEM_GetGameData(gsys);
	{
		PLAYER_WORK * myplayer = GAMEDATA_GetMyPlayerWork(mcw->gamedata);
		ZONEID next = PLAYERWORK_getZoneID(myplayer);
		next ++;
		if (next >= FIELDDATA_GetMapIDMax()) {
			next = 0;
		}
		mcw->next_map = next;
	}
	return event;
}
//============================================================================================
//
//	イベント：別画面呼び出し
//
//============================================================================================
extern const GFL_PROC_DATA TestProg1MainProcData;
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELD_MAIN_WORK * fieldmap;
}CHANGE_SAMPLE_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent(GMEVENT * event, int * seq, void * work)
{
	CHANGE_SAMPLE_WORK * csw = work;
	GAMESYS_WORK *gsys = csw->gsys;

	switch(*seq) {
	case 0:
		FIELDMAP_Close(csw->fieldmap);
		(*seq) ++;
		break;
	case 1:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		(*seq) ++;
		break;
	case 2:
		GAMESYSTEM_CallProc(gsys, NO_OVERLAY_ID, &TestProg1MainProcData, NULL);
		(*seq) ++;
		break;
	case 3:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		(*seq) ++;
		break;
	case 4:
		GAMESYSTEM_CallFieldProc(gsys);
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_FieldSample(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, GameChangeEvent, sizeof(CHANGE_SAMPLE_WORK));
	CHANGE_SAMPLE_WORK * csw = GMEVENT_GetEventWork(event);
	csw->gsys = gsys;
	csw->fieldmap = fieldmap;
	return event;
}
