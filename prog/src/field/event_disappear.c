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
#include "fldeff_kemuri.h"
#include "fldeff_gyoe.h"
#include "sound/pm_sndsys.h"
#include "field_task.h"
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_player_rot.h"
#include "field_task_player_trans.h"
#include "field_task_player_drawoffset.h"


//==========================================================================================
// ■定数
//========================================================================================== 
#define ZOOM_IN_DIST   (50 << FX32_SHIFT)   // カメラのズームイン距離
#define ZOOM_IN_FRAME  (60)   // ズームインに掛かるフレーム数
#define ZOOM_OUT_FRAME (60)   // ズームアウトに掛かるフレーム数


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct
{
  int                    frame;   // フレーム数カウンタ
  FIELDMAP_WORK*     pFieldmap;   // 動作フィールドマップ
  FIELD_CAMERA_MODE cameraMode;   // イベント開始時のカメラモード
  VecFx32        sandStreamPos;   // 流砂中心部の位置( 流砂イベントのみ使用 )

} EVENT_WORK;


//==========================================================================================
// ■プロトタイプ宣言
//========================================================================================== 
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Rotate( GMEVENT* event, int* seq, void* work );      // 回転
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_FallInSand( GMEVENT* event, int* seq, void* work );  // 流砂


//========================================================================================== 
// ■公開関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( 流砂 )
 *
 * @param parent     親イベント
 * @param gsys       ゲームシステム
 * @param fieldmap   フィールドマップ
 * @param stream_pos 流砂中心部の座標
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_FallInSand( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, const VecFx32* stream_pos )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_FallInSand, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work            = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->pFieldmap = fieldmap;
  work->frame     = 0;
  VEC_Set( &work->sandStreamPos, stream_pos->x, stream_pos->y, stream_pos->z );

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
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( ew->pFieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    ew->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    // タスク登録
    { 
      FIELD_TASK* rot;
      FIELD_TASK* zoom;
      FIELD_TASK_MAN* man;
      rot  = FIELD_TASK_PlayerRotate_SpeedUp( ew->pFieldmap, 120, 10 );
      zoom = FIELD_TASK_CameraLinearZoom( ew->pFieldmap, ZOOM_IN_FRAME, -ZOOM_IN_DIST );
      man = FIELDMAP_GetTaskManager( ew->pFieldmap );
      FIELD_TASK_MAN_AddTask( man, rot, NULL );
      FIELD_TASK_MAN_AddTask( man, zoom, NULL );
    }
    ++( *seq );
    break;
  case 1:
    if( 100 < ew->frame++ )
    { // フェードアウト開始
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 1 );
      ++( *seq );
    }
    break;
  case 2:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( ew->pFieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  // カメラモードの復帰
  case 3:
    FIELD_CAMERA_ChangeMode( camera, ew->cameraMode );
    ++( *seq );
    break;
  case 4:
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
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player );
  FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( ew->pFieldmap );
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( ew->pFieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    ew->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    { // タスク登録
      FIELD_TASK* move;
      FIELD_TASK_MAN* man;
      move = FIELD_TASK_TransPlayer( ew->pFieldmap, 120, &ew->sandStreamPos );
      man = FIELDMAP_GetTaskManager( ew->pFieldmap );
      FIELD_TASK_MAN_AddTask( man, move, NULL );
    }
    // びっくりマーク表示
    FLDEFF_GYOE_SetMMdlNonDepend( fectrl, mmdl, FLDEFF_GYOETYPE_GYOE, TRUE );
    PMSND_PlaySE( SEQ_SE_FLD_15 );
    ew->frame = 0;
    ++( *seq );
    break;
  // タスク終了待ち&砂埃
  case 1:
    { // 向きを変える
      int key = GFL_UI_KEY_GetCont();
      if( key & PAD_KEY_UP )    MMDL_SetAcmd( mmdl, AC_STAY_WALK_U_4F );
      if( key & PAD_KEY_DOWN )  MMDL_SetAcmd( mmdl, AC_STAY_WALK_D_4F );
      if( key & PAD_KEY_LEFT )  MMDL_SetAcmd( mmdl, AC_STAY_WALK_L_4F );
      if( key & PAD_KEY_RIGHT ) MMDL_SetAcmd( mmdl, AC_STAY_WALK_R_4F );
    }
    // 砂埃
    if( ew->frame++ % 10 == 0 )
    {
      FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
    }
    // タスク終了チェック
    {
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( ew->pFieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        // 影を消す
        MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );     
        // タスク登録
        { 
          VecFx32 vec;
          FIELD_TASK* move;
          FIELD_TASK* rot;
          FIELD_TASK* zoom;
          FIELD_TASK_MAN* man;
          VEC_Set( &vec, 0, -50<<FX32_SHIFT, 0 );
          move = FIELD_TASK_TransDrawOffset( ew->pFieldmap, 80, &vec );
          rot  = FIELD_TASK_PlayerRotate( ew->pFieldmap, 80, 10 );
          zoom = FIELD_TASK_CameraLinearZoom( ew->pFieldmap, 30, -50<<FX32_SHIFT );
          man = FIELDMAP_GetTaskManager( ew->pFieldmap );
          FIELD_TASK_MAN_AddTask( man, move, NULL );
          FIELD_TASK_MAN_AddTask( man, rot, NULL );
          FIELD_TASK_MAN_AddTask( man, zoom, NULL );
        }
        ew->frame = 0;
        ++( *seq );
      }
    }
    break;
  // フェードアウト開始
  case 2:
    if( 30 < ew->frame++ )
    {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0 );
      ++( *seq );
    }
    break;
  // タスクの終了待ち
  case 3:
    { 
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( ew->pFieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  // カメラモードの復帰
  case 4:
    FIELD_CAMERA_ChangeMode( camera, ew->cameraMode );
    ++( *seq );
    break;
  case 5:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
