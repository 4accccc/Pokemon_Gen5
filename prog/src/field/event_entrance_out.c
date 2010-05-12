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
#include <gflib.h>
#include "gamesystem/game_event.h"
#include "gamesystem/iss_sys.h"
#include "gamesystem/iss_dungeon_sys.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h" 
#include "field/eventdata_sxy.h"
#include "field/zonedata.h"            // for ZONEDATA_GetBGMID
#include "sound/pm_sndsys.h"
#include "sound/bgm_info.h"

#include "field_sound.h" 
#include "event_appear.h"              // for EVENT_APPEAR_xxxx
#include "event_fieldmap_control.h"    // for EVENT_FieldFadeIn
#include "event_entrance_effect.h"     // for EVENT_FieldDoorOutAnime
#include "event_fldmmdl_control.h"     // for EVENT_PlayerOneStepAnime
#include "field_place_name.h"          // for FIELD_PLACE_NAME_Display
#include "fieldmap.h"                  // for FIELDMAP_GetPlaceNameSys

#include "event_entrance_out.h"



//=======================================================================================
// ■定数
//=======================================================================================
// イベント処理関数のタイプ
typedef enum {
  EVENTFUNC_TYPE_NONE,    // ドアなし
  EVENTFUNC_TYPE_DOOR,    // ドアあり
  EVENTFUNC_TYPE_STEP,    // 階段
  EVENTFUNC_TYPE_WARP,    // ワープ
  EVENTFUNC_TYPE_SPx,     // 特殊
  EVENTFUNC_TYPE_INTRUDE, // 侵入
  EVENTFUNC_TYPE_NUM,
} EVENTFUNC_TYPE;


//=======================================================================================
// ■イベントワーク
//=======================================================================================
typedef struct {

  GAMESYS_WORK*     gameSystem;
  GAMEDATA*         gameData;
  FIELDMAP_WORK*    fieldmap;
  LOCATION          location;          // 遷移先指定
  u16               prevZoneID;        // 遷移前のゾーン
  EXIT_TYPE         exitType;          // 出入り口タイプ
  BOOL              seasonDisplayFlag; // 季節表示を行うかどうか
  u8                startSeason;       // 最初に表示する季節
  u8                endSeason;         // 最後に表示する季節
  FIELD_FADE_TYPE   fadeInType;        // 季節表示がない場合のF/Iタイプ

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
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeIntrude( GMEVENT* event, int* seq, void* wk );


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
 * @param prevZoneID        遷移前のゾーン
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
                            u16 prevZoneID,
                            BOOL seasonDisplayFlag,
                            u8 startSeason,
                            u8 endSeason )
{
  GMEVENT* event;
  EVENT_WORK* work; 
  EXIT_TYPE exitType; 
  EVENTFUNC_TYPE funcType;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[ EVENTFUNC_TYPE_NUM ] = 
  {
    EVENT_FUNC_EntranceOut_ExitTypeNone,   // EVENTFUNC_TYPE_NONE ドアなし
    EVENT_FUNC_EntranceOut_ExitTypeDoor,   // EVENTFUNC_TYPE_DOOR ドアあり
    EVENT_FUNC_EntranceOut_ExitTypeStep,   // EVENTFUNC_TYPE_STEP 階段
    EVENT_FUNC_EntranceOut_ExitTypeWarp,   // EVENTFUNC_TYPE_WARP ワープ
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    // EVENTFUNC_TYPE_SPx, 特殊
    EVENT_FUNC_EntranceOut_ExitTypeIntrude, // EVENTFUNC_TYPE_INTRUDE 侵入
  };

  // EXIT_TYPEを決定
  if( location.type == LOCATION_TYPE_DIRECT ) {
    exitType = EXIT_TYPE_NONE;
  }
  else {
    EVENTDATA_SYSTEM* eventData;
    const CONNECT_DATA* connectData;

    eventData   = GAMEDATA_GetEventData( gameData );
    connectData = EVENTDATA_GetConnectByID( eventData, location.exit_id );
    exitType    = CONNECTDATA_GetExitType( connectData );
  } 

  // イベント処理関数を決定
  switch( exitType ) {
  case EXIT_TYPE_NONE:    funcType = EVENTFUNC_TYPE_NONE;    break;
  case EXIT_TYPE_MAT:     funcType = EVENTFUNC_TYPE_NONE;    break;
  case EXIT_TYPE_STAIRS:  funcType = EVENTFUNC_TYPE_STEP;    break;
  case EXIT_TYPE_DOOR:    funcType = EVENTFUNC_TYPE_DOOR;    break;
  case EXIT_TYPE_WALL:    funcType = EVENTFUNC_TYPE_STEP;    break;
  case EXIT_TYPE_WARP:    funcType = EVENTFUNC_TYPE_WARP;    break;
  case EXIT_TYPE_INTRUDE: funcType = EVENTFUNC_TYPE_INTRUDE; break;
  default:                funcType = EVENTFUNC_TYPE_SPx;     break;
  }

  // イベント作成
  event = GMEVENT_Create( gameSystem, parent, eventFuncTable[ funcType ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work                    = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem        = gameSystem;
  work->gameData          = gameData;
  work->fieldmap          = fieldmap;
  work->prevZoneID        = prevZoneID;
  work->location          = location;
  work->exitType          = exitType;
  work->seasonDisplayFlag = seasonDisplayFlag;
  work->startSeason       = startSeason;
  work->endSeason         = endSeason;
  work->fadeInType        = FIELD_FADE_GetFadeInType( prevZoneID, location.zone_id );

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

  switch( *seq ) {
  // カメラのセットアップ
  case 0:
    (*seq)++;
    break;

  // BGM再生開始
  case 1:
    FSND_PlayStartBGM( fieldSound, gameData, work->location.zone_id );
    (*seq)++;
    break;

  // 画面フェード開始
  case 2:
    if( work->seasonDisplayFlag ) { 
      // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else {
      // 基本フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn( gameSystem, fieldmap, work->fadeInType, FIELD_FADE_WAIT, TRUE, 0, 0 ) );
    }
    (*seq)++;
    break;

  // イベント終了
  case 3:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
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

  switch( *seq ) {
  case 0:
    // ドア退出イベント
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, TRUE, work->seasonDisplayFlag, 
                                 work->startSeason, work->endSeason, work->fadeInType,
                                 work->exitType ) );
    (*seq)++;
    break;

  // イベント終了
  case 1:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
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

  switch( *seq ) {
  // 画面フェードイン開始
  case 0: 
    if( work->seasonDisplayFlag ) { 
      // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else { 
      // クロスフェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn( gameSystem, fieldmap, work->fadeInType, FIELD_FADE_WAIT, TRUE, 0, 0 ) );
    }
    (*seq)++;
    break;

  // BGM 操作
  case 1:  
    {
      ISS_SYS* iss = GAMESYSTEM_GetIssSystem( gameSystem );
      ISS_DUNGEON_SYS* issD = ISS_SYS_GetIssDungeonSystem( iss );
      // 遷移の前後ともにISS-Dの制御が有効
      if( ISS_DUNGEON_SYS_IsActiveAt(issD, work->prevZoneID) && 
          ISS_DUNGEON_SYS_IsActiveAt(issD, work->location.zone_id) ) {
        // フェードイン
        GMEVENT_CallEvent( event, EVENT_FSND_FadeInBGM( gameSystem, FSND_FADE_FAST ) );
      }
      else {
        FSND_PlayStartBGM( fieldSound, gameData, work->location.zone_id );
      }
    }
    (*seq)++;
    break;

  // 自機の一歩移動アニメ
  case 2:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gameSystem, fieldmap) );
    (*seq)++;
    break;

  // イベント終了
  case 3:
    FIELD_PLACE_NAME_Display(
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
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

  switch( *seq ) {
  // ワープ出現イベント
  case 0:
		GMEVENT_CallEvent(event, EVENT_APPEAR_Warp( NULL, gameSystem, fieldmap ) );
    (*seq)++;
    break;

  // イベント終了
  case 1:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
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
  FIELD_CAMERA*  camera     = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq ) {
  // ドア退出イベント
  case 0:
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, FALSE, 
                                 work->seasonDisplayFlag, work->startSeason, work->endSeason, work->fadeInType, work->exitType ) );
    (*seq)++;
    break;

  // 地名表示更新リクエスト発行
  case 1:
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    (*seq)++;
    break;

  // イベント終了
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//---------------------------------------------------------------------------------------
/**
 * @breif 侵入時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeIntrude( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch( *seq ) {
  // カメラのセットアップ
  case 0:
    (*seq)++;
    break;

  // BGM再生開始
  case 1:
    FSND_PlayStartBGM( fieldSound, gameData, work->location.zone_id );
    (*seq)++;
    break;

  // 画面フェード開始
  case 2:
    // 基本フェード
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Cross( gameSystem, fieldmap ) );
    (*seq)++;
    break;

  // イベント終了
  case 3:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
