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
// ■イベントワーク
//=======================================================================================
typedef struct
{
  GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
  FIELDMAP_WORK* fieldmap;
  LOCATION       location;           // 遷移先指定
  BOOL           seasonDisplayFlag;  // 季節表示を行うかどうか
  u8             startSeason;        // 最初に表示する季節
  u8             endSeason;          // 最後に表示する季節
} EVENT_WORK;


//=======================================================================================
// ■非公開関数のプロトタイプ宣言
//======================================================================================= 
// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeNone( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeDoor( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeStep( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeWarp( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSPx( GMEVENT* event, int* seq, void* wk );


//=======================================================================================
// ■公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口退出イベントを作成する
 *
 * @param parent            親イベント
 * @param gameSystem
 * @param gameData
 * @param fieldmap
 * @param location          遷移先指定
 * @param seasonDisplayFlag 季節表示を行うかどうか
 * @param startSeason       最初に表示する季節
 * @param endtSeason        最後に表示する季節
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceOut( GMEVENT* parent, 
                            GAMESYS_WORK* gameSystem,
                            GAMEDATA* gameData, 
                            FIELDMAP_WORK* fieldmap, 
                            LOCATION location,
                            BOOL seasonDisplayFlag,
                            u8 startSeason,
                            u8 endSeason )
{
  GMEVENT* event;
  EVENT_WORK* work; 
  EXIT_TYPE exitType; 

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[] = {
    EVENT_FUNC_EntranceOut_ExitTypeNone,   //EXIT_TYPE_NONE
    EVENT_FUNC_EntranceOut_ExitTypeNone,   //EXIT_TYPE_MAT
    EVENT_FUNC_EntranceOut_ExitTypeStep,   //EXIT_TYPE_STAIRS
    EVENT_FUNC_EntranceOut_ExitTypeDoor,   //EXIT_TYPE_DOOR
    EVENT_FUNC_EntranceOut_ExitTypeStep,   //EXIT_TYPE_WALL
    EVENT_FUNC_EntranceOut_ExitTypeWarp,   //EXIT_TYPE_WARP
    EVENT_FUNC_EntranceOut_ExitTypeNone,   //EXIT_TYPE_INTRUDE
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP1
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP2
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP3
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP4
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP5
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP6
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP7
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP8
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP9
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP10
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP11
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP12
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP13
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP14
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP15
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP16
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP17
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP18
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP19
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP20
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP21
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP22
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP23
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP24
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP25
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP26
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP27
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP28
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP29
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    //EXIT_TYPE_SP30
  };

  // EXIT_TYPEを決定
  if( location.type == LOCATION_TYPE_DIRECT )
  {
    exitType = EXIT_TYPE_NONE;
  }
  else
  {
    EVENTDATA_SYSTEM* eventData;
    const CONNECT_DATA* connectData;

    eventData   = GAMEDATA_GetEventData( gameData );
    connectData = EVENTDATA_GetConnectByID( eventData, location.exit_id );
    exitType    = CONNECTDATA_GetExitType( connectData );
  } 

  // イベント作成
  event = GMEVENT_Create( gameSystem, parent, eventFuncTable[ exitType ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work                     = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem         = gameSystem;
  work->gameData           = gameData;
  work->fieldmap           = fieldmap;
  work->location           = location;
  work->seasonDisplayFlag  = seasonDisplayFlag;
  work->startSeason        = startSeason;
  work->endSeason          = endSeason;

  return event;
}


//=======================================================================================
// ■非公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeNone( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch( *seq )
  {
  case 0:
    // BGM再生開始
    FSND_PlayStartBGM( fieldSound );
    (*seq)++;
    break;
  case 1:
    if( work->seasonDisplayFlag )
    { // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else
    { // クロスフェード
      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Cross( gameSystem, fieldmap ) );
    }
    (*seq)++;
    break;
  case 2:
    // 地名表示更新リクエスト
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeDoor( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // ドア退出イベント
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, TRUE, 
                                 work->seasonDisplayFlag, work->startSeason, work->endSeason ) );
    (*seq)++;
    break;
  case 1:
    // 地名表示更新リクエスト
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    (*seq)++;
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
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeStep( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch( *seq )
  {
  case 0: 
    // 画面フェードイン
    if( work->seasonDisplayFlag )
    { // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else
    { // クロスフェード
      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Cross( gameSystem, fieldmap ) );
    }
    (*seq)++;
    break;
  case 1:  // BGMフェードイン
    { // 現在のBGMがダンジョンISS ==> BGMフェードイン
      BGM_INFO_SYS* bgm_info = GAMEDATA_GetBGMInfoSys( gameData );
      u8 season = GAMEDATA_GetSeasonID( gameData );
      u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
      u32 bgm_now = ZONEDATA_GetBGMID( zone_id, season );
      u8 iss_type_now = BGM_INFO_GetIssType( bgm_info, bgm_now ); 
      if( iss_type_now == ISS_TYPE_DUNGEON )
      {
        FIELD_SOUND* fieldSound = GAMEDATA_GetFieldSound( gameData );
        GMEVENT* fadeInEvent = EVENT_FSND_FadeInBGM( gameSystem, FSND_FADE_SHORT );
        GMEVENT_CallEvent( event, fadeInEvent );
      }
      else
      {
        FSND_PlayStartBGM( fieldSound );
      }
    }
    (*seq)++;
    break;
  case 2:
    FIELD_PLACE_NAME_Display(FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id);
    (*seq)++;
    break;
  case 3:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gameSystem, fieldmap) );
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ワープ時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeWarp( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;

  switch( *seq )
  {
  case 0:
    // ワープ出現イベント
		GMEVENT_CallEvent(event, EVENT_APPEAR_Warp( NULL, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1:
    // 地名表示更新リクエスト
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    (*seq)++;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SPx )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSPx( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // ドア退出イベント
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, FALSE, 
                                 work->seasonDisplayFlag, work->startSeason, work->endSeason ) );
    (*seq)++;
    break;
  case 1:
    // 地名表示更新リクエスト
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    (*seq)++;
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
