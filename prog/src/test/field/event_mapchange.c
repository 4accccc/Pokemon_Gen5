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

typedef enum {
	FIELD_FADE_BLACK = 0,
	FIELD_FADE_WHITE,
}FIELD_FADE_TYPE;

GMEVENT * EVENT_FieldFadeOut(GAMESYS_WORK *gsys, FIELD_MAIN_WORK * fieldmap, FIELD_FADE_TYPE type);
GMEVENT * EVENT_FieldFadeIn(GAMESYS_WORK *gsys, FIELD_MAIN_WORK * fieldmap, FIELD_FADE_TYPE type);
GMEVENT * EVENT_FieldOpen(GAMESYS_WORK *gsys);
GMEVENT * EVENT_FieldClose(GAMESYS_WORK *gsys, FIELD_MAIN_WORK * fieldmap);

//============================================================================================
//
//		サブイベント
//
//============================================================================================
typedef struct {
	FIELD_FADE_TYPE fade_type;
}FADE_EVENT_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldFadeOutEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK * few = work;
	switch (*seq) {
	case 0:
		GFL_FADE_SetMasterBrightReq(
				GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,
				0, 16, 0);
		(*seq) ++;
		break;
	case 1:
		if (GFL_FADE_CheckFade() == FALSE) {
			return GMEVENT_RES_FINISH;
		}
		break;
	}
	return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldFadeOut(GAMESYS_WORK *gsys, FIELD_MAIN_WORK * fieldmap, FIELD_FADE_TYPE type)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldFadeOutEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
	few->fade_type = type;

	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldFadeInEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK * few = work;
	switch (*seq) {
	case 0:
		GFL_FADE_SetMasterBrightReq(
				GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,
				16, 0, 0);
		(*seq) ++;
		break;
	case 1:
		if (GFL_FADE_CheckFade() == FALSE) {
			return GMEVENT_RES_FINISH;
		}
		break;
	}

	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldFadeIn(GAMESYS_WORK *gsys, FIELD_MAIN_WORK * fieldmap, FIELD_FADE_TYPE type)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldFadeInEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
	few->fade_type = type;

	return event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELD_MAIN_WORK * fieldmap;
	GAMEDATA * gamedata;
}FIELD_OPENCLOSE_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldCloseEvent(GMEVENT * event, int * seq, void *work)
{
	FIELD_OPENCLOSE_WORK * focw = work;
	GAMESYS_WORK * gsys = focw->gsys;
	FIELD_MAIN_WORK * fieldmap = focw->fieldmap;
	switch (*seq) {
	case 0:
		FIELDMAP_Close(fieldmap);
		(*seq) ++;
		break;
	case 1:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldClose(GAMESYS_WORK *gsys, FIELD_MAIN_WORK * fieldmap)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldCloseEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = fieldmap;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldOpenEvent(GMEVENT * event, int *seq, void*work)
{
	FIELD_OPENCLOSE_WORK * focw = work;
	GAMESYS_WORK * gsys = focw->gsys;
	FIELD_MAIN_WORK * fieldmap;	// = focw->fieldmap;
	switch(*seq) {
	case 0:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		GAMESYSTEM_CallFieldProc(gsys);
		(*seq) ++;
		break;
	case 1:
		fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
		if (FieldMain_IsFieldUpdate(fieldmap) == FALSE) break;
		FieldMain_UpdateFieldFunc(fieldmap);
		(*seq) ++;
		break;
	case 2:
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldOpen(GAMESYS_WORK *gsys)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldOpenEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = NULL;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);
	return event;
}

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
	FIELD_MAIN_WORK * fieldmap;
	switch (*seq) {
	case 0:
		UpdateMapParams(gsys, &fmw->new_loc);
		(*seq)++;
		break;
	case 1:
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq)++;
		break;
	case 2:
		fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
		GMEVENT_CallEvent(event,EVENT_FieldFadeIn(gsys, fieldmap, 0));
		(*seq) ++;
		break;
	case 3:
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
	FIELD_MAIN_WORK * fieldmap = mcw->fieldmap;
	GAMEDATA * gamedata = mcw->gamedata;
	switch (*seq) {
	case 0:
		//フィールドマップをフェードアウト
		GMEVENT_CallEvent(event, EVENT_FieldFadeOut(gsys, fieldmap, 0));
		(*seq)++;
		break;
	case 1:
		//フィールドマップを終了待ち
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
		(*seq)++;
		break;
	case 2:
		//新しいマップID、初期位置をセット
		UpdateMapParams(gsys, &mcw->new_loc);
		(*seq)++;
		break;
	case 3:
		//フィールドマップを開始待ち
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq) ++;
		break;
	case 4:
		//フィールドマップをフェードイン
		GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, fieldmap, 0));
		(*seq) ++;
		break;
	case 5:
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
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELD_MAIN_WORK * fieldmap;
	FSOverlayID ov_id;
	const GFL_PROC_DATA * proc_data;
	void * proc_work;
}CHANGE_SAMPLE_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent(GMEVENT * event, int * seq, void * work)
{
	CHANGE_SAMPLE_WORK * csw = work;
	GAMESYS_WORK *gsys = csw->gsys;

	switch(*seq) {
	case 0:
		GMEVENT_CallEvent(event, EVENT_FieldFadeOut(gsys, csw->fieldmap, 0));
		(*seq) ++;
		break;
	case 1:
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, csw->fieldmap));
		(*seq) ++;
		break;
	case 2:
		GAMESYSTEM_CallProc(gsys, csw->ov_id, csw->proc_data, csw->proc_work);
		(*seq) ++;
		break;
	case 3:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		(*seq) ++;
		break;
	case 4:
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq) ++;
		break;
	case 5:
		GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, csw->fieldmap, 0));
		(*seq) ++;
		break;
	case 6:
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}

FS_EXTERN_OVERLAY(watanabe_sample);
extern const GFL_PROC_DATA TestProg1MainProcData;
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_FieldSample(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap)
{
#if 1
	GMEVENT * event = GMEVENT_Create(gsys, NULL, GameChangeEvent, sizeof(CHANGE_SAMPLE_WORK));
	CHANGE_SAMPLE_WORK * csw = GMEVENT_GetEventWork(event);
	csw->gsys = gsys;
	csw->fieldmap = fieldmap;
	csw->ov_id = FS_OVERLAY_ID(watanabe_sample);
	csw->proc_data = & TestProg1MainProcData;
	csw->proc_work = NULL;
#endif
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
static void MakeNewLocation(const EVENTDATA_SYSTEM * evdata, const LOCATION * loc_new,
		LOCATION * loc_tmp, BOOL * sp_exit_flag)
{
	*sp_exit_flag = FALSE;
	//開始位置セット
	if (loc_new->exit_id == DOOR_ID_JUMP_CODE) {
		*loc_tmp = *loc_new;
	} else {
		const CONNECT_DATA * cnct = EVENTDATA_GetConnectByID(evdata, loc_new->exit_id);
		if (cnct == NULL) {
			//本当はexit_idからデータを引っ張る
			OS_Printf("connect: debug default position\n");
			LOCATION_DEBUG_SetDefaultPos(loc_tmp, loc_new->zone_id);
		} else {
			CONNECTDATA_SetLocation(cnct, loc_tmp);
			if (loc_tmp->exit_id == EXIT_ID_SPECIAL) {
				*sp_exit_flag = TRUE;
			}
				//loc->zone_id = connect->link_zone_id;
				//loc->exit_id = connect->link_exit_id;
				//loc->pos = connect->pos;
				//loc->dir_id = connect->exit_type;
			loc_tmp->zone_id = loc_new->zone_id;
			loc_tmp->exit_id = loc_new->exit_id;
			switch (loc_tmp->dir_id) {
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
	u16 direction;
	BOOL sp_exit_flag;

	//イベント起動データの読み込み
	EVENTDATA_SYS_Load(evdata, zone_id);

	//開始位置セット
	MakeNewLocation(evdata, new_loc, &loc_tmp, &sp_exit_flag);
	if (sp_exit_flag) {
		//special_location = entrance_location;
		GAMEDATA_SetSpecialLocation(gamedata, GAMEDATA_GetEntranceLocation(gamedata));
	}

	PLAYERWORK_setZoneID(mywork, loc_tmp.zone_id);
	PLAYERWORK_setPosition(mywork, &loc_tmp.pos);
	switch (loc_tmp.dir_id) {
	default:
	case EXIT_TYPE_UP:		direction = 0x0000; break;
	case EXIT_TYPE_LEFT:	direction = 0x4000; break;
	case EXIT_TYPE_DOWN:	direction = 0x8000; break;
	case EXIT_TYPE_RIGHT:	direction = 0xc000; break;
	}
	PLAYERWORK_setDirection(mywork, direction);
	//開始位置を記憶しておく
	GAMEDATA_SetStartLocation(gamedata, &loc_tmp);
}



