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

#include "arc/arc_def.h"
#include "../../resource/fldmapdata/entrance_camera/entrance_camera.naix"


//=======================================================================================
// ■イベント ワーク
//=======================================================================================
typedef struct
{
  GAMESYS_WORK*      gsys;
  GAMEDATA*         gdata;
  FIELDMAP_WORK* fieldmap;
  LOCATION       location;  // 遷移先指定
  EXIT_TYPE      exitType;  // 出入り口タイプ
}
EVENT_WORK;


//=======================================================================================
// ■非公開関数のプロトタイプ宣言
//======================================================================================= 

// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp(GMEVENT * event, int *seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSPx(GMEVENT* event, int* seq, void* wk);


//=======================================================================================
// ■公開関数の定義
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
  int eventFuncIndex;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[] = 
  {
    EVENT_FUNC_EntranceIn_ExitTypeNone,   //EXIT_TYPE_NONE
    EVENT_FUNC_EntranceIn_ExitTypeNone,   //EXIT_TYPE_MAT
    EVENT_FUNC_EntranceIn_ExitTypeStep,   //EXIT_TYPE_STAIRS
    EVENT_FUNC_EntranceIn_ExitTypeDoor,   //EXIT_TYPE_DOOR
    EVENT_FUNC_EntranceIn_ExitTypeStep,   //EXIT_TYPE_WALL
    EVENT_FUNC_EntranceIn_ExitTypeWarp,   //EXIT_TYPE_WARP
    EVENT_FUNC_EntranceIn_ExitTypeNone,   //EXIT_TYPE_INTRUDE
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP1
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP2
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP3
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP4
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP5
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP6
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP7
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP8
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP9
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP10
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP11
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP12
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP13
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP14
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP15
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP16
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP17
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP18
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP19
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP20
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP21
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP22
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP23
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP24
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP25
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP26
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP27
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP28
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP29
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    //EXIT_TYPE_SP30
  };

  // イベント作成
  event = GMEVENT_Create( gsys, parent, eventFuncTable[ exit_type ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->gdata    = gdata;
  work->fieldmap = fieldmap;
  work->location = location;
  work->exitType = exit_type;

  // 作成したイベントを返す
  return event;
}


//=======================================================================================
// ■非公開関数の定義
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
    { // BGM再生準備
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
      u16 nowZoneID = FIELDMAP_GetZoneID( fieldmap );
      FSND_StandByNextMapBGM( fsnd, gamedata, nowZoneID, event_work->location.zone_id );
    }
    (*seq)++;
    break;
  case 1:
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
  case 2:
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
      u32 bgm_next = FSND_GetFieldBGM( gamedata, event_work->location.zone_id );
      u32 bgm_now = PMSND_GetBGMsoundNo();
      u8 iss_type_next = BGM_INFO_GetIssType( bgm_info, bgm_next ); 
      u8 iss_type_now = BGM_INFO_GetIssType( bgm_info, bgm_now ); 
      if( ( iss_type_next == ISS_TYPE_DUNGEON ) &&
          ( iss_type_now == ISS_TYPE_DUNGEON ) )
      { // BGMフェードアウト
        GMEVENT* fadeOutEvent;
        FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
        fadeOutEvent = EVENT_FSND_FadeOutBGM( gsys, FSND_FADE_SHORT );
        GMEVENT_CallEvent( event, fadeOutEvent );
      }
      else
      { // BGM再生準備
        FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gamedata );
        u16 nowZoneID = FIELDMAP_GetZoneID( fieldmap );
        FSND_StandByNextMapBGM( fsnd, gamedata, nowZoneID, event_work->location.zone_id );
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
		GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Warp(NULL, gsys, fieldmap) );
    ++ *seq;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//=======================================================================================
// ■特殊進入イベントのカメラ動作データ
//=======================================================================================
typedef struct 
{
  u32 exitType;        // 出入り口タイプ
  u32 pitch;           // ピッチ
  u32 yaw;             // ヨー
  u32 length;          // 距離
  u32 targetOffsetX;   // ターゲットオフセットx
  u32 targetOffsetY;   // ターゲットオフセットy
  u32 targetOffsetZ;   // ターゲットオフセットz
  u32 frame;           // フレーム数

} ENTRANCE_CAMERA_ACTION;


//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SPx )
 *
 * ※カメラのアニメーション → ドア進入アニメ
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSPx(GMEVENT* event, int* seq, void* wk)
{
	EVENT_WORK*         work = wk;
	GAMESYS_WORK* gameSystem = work->gsys;
	FIELDMAP_WORK*  fieldmap = work->fieldmap;
  FIELD_CAMERA*     camera = FIELDMAP_GetFieldCamera( fieldmap );

  // 出入り口タイプに対応するカメラデータのインデックス
  const ARCDATID dataIndex[] = 
  {
    0, 0, 0, 0, 0, 0, 0,  // EXIT_TYPE_NONE - EXIT_TYPE_INTRUDE
    NARC_entrance_camera_exit_type_sp1_bin,    // EXIT_TYPE_SP1
    NARC_entrance_camera_exit_type_sp2_bin,    // EXIT_TYPE_SP2
    NARC_entrance_camera_exit_type_sp3_bin,    // EXIT_TYPE_SP3
    NARC_entrance_camera_exit_type_sp4_bin,    // EXIT_TYPE_SP4
    NARC_entrance_camera_exit_type_sp5_bin,    // EXIT_TYPE_SP5
    NARC_entrance_camera_exit_type_sp6_bin,    // EXIT_TYPE_SP6
    NARC_entrance_camera_exit_type_sp7_bin,    // EXIT_TYPE_SP7
    NARC_entrance_camera_exit_type_sp8_bin,    // EXIT_TYPE_SP8
    NARC_entrance_camera_exit_type_sp9_bin,    // EXIT_TYPE_SP9
    NARC_entrance_camera_exit_type_sp10_bin,   // EXIT_TYPE_SP10
    NARC_entrance_camera_exit_type_sp11_bin,   // EXIT_TYPE_SP11
    NARC_entrance_camera_exit_type_sp12_bin,   // EXIT_TYPE_SP12
    NARC_entrance_camera_exit_type_sp13_bin,   // EXIT_TYPE_SP13
    NARC_entrance_camera_exit_type_sp14_bin,   // EXIT_TYPE_SP14
    NARC_entrance_camera_exit_type_sp15_bin,   // EXIT_TYPE_SP15
    NARC_entrance_camera_exit_type_sp16_bin,   // EXIT_TYPE_SP16
    NARC_entrance_camera_exit_type_sp17_bin,   // EXIT_TYPE_SP17
    NARC_entrance_camera_exit_type_sp18_bin,   // EXIT_TYPE_SP18
    NARC_entrance_camera_exit_type_sp19_bin,   // EXIT_TYPE_SP19
    NARC_entrance_camera_exit_type_sp20_bin,   // EXIT_TYPE_SP20
    NARC_entrance_camera_exit_type_sp21_bin,   // EXIT_TYPE_SP21
    NARC_entrance_camera_exit_type_sp22_bin,   // EXIT_TYPE_SP22
    NARC_entrance_camera_exit_type_sp23_bin,   // EXIT_TYPE_SP23
    NARC_entrance_camera_exit_type_sp24_bin,   // EXIT_TYPE_SP24
    NARC_entrance_camera_exit_type_sp25_bin,   // EXIT_TYPE_SP25
    NARC_entrance_camera_exit_type_sp26_bin,   // EXIT_TYPE_SP26
    NARC_entrance_camera_exit_type_sp27_bin,   // EXIT_TYPE_SP27
    NARC_entrance_camera_exit_type_sp28_bin,   // EXIT_TYPE_SP28
    NARC_entrance_camera_exit_type_sp29_bin,   // EXIT_TYPE_SP29
    NARC_entrance_camera_exit_type_sp30_bin,   // EXIT_TYPE_SP30
  };

  // 処理シーケンス
  enum{
    SEQ_CREATE_CAMERA_EFFECT_TASK,  // カメラ演出タスクの作成
    SEQ_WAIT_CAMERA_EFFECT_TASK,    // カメラ演出タスク終了待ち
    SEQ_CAMERA_STOP_TRACE_REQUEST,  // カメラの自機追従OFFリクエスト発行
    SEQ_WAIT_CAMERA_TRACE,          // カメラの自機追従処理の終了待ち
    SEQ_CAMERA_TRACE_OFF,           // カメラの自機追従OFF
    SEQ_DOOR_IN_ANIME,              // ドア進入イベント
    SEQ_EXIT,                       // イベント終了
  };

  switch( *seq )
  {
  // カメラ演出タスクの作成
  case SEQ_CREATE_CAMERA_EFFECT_TASK:
    {
      u16 frame;
      u16 pitch, yaw;
      fx32 length;
      VecFx32 targetOffset;
      // 各パラメータ取得
      {
        ENTRANCE_CAMERA_ACTION cameraAction;
        GFL_ARC_LoadData( &cameraAction, ARCID_ENTRANCE_CAMERA, dataIndex[ work->exitType ] );
        frame  = cameraAction.frame;
        pitch  = cameraAction.pitch;
        yaw    = cameraAction.yaw;
        length = cameraAction.length << FX32_SHIFT;
        VEC_Set( &targetOffset, 
                 cameraAction.targetOffsetX << FX32_SHIFT,
                 cameraAction.targetOffsetY << FX32_SHIFT,
                 cameraAction.targetOffsetZ << FX32_SHIFT );
        // DEBUG:
        OBATA_Printf( "frame   = %d\n", cameraAction.frame );
        OBATA_Printf( "pitch   = %x\n", cameraAction.pitch );
        OBATA_Printf( "yaw     = %x\n", cameraAction.yaw );
        OBATA_Printf( "length  = %x\n", cameraAction.length );
        OBATA_Printf( "offsetX = %x\n", cameraAction.targetOffsetX );
        OBATA_Printf( "offsetY = %x\n", cameraAction.targetOffsetX );
        OBATA_Printf( "offsetZ = %x\n", cameraAction.targetOffsetX );
      }
      // タスク登録
      {
        FIELD_TASK_MAN* taskMan;
        FIELD_TASK* zoomTaks;
        FIELD_TASK* pitchTask;
        FIELD_TASK* yawTask;
        FIELD_TASK* targetOffsetTask;
        // 生成
        zoomTaks  = FIELD_TASK_CameraLinearZoom( fieldmap, frame, length );
        pitchTask = FIELD_TASK_CameraRot_Pitch( fieldmap, frame, pitch );
        yawTask   = FIELD_TASK_CameraRot_Yaw( fieldmap, frame, yaw );
        targetOffsetTask = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &targetOffset );
        // 登録
        taskMan = FIELDMAP_GetTaskManager( fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, zoomTaks, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, pitchTask, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, yawTask, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, targetOffsetTask, NULL );
      }
    }
    *seq = SEQ_WAIT_CAMERA_EFFECT_TASK;
    break;
  // タスク終了待ち
  case SEQ_WAIT_CAMERA_EFFECT_TASK:
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ){ *seq = SEQ_CAMERA_STOP_TRACE_REQUEST; }
    }
    break;
  // カメラのトレース処理停止リクエスト発行
  case SEQ_CAMERA_STOP_TRACE_REQUEST:
    FIELD_CAMERA_StopTraceRequest( camera );
    *seq = SEQ_WAIT_CAMERA_TRACE;
    break;
  // カメラのトレース処理終了待ち
  case SEQ_WAIT_CAMERA_TRACE: 
    if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ){ *seq = SEQ_CAMERA_TRACE_OFF; }
    break;
  // カメラのトレースOFF
  case SEQ_CAMERA_TRACE_OFF:
    FIELD_CAMERA_FreeTarget( camera );
    *seq = SEQ_DOOR_IN_ANIME;
    break;
  // ドア進入アニメ
  case SEQ_DOOR_IN_ANIME:
    GMEVENT_CallEvent( event, EVENT_FieldDoorInAnime( gameSystem, fieldmap, &work->location, FALSE ) );
    *seq = SEQ_EXIT;
    break;
  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
