////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  退場イベント
 * @file   event_disappear.c
 * @author obata
 * @date   2009.08.28
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "event_disappear.h"
#include "fieldmap.h"
#include "field_player.h"
#include "fieldmap_tcb.h"


//==========================================================================================
/**
 * @brief 定数
 */
//==========================================================================================

#define ZOOM_IN_DIST   (50 << FX32_SHIFT)   // カメラのズームイン距離
#define ZOOM_IN_FRAME  (60)   // ズームインに掛かるフレーム数
#define ZOOM_OUT_FRAME (60)   // ズームアウトに掛かるフレーム数


//==========================================================================================
/**
 * @brief イベントワーク
 */
//==========================================================================================
typedef struct
{
  int                frame;   // フレーム数カウンタ
  FIELDMAP_WORK* pFieldmap;   // 動作フィールドマップ
}
EVENT_WORK;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
// イベント処理関数
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Rotate( GMEVENT* event, int* seq, void* work );      // 回転
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_RollingJump( GMEVENT* event, int* seq, void* work ); // 回転ジャンプ
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_FallInSand( GMEVENT* event, int* seq, void* work );  // 流砂


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( 流砂 )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_FallInSand( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_FallInSand, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work            = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->pFieldmap = fieldmap;
  work->frame     = 0;

  // 作成したイベントを返す
  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( ワープ )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_Warp( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  //event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_Rotate, sizeof( EVENT_WORK ) );
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_RollingJump, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work            = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->pFieldmap = fieldmap;
  work->frame     = 0;

  // 作成したイベントを返す
  return event;
} 


//========================================================================================== 
/**
 * @brief 非公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベント処理関数( 回転 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Rotate( GMEVENT* event, int* seq, void* work )
{
  EVENT_WORK*       ew = (EVENT_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ew->pFieldmap );

  switch( *seq )
  {
  // 自機回転タスクの追加
  case 0:
    FIELDMAP_TCB_AddTask_RotatePlayer_SpeedUp( ew->pFieldmap, 120, 10 );    // 自機回転
    FIELDMAP_TCB_CAMERA_AddTask_Zoom( ew->pFieldmap, ZOOM_IN_FRAME, -ZOOM_IN_DIST ); // ズームイン
    ++( *seq );
    break;
  // フェードアウト開始
  case 1:
    if( 100 < ew->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 1 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 2:
    if( 120 < ew->frame++ )
    {
      ++( *seq );
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベント処理関数( 回転ジャンプ )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_RollingJump( GMEVENT* event, int* seq, void* work )
{
  EVENT_WORK*       ew = (EVENT_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ew->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_WARP_PLAYER_AddTask_DisappearUp( ew->pFieldmap, 60, 300 );   // 自機移動
    FIELDMAP_TCB_AddTask_RotatePlayer_SpeedUp( ew->pFieldmap, 30, 10 );         // 自機回転
    ++( *seq );
    break;
  // フェードアウト開始
  case 1:
    if( 30 < ew->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 2:
    if( 60 < ew->frame++ )
    {
      ++( *seq );
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベント処理関数( 流砂 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_FallInSand( GMEVENT* event, int* seq, void* work )
{
  EVENT_WORK*       ew = (EVENT_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ew->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_WARP_PLAYER_AddTask_DisappearUp( ew->pFieldmap, 80, -50 );   // 自機移動
    FIELDMAP_TCB_AddTask_RotatePlayer( ew->pFieldmap, 80, 10 );   // 回転
    ++( *seq );
    break;
  // フェードアウト開始
  case 1:
    if( 30 < ew->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 2:
    if( 80 < ew->frame++ )
    {
      ++( *seq );
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
