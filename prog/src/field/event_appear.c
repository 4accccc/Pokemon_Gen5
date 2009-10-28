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
#include "event_appear.h"
#include "fieldmap.h"
#include "field_player.h"
#include "fieldmap_tcb.h"
#include "fldeff_kemuri.h"
#include "sound/pm_sndsys.h"


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
// 登場イベント処理関数
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Rotate( GMEVENT* event, int* seq, void* work );      // 回転
static GMEVENT_RESULT EVENT_FUNC_APPEAR_RollingFall( GMEVENT* event, int* seq, void* work ); // 回転落下
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* work );        // 落下


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
  EVENT_WORK* work;

  // イベントを作成
  //event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Rotate, sizeof( EVENT_WORK ) );
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_RollingFall, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work            = (EVENT_WORK*)GMEVENT_GetEventWork( event );
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
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Fall, sizeof( EVENT_WORK ) );

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
 * @brief 登場イベント処理関数( 回転 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Rotate( GMEVENT* event, int* seq, void* work )
{
  EVENT_WORK*       ew = (EVENT_WORK*)work;
  GFL_TCBSYS*   tcbsys = FIELDMAP_GetFieldmapTCBSys( ew->pFieldmap );
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( ew->pFieldmap );

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
    FIELDMAP_TCB_AddTask_RotatePlayer_SpeedDown( ew->pFieldmap, 120, 10 );   // 自機回転
    FIELDMAP_TCB_AddTask_CameraZoom( ew->pFieldmap, ZOOM_OUT_FRAME, ZOOM_IN_DIST );  // ズームアウト
    ++( *seq );
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
 * @brief 登場イベント処理関数( 回転落下 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_RollingFall( GMEVENT* event, int* seq, void* work )
{
  EVENT_WORK*       ew = (EVENT_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ew->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_AddTask_AppearPlayer_LinearDown( ew->pFieldmap, 40, 300 );  // 自機移動
    FIELDMAP_TCB_AddTask_RotatePlayer_SpeedDown( ew->pFieldmap, 60, 10 );    // 自機回転
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
 * @brief 登場イベント処理関数( 落下 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* work )
{
  EVENT_WORK*       ew = (EVENT_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( ew->pFieldmap );

  switch( *seq )
  {
  // タスクの追加
  case 0:
    FIELDMAP_TCB_AddTask_AppearPlayer_Fall( ew->pFieldmap, 40, 250 );  // 自機移動
    ++( *seq );
    break;
  // フェードイン開始
  case 1:
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
    // SE再生(落下音)
    PMSND_PlaySE( SEQ_SE_FLD_16 );
    ++( *seq );
    break;
  // タスクの終了待ち
  case 2:
    if( 40 < ew->frame++ )
    {
      // 砂埃を出す
      MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player );
      FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( ew->pFieldmap );
      FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
      // SE再生(着地音)
      PMSND_PlaySE( SEQ_SE_FLD_17 );
      ++( *seq );
    }
    break;
  // 砂埃
  case 3: 
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
