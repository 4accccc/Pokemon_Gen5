/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   event_entrance_in.c
 * @breif  出入口への進入時のイベント作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h> 
#include "gamesystem/game_event.h"
#include "gamesystem/iss_sys.h"
#include "gamesystem/iss_dungeon_sys.h"
#include "sound/pm_sndsys.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h"
#include "field_comm/intrude_snd_def.h"

#include "fieldmap.h"
#include "field_sound.h"
#include "event_entrance_in.h" 
#include "event_fieldmap_control.h"  // for EVENT_FieldFadeOut_xxxx
#include "event_entrance_effect.h"   // for EVENT_FieldDoorInAnime
#include "event_fldmmdl_control.h"   // for PlayerOneStepStart, CheckPlayerOneStepEnd
#include "event_disappear.h"         // for EVENT_DISAPPEAR_xxxx
#include "entrance_event_common.h"   // for ENTRANCE_EVDATA



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
// ■イベント ワーク
//=======================================================================================
typedef struct {

  FIELD_CAMERA_MODE initCameraMode;  // イベント開始時のカメラモード
  GFL_TCB*          oneStepTCB;      // 自機の一歩移動TCB

  // 出入り口イベントの共通ワーク
  ENTRANCE_EVDATA* evdata; 

} EVENT_WORK;


//=======================================================================================
// ■index
//======================================================================================= 
// 各 EXIT_TYPE ごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSPx( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeIntrude( GMEVENT * event, int *seq, void * wk );
// BGM
static BOOL CheckBGMFadeByMapChangeAtStep( const EVENT_WORK* work );
static BOOL CheckDungeonISSValidityForMapChange( const EVENT_WORK* work );
static PLAYER_MOVE_FORM GetPlayerMoveForm( const EVENT_WORK* work );
static void StandByNextBGM( EVENT_WORK* work );
static void CallBGMFadeOutEvent( EVENT_WORK* work, GMEVENT* parentEvent );
// 画面のフェードアウト
static void CallFadeOutEvent( const EVENT_WORK* work, GMEVENT* parentEvent );

//=======================================================================================
// ■public func
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口進入イベントを作成する
 *
 * @param evdata 出入り口イベントの共通データ
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceIn( ENTRANCE_EVDATA* evdata )
{
  GMEVENT* event;
  EVENT_WORK* work;
  EVENTFUNC_TYPE funcType;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[ EVENTFUNC_TYPE_NUM ] = 
  {
    EVENT_FUNC_EntranceIn_ExitTypeNone,   // EVENTFUNC_TYPE_NONE ドアなし
    EVENT_FUNC_EntranceIn_ExitTypeDoor,   // EVENTFUNC_TYPE_DOOR ドアあり
    EVENT_FUNC_EntranceIn_ExitTypeStep,   // EVENTFUNC_TYPE_STEP 階段
    EVENT_FUNC_EntranceIn_ExitTypeWarp,   // EVENTFUNC_TYPE_WARP ワープ
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EVENTFUNC_TYPE_SPx, 特殊
    EVENT_FUNC_EntranceIn_ExitTypeIntrude,// EVENTFUNC_TYPE_INTRUDE 侵入
  };

  // イベント処理関数を決定
  switch( evdata->exit_type_in ) {
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
  event = GMEVENT_Create( evdata->gameSystem, evdata->parentEvent, 
      eventFuncTable[ funcType ], sizeof( EVENT_WORK ) );

  // イベントワーク初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->oneStepTCB = NULL; 
  work->evdata     = evdata;

  return event;
}


//=======================================================================================
// ■private func
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;

  switch ( *seq ) {
  case 0:
    // BGM 変更準備
    StandByNextBGM( work );

    // SE 再生
    if( evdata->fadeout_type != FIELD_FADE_CROSS ) { PMSND_PlaySE( SEQ_SE_KAIDAN ); }

    // 画面フェードアウト開始
    CallFadeOutEvent( work, event );

    (*seq)++; 
    break;

  case 1:
    return GMEVENT_RES_FINISH; // イベント終了
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;

  switch( *seq ) {
  case 0:
    // ドア進入イベント
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( evdata ) );
    (*seq)++;
    break;

  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 階段の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work     = wk;
  ENTRANCE_EVDATA* evdata   = work->evdata;
	FIELDMAP_WORK*   fieldmap = evdata->fieldmap;

  switch ( *seq ) {
  case 0:  
    // BGM 操作
    if( CheckBGMFadeByMapChangeAtStep(work) ) {
      CallBGMFadeOutEvent( work, event );
    }
    else {
      StandByNextBGM( work );
    }

    // 自機の一歩移動アニメ開始
    work->oneStepTCB = PlayerOneStepAnimeStart( fieldmap );

    (*seq)++;
    break;

  case 1:
    // 自機の一歩移動アニメが終了
    if( CheckPlayerOneStepAnimeEnd( work->oneStepTCB ) ) {
      // クロスフェードでなければ, SEを再生
      if( evdata->fadeout_type != FIELD_FADE_CROSS ) { PMSND_PlaySE( SEQ_SE_KAIDAN ); }
      // 画面フェードアウト開始
      CallFadeOutEvent( work, event ); 
      (*seq)++; 
    }
    break;

  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ワープ時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  switch( *seq ) {
  case 0:
    // ワープ退出イベント
		GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Warp( NULL, gameSystem, fieldmap ) );
    (*seq)++;
    break;

  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SPx )
 *
 * ※カメラのアニメーション → ドア進入アニメ
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSPx( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;
  FIELD_CAMERA*    camera     = FIELDMAP_GetFieldCamera( fieldmap );

  // 処理シーケンス
  enum {
    SEQ_INIT,               // イベント開始
    SEQ_WAIT_CAMERA_TRACE,  // カメラの自機追従処理の終了待ち
    SEQ_DOOR_IN_ANIME,      // ドア進入イベント
    SEQ_EXIT,               // イベント終了
  };

  switch( *seq ) {
  case SEQ_INIT:
    // トレースシステムが有効 and トレース処理中
    if( FIELD_CAMERA_CheckTraceSys(camera) && FIELD_CAMERA_CheckTrace(camera) ) {
        FIELD_CAMERA_StopTraceRequest( camera );
        *seq = SEQ_WAIT_CAMERA_TRACE;
    }
    else {
      *seq = SEQ_DOOR_IN_ANIME;
    }
    return GMEVENT_RES_CONTINUE_DIRECT;

  // カメラのトレース処理終了待ち
  case SEQ_WAIT_CAMERA_TRACE: 
    if( FIELD_CAMERA_CheckTraceSys( camera ) == FALSE ) { 
      *seq = SEQ_DOOR_IN_ANIME; 
      return GMEVENT_RES_CONTINUE_DIRECT;
    }
    else if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ) {
      FIELD_CAMERA_FreeTarget( camera );
      *seq = SEQ_DOOR_IN_ANIME;
      return GMEVENT_RES_CONTINUE_DIRECT;
    }
    break;

  // ドア進入アニメ
  case SEQ_DOOR_IN_ANIME:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( evdata ) );
    *seq = SEQ_EXIT;
    return GMEVENT_RES_CONTINUE_DIRECT;

  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//---------------------------------------------------------------------------------------
/**
 * @breif 侵入時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeIntrude( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  switch ( *seq ) {
  case 0:
    // BGM 変更準備
    StandByNextBGM( work );
    PMSND_PlaySE( INTSE_WARP );
    // 画面フェードアウト開始
    GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Cross( gameSystem, fieldmap ) );
    (*seq)++;
    break;

  case 1:
    return GMEVENT_RES_FINISH; // イベント終了
  }
  return GMEVENT_RES_CONTINUE;
}


//---------------------------------------------------------------------------------------
/**
 * @brief 階段によるマップチェンジで BGM をフェードアウトさせるかどうかを判定する
 */
//---------------------------------------------------------------------------------------
static BOOL CheckBGMFadeByMapChangeAtStep( const EVENT_WORK* work )
{ 
  const ENTRANCE_EVDATA* evdata = work->evdata;
  GAMEDATA* gameData = evdata->gameData;

  if( CheckDungeonISSValidityForMapChange( work ) == FALSE ) {
    return FALSE; // ダンジョンISSが無関係
  }

  if( FSND_CheckValidSpecialBGM( gameData, evdata->nextLocation.zone_id ) ) {
    return FALSE; // 特殊BGMあり
  }

  if( (GetPlayerMoveForm( work ) == PLAYER_MOVE_FORM_CYCLE) &&
      (FSND_GetLastBGM( gameData ) == SEQ_BGM_BICYCLE) ) {
    return FALSE; // 自転車に乗っていて, 自転車曲が再生中or再生予定
  }

  return TRUE;
}

/**
 * @brief マップチェンジ中のダンジョンISSの有効性をチェックする
 */
static BOOL CheckDungeonISSValidityForMapChange( const EVENT_WORK* work )
{
  const ENTRANCE_EVDATA* evdata = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
  ISS_SYS*         iss        = GAMESYSTEM_GetIssSystem( gameSystem );
  ISS_DUNGEON_SYS* issD       = ISS_SYS_GetIssDungeonSystem( iss );

  // 遷移の前後ともにISS-Dの制御が有効
  if( ISS_DUNGEON_SYS_IsOn( issD ) &&
      ISS_DUNGEON_SYS_IsActiveAt(issD, evdata->prev_zone_id) && 
      ISS_DUNGEON_SYS_IsActiveAt(issD, evdata->nextLocation.zone_id) ) {
    return TRUE;
  }

  return FALSE;
}

/**
 * @brief 自機の移動フォームを取得する
 */
static PLAYER_MOVE_FORM GetPlayerMoveForm( const EVENT_WORK* work )
{
  const FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->evdata->fieldmap );
  return FIELD_PLAYER_GetMoveForm( player );
}


//---------------------------------------------------------------------------------------
/**
 * @brief BGM の変更準備
 *
 * @param work
 */
//---------------------------------------------------------------------------------------
static void StandByNextBGM( EVENT_WORK* work )
{
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMEDATA*        gameData   = evdata->gameData;
  FIELD_SOUND*     fieldSound = GAMEDATA_GetFieldSound( gameData );

  FSND_StandByNextMapBGM( fieldSound, gameData, evdata->nextLocation.zone_id, evdata->end_season );

  evdata->BGM_standby_flag = TRUE;
}

//---------------------------------------------------------------------------------------
/**
 * @brief BGM フェードアウトイベントを呼ぶ
 *
 * @param work
 * @param parentEvent
 */
//---------------------------------------------------------------------------------------
static void CallBGMFadeOutEvent( EVENT_WORK* work, GMEVENT* parentEvent )
{
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem; 
  GMEVENT* event;

  event = EVENT_FSND_FadeOutBGM( gameSystem, FSND_FADE_SHORT );
  GMEVENT_CallEvent( parentEvent, event );

  evdata->BGM_fadeout_flag = TRUE;
}

//---------------------------------------------------------------------------------------
/**
 * @brief 画面のフェードアウトイベントを呼び出す
 *
 * @param work
 * @param parentEvent
 */
//---------------------------------------------------------------------------------------
static void CallFadeOutEvent( const EVENT_WORK* work, GMEVENT* parentEvent )
{
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  // 季節フェード
  if( evdata->season_disp_flag ) { 
    GMEVENT_CallEvent( parentEvent, 
        EVENT_FieldFadeOut_Season( gameSystem, fieldmap, FIELD_FADE_WAIT ) );
  }
  // 基本フェード
  else { 
    GMEVENT_CallEvent( parentEvent, 
        EVENT_FieldFadeOut( gameSystem, fieldmap, evdata->fadeout_type, FIELD_FADE_WAIT ) );
  }
}
