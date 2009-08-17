#include "event_camera_rotate.h"
#include "gamesystem/game_event.h"
#include "field/field_camera.h"
#include "field/fieldmap.h"

typedef struct
{
  FIELDMAP_WORK* pFieldmap;
  u16 frame;        // 経過フレーム数
  u16 endFramePitch;     // 終了フレーム
  u16 endFrameYaw;     // 終了フレーム
  u16 endFrameDist;     // 終了フレーム
  u16 startPitch;   // ピッチ初期値
  u16 endPitch;     // ピッチ最終値
  u32 startYaw;     // ヨー初期値
  u32 endYaw;       // ヨー最終値
  fx32 startDist;   // カメラ距離初期値
  fx32 endDist;     // カメラ距離最終値
}
EVENT_WORK;


static u16 CalcYaw( EVENT_WORK* p_work );
static u16 CalcPitch( EVENT_WORK* p_work );
static fx32 CalcDist( EVENT_WORK* p_work );

GMEVENT_RESULT RotateCamera( GMEVENT* p_event, int* p_seq, void* p_work );
GMEVENT_RESULT RotateCameraStep( GMEVENT* p_event, int* p_seq, void* p_work );
GMEVENT_RESULT RotateCameraStep2( GMEVENT* p_event, int* p_seq, void* p_work );
GMEVENT_RESULT RotateCameraInOneFrame( GMEVENT* p_event, int* p_seq, void* p_work );


GMEVENT* EVENT_CameraRotateLeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep2, sizeof( EVENT_WORK ) );
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
  p_work->endYaw = 0xffff / 360 * 90;
  p_work->startDist = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist = 100 << FX32_SHIFT;

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

  // Farプレーンを半分にする
  {
    fx32 far = FIELD_CAMERA_GetFar( p_camera );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
    far /= 2;
    FIELD_CAMERA_SetFar( p_camera, far );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
  } 
  // Nearプレーンを設定する
  {
    fx32 near = FIELD_CAMERA_GetNear( p_camera ); 
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
    near = 50 << FX32_SHIFT;
    FIELD_CAMERA_SetNear( p_camera, near );
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
  }

  // 生成したイベントを返す
  return p_event;
}


GMEVENT* EVENT_CameraRotateRightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep2, sizeof( EVENT_WORK ) );
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
  p_work->endDist = 100 << FX32_SHIFT;

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

  // Farプレーンを半分にする
  {
    fx32 far = FIELD_CAMERA_GetFar( p_camera );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
    far /= 2;
    FIELD_CAMERA_SetFar( p_camera, far );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
  } 
  // Nearプレーンを設定する
  {
    fx32 near = FIELD_CAMERA_GetNear( p_camera ); 
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
    near = 50 << FX32_SHIFT;
    FIELD_CAMERA_SetNear( p_camera, near );
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
  }

  // 生成したイベントを返す
  return p_event;
}


GMEVENT* EVENT_CameraRotateLeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep2, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraInOneFrame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame = 0;
  p_work->endFramePitch = 10;
  p_work->endFrameYaw   = 10;
  p_work->endFrameDist  = 10;
  p_work->startPitch = FIELD_CAMERA_GetAnglePitch( p_camera );
  p_work->endPitch = 0x25d8;
  p_work->startYaw = FIELD_CAMERA_GetAngleYaw( p_camera );
  p_work->endYaw = 0;
  p_work->startDist = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist = 0xed << FX32_SHIFT;

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

  // Farプレーンを半分にする
  {
    fx32 far = FIELD_CAMERA_GetFar( p_camera );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
    far /= 2;
    FIELD_CAMERA_SetFar( p_camera, far );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
  } 
  // Nearプレーンを設定する
  {
    fx32 near = FIELD_CAMERA_GetNear( p_camera ); 
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
    near = 50 << FX32_SHIFT;
    FIELD_CAMERA_SetNear( p_camera, near );
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
  }

  // 生成したイベントを返す
  return p_event;
}


GMEVENT* EVENT_CameraRotateRightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*      p_event;
  EVENT_WORK*   p_work;
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  // イベント生成
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCamera, sizeof( EVENT_WORK ) );
  // p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep, sizeof( EVENT_WORK ) );
  p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraStep2, sizeof( EVENT_WORK ) );
  //p_event = GMEVENT_Create( p_gsys, NULL, RotateCameraInOneFrame, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  p_work = GMEVENT_GetEventWork( p_event );
  p_work->pFieldmap = p_fieldmap;
  p_work->frame = 0;
  p_work->endFramePitch = 10;
  p_work->endFrameYaw   = 10;
  p_work->endFrameDist  = 10;
  p_work->startPitch = FIELD_CAMERA_GetAnglePitch( p_camera );
  p_work->endPitch = 0x25d8;
  p_work->startYaw = 0xffff / 360 * 270;
  p_work->endYaw = 0;
  p_work->startDist = FIELD_CAMERA_GetAngleLen( p_camera );
  p_work->endDist = 0xed << FX32_SHIFT;

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

  // Farプレーンを半分にする
  {
    fx32 far = FIELD_CAMERA_GetFar( p_camera );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
    far /= 2;
    FIELD_CAMERA_SetFar( p_camera, far );
    OBATA_Printf( "----------------------------------------------far = %x(%d)\n", far, far>>FX32_SHIFT );
  } 
  // Nearプレーンを設定する
  {
    fx32 near = FIELD_CAMERA_GetNear( p_camera ); 
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
    near = 50 << FX32_SHIFT;
    FIELD_CAMERA_SetNear( p_camera, near );
    OBATA_Printf( "----------------------------------------------near = %x(%d)\n", near, near>>FX32_SHIFT );
  }

  // 生成したイベントを返す
  return p_event;
}



GMEVENT_RESULT RotateCamera( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA* p_camera     = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );
  u16 pitch;
  u16 yaw;
  fx32 dist;

  // カメラパラメータを更新
  pitch = CalcPitch( p_event_work );
  yaw   = CalcYaw( p_event_work );
  dist  = CalcDist( p_event_work );
  FIELD_CAMERA_SetAnglePitch( p_camera, pitch );
  FIELD_CAMERA_SetAngleYaw( p_camera, yaw );
  FIELD_CAMERA_SetAngleLen( p_camera, dist );
  

  // 指定回数動作したら終了
  p_event_work->frame++;
  if( ( p_event_work->endFrameYaw < p_event_work->frame ) &&
      ( p_event_work->endFramePitch < p_event_work->frame ) &&
      ( p_event_work->endFrameDist < p_event_work->frame ) )
  {
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}



GMEVENT_RESULT RotateCameraStep( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA* p_camera     = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );
  u16 pitch;
  u16 yaw;
  fx32 dist;
  
  switch( *p_seq )
  {
  case 0:
    if(p_event_work->frame <= p_event_work->endFrameDist )
    {
      float progress = p_event_work->frame / (float)p_event_work->endFrameDist;
      dist = FX_F32_TO_FX32( ( (1-progress) * FX_FX32_TO_F32(p_event_work->startDist) ) + ( progress * FX_FX32_TO_F32(p_event_work->endDist) ) );
      FIELD_CAMERA_SetAngleLen( p_camera, dist );
    }
    if( p_event_work->endFrameDist < ++(p_event_work->frame) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 1:
    if(p_event_work->frame <= p_event_work->endFramePitch )
    {
      float progress = p_event_work->frame / (float)p_event_work->endFramePitch;
      pitch = (u16)( ( (1-progress) * p_event_work->startPitch ) + ( progress * p_event_work->endPitch ) );
      FIELD_CAMERA_SetAnglePitch( p_camera, pitch );
    }
    if( p_event_work->endFramePitch < ++(p_event_work->frame) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 2:
    if(p_event_work->frame <= p_event_work->endFrameYaw )
    {
      float progress = p_event_work->frame / (float)p_event_work->endFrameYaw;
      yaw = (u16)( ( (1-progress) * p_event_work->startYaw ) + ( progress * p_event_work->endYaw ) );
      FIELD_CAMERA_SetAngleYaw( p_camera, yaw );
    }
    if( p_event_work->endFrameYaw < ++(p_event_work->frame) )
    {
      return GMEVENT_RES_FINISH;
      p_event_work->frame = 0;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}


GMEVENT_RESULT RotateCameraStep2( GMEVENT* p_event, int* p_seq, void* p_work )
{
  EVENT_WORK*   p_event_work = (EVENT_WORK*)p_work;
  FIELD_CAMERA* p_camera     = FIELDMAP_GetFieldCamera( p_event_work->pFieldmap );
  u16 pitch;
  u16 yaw;
  fx32 dist;
  
  switch( *p_seq )
  {
  case 0:
    if(p_event_work->frame <= p_event_work->endFrameDist )
    {
      float progress = p_event_work->frame / (float)p_event_work->endFrameDist;
      dist = FX_F32_TO_FX32( ( (1-progress) * FX_FX32_TO_F32(p_event_work->startDist) ) + ( progress * FX_FX32_TO_F32(p_event_work->endDist) ) );
      FIELD_CAMERA_SetAngleLen( p_camera, dist );
    }
    if(p_event_work->frame <= p_event_work->endFramePitch )
    {
      float progress = p_event_work->frame / (float)p_event_work->endFramePitch;
      pitch = (u16)( ( (1-progress) * p_event_work->startPitch ) + ( progress * p_event_work->endPitch ) );
      FIELD_CAMERA_SetAnglePitch( p_camera, pitch );
    }
    p_event_work->frame++;
    if( ( p_event_work->endFramePitch < (p_event_work->frame) ) &&
        ( p_event_work->endFrameDist  < (p_event_work->frame) ) )
    {
      ++ *p_seq;
      p_event_work->frame = 0;
    }
    break;
  case 1:
  case 2:
    if(p_event_work->frame <= p_event_work->endFrameYaw )
    {
      float progress = p_event_work->frame / (float)p_event_work->endFrameYaw;
      yaw = (u16)( ( (1-progress) * p_event_work->startYaw ) + ( progress * p_event_work->endYaw ) );
      FIELD_CAMERA_SetAngleYaw( p_camera, yaw );
    }
    if( p_event_work->endFrameYaw < ++(p_event_work->frame) )
    {
      return GMEVENT_RES_FINISH;
      p_event_work->frame = 0;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}



GMEVENT_RESULT RotateCameraInOneFrame( GMEVENT* p_event, int* p_seq, void* p_work )
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
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}



static u16 CalcPitch( EVENT_WORK* p_work )
{
  u16 pitch;
  if(p_work->frame <= p_work->endFramePitch )
  {
    float progress = p_work->frame / (float)p_work->endFramePitch;
    pitch = (u16)( ( (1-progress) * p_work->startPitch ) + ( progress * p_work->endPitch ) );
  }

  return pitch;
}

static u16 CalcYaw( EVENT_WORK* p_work )
{
  u16 yaw;
  if(p_work->frame <= p_work->endFrameYaw )
  {
    float progress = p_work->frame / (float)p_work->endFrameYaw;
    yaw = (u16)( ( (1-progress) * p_work->startYaw ) + ( progress * p_work->endYaw ) );
  }
  return yaw;
}

static fx32 CalcDist( EVENT_WORK* p_work )
{
  fx32 dist;
  if(p_work->frame <= p_work->endFrameDist )
  {
    float progress = p_work->frame / (float)p_work->endFrameDist;
    dist = FX_F32_TO_FX32( ( (1-progress) * FX_FX32_TO_F32(p_work->startDist) ) + ( progress * FX_FX32_TO_F32(p_work->endDist) ) );
  }
  return dist;
}
