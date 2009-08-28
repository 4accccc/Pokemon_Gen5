////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  退場＆登場イベント
 * @file   event_appear.c
 * @author obata
 * @date   2009.08.28
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "event_warp.h"
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
WARP_WORK;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
// イベント処理関数
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Rotate( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Rotate( GMEVENT* event, int* seq, void* work ); 
static GMEVENT_RESULT EVENT_FUNC_APPEAR_RollingJump( GMEVENT* event, int* seq, void* work ); 
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_RollingJump( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* work ); 
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_FallInSand( GMEVENT* event, int* seq, void* work );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( ワープ )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Warp( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  WARP_WORK* work;

  // イベントを作成
  //event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Rotate, sizeof( WARP_WORK ) );
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_RollingJump, sizeof( WARP_WORK ) );

  // イベントワークを初期化
  work            = (WARP_WORK*)GMEVENT_GetEventWork( event );
  work->pFieldmap = fieldmap;
  work->frame     = 0;

  // 作成したイベントを返す
  return event;
}

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
  WARP_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_FallInSand, sizeof( WARP_WORK ) );

  // イベントワークを初期化
  work            = (WARP_WORK*)GMEVENT_GetEventWork( event );
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
  WARP_WORK* work;

  // イベントを作成
  //event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_Rotate, sizeof( WARP_WORK ) );
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_RollingJump, sizeof( WARP_WORK ) );

  // イベントワークを初期化
  work            = (WARP_WORK*)GMEVENT_GetEventWork( event );
  work->pFieldmap = fieldmap;
  work->frame     = 0;

  // 作成したイベントを返す
  return event;
} 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( 落下 )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Fall( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  WARP_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Fall, sizeof( WARP_WORK ) );

  // イベントワークを初期化
  work            = (WARP_WORK*)GMEVENT_GetEventWork( event );
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
  WARP_WORK*    ww     = (WARP_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ww->pFieldmap );

  switch( *seq )
  {
  // 自機回転タスクの追加
  case 0:
    FIELDMAP_TCB_ROT_PLAYER_AddTask_SpeedUp( ww->pFieldmap, 120, 10 );    // 自機回転
    FIELDMAP_TCB_CAMERA_AddTask_Zoom( ww->pFieldmap, ZOOM_IN_FRAME, -ZOOM_IN_DIST ); // ズームイン
    ++( *seq );
    break;
  // フェードアウト開始
  case 1:
    if( 100 < ww->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 1 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 2:
    if( 120 < ww->frame++ )
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
 * @brief 登場イベント処理関数( 回転 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Rotate( GMEVENT* event, int* seq, void* work )
{
  WARP_WORK*        ww = (WARP_WORK*)work;
  GFL_TCBSYS*   tcbsys = FIELDMAP_GetFieldmapTCBSys( ww->pFieldmap );
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( ww->pFieldmap );

  switch( *seq )
  {
  // フェードイン開始
  case 0:
    FIELD_CAMERA_SetAngleLen( camera, FIELD_CAMERA_GetAngleLen( camera ) - ZOOM_IN_DIST );// カメラ初期設定
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 1 );
    ++( *seq );
    break;
  // タスクの追加
  case 1:
    FIELDMAP_TCB_ROT_PLAYER_AddTask_SlowDown( ww->pFieldmap, 120, 10 );   // 自機回転
    FIELDMAP_TCB_CAMERA_AddTask_Zoom( ww->pFieldmap, ZOOM_OUT_FRAME, ZOOM_IN_DIST );  // ズームアウト
    ++( *seq );
    break;
  // タスクの終了待ち
  case 2:
    if( 120 < ww->frame++ )
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
 * @brief 退場イベント処理関数( ジャンプ )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_RollingJump( GMEVENT* event, int* seq, void* work )
{
  WARP_WORK*    ww     = (WARP_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ww->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_WARP_PLAYER_AddTask_DisappearUp( ww->pFieldmap, 60, 300 );   // 自機移動
    FIELDMAP_TCB_ROT_PLAYER_AddTask_SpeedUp( ww->pFieldmap, 30, 10 );         // 自機回転
    ++( *seq );
    break;
  // フェードアウト開始
  case 1:
    if( 30 < ww->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 2:
    if( 60 < ww->frame++ )
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
 * @brief 登場イベント処理関数( ジャンプ )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_RollingJump( GMEVENT* event, int* seq, void* work )
{
  WARP_WORK*    ww     = (WARP_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ww->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_WARP_PLAYER_AddTask_AppearDown( ww->pFieldmap, 40, 300 );  // 自機移動
    FIELDMAP_TCB_ROT_PLAYER_AddTask_SlowDown( ww->pFieldmap, 60, 10 );      // 自機回転
    ++( *seq );
    break;
  // フェードイン開始
  case 1:
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
    ++( *seq );
    break;
  // タスクの終了待ち
  case 2:
    if( 60 < ww->frame++ )
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
 * @brief 登場イベント処理関数( 落下 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* work )
{
  WARP_WORK*    ww     = (WARP_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ww->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_WARP_PLAYER_AddTask_FallIn( ww->pFieldmap, 40, 250 );  // 自機移動
    ++( *seq );
    break;
  // フェードイン開始
  case 1:
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
    ++( *seq );
    break;
  // タスクの終了待ち
  case 2:
    if( 60 < ww->frame++ )
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
  WARP_WORK*    ww     = (WARP_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ww->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_WARP_PLAYER_AddTask_DisappearUp( ww->pFieldmap, 80, -50 );   // 自機移動
    FIELDMAP_TCB_ROT_PLAYER_AddTask( ww->pFieldmap, 80, 10 );   // 回転
    ++( *seq );
    break;
  // フェードアウト開始
  case 1:
    if( 30 < ww->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 2:
    if( 80 < ww->frame++ )
    {
      ++( *seq );
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
