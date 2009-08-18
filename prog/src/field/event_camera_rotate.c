///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  横向きドアに出入りする際のカメラ回転イベント
 * @author obata
 * @date   2009.08.17
 */
///////////////////////////////////////////////////////////////////////////////////////////////
#include "event_camera_rotate.h"
#include "gamesystem/game_event.h"
#include "field/field_camera.h"
#include "field/fieldmap.h"


//=============================================================================================
/**
 * @brief 定数
 */
//=============================================================================================

#define CAMERA_DIST_MIN (100 << FX32_SHIFT)


//=============================================================================================
/**
 * @brief イベント・ワーク
 */
//=============================================================================================
typedef struct
{
  FIELDMAP_WORK* pFieldmap;
  u16 frame;        // 経過フレーム数
  u16 endFramePitch;// 終了フレーム
  u16 endFrameYaw;  // 終了フレーム
  u16 endFrameDist; // 終了フレーム
  u16 startPitch;   // ピッチ初期値(2πラジアンを65536分割した値を単位とする数)
  u16 endPitch;     // ピッチ最終値(2πラジアンを65536分割した値を単位とする数)
  u32 startYaw;     // ヨー初期値(2πラジアンを65536分割した値を単位とする数)
  u32 endYaw;       // ヨー最終値(2πラジアンを65536分割した値を単位とする数)
  fx32 startDist;   // カメラ距離初期値
  fx32 endDist;     // カメラ距離最終値
}
EVENT_WORK;


//=============================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//=============================================================================================
static void SetFarNear( FIELDMAP_WORK* p_fieldmap );
static void ResetFarNear( FIELDMAP_WORK* p_fieldmap );
static void UpdateYaw( EVENT_WORK* p_work );
static void UpdatePitch( EVENT_WORK* p_work );
static void UpdateDist( EVENT_WORK* p_work );

// イベント生成関数
static GMEVENT* EVENT_CameraRotateLeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_CameraRotateRightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_CameraRotateLeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_CameraRotateRightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );

// イベント処理関数
static GMEVENT_RESULT RotateCamera( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT RotateCamera3Step( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT RotateCamera2Step( GMEVENT* p_event, int* p_seq, void* p_work );
static GMEVENT_RESULT RotateCameraInOneFrame( GMEVENT* p_event, int* p_seq, void* p_work );


//=============================================================================================
/**
 * @brief 公開関数の定義
 */
//=============================================================================================

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ回転イベントを呼び出す( 入った時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
void EVENT_CAMERA_ROTATE_CallDoorInEvent( GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16            dir    = MMDL_GetDirDisp( fmmdl );

  // 向きに応じたイベントを呼び出す
  if( ( dir == DIR_LEFT ) )
  {
    GMEVENT_CallEvent( p_parent, EVENT_CameraRotateLeftDoorIn(p_gsys, p_fieldmap) );
  } 
  else if( ( dir == DIR_RIGHT ) )
  {
    GMEVENT_CallEvent( p_parent, EVENT_CameraRotateRightDoorIn(p_gsys, p_fieldmap) );
  } 
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief 左右にあるドアから出てきた際の, カメラの初期設定を行う
 *
 * @param p_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------------------------
void EVENT_CAMERA_ROTATE_PrepareDoorOut( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16            dir    = MMDL_GetDirDisp( fmmdl );
  FIELD_CAMERA*  cam    = FIELDMAP_GetFieldCamera( p_fieldmap );

  // 主人公の向きに応じたカメラ設定
  if( dir == DIR_LEFT )
  {
    FIELD_CAMERA_SetAngleYaw( cam, 0xffff/360*270 );
    FIELD_CAMERA_SetAnglePitch( cam, 0xffff/360*20 );
    FIELD_CAMERA_SetAngleLen( cam, 100<<FX32_SHIFT );
    SetFarNear( p_fieldmap );
  }
  else if( dir == DIR_RIGHT )
  {
    FIELD_CAMERA_SetAngleYaw( cam, 0xffff/360*90 );
    FIELD_CAMERA_SetAnglePitch( cam, 0xffff/360*20 );
    FIELD_CAMERA_SetAngleLen( cam, 100<<FX32_SHIFT );
    SetFarNear( p_fieldmap );
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ回転イベントを呼び出す( 出た時 )
 *
 * @param p_parent   親イベント
 * @param p_gsys     ゲームシステム
 * @param p_fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------------
void EVENT_CAMERA_ROTATE_CallDoorOutEvent( GMEVENT* p_parent, GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16            dir    = MMDL_GetDirDisp( fmmdl );

  // 向きに応じたイベントを呼び出す
  if( ( dir == DIR_LEFT ) )
  {
    GMEVENT_CallEvent( p_parent, EVENT_CameraRotateRightDoorOut(p_gsys, p_fieldmap) );
  } 
  else if( ( dir == DIR_RIGHT ) )
  {
    GMEVENT_CallEvent( p_parent, EVENT_CameraRotateLeftDoorOut(p_gsys, p_fieldmap) );
  } 
}


//=============================================================================================
/**
 * @brief 非公開関数の定義
 */
//=============================================================================================

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
static GMEVENT* EVENT_CameraRotateLeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera3Step, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraInOneFrame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap     = p_fieldmap;
  p_work->frame         = 0;
  p_work->endFramePitch = 10;
  p_work->endFrameYaw   = 10;
  p_work->endFrameDist  = 10;
  p_work->startPitch    = FIELD_CAMERA_GetAnglePitch( p_camera );
  p_work->endPitch      = 0xffff / 360 * 20;
  p_work->startYaw      = FIELD_CAMERA_GetAngleYaw( p_camera );
  p_work->endYaw        = 0xffff / 360 * 90;
  p_work->startDist     = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist       = CAMERA_DIST_MIN;

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方のヨー値を360度分の下駄を履かせる.
  if( p_work->startYaw < p_work->endYaw )
  {
    if( 0x7fff < (p_work->endYaw - p_work->startYaw) )
    {
      p_work->startYaw += 0xffff;
    }
  }
  else if( p_work->endYaw < p_work->startYaw )
  {
    if( 0x7fff < (p_work->startYaw - p_work->endYaw) )
    {
      p_work->endYaw += 0xffff;
    }
  } 

  // カメラの初期設定
  SetFarNear( p_work->pFieldmap );

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
static GMEVENT* EVENT_CameraRotateRightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera3Step, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraInOneFrame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame = 0;
  p_work->endFramePitch = 10;
  p_work->endFrameYaw   = 10;
  p_work->endFrameDist  = 10;
  p_work->startPitch = FIELD_CAMERA_GetAnglePitch( p_camera );
  p_work->endPitch = 0xffff / 360 * 20;
  p_work->startYaw = FIELD_CAMERA_GetAngleYaw( p_camera );
  p_work->endYaw = 0xffff / 360 * 270;
  p_work->startDist = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist = CAMERA_DIST_MIN; 

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方のヨー値を360度分の下駄を履かせる.
  if( p_work->startYaw < p_work->endYaw )
  {
    if( 0x7fff < (p_work->endYaw - p_work->startYaw) )
    {
      p_work->startYaw += 0xffff;
    }
  }
  else if( p_work->endYaw < p_work->startYaw )
  {
    if( 0x7fff < (p_work->startYaw - p_work->endYaw) )
    {
      p_work->endYaw += 0xffff;
    }
  } 

  // カメラの初期設定
  SetFarNear( p_work->pFieldmap );

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
static GMEVENT* EVENT_CameraRotateLeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*         p_event;
  EVENT_WORK*      p_work;
  FIELD_CAMERA*    p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // カメラのデフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera3Step, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraInOneFrame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap     = p_fieldmap;
  p_work->frame         = 0;
  p_work->endFramePitch = 10;
  p_work->endFrameYaw   = 10;
  p_work->endFrameDist  = 10;
  p_work->startPitch    = FIELD_CAMERA_GetAnglePitch( p_camera );
  p_work->endPitch      = def_param.Angle.x;
  p_work->startYaw      = FIELD_CAMERA_GetAngleYaw( p_camera );
  p_work->endYaw        = def_param.Angle.y;
  p_work->startDist     = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist       = def_param.Distance * FX32_ONE;

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方のヨー値を360度分の下駄を履かせる.
  if( p_work->startYaw < p_work->endYaw )
  {
    if( 0x7fff < (p_work->endYaw - p_work->startYaw) )
    {
      p_work->startYaw += 0xffff;
    }
  }
  else if( p_work->endYaw < p_work->startYaw )
  {
    if( 0x7fff < (p_work->startYaw - p_work->endYaw) )
    {
      p_work->endYaw += 0xffff;
    }
  } 

  // カメラの初期設定
  SetFarNear( p_work->pFieldmap );

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
static GMEVENT* EVENT_CameraRotateRightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // カメラのデフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera3Step, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraInOneFrame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap     = p_fieldmap;
  p_work->frame         = 0;
  p_work->endFramePitch = 10;
  p_work->endFrameYaw   = 10;
  p_work->endFrameDist  = 10;
  p_work->startPitch    = FIELD_CAMERA_GetAnglePitch( p_camera );
  p_work->endPitch      = def_param.Angle.x;
  p_work->startYaw      = FIELD_CAMERA_GetAngleYaw( p_camera );
  p_work->endYaw        = def_param.Angle.y;
  p_work->startDist     = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist       = def_param.Distance * FX32_ONE;

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方のヨー値を360度分の下駄を履かせる.
  if( p_work->startYaw < p_work->endYaw )
  {
    if( 0x7fff < (p_work->endYaw - p_work->startYaw) )
    {
      p_work->startYaw += 0xffff;
    }
  }
  else if( p_work->endYaw < p_work->startYaw )
  {
    if( 0x7fff < (p_work->startYaw - p_work->endYaw) )
    {
      p_work->endYaw += 0xffff;
    }
  } 

  // カメラの初期設定
  SetFarNear( p_work->pFieldmap );

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
  OBATA_Printf( "Set FarPlane and NearPlane\n" );
  OBATA_Printf( "before far  = %x(%d)\n", far, far>>FX32_SHIFT );
  OBATA_Printf( "before near = %x(%d)\n", near, near>>FX32_SHIFT );

  // Farプレーンを半分にする
  far /= 2;
  FIELD_CAMERA_SetFar( p_camera, far );

  // Nearプレーンを設定する
  near = 50 << FX32_SHIFT;
  FIELD_CAMERA_SetNear( p_camera, near );

  // DEBUG:
  OBATA_Printf( "after far  = %x(%d)\n", far, far>>FX32_SHIFT );
  OBATA_Printf( "after near = %x(%d)\n", near, near>>FX32_SHIFT );
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラのfarプレーン・nearプレーンをデフォルト値に戻す
 *
 * @param p_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------------------------
static void ResetFarNear( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_CAMERA*    p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // デフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // デフォルト値に戻す
  FIELD_CAMERA_SetFar( p_camera, def_param.Far );
  FIELD_CAMERA_SetNear( p_camera, def_param.Near );

  // DEBUG:
  OBATA_Printf( "reset far  = %d\n", def_param.Far >> FX32_SHIFT );
  OBATA_Printf( "reset near = %d\n", def_param.Near >> FX32_SHIFT );
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief ピッチを更新する
 *
 * @param p_work イベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdatePitch( EVENT_WORK* p_work )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_work->pFieldmap );
  u16           pitch;
  float         progress;

  if(p_work->frame <= p_work->endFramePitch )
  { 
    progress = p_work->frame / (float)p_work->endFramePitch;
    pitch    = (u16)( ( (1-progress) * p_work->startPitch ) + ( progress * p_work->endPitch ) );
    FIELD_CAMERA_SetAnglePitch( p_camera, pitch );
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief ヨーを更新する
 *
 * @param p_work イベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdateYaw( EVENT_WORK* p_work )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_work->pFieldmap );
  u16           yaw;
  float         progress;
  if(p_work->frame <= p_work->endFrameYaw )
  {
    progress = p_work->frame / (float)p_work->endFrameYaw;
    yaw      = (u16)( ( (1-progress) * p_work->startYaw ) + ( progress * p_work->endYaw ) );
    FIELD_CAMERA_SetAngleYaw( p_camera, yaw );
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ距離を更新する
 *
 * @param p_work イベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdateDist( EVENT_WORK* p_work )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_work->pFieldmap );
  fx32          dist;
  float         progress;
  if(p_work->frame <= p_work->endFrameDist )
  {
    progress = p_work->frame / (float)p_work->endFrameDist;
    dist     = FX_F32_TO_FX32( ( (1-progress) * FX_FX32_TO_F32(p_work->startDist) ) + ( progress * FX_FX32_TO_F32(p_work->endDist) ) );
  FIELD_CAMERA_SetAngleLen( p_camera, dist ); 
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( ヨー・ピッチ・距離を同時に変更 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT RotateCamera( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  // カメラパラメータを更新
  UpdatePitch( p_event_work );
  UpdateYaw( p_event_work );
  UpdateDist( p_event_work );

  // 指定回数動作したら終了
  p_event_work->frame++;
  if( ( p_event_work->endFrameYaw < p_event_work->frame ) &&
      ( p_event_work->endFramePitch < p_event_work->frame ) &&
      ( p_event_work->endFrameDist < p_event_work->frame ) )
  {
    ResetFarNear( p_event_work->pFieldmap );
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( ヨー・ピッチ・距離を段階的に変更 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT RotateCamera3Step( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA* p_camera     = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );
  
  switch( *p_seq )
  {
  case 0:
    UpdateDist( p_event_work );
    if( p_event_work->endFrameDist < ++(p_event_work->frame) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 1:
    UpdatePitch( p_event_work );
    if( p_event_work->endFramePitch < ++(p_event_work->frame) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 2:
    UpdateYaw( p_event_work );
    if( p_event_work->endFrameYaw < ++(p_event_work->frame) )
    {
      ResetFarNear( p_event_work->pFieldmap );
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( ヨー・ピッチ・距離を段階的に変更 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT RotateCamera2Step( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  
  switch( *p_seq )
  {
  case 0:
    UpdateDist( p_event_work );
    UpdatePitch( p_event_work );
    p_event_work->frame++;
    if( ( p_event_work->endFramePitch < (p_event_work->frame) ) &&
        ( p_event_work->endFrameDist  < (p_event_work->frame) ) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 1:
    UpdateYaw( p_event_work );
    p_event_work->frame++;
    if( p_event_work->endFrameYaw < p_event_work->frame )
    {
      ResetFarNear( p_event_work->pFieldmap );
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief イベント処理関数( ヨー・ピッチ・距離を瞬時に変更 )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT RotateCameraInOneFrame( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA* p_camera     = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );

  switch( *p_seq )
  {
  case 0:
    if( 0 < ++(p_event_work->frame) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 1:
      FIELD_CAMERA_SetAngleLen( p_camera, p_event_work->endDist );
      FIELD_CAMERA_SetAngleYaw( p_camera, p_event_work->endYaw );
      FIELD_CAMERA_SetAnglePitch( p_camera, p_event_work->endPitch );
      ++ *p_seq;
    break;
  case 2:
    if( 10 < ++(p_event_work->frame) )
    {
      ++ *p_seq;
      ResetFarNear( p_event_work->pFieldmap );
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}
