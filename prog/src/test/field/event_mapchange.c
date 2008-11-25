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
#include "field/location.h"
#include "field/eventdata_sxy.h"

#include "event_mapchange.h"

static void MakeNextLocation(LOCATION * loc, u16 zone_id, s16 exit_id);
static void UpdateMapParams(GAMESYS_WORK * gsys, const LOCATION * new_loc);
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
	LOCATION new_loc;
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
		UpdateMapParams(gsys, &fmw->new_loc);
		(*seq)++;
		break;
	case 1:
		GAMESYSTEM_CallFieldProc(gsys);
		(*seq)++;
		break;
	case 2:
		if (!GAMESYSTEM_IsProcExists(gsys)) break;

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

	event = GMEVENT_Create(gsys, NULL, EVENT_FirstMapIn, sizeof(FIRST_MAPIN_WORK));
	fmw = GMEVENT_GetEventWork(event);
	fmw->gsys = gsys;
	fmw->gamedata = GAMESYSTEM_GetGameData(gsys);
	fmw->game_init_work = game_init_work;
	MakeNextLocation(&fmw->new_loc, game_init_work->mapid, 0);
	return event;
}

//============================================================================================
//
//	イベント：デバッグ用マップ切り替え
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	GAMEDATA * gamedata;
	FIELD_MAIN_WORK * fieldmap;
	LOCATION new_loc;
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
		//フィールドマップを終了する
		FIELDMAP_Close(mcw->fieldmap);
		(*seq)++;
		break;
	case 1:
		//フィールドマップを終了待ち
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		(*seq)++;
		break;
	case 2:
		//新しいマップID、初期位置をセット
		UpdateMapParams(gsys, &mcw->new_loc);
		//フィールドマップを開始リクエスト
		GAMESYSTEM_CallFieldProc(gsys);
		(*seq)++;
		break;
	case 3:
		//フィールドマップを開始待ち
		if (!GAMESYSTEM_IsProcExists(gsys)) break;

		return GMEVENT_RES_FINISH;

	}
	return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeMapPos(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos)
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
	mcw->gsys = gsys;
	mcw->fieldmap = fieldmap;
	mcw->gamedata = GAMESYSTEM_GetGameData(gsys);
	LOCATION_Set(&mcw->new_loc, zone_id, DOOR_ID_JUMP_CODE, 0/* DIR */,
			pos->x, pos->y, pos->z);
	
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeMap(GAMESYS_WORK * gsys,
		FIELD_MAIN_WORK * fieldmap, u16 zone_id, s16 exit_id)
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
	mcw->gsys = gsys;
	mcw->fieldmap = fieldmap;
	mcw->gamedata = GAMESYSTEM_GetGameData(gsys);
	MakeNextLocation(&mcw->new_loc, zone_id, exit_id);
	return event;
}

//------------------------------------------------------------------
//	※マップIDをインクリメントしている。最大値になったら先頭に戻る
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap)
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	PLAYER_WORK * myplayer = GAMEDATA_GetMyPlayerWork(gamedata);
	ZONEID next = PLAYERWORK_getZoneID(myplayer);
	next ++;
	if (next >= ZONEDATA_GetZoneIDMax()) {
		next = 0;
	}
	return DEBUG_EVENT_ChangeMap(gsys, fieldmap, next, 0);
}

//------------------------------------------------------------------
//	※マップID指定型 イベント切り替え
//------------------------------------------------------------------
void DEBUG_EVENT_ChangeEventMapChange(
	GAMESYS_WORK *gsys, GMEVENT *event,
	FIELD_MAIN_WORK *fieldmap, ZONEID zone_id )
{
	if (zone_id >= ZONEDATA_GetZoneIDMax()) {
		GF_ASSERT( 0 );
		zone_id = 0;
	}
	GMEVENT_ChangeEvent(event, DEBUG_EVENT_ChangeMap(gsys, fieldmap, zone_id, 0));
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

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void MakeNextLocation(LOCATION * loc, u16 zone_id, s16 exit_id)
{
	LOCATION_Set(loc, zone_id, exit_id, 0, 0, 0, 0);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void MakeNewLocation(const EVENTDATA_SYSTEM * evdata, const LOCATION * loc_prep,
		LOCATION * loc_tmp)
{
	//開始位置セット
	if (loc_prep->door_id == DOOR_ID_JUMP_CODE) {
		*loc_tmp = *loc_prep;
	} else {
		const CONNECT_DATA * cnct = EVENTDATA_GetConnectByID(evdata, loc_prep->door_id);
		if (cnct == NULL) {
			//本当はDOOR_IDからデータを引っ張る
			OS_Printf("connect: debug default position\n");
			LOCATION_DEBUG_SetDefaultPos(loc_tmp, loc_prep->zone_id);
		} else {
			CONNECTDATA_SetLocation(cnct, loc_tmp);
			switch (cnct->exit_type) {
			case EXIT_TYPE_UP:
				loc_tmp->pos.z -= FX32_ONE * 16;
				break;
			case EXIT_TYPE_DOWN:
				loc_tmp->pos.z += FX32_ONE * 16;
				break;
			case EXIT_TYPE_LEFT:
				loc_tmp->pos.x -= FX32_ONE * 16;
				break;
			case EXIT_TYPE_RIGHT:
				loc_tmp->pos.x += FX32_ONE * 16;
				break;
			default:
				break;
			}
		}
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void UpdateMapParams(GAMESYS_WORK * gsys, const LOCATION * new_loc)
{
	LOCATION loc_tmp;
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	PLAYER_WORK * mywork = GAMEDATA_GetMyPlayerWork(gamedata);
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
	u16 zone_id = new_loc->zone_id;

	//イベント起動データの読み込み
	EVENTDATA_SYS_Load(evdata, zone_id);

	//開始位置セット
	MakeNewLocation(evdata, new_loc, &loc_tmp);

	PLAYERWORK_setZoneID(mywork, loc_tmp.zone_id);
	PLAYERWORK_setPosition(mywork, &loc_tmp.pos);
	PLAYERWORK_setDirection(mywork, loc_tmp.dir_id);
	//開始位置を記憶しておく
	GAMEDATA_SetStartLocation(gamedata, &loc_tmp);
}



