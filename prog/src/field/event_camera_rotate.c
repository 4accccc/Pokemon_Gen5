///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ドアに出入りする際のカメラ動作イベント
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
#define ZOOM_DIST (15 << FX32_SHIFT)


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

// イベントワーク設定関数
static void InitWork( EVENT_WORK* p_work, FIELDMAP_WORK* p_fieldmap );
static void SetPitchAction( EVENT_WORK* p_work, u16 time, u16 start, u16 end );
static void SetYawAction( EVENT_WORK* p_work, u16 time, u32 start, u32 end );
static void SetDistAction( EVENT_WORK* p_work, u16 time, fx32 start, fx32 end );

// カメラ動作関数
static void UpdateYaw( EVENT_WORK* p_work );
static void UpdatePitch( EVENT_WORK* p_work );
static void UpdateDist( EVENT_WORK* p_work );

// イベント生成関数
static GMEVENT* EVENT_UpDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_LeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_RightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_UpDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_LeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT* EVENT_RightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );

// イベント処理関数
static GMEVENT_RESULT Zoom( GMEVENT* p_event, int* p_seq, void* p_work );
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
void EVENT_CAMERA_ROTATE_PrepareDoorOut( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16            dir    = MMDL_GetDirDisp( fmmdl );
  FIELD_CAMERA*  cam    = FIELDMAP_GetFieldCamera( p_fieldmap );

  // 主人公の向きに応じたカメラ設定
  switch( dir )
  {
  case DIR_DOWN:
    FIELD_CAMERA_SetAngleLen( cam, FIELD_CAMERA_GetAngleLen( cam ) - ZOOM_DIST );
    break;
  case DIR_LEFT:
    FIELD_CAMERA_SetAngleYaw( cam, 0xffff/360*270 );
    FIELD_CAMERA_SetAnglePitch( cam, 0xffff/360*20 );
    FIELD_CAMERA_SetAngleLen( cam, 100<<FX32_SHIFT );
    SetFarNear( p_fieldmap );
    break;
  case DIR_RIGHT:
    FIELD_CAMERA_SetAngleYaw( cam, 0xffff/360*90 );
    FIELD_CAMERA_SetAnglePitch( cam, 0xffff/360*20 );
    FIELD_CAMERA_SetAngleLen( cam, 100<<FX32_SHIFT );
    SetFarNear( p_fieldmap );
    break;
  default:
    break;
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
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, Zoom, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  InitWork( p_work, p_fieldmap );
  SetDistAction( p_work, 6,
      FIELD_CAMERA_GetAngleLen( p_camera ), FIELD_CAMERA_GetAngleLen( p_camera ) - ZOOM_DIST );

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
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  InitWork( p_work, p_fieldmap );
  SetPitchAction( p_work, 10, FIELD_CAMERA_GetAnglePitch( p_camera ), 0xffff / 360 * 20 );
  SetYawAction  ( p_work, 10, FIELD_CAMERA_GetAngleYaw( p_camera ),   0xffff / 360 * 90 );
  SetDistAction ( p_work, 10, FIELD_CAMERA_GetAngleLen( p_camera ),   CAMERA_DIST_MIN );

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
static GMEVENT* EVENT_RightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  InitWork( p_work, p_fieldmap );
  SetPitchAction( p_work, 10, FIELD_CAMERA_GetAnglePitch( p_camera ), 0xffff / 360 * 20 );
  SetYawAction  ( p_work, 10, FIELD_CAMERA_GetAngleYaw( p_camera ),   0xffff / 360 * 270 );
  SetDistAction ( p_work, 10, FIELD_CAMERA_GetAngleLen( p_camera ), CAMERA_DIST_MIN ); 

  // カメラの初期設定
  SetFarNear( p_work->pFieldmap );

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
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // カメラのデフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, Zoom, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  InitWork( p_work, p_fieldmap );
  SetDistAction( p_work, 6, FIELD_CAMERA_GetAngleLen( p_camera ), def_param.Distance * FX32_ONE );

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
  FIELD_CAMERA*    p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // カメラのデフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  InitWork( p_work, p_fieldmap );
  SetPitchAction( p_work, 10, FIELD_CAMERA_GetAnglePitch( p_camera ), def_param.Angle.x );
  SetYawAction  ( p_work, 10, FIELD_CAMERA_GetAngleYaw( p_camera ),   def_param.Angle.y );
  SetDistAction ( p_work, 10, FIELD_CAMERA_GetAngleLen( p_camera ),   def_param.Distance * FX32_ONE );

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
static GMEVENT* EVENT_RightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  FLD_CAMERA_PARAM def_param;

  // カメラのデフォルト・パラメータを取得
  FIELD_CAMERA_GetInitialParameter( p_camera, &def_param );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera2Step, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  InitWork( p_work, p_fieldmap );
  SetPitchAction( p_work, 10, FIELD_CAMERA_GetAnglePitch( p_camera ), def_param.Angle.x );
  SetYawAction  ( p_work, 10, FIELD_CAMERA_GetAngleYaw( p_camera ),   def_param.Angle.y );
  SetDistAction ( p_work, 10, FIELD_CAMERA_GetAngleLen( p_camera ),   def_param.Distance * FX32_ONE );

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
 * @brief イベントワークを初期化する
 *
 * @param p_work     初期化する変数
 * @param p_fieldmap 設定するフィールドマップ
 */
//-----------------------------------------------------------------------------------------------
static void InitWork( EVENT_WORK* p_work, FIELDMAP_WORK* p_fieldmap )
{
  p_work->pFieldmap     = p_fieldmap;
  p_work->frame         = 0;
  p_work->endFramePitch = 0;
  p_work->endFrameYaw   = 0;
  p_work->endFrameDist  = 0;
  p_work->startPitch    = 0;
  p_work->endPitch      = 0;
  p_work->startYaw      = 0;
  p_work->endYaw        = 0;
  p_work->startDist     = 0;
  p_work->endDist       = 0;
}

//-----------------------------------------------------------------------------------------------
/**
 * @breif ピッチの動作を設定する
 *
 * @param p_work  設定対象イベントワーク
 * @param time    動作フレーム数
 * @param start   初期値
 * @param end     終了値
 */
//-----------------------------------------------------------------------------------------------
static void SetPitchAction( EVENT_WORK* p_work, u16 time, u16 start, u16 end )
{
  p_work->startPitch    = start;
  p_work->endPitch      = end;
  p_work->endFramePitch = time;
}

//-----------------------------------------------------------------------------------------------
/**
 * @breif ヨーの動作を設定する
 *
 * @param p_work  設定対象イベントワーク
 * @param time    動作フレーム数
 * @param start   初期値
 * @param end     終了値
 */
//-----------------------------------------------------------------------------------------------
static void SetYawAction( EVENT_WORK* p_work, u16 time, u32 start, u32 end )
{
  p_work->startYaw    = start;
  p_work->endYaw      = end;
  p_work->endFrameYaw = time;

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
}

//-----------------------------------------------------------------------------------------------
/**
 * @breif カメラ距離の動作を設定する
 *
 * @param p_work  設定対象イベントワーク
 * @param time    動作フレーム数
 * @param start   初期値
 * @param end     終了値
 */
//-----------------------------------------------------------------------------------------------
static void SetDistAction( EVENT_WORK* p_work, u16 time, fx32 start, fx32 end )
{
  p_work->startDist    = start;
  p_work->endDist      = end;
  p_work->endFrameDist = time;
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
 * @brief イベント処理関数( ズームイン or ズームアウト )
 *
 * @param p_event イベント
 * @param p_seq   シーケンス番号
 * @param p_work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//-----------------------------------------------------------------------------------------------
static GMEVENT_RESULT Zoom( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK* p_event_work = (EVENT_WORK*)p_work;

  // カメラパラメータを更新
  UpdateDist( p_event_work );

  // 指定回数動作したら終了
  p_event_work->frame++;
  if( p_event_work->endFrameDist < p_event_work->frame )
  {
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
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
