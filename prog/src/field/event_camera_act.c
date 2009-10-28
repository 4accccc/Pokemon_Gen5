///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   event_camera_act.c
 * @brief  ドアに出入りする際のカメラ動作イベント
 * @author obata
 * @date   2009.08.17
 */
///////////////////////////////////////////////////////////////////////////////////////////////
#include "event_camera_act.h"
#include "gamesystem/game_event.h"
#include "field/field_camera.h"
#include "field/fieldmap.h"
#include "fieldmap_tcb.h"


//=============================================================================================
/**
 * @brief 定数
 */
//=============================================================================================

// カメラの設定
#define NEAR_PLANE (50 << FX32_SHIFT)
#define FAR_PLANE  (512 << FX32_SHIFT)

// 上ドアへの出入り
#define U_DOOR_FRAME     (6)
#define U_DOOR_ZOOM_DIST (15 << FX32_SHIFT)   // ズームする距離

// 左ドアへの出入り
#define L_DOOR_FRAME (10)
#define L_DOOR_PITCH (0xffff * 20 / 360)
#define L_DOOR_YAW   (0xffff * 90 / 360)
#define L_DOOR_ZOOM_DIST  (120 << FX32_SHIFT)

// 右ドアへの出入り
#define R_DOOR_FRAME (10)
#define R_DOOR_PITCH (0xffff * 20 / 360)
#define R_DOOR_YAW   (0xffff * 270 / 360)
#define R_DOOR_ZOOM_DIST  (120 << FX32_SHIFT)



//=============================================================================================
/**
 * @brief イベント・ワーク
 */
//=============================================================================================
typedef struct
{
  FIELDMAP_WORK* pFieldmap;   // 操作対象のフィールドマップ
  u16            frame;       // 経過フレーム数
}
EVENT_WORK;


//=============================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//=============================================================================================
static void SetFarNear( FIELDMAP_WORK* p_fieldmap );

// イベント生成関数
static GMEVENT* EVENT_UpDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_LeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_RightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_UpDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_LeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_RightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );

// イベント処理関数
static GMEVENT_RESULT EVENT_FUNC_UpDoorIn( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_LeftDoorIn( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_RightDoorIn( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_RightDoorIn_2Step( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_RightDoorIn_1Frame( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_UpDoorOut( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_LeftDoorOut( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_RightDoorOut( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_RightDoorOut_2Step( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT EVENT_FUNC_RightDoorOut_1Frame( GMEVENT* p_event, int* p_seq, void* p_work );


//=============================================================================================
/**
 * @brief 公開関数の定義
 */
//=============================================================================================

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ動作イベントを呼び出す( 入った時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
void EVENT_CAMERA_ACT_CallDoorInEvent( GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16            dir    = MMDL_GetDirDisp( fmmdl );

  // 主人公の向きに応じたイベントを呼び出す
  switch( dir )
  {
  case DIR_UP:
    GMEVENT_CallEvent( p_parent, EVENT_UpDoorIn(p_gsys, p_fieldmap) );
    break;
  case DIR_LEFT:
    GMEVENT_CallEvent( p_parent, EVENT_LeftDoorIn(p_gsys, p_fieldmap) );
    break;
  case DIR_RIGHT:
    GMEVENT_CallEvent( p_parent, EVENT_RightDoorIn(p_gsys, p_fieldmap) );
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief ドアから出てきた際の, カメラの初期設定を行う
 *
 * @param p_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------------------------
void EVENT_CAMERA_ACT_PrepareForDoorOut( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER*    player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL*            fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16              dir    = MMDL_GetDirDisp( fmmdl );
  FIELD_CAMERA*    cam    = FIELDMAP_GetFieldCamera( p_fieldmap );

  // 主人公の向きに応じたカメラ設定
  switch( dir )
  {
  case DIR_DOWN:
    FIELD_CAMERA_SetAngleLen( cam, FIELD_CAMERA_GetAngleLen( cam ) - U_DOOR_ZOOM_DIST);
    break;
  case DIR_LEFT:
    FIELD_CAMERA_SetAnglePitch( cam, R_DOOR_PITCH );
    FIELD_CAMERA_SetAngleYaw( cam, R_DOOR_YAW );
    FIELD_CAMERA_SetAngleLen( cam, FIELD_CAMERA_GetAngleLen( cam ) - R_DOOR_ZOOM_DIST );
    SetFarNear( p_fieldmap );
    break;
  case DIR_RIGHT:
    FIELD_CAMERA_SetAnglePitch( cam, L_DOOR_PITCH );
    FIELD_CAMERA_SetAngleYaw( cam, L_DOOR_YAW );
    FIELD_CAMERA_SetAngleLen( cam, FIELD_CAMERA_GetAngleLen( cam ) - L_DOOR_ZOOM_DIST );
    SetFarNear( p_fieldmap );
    break;
  default:
    break;
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ動作イベントを呼び出す( 出た時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
void EVENT_CAMERA_ACT_CallDoorOutEvent( GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16            dir    = MMDL_GetDirDisp( fmmdl );

  // 主人公の向きに応じたイベントを呼び出す
  switch( dir )
  {
  case DIR_DOWN:
    GMEVENT_CallEvent( p_parent, EVENT_UpDoorOut(p_gsys, p_fieldmap) );
    break;
  case DIR_LEFT:
    GMEVENT_CallEvent( p_parent, EVENT_RightDoorOut(p_gsys, p_fieldmap) );
    break;
  case DIR_RIGHT:
    GMEVENT_CallEvent( p_parent, EVENT_LeftDoorOut(p_gsys, p_fieldmap) );
    break;
  default:
    break;
  }
}

//--------------------------------------------------------------------
/**
 * @breif カメラのNearプレーンとFarプレーンをデフォルト設定に戻す
 *
 * @param p_fieldmap フィールドマップ
 */
//--------------------------------------------------------------------
void EVENT_CAMERA_ACT_ResetCameraParameter( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_CAMERA*    p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // デフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // デフォルト値に戻す
  FIELD_CAMERA_SetFar( p_camera, def_param.Far );
  FIELD_CAMERA_SetNear( p_camera, def_param.Near );

  // DEBUG:
  /*
  OBATA_Printf( "reset far  = %d\n", def_param.Far >> FX32_SHIFT );
  OBATA_Printf( "reset near = %d\n", def_param.Near >> FX32_SHIFT );
  */
}


//=============================================================================================
/**
 * @brief 非公開関数の定義
 */
//=============================================================================================

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ動作イベント( 上にあるドアに入った時 )
 *
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT* EVENT_UpDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_UpDoorIn, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work            = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame     = 0;

  // 生成したイベントを返す
  return p_event;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ回転イベント( 左にあるドアに入った時 )
 *
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT* EVENT_LeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_LeftDoorIn, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work            = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame     = 0;

  // 生成したイベントを返す
  return p_event;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ回転イベント( 右にあるドアに入った時 )
 *
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT* EVENT_RightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_RightDoorIn, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_RightDoorIn_2Step, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_RightDoorIn_1Frame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work            = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame     = 0;

  // 生成したイベントを返す
  return p_event;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ動作イベント( 上にあるドアから出てきたとき )
 *
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT* EVENT_UpDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_UpDoorOut, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work            = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame     = 0;

  // 生成したイベントを返す
  return p_event;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ回転イベント( 左にあるドアから出てきた時 )
 *
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT* EVENT_LeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*         p_event;
  EVENT_WORK*      p_work;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_LeftDoorOut, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work            = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame     = 0;

  // 生成したイベントを返す
  return p_event;
}


//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ回転イベント( 右にあるドアから出てきた時 )
 *
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT* EVENT_RightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_RightDoorOut, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_FUNC_RightDoorOut_2Step, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work            = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame     = 0;
  p_work = GMEVENT_GetEventWork( p_event );

  // 生成したイベントを返す
  return p_event;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラのfarプレーン・nearプレーンを設定する
 *
 * @param p_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------------------------
static void SetFarNear( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  fx32          far      = FIELD_CAMERA_GetFar( p_camera ); 
  fx32          near     = FIELD_CAMERA_GetNear( p_camera ); 

  // DEBUG:
  /*
  OBATA_Printf( "Set FarPlane and NearPlane\n" );
  OBATA_Printf( "before far  = %x(%d)\n", far, far>>FX32_SHIFT );
  OBATA_Printf( "before near = %x(%d)\n", near, near>>FX32_SHIFT );
  */

  // Farプレーンを半分にする
  far = FAR_PLANE;
  FIELD_CAMERA_SetFar( p_camera, far );

  // Nearプレーンを設定する
  near = NEAR_PLANE;
  FIELD_CAMERA_SetNear( p_camera, near );

  // DEBUG:
  /*
  OBATA_Printf( "after far  = %x(%d)\n", far, far>>FX32_SHIFT );
  OBATA_Printf( "after near = %x(%d)\n", near, near>>FX32_SHIFT );
  */
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 上出入口への進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_UpDoorIn( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  switch( *p_seq )
  {
  case 0:
    FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, U_DOOR_FRAME, -U_DOOR_ZOOM_DIST );
    ++( *p_seq );
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 左出入口への進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_LeftDoorIn( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  switch( *p_seq )
  {
  case 0:
    SetFarNear( p_event_work->pFieldmap );
    FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, L_DOOR_FRAME, -L_DOOR_ZOOM_DIST );
    FIELDMAP_TCB_AddTask_CameraRotate_Yaw( p_event_work->pFieldmap, L_DOOR_FRAME, L_DOOR_YAW );
    FIELDMAP_TCB_AddTask_CameraRotate_Pitch( p_event_work->pFieldmap, L_DOOR_FRAME, L_DOOR_PITCH );
    ++( *p_seq );
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 右出入口への進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_RightDoorIn( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  switch( *p_seq )
  {
  case 0:
    SetFarNear( p_event_work->pFieldmap );
    FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, R_DOOR_FRAME, -R_DOOR_ZOOM_DIST );
    FIELDMAP_TCB_AddTask_CameraRotate_Yaw( p_event_work->pFieldmap, R_DOOR_FRAME, R_DOOR_YAW );
    FIELDMAP_TCB_AddTask_CameraRotate_Pitch( p_event_work->pFieldmap, R_DOOR_FRAME, R_DOOR_PITCH );
    ++( *p_seq );
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 右出入口への進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_RightDoorIn_2Step( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  switch( *p_seq )
  {
  case 0:
    SetFarNear( p_event_work->pFieldmap );
    FIELDMAP_TCB_AddTask_CameraRotate_Yaw( p_event_work->pFieldmap, 20, R_DOOR_YAW );
    FIELDMAP_TCB_AddTask_CameraRotate_Pitch( p_event_work->pFieldmap, 20, R_DOOR_PITCH );
    p_event_work->frame = 0;
    ++( *p_seq );
    break;
  case 1:
    if( 10 < p_event_work->frame++ )
    {
      ++( *p_seq );
    }
    break;
  case 2:
    FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, R_DOOR_FRAME, -R_DOOR_ZOOM_DIST );
    ++( *p_seq );
    p_event_work->frame = 0;
    break;
  case 3:
    if( 20 < p_event_work->frame++ )
    {
      ++( *p_seq );
    }
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 右出入口への進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_RightDoorIn_1Frame( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA* cam          = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );

  SetFarNear( p_event_work->pFieldmap );
  FIELD_CAMERA_SetAnglePitch( cam, R_DOOR_PITCH );
  FIELD_CAMERA_SetAngleYaw( cam, R_DOOR_YAW );
  FIELD_CAMERA_SetAngleLen( cam, FIELD_CAMERA_GetAngleLen( cam ) - R_DOOR_ZOOM_DIST );

  return GMEVENT_RES_FINISH;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 上出入口からの進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_UpDoorOut( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  switch( *p_seq )
  {
  case 0:
    FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, U_DOOR_FRAME, U_DOOR_ZOOM_DIST );
    ++( *p_seq );
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 左出入口からの進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_LeftDoorOut( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA*   p_camera = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );

  switch( *p_seq )
  {
  case 0:
    {
      FLD_CAMERA_PARAM def_param; 
      FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );
      SetFarNear( p_event_work->pFieldmap );
      FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, L_DOOR_FRAME, L_DOOR_ZOOM_DIST );
      FIELDMAP_TCB_AddTask_CameraRotate_Yaw( p_event_work->pFieldmap, L_DOOR_FRAME, def_param.Angle.y );
      FIELDMAP_TCB_AddTask_CameraRotate_Pitch( p_event_work->pFieldmap, L_DOOR_FRAME, def_param.Angle.x );
    }
    ++( *p_seq );
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 右出入口からの進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_RightDoorOut( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA*   p_camera = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );

  switch( *p_seq )
  {
  case 0:
    {
      FLD_CAMERA_PARAM def_param; 
      FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );
      SetFarNear( p_event_work->pFieldmap );
      FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, R_DOOR_FRAME, R_DOOR_ZOOM_DIST );
      FIELDMAP_TCB_AddTask_CameraRotate_Yaw( p_event_work->pFieldmap, R_DOOR_FRAME, def_param.Angle.y );
      FIELDMAP_TCB_AddTask_CameraRotate_Pitch( p_event_work->pFieldmap, R_DOOR_FRAME, def_param.Angle.x );
    }
    ++( *p_seq );
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 右出入口からの進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_RightDoorOut_2Step( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA*        cam = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap ); 
  FLD_CAMERA_PARAM def_param; 

  // デフォルトパラメータを取得
  FIELD_CAMERA_GetInitialParameter( cam, &def_param );

  switch( *p_seq )
  {
  case 0:
    SetFarNear( p_event_work->pFieldmap );
    FIELDMAP_TCB_AddTask_CameraZoom( p_event_work->pFieldmap, 20, R_DOOR_ZOOM_DIST );
    p_event_work->frame = 0;
    ++( *p_seq );
    break;
  case 1:
    if( 10 < p_event_work->frame++ )
    {
      ++( *p_seq );
    }
    break;
  case 2:
    FIELDMAP_TCB_AddTask_CameraRotate_Yaw( p_event_work->pFieldmap, 15, def_param.Angle.y );
    FIELDMAP_TCB_AddTask_CameraRotate_Pitch( p_event_work->pFieldmap, 15, def_param.Angle.x );
    ++( *p_seq );
    p_event_work->frame = 0;
    break;
  case 3:
    if( 20 < p_event_work->frame++ )
    {
      ++( *p_seq );
    }
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( 右出入口からの進入 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_RightDoorOut_1Frame( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA*   cam = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap ); 
  FLD_CAMERA_PARAM def_param; 

  // デフォルトパラメータを取得
  FIELD_CAMERA_GetInitialParameter( cam, &def_param );

  SetFarNear( p_event_work->pFieldmap );
  FIELD_CAMERA_SetAnglePitch( cam, def_param.Angle.x );
  FIELD_CAMERA_SetAngleYaw( cam, def_param.Angle.y );
  FIELD_CAMERA_SetAngleLen( cam, def_param.Distance );

  return GMEVENT_RES_CONTINUE;
}
