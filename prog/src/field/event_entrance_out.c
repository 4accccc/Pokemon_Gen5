/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   event_entrance_out.c
 * @breif  出入口からの退出時のイベント作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include "gamesystem/game_event.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h"

#include "event_entrance_out.h"

#include "field/eventdata_sxy.h"
#include "event_fieldmap_control.h" // EVENT_FieldFadeIn
#include "event_entrance_effect.h"  // EVENT_FieldDoorOutAnime
#include "event_fldmmdl_control.h"  // EVENT_PlayerOneStepAnime
#include "field_place_name.h" // FIELD_PLACE_NAME_Display
#include "fieldmap.h"       // FIELDMAP_GetPlaceNameSys
#include "field_sound.h"
#include "sound/pm_sndsys.h"
#include "sound/bgm_info.h"
#include "../../resource/sound/bgm_info/iss_type.h"
#include "field/zonedata.h"  // for ZONEDATA_GetBGMID
#include "event_appear.h"  // for EVENT_APPEAR_xxxx



//=======================================================================================
/**
 * @brief イベント・ワーク
 */
//=======================================================================================
typedef struct
{
  GAMESYS_WORK*    gsys;
  GAMEDATA*        gdata;
  FIELDMAP_WORK* fieldmap;
  LOCATION         location;  // 遷移先指定
}
EVENT_WORK;


//=======================================================================================
/**
 * @breif 非公開関数のプロトタイプ宣言
 */
//======================================================================================= 
// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeNone(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeDoor(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeStep(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeWarp(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP1(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP2(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP3(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP4(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP5(GMEVENT * event, int *seq, void * work);


//=======================================================================================
/**
 * @breif 公開関数の定義
 */
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口退出イベントを作成する
 *
 * @param parent    親イベント
 * @param gsys      ゲームシステム
 * @param gdata     ゲームデータ
 * @param fieldmap  フィールドマップ
 * @param location  遷移先指定
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceOut( GMEVENT* parent, 
                           GAMESYS_WORK* gsys,
                           GAMEDATA* gdata, 
                           FIELDMAP_WORK* fieldmap, 
                           LOCATION location )
{
  GMEVENT* event;
  EVENT_WORK* work;
  EXIT_TYPE exit_type;


  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[] = {
    EVENT_FUNC_EntranceOut_ExitTypeNone,   //EXIT_TYPE_NONE
    EVENT_FUNC_EntranceOut_ExitTypeNone,   //EXIT_TYPE_MAT
    EVENT_FUNC_EntranceOut_ExitTypeStep,   //EXIT_TYPE_STAIRS
    EVENT_FUNC_EntranceOut_ExitTypeDoor,   //EXIT_TYPE_DOOR
    EVENT_FUNC_EntranceOut_ExitTypeStep,   //EXIT_TYPE_WALL
    EVENT_FUNC_EntranceOut_ExitTypeWarp,   //EXIT_TYPE_WARP
    EVENT_FUNC_EntranceOut_ExitTypeSP1,    //EXIT_TYPE_SP1
    EVENT_FUNC_EntranceOut_ExitTypeSP2,    //EXIT_TYPE_SP2
    EVENT_FUNC_EntranceOut_ExitTypeSP3,    //EXIT_TYPE_SP3
    EVENT_FUNC_EntranceOut_ExitTypeSP4,    //EXIT_TYPE_SP4
    EVENT_FUNC_EntranceOut_ExitTypeSP5,    //EXIT_TYPE_SP5
  };

  // EXIT_TYPEを決定
  if( location.type == LOCATION_TYPE_DIRECT )
  {
    exit_type = EXIT_TYPE_NONE;
  }
  else
  {
    EVENTDATA_SYSTEM*   evdata = GAMEDATA_GetEventData( gdata );
    const CONNECT_DATA* cnct   = EVENTDATA_GetConnectByID( evdata, location.exit_id );
    exit_type                  = CONNECTDATA_GetExitType( cnct );
  } 

  // イベント作成
  event = GMEVENT_Create( gsys, parent, eventFuncTable[ exit_type ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->gdata    = gdata;
  work->fieldmap = fieldmap;
  work->location = location;

  // 作成したイベントを返す
  return event;
}


//=======================================================================================
/**
 * @breif 非公開関数の定義
 */
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeNone(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;
	GAMEDATA*        gamedata   = event_work->gdata;

  switch (*seq)
  {
  case 0:
    { // フェードイン
      GMEVENT* fade_event;
      FIELD_STATUS* fstatus; 
      fstatus = GAMEDATA_GetFieldStatus( gamedata );
      if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示あり)
      { // 季節フェード
        fade_event = EVENT_FieldFadeIn_Season(gsys, fieldmap);
      }
      else
      { // クロスフェード
        fade_event = EVENT_FieldFadeIn_Cross(gsys, fieldmap);
      }
      GMEVENT_CallEvent( event, fade_event );
    }
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeDoor(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( gsys, fieldmap, TRUE ) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 階段の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeStep(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;
	GAMEDATA*        gamedata   = event_work->gdata;

  switch (*seq)
  {
  case 0: // 画面・BGMフェードイン
    { // フェードイン
      GMEVENT* fade_event;
      FIELD_STATUS* fstatus; 
      fstatus = GAMEDATA_GetFieldStatus( gamedata );
      if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示あり)
      { // 季節フェード
        fade_event = EVENT_FieldFadeIn_Season(gsys, fieldmap);
      }
      else
      { // クロスフェード
        fade_event = EVENT_FieldFadeIn_Cross(gsys, fieldmap);
      }
      GMEVENT_CallEvent( event, fade_event );
    }
    { // 現在のBGMがダンジョンISS ==> BGMフェードイン
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      BGM_INFO_SYS* bgm_info = GAMEDATA_GetBGMInfoSys( gamedata );
      u8 season = GAMEDATA_GetSeasonID( gamedata );
      u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
      u32 bgm_now = ZONEDATA_GetBGMID( zone_id, season );
      u8 iss_type_now = BGM_INFO_GetIssType( bgm_info, bgm_now ); 
      if( iss_type_now == ISS_TYPE_DUNGEON )
      {
        FIELD_SOUND_FadeInBGM( fsnd, bgm_now, 20 );
      }
    }
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gsys, fieldmap) );
    ++ *seq;
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ワープ時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeWarp(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;
	GAMEDATA*        gamedata   = event_work->gdata;

  switch (*seq)
  {
  case 0:
		GMEVENT_CallEvent(event, EVENT_APPEAR_Teleport(NULL, gsys, fieldmap) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SP1 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP1(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( gsys, fieldmap, FALSE ) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SP2 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP2(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( gsys, fieldmap, FALSE ) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SP3 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP3(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( gsys, fieldmap, FALSE ) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SP4 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP4(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( gsys, fieldmap, FALSE ) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SP5 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSP5(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch (*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( gsys, fieldmap, FALSE ) );
    ++ *seq;
    break;
  case 1:
    { 
      fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), event_work->location.zone_id);
    }
    ++ *seq;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
