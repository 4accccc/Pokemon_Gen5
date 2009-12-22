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

#include "gamesystem/game_event.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h"

#include "event_entrance_in.h"

#include "event_fieldmap_control.h" // EVENT_FieldFadeOut
#include "event_entrance_effect.h"  // EVENT_FieldDoorInAnime
#include "event_fldmmdl_control.h"  // EVENT_PlayerOneStepAnime
#include "sound/pm_sndsys.h"
#include "field_sound.h"
#include "sound/bgm_info.h"
#include "../../resource/sound/bgm_info/iss_type.h"
#include "event_disappear.h"  // for EVENT_DISAPPEAR_xxxx
#include "field_status_local.h"  // for FIELD_STATUS_

#include "fieldmap.h"
#include "field_task.h"  
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"

//=======================================================================================
/**
 * @brief イベント・ワーク
 */
//=======================================================================================
typedef struct
{
  GAMESYS_WORK*    gsys;
  GAMEDATA*        gdata;
  FIELDMAP_WORK* fieldmap;
  LOCATION         location;  // 遷移先指定
}
EVENT_WORK;


//=======================================================================================
/**
 * @breif 非公開関数のプロトタイプ宣言
 */
//======================================================================================= 

// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP1(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP2(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP3(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP4(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP5(GMEVENT * event, int *seq, void * work);


//=======================================================================================
/**
 * @breif 公開関数の定義
 */
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口進入イベントを作成する
 *
 * @param parent    親イベント
 * @param gsys      ゲームシステム
 * @param gdata     ゲームデータ
 * @param fieldmap  フィールドマップ
 * @param location  遷移先指定
 * @param exit_type 遷移タイプ指定
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceIn( GMEVENT* parent, 
                           GAMESYS_WORK* gsys,
                           GAMEDATA* gdata, 
                           FIELDMAP_WORK* fieldmap, 
                           LOCATION location, 
                           EXIT_TYPE exit_type )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[] = 
  {
    EVENT_FUNC_EntranceIn_ExitTypeNone,   //EXIT_TYPE_NONE
    EVENT_FUNC_EntranceIn_ExitTypeNone,   //EXIT_TYPE_MAT
    EVENT_FUNC_EntranceIn_ExitTypeStep,   //EXIT_TYPE_STAIRS
    EVENT_FUNC_EntranceIn_ExitTypeDoor,   //EXIT_TYPE_DOOR
    EVENT_FUNC_EntranceIn_ExitTypeStep,   //EXIT_TYPE_WALL
    EVENT_FUNC_EntranceIn_ExitTypeWarp,   //EXIT_TYPE_WARP
    EVENT_FUNC_EntranceIn_ExitTypeSP1,    //EXIT_TYPE_SP1
    EVENT_FUNC_EntranceIn_ExitTypeSP2,    //EXIT_TYPE_SP2
    EVENT_FUNC_EntranceIn_ExitTypeSP3,    //EXIT_TYPE_SP3
    EVENT_FUNC_EntranceIn_ExitTypeSP4,    //EXIT_TYPE_SP4
    EVENT_FUNC_EntranceIn_ExitTypeSP5,    //EXIT_TYPE_SP5
  };

  // イベント作成
  event = GMEVENT_Create( gsys, parent, eventFuncTable[ exit_type ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->gdata    = gdata;
  work->fieldmap = fieldmap;
  work->location = location;

  // 作成したイベントを返す
  return event;
}


//=======================================================================================
/**
 * @breif 非公開関数の定義
 */
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;
	GAMEDATA*        gamedata   = event_work->gdata;

  switch ( *seq )
  {
  case 0:
    { // BGM更新リクエスト
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      PLAYER_WORK* player = GAMEDATA_GetPlayerWork( gamedata, 0 );
      PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
      FIELD_SOUND_ChangePlayZoneBGM( fsnd, gamedata, form, event_work->location.zone_id );
    }
    { // フェードアウト
      GMEVENT* fade_event;
      FIELD_STATUS* fstatus; 
      fstatus = GAMEDATA_GetFieldStatus( gamedata );
      if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示あり)
      { // 輝度フェード
        fade_event = EVENT_FieldFadeOut_Black(gsys, fieldmap, FIELD_FADE_WAIT);
      }
      else
      { // クロスフェード
        fade_event = EVENT_FieldFadeOut_Cross(gsys, fieldmap);
      }
      GMEVENT_CallEvent( event, fade_event );
    }
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &event_work->location, TRUE ) );
    ++ *seq;
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
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;
	GAMEDATA*        gamedata   = event_work->gdata;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gsys, fieldmap) );
    ++ *seq;
    break;
  case 1: // BGMフェードアウト
    { // 現在のBGMがダンジョンISS && 次のBGMもダンジョンISS ==> BGMフェードアウト
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      BGM_INFO_SYS* bgm_info = GAMEDATA_GetBGMInfoSys( gamedata );
      PLAYER_WORK* player = GAMEDATA_GetPlayerWork( gamedata, 0 );
      PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player );
      u32 bgm_next = FIELD_SOUND_GetFieldBGMNo( gamedata, form, event_work->location.zone_id );
      u32 bgm_now = PMSND_GetBGMsoundNo();
      u8 iss_type_next = BGM_INFO_GetIssType( bgm_info, bgm_next ); 
      u8 iss_type_now = BGM_INFO_GetIssType( bgm_info, bgm_now ); 
      if( ( iss_type_next == ISS_TYPE_DUNGEON ) &&
          ( iss_type_now == ISS_TYPE_DUNGEON ) )
      {
        FIELD_SOUND_FadeOutBGM( fsnd, 20 );
      }
    }
    ++ *seq;
    break;
  case 2: // BGMフェードアウト待ち
    if( PMSND_CheckFadeOnBGM() != TRUE )
    { 
      PMSND_PlaySE( SEQ_SE_KAIDAN );    // SE
      { // フェードアウト
        GMEVENT* fade_event;
        FIELD_STATUS* fstatus; 
        fstatus = GAMEDATA_GetFieldStatus( gamedata );
        if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示あり)
        { // 輝度フェード
          fade_event = EVENT_FieldFadeOut_Black(gsys, fieldmap, FIELD_FADE_WAIT);
        }
        else
        { // クロスフェード
          fade_event = EVENT_FieldFadeOut_Cross(gsys, fieldmap);
        }
        GMEVENT_CallEvent( event, fade_event );
      }
      ++ *seq;
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ワープ時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK*   fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
		GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Teleport(NULL, gsys, fieldmap) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SP1 )
 *
 * ※C01のジム
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP1(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    // タスク登録
    {
      u16 frame;
      fx32 val_len;
      u16 val_pitch, val_yaw;
      VecFx32 val_target;
      FIELD_TASK_MAN* man;
      FIELD_TASK* zoom;
      FIELD_TASK* pitch;
      FIELD_TASK* yaw;
      FIELD_TASK* target;
      frame     = 20;
      val_len   = 0x00ed << FX32_SHIFT;
      val_pitch = 0x25fc;
      val_yaw   = 0x0000;
      VEC_Set( &val_target, 0x0000, 0x0028<<FX32_SHIFT, 0xfffD9000 );
      zoom   = FIELD_TASK_CameraLinearZoom( fieldmap, frame, val_len );
      pitch  = FIELD_TASK_CameraRot_Pitch( fieldmap, frame, val_pitch );
      yaw    = FIELD_TASK_CameraRot_Yaw( fieldmap, frame, val_yaw );
      target = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &val_target );
      man = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( man, zoom, NULL );
      FIELD_TASK_MAN_AddTask( man, pitch, NULL );
      FIELD_TASK_MAN_AddTask( man, yaw, NULL );
      FIELD_TASK_MAN_AddTask( man, target, NULL );
    }
    ++ *seq;
    break;
  case 1:
    // タスク終了待ち
    {
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++ *seq; 
      }
    }
    break;
  case 2: 
    { // カメラ主人公追尾OFF
      FIELD_CAMERA* camera;
      camera = FIELDMAP_GetFieldCamera( fieldmap );
      FIELD_CAMERA_FreeTarget( camera );
    }
    ++ *seq;
    break;
  case 3:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &event_work->location, FALSE ) );
    ++ *seq;
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SP2 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP2(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &event_work->location, FALSE ) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SP3 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP3(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &event_work->location, FALSE ) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SP4 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP4(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &event_work->location, FALSE ) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SP5 )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSP5(GMEVENT * event, int *seq, void * work)
{
	EVENT_WORK*      event_work = work;
	GAMESYS_WORK*    gsys       = event_work->gsys;
	FIELDMAP_WORK* fieldmap   = event_work->fieldmap;

  switch ( *seq )
  {
  case 0:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gsys, fieldmap, &event_work->location, FALSE ) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
