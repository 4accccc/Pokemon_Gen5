//============================================================================================
/**
 * @file	event_mapchange.c
 * @brief マップ遷移関連イベント
 * @date	2008.11.04
 * @author	tamada GAME FREAK inc.
 *
 * @todo  FIELD_STATUS_SetFieldInitFlagをどこかに機能としてまとめるべきか検討
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/iss_sys.h"

#include "system/main.h"

#include "field/zonedata.h"
#include "field/fieldmap.h"
#include "field/location.h"
#include "field/rail_location.h"  //RAIL_LOCATION
#include "field/eventdata_sxy.h"
#include "field/fldmmdl.h"
#include "field/fieldskill_mapeff.h"
#include "field/field_status_local.h" //FIELD_STATUS

#include "event_fieldmap_control.h"
#include "event_mapchange.h"
#include "sound/pm_sndsys.h"		//サウンドシステム参照

#include "event_fldmmdl_control.h"
#include "field_place_name.h"   //FIELD_PLACE_NAME_DisplayForce

#include "script.h"

#include "fieldmap/zone_id.h"   //※check　ユニオンのSubScreen設定暫定対処の為

#include "warpdata.h"

#include "field_sound.h"

#include "event_entrance_in.h"
#include "event_entrance_out.h"
#include "event_appear.h"
#include "event_disappear.h"

#include "field/field_flagcontrol.h" //FIELD_FLAGCONT_INIT～

#include "savedata/gimmickwork.h"   //for GIMMICKWORK

#include "net_app/union/union_main.h" // for UNION_CommBoot

#include "savedata/gametime.h"  // for GMTIME
#include "gamesystem/pm_season.h"  // for PMSEASON_TOTAL

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void MAPCHG_setupMapTools( GAMESYS_WORK * gsys, const LOCATION * loc_req );
static void MAPCHG_releaseMapTools( GAMESYS_WORK * gsys );

static void MAPCHG_updateGameData( GAMESYS_WORK * gsys, const LOCATION * loc_req );

static void MAPCHG_loadMMdl( GAMEDATA * gamedata, const LOCATION *loc_req );
static void MAPCHG_releaseMMdl( GAMEDATA * gamedata );

static void MAPCHG_setupFieldSkillMapEff( GAMEDATA * gamedata, const LOCATION *loc_req );

static void setFirstBGM(GAMEDATA * gamedata, u16 zone_id);
static void AssignGimmickID(GAMEDATA * gamedata, int inZoneID);

static GMEVENT_RESULT EVENT_MapChangeNoFade(GMEVENT * event, int *seq, void*work);


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
	LOCATION loc_req;
}FIRST_MAPIN_WORK;

typedef struct GMK_ASSIGN_DATA_tag
{
  u32 ZoneID;
  u16 ResNum;
  u16 ObjNum;
}GMK_ASSIGN_DATA;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FirstMapIn(GMEVENT * event, int *seq, void *work)
{
	FIRST_MAPIN_WORK * fmw = work;
	GAMESYS_WORK * gsys = fmw->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	GAME_INIT_WORK * game_init_work = fmw->game_init_work;
	FIELDMAP_WORK * fieldmap;
	switch (*seq) {
	case 0:
		switch(game_init_work->mode){
		case GAMEINIT_MODE_FIRST:
      SCRIPT_CallGameStartInitScript( gsys, GFL_HEAPID_APP );
      { 
        FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
      }
      //新しいマップモードなど機能指定を行う
      MAPCHG_setupMapTools( gsys, &fmw->loc_req );
      //新しいマップID、初期位置をセット
      MAPCHG_updateGameData( gsys, &fmw->loc_req );
			break;

		case GAMEINIT_MODE_DEBUG:
      SCRIPT_CallDebugGameStartInitScript( gsys, GFL_HEAPID_APP );
      { 
        FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
      }
      //新しいマップモードなど機能指定を行う
      MAPCHG_setupMapTools( gsys, &fmw->loc_req );
      //新しいマップID、初期位置をセット
      MAPCHG_updateGameData( gsys, &fmw->loc_req );
			break;

    case GAMEINIT_MODE_CONTINUE:
      //新しいマップモードなど機能指定を行う
      MAPCHG_setupMapTools( gsys, &fmw->loc_req );
      //コンティニューによるフラグ落とし処理
      FIELD_FLAGCONT_INIT_Continue( gamedata, fmw->loc_req.zone_id );
      break;

    default:
      GF_ASSERT(0);
		}
		
		setFirstBGM(fmw->gamedata, fmw->loc_req.zone_id);
		
		(*seq)++;
		break;
	case 1:
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq)++;
		break;
	case 2:
		fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
		GMEVENT_CallEvent(event,EVENT_FieldFadeIn(gsys, fieldmap, 0, FIELD_FADE_WAIT));
		(*seq) ++;
		break;
	case 3:
		fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
    FIELD_PLACE_NAME_DisplayForce(FIELDMAP_GetPlaceNameSys(fieldmap), fmw->loc_req.zone_id);
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief ゲーム開始処理
 *
 * @todo
 * コンティニュー処理を内部で切り分けているので
 * きちんと関数分割する必要がある
 */
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
	switch(game_init_work->mode){
	case GAMEINIT_MODE_CONTINUE:
		LOCATION_SetDirect(&fmw->loc_req, game_init_work->mapid, game_init_work->dir, 
			game_init_work->pos.x, game_init_work->pos.y, game_init_work->pos.z);
		break;
	case GAMEINIT_MODE_FIRST:
		LOCATION_SetGameStart(&fmw->loc_req);
		break;
	case GAMEINIT_MODE_DEBUG:
		LOCATION_DEBUG_SetDefaultPos(&fmw->loc_req, game_init_work->mapid);
		break;
	}
	return event;
}

//============================================================================================
//
//		イベント：ゲーム終了
//
//============================================================================================
typedef struct {
	GAMESYS_WORK * gsys;
	FIELDMAP_WORK * fieldmap;
}GAME_END_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameEndEvent(GMEVENT * event, int *seq, void *work)
{
	GAME_END_WORK * gew = work;
	switch (*seq) {
	case 0:
		//フィールドマップをフェードアウト
		GMEVENT_CallEvent(event, EVENT_FieldFadeOut(gew->gsys, gew->fieldmap, 0, FIELD_FADE_WAIT));
		//通信が動いている場合は終了させる
		if(GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gew->gsys)) != GAME_COMM_NO_NULL){
      GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gew->gsys));
    }
		(*seq)++;
		break;
	case 1:
	  //通信終了待ち
	  if(GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gew->gsys)) != GAME_COMM_NO_NULL){
      break;
    }
		//フィールドマップを終了待ち
		GMEVENT_CallEvent(event, EVENT_FieldClose(gew->gsys, gew->fieldmap));
		(*seq)++;
		break;
	case 2:
		//プロセスを終了し、イベントも終了させるとゲームを終わる
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	デバッグ用：ゲーム終了
 *
 * @note
 * 実際にはゲームを終了させるというインターフェイスは
 * ポケモンに存在しないので、この関数はあくまでデバッグ用。
 */
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_GameEnd( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, GameEndEvent, sizeof(GAME_END_WORK));
	GAME_END_WORK * gew = GMEVENT_GetEventWork(event);
	gew->gsys = gsys;
	gew->fieldmap = fieldmap;
	return event;
}

//============================================================================================
//
//	イベント：マップ切り替え
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief マップ遷移処理イベント用ワーク
 */
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	GAMEDATA * gamedata;
	FIELDMAP_WORK * fieldmap;
  u16 before_zone_id;         ///<遷移前マップID
	LOCATION loc_req;           ///<遷移先指定
  EXIT_TYPE exit_type;
  VecFx32 stream_pos;  ///<流砂遷移時にのみ使用
}MAPCHANGE_WORK;

typedef MAPCHANGE_WORK* MAPCHANGE_WORK_PTR;


//------------------------------------------------------------------
/**
 * @brief 季節を更新する
 */
//------------------------------------------------------------------
static void UpdateSeason( GAMEDATA* gamedata )
{
  RTCDate date_start, date_now;
  RTCTime time_start;
  GMTIME* gmtime;
  SAVE_CONTROL_WORK* scw;
  u8 season;

  // ゲーム開始日時を取得
  scw = GAMEDATA_GetSaveControlWork( gamedata );
  gmtime = SaveData_GetGameTime( scw );
  RTC_ConvertSecondToDateTime( &date_start, &time_start, gmtime->start_sec );
  // 現在の日付を取得
  RTC_GetDate( &date_now );
  // 現在の季節を求める
  season = (date_now.month - date_start.month + 12) % PMSEASON_TOTAL;
  // 季節が変化したら, 季節表示フラグを立てる
  if( season != GAMEDATA_GetSeasonID(gamedata) )
  {
    FIELD_STATUS* fstatus = GAMEDATA_GetFieldStatus( gamedata );
    FIELD_STATUS_SetSeasonDispFlag( fstatus, TRUE );
    // DEBUG:
    OBATA_Printf( "update season: ==> %d\n", season );
  }
  // ゲームデータ更新
  GAMEDATA_SetSeasonID( gamedata, season );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_MapChangeCore( GMEVENT* event, int* seq, void* work )
{
	MAPCHANGE_WORK*     mcw = *( (MAPCHANGE_WORK_PTR*)work );
	GAMESYS_WORK*      gsys = mcw->gsys;
	FIELDMAP_WORK* fieldmap = mcw->fieldmap;
	GAMEDATA*      gamedata = mcw->gamedata;

	switch( *seq )
  {
	case 0:
		//フィールドマップを終了待ち
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
		(*seq)++;
		break;
	case 1:
    //マップモードなど機能指定を解除する
    MAPCHG_releaseMapTools( gsys );

    { 
      FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
    }

    // 季節の更新
    UpdateSeason( gamedata );

    //新しいマップモードなど機能指定を行う
    MAPCHG_setupMapTools( gsys, &mcw->loc_req );
    
		//新しいマップID、初期位置をセット
		MAPCHG_updateGameData( gsys, &mcw->loc_req );

		(*seq)++;
		break;
	case 2:
		//フィールドマップを開始待ち
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq) ++;
		break;
  case 3:
    // BGMフェードアウト終了待ち
    {
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      if( FIELD_SOUND_IsBGMFade(fsnd) != TRUE )
      { 
        PLAYER_WORK* player = GAMEDATA_GetPlayerWork( gamedata, 0 );
        PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
        FIELD_SOUND_ChangePlayZoneBGM( fsnd, gamedata, form, mcw->loc_req.zone_id );
        (*seq)++;
      }
    }
    break;
  case 4:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT* EVENT_MapChangeCore( MAPCHANGE_WORK* mcw )
{
  GMEVENT* event;
  MAPCHANGE_WORK_PTR* work;

  event = GMEVENT_Create( mcw->gsys, NULL, EVENT_FUNC_MapChangeCore, sizeof( MAPCHANGE_WORK_PTR ) );
  work  = GMEVENT_GetEventWork( event );
  *work = mcw;

  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChange(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK * mcw = work;
	GAMESYS_WORK  * gsys = mcw->gsys;
	FIELDMAP_WORK * fieldmap = mcw->fieldmap;
	GAMEDATA * gamedata = mcw->gamedata;
	switch (*seq) {
  case 0:
    // 動作モデルの移動を止める
    {
      MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDLSYS_PauseMoveProc( fmmdlsys );
    }
    // 入口進入イベント
    GMEVENT_CallEvent( event, 
        EVENT_EntranceIn( event, gsys, gamedata, fieldmap, mcw->loc_req, mcw->exit_type ) );
		(*seq)++;
		break;
  case 1:
    // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 2:
    // 入口退出イベント
    GMEVENT_CallEvent( event, EVENT_EntranceOut( event, gsys, gamedata, fieldmap, mcw->loc_req ) );
		(*seq) ++;
		break;
  case 3:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeNoFade(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK * mcw = work;
	GAMESYS_WORK  * gsys = mcw->gsys;
	FIELDMAP_WORK * fieldmap = mcw->fieldmap;
	GAMEDATA * gamedata = mcw->gamedata;
	switch (*seq) {
  case 0:
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, fieldmap) );
    (*seq) ++;
    break;
	case 1:
    { // BGM更新リクエスト
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      PLAYER_WORK* player = GAMEDATA_GetPlayerWork( gamedata, 0 );
      PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
      FIELD_SOUND_ChangePlayZoneBGM( fsnd, gamedata, form, mcw->loc_req.zone_id );
    }
		(*seq)++;
		break;
  case 2:
    // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 3:
    FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), mcw->loc_req.zone_id);
		(*seq) ++;
		break;
  case 4:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeBySandStream(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK*       mcw = work;
	GAMESYS_WORK*        gsys = mcw->gsys;
	FIELDMAP_WORK* fieldmap = mcw->fieldmap;
	GAMEDATA*        gamedata = mcw->gamedata;

	switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, fieldmap) );
    (*seq) ++;
    break;
	case 1: // 流砂退場イベント
    GMEVENT_CallEvent( 
        event, EVENT_DISAPPEAR_FallInSand( event, gsys, fieldmap, &mcw->stream_pos ) );
		(*seq)++;
		break;
  case 2: // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 3: // 流砂登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Fall( event, gsys, fieldmap ) );
		(*seq) ++;
		break;
  case 4:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeByAnanukenohimo(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK*     mcw = work;
	GAMESYS_WORK*      gsys = mcw->gsys;
	GAMEDATA*      gamedata = mcw->gamedata;
	FIELDMAP_WORK* fieldmap = mcw->fieldmap;

	switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, fieldmap) );
    (*seq) ++;
    break;
	case 1: // 退場イベント
    GMEVENT_CallEvent( 
        event, EVENT_DISAPPEAR_Ananukenohimo( event, gsys, fieldmap ) );
		(*seq)++;
		break;
  case 2: // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 3: // 登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Ananukenohimo( event, gsys, fieldmap ) );
		(*seq) ++;
		break;
  case 4:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeByAnawohoru(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK*     mcw = work;
	GAMESYS_WORK*      gsys = mcw->gsys;
	GAMEDATA*      gamedata = mcw->gamedata;
	FIELDMAP_WORK* fieldmap = mcw->fieldmap;

	switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, fieldmap) );
    (*seq) ++;
    break;
	case 1: // 退場イベント
    GMEVENT_CallEvent( 
        event, EVENT_DISAPPEAR_Anawohoru( event, gsys, fieldmap ) );
		(*seq)++;
		break;
  case 2: // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 3: // 登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Anawohoru( event, gsys, fieldmap ) );
		(*seq) ++;
		break;
  case 4:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeByTeleport(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK*     mcw = work;
	GAMESYS_WORK*      gsys = mcw->gsys;
	GAMEDATA*      gamedata = mcw->gamedata;
	FIELDMAP_WORK* fieldmap = mcw->fieldmap;

	switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, fieldmap) );
    (*seq) ++;
    break;
	case 1: // 退場イベント
    GMEVENT_CallEvent( 
        event, EVENT_DISAPPEAR_Teleport( event, gsys, fieldmap ) );
		(*seq)++;
		break;
  case 2: // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 3: // 登場イベント
    GMEVENT_CallEvent( event, EVENT_APPEAR_Teleport( event, gsys, fieldmap ) );
		(*seq) ++;
		break;
  case 4:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_MapChangeToUnion(GMEVENT * event, int *seq, void*work)
{
	MAPCHANGE_WORK*     mcw = work;
	GAMESYS_WORK*      gsys = mcw->gsys;
	FIELDMAP_WORK* fieldmap = mcw->fieldmap;
	GAMEDATA*      gamedata = mcw->gamedata;

	switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, fieldmap) );
    (*seq) ++;
    break;
	case 1:
    // 入口進入イベント
    GMEVENT_CallEvent( event, 
        EVENT_EntranceIn( event, gsys, gamedata, fieldmap, mcw->loc_req, mcw->exit_type ) );
		(*seq)++;
		break;
  case 2: // マップチェンジ・コア・イベント
    GMEVENT_CallEvent( event, EVENT_MapChangeCore( mcw ) );
		(*seq)++;
    break;
	case 3: // ユニオン通信起動
    UNION_CommBoot( gsys );
		(*seq) ++;
		break;
	case 4:
    // 入口退出イベント
    GMEVENT_CallEvent( event, EVENT_EntranceOut( event, gsys, gamedata, fieldmap, mcw->loc_req ) );
		(*seq) ++;
		break;
  case 5:
		return GMEVENT_RES_FINISH; 
	}
	return GMEVENT_RES_CONTINUE;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void MAPCHANGE_WORK_init(MAPCHANGE_WORK * mcw, GAMESYS_WORK * gsys)
{
  mcw->gsys = gsys;
  mcw->gamedata = GAMESYSTEM_GetGameData( gsys );
  mcw->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  mcw->before_zone_id = FIELDMAP_GetZoneID( mcw->fieldmap );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_ChangeMapPos(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos, u16 dir )
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys );
	
	LOCATION_SetDirect(&mcw->loc_req, zone_id, dir, pos->x, pos->y, pos->z);
  mcw->exit_type = EXIT_TYPE_NONE;
	
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeMapRailLocation(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		u16 zone_id, const RAIL_LOCATION * rail_loc, u16 dir )
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys );
	
	LOCATION_SetDirectRail(&mcw->loc_req, zone_id, dir, rail_loc->rail_index, rail_loc->line_grid, rail_loc->width_grid);
  mcw->exit_type = EXIT_TYPE_NONE;
	
	return event;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeMapDefaultPos(GAMESYS_WORK * gsys,
		FIELDMAP_WORK * fieldmap, u16 zone_id)
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys );

	if (zone_id >= ZONEDATA_GetZoneIDMax()) {
		GF_ASSERT( 0 );
		zone_id = 0;
	}
	LOCATION_DEBUG_SetDefaultPos(&mcw->loc_req, zone_id);
  mcw->exit_type = EXIT_TYPE_NONE;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（ 流砂 ）
 * @param	gsys		      ゲームシステムへのポインタ
 * @param	fieldmap	    フィールドシステムへのポインタ
 * @param disappear_pos 流砂中心点の座標
 * @param	zone_id		    遷移するマップのZONE指定
 * @param appear        遷移先での座標
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
GMEVENT * EVENT_ChangeMapBySandStream(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, 
    const VecFx32* disappear_pos, u16 zone_id, const VecFx32* appear_pos )
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeBySandStream, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys ); 
	LOCATION_SetDirect(
      &mcw->loc_req, zone_id, DIR_DOWN, appear_pos->x, appear_pos->y, appear_pos->z);
  mcw->exit_type = EXIT_TYPE_NONE;
  VEC_Set( &mcw->stream_pos, disappear_pos->x, disappear_pos->y, disappear_pos->z );
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（ あなぬけのヒモ ）
 * @param	gsys ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_ChangeMapByAnanukenohimo( GAMESYS_WORK * gsys )
{
	GMEVENT* event;
	MAPCHANGE_WORK* work;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeByAnanukenohimo, sizeof(MAPCHANGE_WORK));
	work = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( work, gsys ); 
  work->loc_req = *(GAMEDATA_GetEscapeLocation( work->gamedata ));
  work->loc_req.type = LOCATION_TYPE_DIRECT;
  work->exit_type = EXIT_TYPE_NONE;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（ あなをほる )
 * @param	gsys ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_ChangeMapByAnawohoru( GAMESYS_WORK * gsys )
{
	GMEVENT* event;
	MAPCHANGE_WORK* work;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeByAnawohoru, sizeof(MAPCHANGE_WORK));
	work = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( work, gsys ); 
  work->loc_req = *(GAMEDATA_GetEscapeLocation( work->gamedata ));
  work->loc_req.type = LOCATION_TYPE_DIRECT;
  work->exit_type = EXIT_TYPE_NONE;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（ テレポート )
 * @param	gsys ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_ChangeMapByTeleport( GAMESYS_WORK * gsys )
{
  u16 warp_id;
	GMEVENT* event;
	MAPCHANGE_WORK* work;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeByTeleport, sizeof(MAPCHANGE_WORK));
	work = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( work, gsys ); 
  warp_id = GAMEDATA_GetWarpID( work->gamedata );
  WARPDATA_GetWarpLocation( warp_id, &work->loc_req );
	LOCATION_DEBUG_SetDefaultPos( &work->loc_req, work->loc_req.zone_id );
  work->loc_req.type = LOCATION_TYPE_DIRECT;
  work->exit_type = EXIT_TYPE_NONE;
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_ChangeMapToUnion( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeToUnion, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys ); 
	LOCATION_DEBUG_SetDefaultPos( &mcw->loc_req, ZONE_ID_UNION );
  mcw->exit_type = EXIT_TYPE_NONE;
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_ChangeMapByConnect(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    const CONNECT_DATA * cnct, LOC_EXIT_OFS exit_ofs)
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChange, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys );

	if (CONNECTDATA_IsSpecialExit(cnct))
  {
		//特殊接続先が指定されている場合、記憶しておいた場所に飛ぶ
		const LOCATION * sp = GAMEDATA_GetSpecialLocation(gamedata);
    mcw->loc_req = *sp;
	}
  else
  {
		CONNECTDATA_SetNextLocation(cnct, &mcw->loc_req, exit_ofs);
	}
  mcw->exit_type = CONNECTDATA_GetExitType(cnct);

  {
    //フィールド→屋内への移動の際は脱出先位置を記憶しておく
    const LOCATION * ent = GAMEDATA_GetEntranceLocation(gamedata);
    if (ZONEDATA_IsFieldMatrixID(ent->zone_id) == TRUE
        && ZONEDATA_IsFieldMatrixID(mcw->loc_req.zone_id) == FALSE)
    {
      GAMEDATA_SetEscapeLocation( gamedata, ent );
    }
  }
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_ChangeMapPosNoFade(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos, u16 dir )
{
	MAPCHANGE_WORK * mcw;
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeNoFade, sizeof(MAPCHANGE_WORK));
	mcw = GMEVENT_GetEventWork(event);
  MAPCHANGE_WORK_init( mcw, gsys );
	
	LOCATION_SetDirect(&mcw->loc_req, zone_id, dir, pos->x, pos->y, pos->z);
  mcw->exit_type = EXIT_TYPE_NONE;
	
	return event;
}



//------------------------------------------------------------------
//	※マップIDをインクリメントしている。最大値になったら先頭に戻る
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	PLAYER_WORK * myplayer = GAMEDATA_GetMyPlayerWork(gamedata);
	ZONEID next = PLAYERWORK_getZoneID(myplayer);
	next ++;
	if (next >= ZONEDATA_GetZoneIDMax()) {
		next = 0;
	}
	return DEBUG_EVENT_ChangeMapDefaultPos(gsys, fieldmap, next);
}

//============================================================================================
//
//
//  マップ遷移初期化処理
//
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief 全滅時のマップ遷移処理（フィールド非生成時）
 */
//--------------------------------------------------------------
void MAPCHG_GameOver( GAMESYS_WORK * gsys )
{
  LOCATION loc_req;
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  u16 warp_id = GAMEDATA_GetWarpID( gamedata );
  //復活ポイントを取得
  WARPDATA_GetRevivalLocation( warp_id, &loc_req );
  {
    LOCATION esc;
    //エスケープポイントを再設定
    WARPDATA_GetEscapeLocation( warp_id, &esc );
    GAMEDATA_SetEscapeLocation( gamedata, &esc );
  }

  //マップモードなど機能指定を解除する
  MAPCHG_releaseMapTools( gsys );

  { 
    FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus(gamedata), TRUE );
  }
  //新しいマップモードなど機能指定を行う
  MAPCHG_setupMapTools( gsys, &loc_req );
  
  //新しいマップID、初期位置をセット
  MAPCHG_updateGameData( gsys, &loc_req );

  //ゲームオーバー時のフラグのクリア
  FIELD_FLAGCONT_INIT_GameOver( gamedata, loc_req.zone_id );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static u16 GetDirValueByDirID(EXIT_DIR dir_id)
{
	switch (dir_id) {
	default:
	case EXIT_DIR_UP:	return 0x0000;
	case EXIT_DIR_LEFT:	return 0x4000;
	case EXIT_DIR_DOWN:	return 0x8000;
	case EXIT_DIR_RIGHT:return 0xc000;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void MakeNewLocation(const EVENTDATA_SYSTEM * evdata, const LOCATION * loc_req,
		LOCATION * loc_tmp)
{
	BOOL result;

	//直接指定の場合はコピーするだけ
	if (loc_req->type == LOCATION_TYPE_DIRECT) {
		*loc_tmp = *loc_req;
		return;
	}
	//開始位置セット
	result = EVENTDATA_SetLocationByExitID(evdata, loc_tmp, loc_req->exit_id, loc_req->exit_ofs );

	if (!result) {
		//デバッグ用処理：本来は不要なはず
		OS_Printf("connect: debug default position\n");
		LOCATION_DEBUG_SetDefaultPos(loc_tmp, loc_req->zone_id);
	}
}

//------------------------------------------------------------------
/**
 * @brief Map切り替え時のデータ更新処理
 *
 * @todo  歩いてマップをまたいだときの処理との共通部分をきちんと処理すること
 */
//------------------------------------------------------------------
static void MAPCHG_updateGameData( GAMESYS_WORK * gsys, const LOCATION * loc_req )
{
	LOCATION loc;
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	PLAYER_WORK * mywork = GAMEDATA_GetMyPlayerWork(gamedata);
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  FIELD_STATUS* fldstatus = GAMEDATA_GetFieldStatus(gamedata);

  { 
    char buf[ZONEDATA_NAME_LENGTH*2];
    ZONEDATA_DEBUG_GetZoneName(buf, loc_req->zone_id);
    TAMADA_Printf("MAPCHG_updateGameData:%s\n", buf);
  }

	
	//開始位置セット
	MakeNewLocation(evdata, loc_req, &loc);

	//特殊接続出入口に出た場合は、前のマップの出入口位置を記憶しておく
	if (loc.type == LOCATION_TYPE_SPID) {
		//special_location = entrance_location;
		GAMEDATA_SetSpecialLocation(gamedata, GAMEDATA_GetEntranceLocation(gamedata));
	}

	//取得したLOCATION情報を自機へ反映
	{
		u16 direction;
		PLAYERWORK_setZoneID(mywork, loc.zone_id);
		direction = GetDirValueByDirID(loc.dir_id);
		PLAYERWORK_setDirection(mywork, direction);

    if( LOCATION_GetPosType( &loc ) == LOCATION_POS_TYPE_3D ){
  		PLAYERWORK_setPosition(mywork, &loc.location_pos.pos);
      PLAYERWORK_setPosType( mywork, LOCATION_POS_TYPE_3D );
    }else{
	  	PLAYERWORK_setRailPosition(mywork, &loc.location_pos.railpos);
      PLAYERWORK_setPosType( mywork, LOCATION_POS_TYPE_RAIL );
    }
	}

	// ISSにゾーン切り替えを通知
	{
		ISS_SYS* iss = GAMESYSTEM_GetIssSystem( gsys );
		ISS_SYS_ZoneChange( iss, loc.zone_id );
	}
	
	//開始位置を記憶しておく
	GAMEDATA_SetStartLocation(gamedata, &loc);

  //ワープ先登録
  {
    u16 warp_id;
    warp_id = WARPDATA_SearchByRoomID( loc.zone_id );
    if (warp_id) {
      GAMEDATA_SetWarpID( gamedata, warp_id );
    }
  }
  //ギミックアサイン
  AssignGimmickID(gamedata, loc.zone_id);

  //特殊スクリプト呼び出し：ゾーン切り替え
  SCRIPT_CallZoneChangeScript( gsys, HEAPID_PROC );

  //マップ遷移時のフラグ初期化
  FIELD_FLAGCONT_INIT_MapJump( gamedata, loc.zone_id );

  //マップ到着フラグセット
  ARRIVEDATA_SetArriveFlag( gamedata, loc.zone_id );

  //新規ゾーンに配置する動作モデルを追加
  MAPCHG_loadMMdl( gamedata, loc_req );

  //フィールド技　マップ効果
  MAPCHG_setupFieldSkillMapEff( gamedata, &loc );
  
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void MAPCHG_loadMMdl( GAMEDATA * gamedata, const LOCATION *loc_req )
{
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  u16 count = EVENTDATA_GetNpcCount( evdata );
  
  if( count ){
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gamedata );
    MMDLSYS *fmmdlsys = GAMEDATA_GetMMdlSys( gamedata );
    const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
    MMDLSYS_SetMMdl( fmmdlsys, header, loc_req->zone_id, count, evwork );
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void MAPCHG_releaseMMdl( GAMEDATA * gamedata )
{
		MMDLSYS_DeleteMMdl( GAMEDATA_GetMMdlSys(gamedata) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　画面効果マスクの設定
 */
//-----------------------------------------------------------------------------
static void MAPCHG_setupFieldSkillMapEff( GAMEDATA * gamedata, const LOCATION *loc_req )
{
  FIELD_STATUS * fldstatus =  GAMEDATA_GetFieldStatus( gamedata ); 
  u32 fs_effmsk = ZONEDATA_GetFieldSkillMapEffMsk( loc_req->zone_id );

  //フラッシュ　システムフラグを見て、マスクを書き換える
  if( FIELDSKILL_MAPEFF_MSK_IS_ON(fs_effmsk, FIELDSKILL_MAPEFF_MSK_FLASH_NEAR) )
  {
    if( FIELD_STATUS_IsFieldSkillFlash(fldstatus) )
    {
      // フラッシュ技使用後に変換
      FIELDSKILL_MAPEFF_MSK_OFF(fs_effmsk, FIELDSKILL_MAPEFF_MSK_FLASH_NEAR);
      FIELDSKILL_MAPEFF_MSK_ON(fs_effmsk, FIELDSKILL_MAPEFF_MSK_FLASH_FAR);
      
    }
  }
  FIELD_STATUS_SetFieldSkillMapEffectMsk( fldstatus, fs_effmsk );
}

//--------------------------------------------------------------
/**
 * @brief
 * @param gamedata
 *
 * @todo  マップをまたいでor画面暗転中もずっと維持されるメモリ状態などはここで設定
 * @todo  下記の※checkの対処を行う
 */
//--------------------------------------------------------------
static void MAPCHG_setupMapTools( GAMESYS_WORK * gsys, const LOCATION * loc_req )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
  //MAPCHGモジュール群はFieldMapが存在しないときに呼ばれる
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  GF_ASSERT( fieldmap == NULL );

  //※check　スクリプトでマップ作成前に実行できるタイミングが出来れば、そこで行うようにしたい
  if(ZONEDATA_IsPalaceField(loc_req->zone_id) || ZONEDATA_IsBingo(loc_req->zone_id)){
    FIELD_STATUS_SetMapMode( GAMEDATA_GetFieldStatus( gamedata ), MAPMODE_INTRUDE );
    //GAMEDATA_SetMapMode(gamedata, MAPMODE_INTRUDE);
  }
  //※check　ユニオンルームへの移動を受付スクリプトで制御するようになったらサブスクリーンモードの
  //         変更もそのスクリプト内で行うようにする
  switch(loc_req->zone_id){
  case ZONE_ID_UNION:
  case ZONE_ID_CLOSSEUM:
  case ZONE_ID_CLOSSEUM02:
    GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_UNION);
    break;
  default:
    GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_NORMAL);
    break;
  } 

	//イベント起動データの読み込み
	EVENTDATA_SYS_Load(evdata, loc_req->zone_id, GAMEDATA_GetSeasonID(gamedata) );

  //マトリックスデータの更新
  {
    MAP_MATRIX * matrix = GAMEDATA_GetMapMatrix( gamedata );
		u16 matID = ZONEDATA_GetMatrixID( loc_req->zone_id );
		MAP_MATRIX_Init( matrix, matID, loc_req->zone_id );
  }

  //フィールド技 マップ効果
  MAPCHG_setupFieldSkillMapEff( gamedata, loc_req );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void MAPCHG_releaseMapTools( GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  //MAPCHGモジュール群はFieldMapが存在しないときに呼ばれる
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  GF_ASSERT( fieldmap == NULL );

  //配置していた動作モデルを削除
  MAPCHG_releaseMMdl( gamedata );

}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setFirstBGM(GAMEDATA * gamedata, u16 zone_id)
{
  FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
  PLAYER_WORK *player = GAMEDATA_GetPlayerWork( gamedata, 0 );
  PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
  u32 no = FIELD_SOUND_GetFieldBGMNo( gamedata, form, zone_id );
  OS_Printf("NEXT BGM NO=%d\n",no);
  FIELD_SOUND_PlayNextBGM_Ex( fsnd, no, 0, 60 );// ゲーム開始時はBGMフェードインで始まる
}
//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
void MAPCHANGE_setPlayerVanish(FIELDMAP_WORK * fieldmap, BOOL vanish_flag)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
  MMDL_SetStatusBitVanish( fmmdl, vanish_flag );
}

//---------------------------------------------------------------------------
/**
 * @brief	ギミックのアサイン
 * 
 * @param	  inZoneID    ゾーンＩＤ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void AssignGimmickID(GAMEDATA * gamedata, int inZoneID)
{
  GIMMICKWORK *work;
  //ギミックワーク取得
	work = GAMEDATA_GetGimmickWork(gamedata);

  //マップジャンプのときのみ初期化する（歩いてゾーンが切り替わった場合は初期化しない）
	GIMMICKWORK_Init(work);

  //ギミックデータ検索
  {
    u32 num, i;
    ARCHANDLE *handle;
    GMK_ASSIGN_DATA *data;
    handle = GFL_ARC_OpenDataHandle( ARCID_GIMMICK_ASSIGN, GFL_HEAP_LOWID(GFL_HEAPID_APP) );
    data = (GMK_ASSIGN_DATA *)GFL_ARC_LoadDataAllocByHandle( handle, 0, GFL_HEAP_LOWID(GFL_HEAPID_APP) );

    num = GFL_ARC_GetDataSizeByHandle( handle, 0 ) / sizeof(GMK_ASSIGN_DATA);

//    OS_Printf("gimmick_num = %d\n",num);

    for (i=0;i<num;i++){
      if ( data[i].ZoneID == inZoneID){
        //ギミック発見。アサインする
        GIMMICKWORK_Assign(work, i+1);
        break;
      }
    }

    GFL_HEAP_FreeMemory( data );
    GFL_ARC_CloseDataHandle( handle );
  }
}

