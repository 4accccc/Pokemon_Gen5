////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  登場イベント
 * @file   event_appear.c
 * @author obata
 * @date   2009.08.28
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "event_appear.h"
#include "fieldmap.h"
#include "field_player.h"
#include "sound/pm_sndsys.h"

#include "field_task.h"  // for FIELD_TASK
#include "field_task_manager.h" // for FIELD_TASK_MAN
#include "field_task_player_rot.h"
#include "field_task_player_fall.h"
#include "field_task_player_drawoffset.h"
#include "field_camera_anime.h" 
#include "fldeff_kemuri.h"  // for FLDEFF_KEMURI_SetMMdl
#include "event_fieldmap_control.h"  // for EVENT_FieldFadeIn
#include "event_season_display.h"  // for GetSeasonDispEventFrame
#include "event_field_fade.h" // for EVENT_FieldFadeIn_xxxx


//==========================================================================================
// ■定数
//========================================================================================== 
#define ZOOM_IN_DIST   (180 << FX32_SHIFT)   // カメラのズームイン距離
#define ZOOM_IN_FRAME  (60)   // ズームインに掛かるフレーム数
#define ZOOM_OUT_FRAME (60)   // ズームアウトに掛かるフレーム数


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct { 

  GAMESYS_WORK*  gsys; 
  FIELDMAP_WORK* fieldmap;

  // カメラアニメーション
  FIELD_CAMERA_MODE cameraMode;   // イベント開始時のカメラモード
  FCAM_ANIME_DATA   FCamAnimeData;
  FCAM_ANIME_WORK*  FCamAnimeWork;

  // フェードイン
  BOOL season_change_flag; // 季節が変化したかどうか
  int  prev_season; // 変更前の季節
  int  now_season;  // 変更後の季節
  
} EVENT_WORK;


//==========================================================================================
// ■プロトタイプ宣言
//========================================================================================== 
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Ananukenohimo( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Anawohoru( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Teleport( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Warp( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_UnionIn( GMEVENT* event, int* seq, void* wk );

// あなぬけのヒモ
static void StartCameraAnime_Ananukenohimo( EVENT_WORK* work ); // カメラの演出を開始する
static void FinishCameraAnime_Ananukenohimo( EVENT_WORK* work ); // カメラの演出を終了する
static void RegisterPlayerEffectTask_Ananukenohimo( EVENT_WORK* work ); // 自機の演出タスクを登録する
static GMEVENT* GetFadeInEvent_Ananukenohimo( EVENT_WORK* work ); // フェードインイベントを取得する
static void CallbackFunc_bySeasonDisp_Ananukenohimo( void* wk ); // 季節表示からのコールバック関数
// あなをほる
static void StartCameraAnime_Anawohoru( EVENT_WORK* work ); // カメラの演出を開始する
static void FinishCameraAnime_Anawohoru( EVENT_WORK* work ); // カメラの演出を終了する
static void RegisterPlayerEffectTask_Anawohoru( EVENT_WORK* work ); // 自機の演出タスクを登録する
static GMEVENT* GetFadeInEvent_Anawohoru( EVENT_WORK* work ); // フェードインイベントを取得する
static void CallbackFunc_bySeasonDisp_Anawohoru( void* wk ); // 季節表示からのコールバック関数


//========================================================================================== 
// ■公開関数
//========================================================================================== 

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
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( あなぬけのヒモ )
 *
 * @param parent             親イベント
 * @param gsys               ゲームシステム
 * @param fieldmap           フィールドマップ
 * @param season_change_flag 季節が変化したかどうか
 * @param prev_season        変更前の季節
 * @param now_season         変更後の季節
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Ananukenohimo( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, 
    BOOL season_change_flag, int prev_season, int now_season )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Ananukenohimo, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap           = fieldmap;
  work->gsys               = gsys;
  work->season_change_flag = season_change_flag;
  work->prev_season        = prev_season;
  work->now_season         = now_season;

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( あなをほる )
 *
 * @param parent             親イベント
 * @param gsys               ゲームシステム
 * @param fieldmap           フィールドマップ
 * @param season_change_flag 季節が変化したかどうか
 * @param prev_season        変更前の季節
 * @param now_season         変更後の季節
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Anawohoru( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, 
    BOOL season_change_flag, int prev_season, int now_season )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Anawohoru, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap           = fieldmap;
  work->gsys               = gsys;
  work->season_change_flag = season_change_flag;
  work->prev_season        = prev_season;
  work->now_season         = now_season;

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( テレポート )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Teleport( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Teleport, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}

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
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Warp, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( ユニオン入室 )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_UnionIn( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_UnionIn, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}

//========================================================================================== 
// ■非公開関数の定義
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( 落下 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    { // 主人公の描画オフセットの初期設定(画面外にいるように設定)
      VecFx32       offset = {0, 250<<FX32_SHIFT, 0};
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
      MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
			MMDL_SetAcmd( mmdl, AC_DIR_D ); // 向きを初期化
    }
    { // フェードイン
      GMEVENT* fadeInEvent;
      fadeInEvent = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fadeInEvent );
    }
    { // タスクの追加
      FIELD_TASK* fall;
      FIELD_TASK_MAN* man;
      FIELD_PLAYER* player;
			MMDL* mmdl;
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
			mmdl   = FIELD_PLAYER_GetMMdl( player ); 
      fall   = FIELD_TASK_PlayerFall( work->fieldmap, mmdl, 40, 250 );
      man    = FIELDMAP_GetTaskManager( work->fieldmap ); 
      FIELD_TASK_MAN_AddTask( man, fall, NULL );
    }
    (*seq)++;
    break;
  case 1:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        (*seq)++;
      }
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    (*seq)++;
    break;
  case 3: 
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 

//==========================================================================================
/**
 * @brief 登場イベント処理関数( あなぬけのヒモ )
 */
//==========================================================================================
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Ananukenohimo( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work       = (EVENT_WORK*)wk;
  GAMESYS_WORK*   gameSystem = work->gsys;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  FIELD_CAMERA*   camera     = FIELDMAP_GetFieldCamera( fieldmap );
  FIELD_TASK_MAN* taskMan     = FIELDMAP_GetTaskManager( fieldmap ); 

  switch( *seq ) {
  case 0:
    // カメラのトレース処理停止リクエスト発行
    FIELD_CAMERA_StopTraceRequest( camera );
    (*seq)++;
    break;
  case 1:
    // カメラのトレース処理終了待ち
    if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ) { (*seq)++; }
    break;

  case 2:
    // 季節表示が有効な場合は, コールバック関数で処理する
    if( work->season_change_flag == FALSE ) {
      RegisterPlayerEffectTask_Ananukenohimo( work ); // 自機の演出タスクの追加
      StartCameraAnime_Ananukenohimo( work ); // カメラ演出開始
    }
    (*seq)++;
    break;

  case 3:
    // 画面フェードイン開始
    GMEVENT_CallEvent( event, GetFadeInEvent_Ananukenohimo( work ) );
    (*seq)++;
    break;

  case 4:
    // タスクの終了待ち
    if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) { (*seq)++; }
    break;

  case 5:
    FinishCameraAnime_Ananukenohimo( work ); // カメラ演出終了
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラの演出を開始する ( あなぬけのヒモ )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void StartCameraAnime_Ananukenohimo( EVENT_WORK* work )
{
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  work->FCamAnimeWork = FCAM_ANIME_CreateWork( work->fieldmap );
  work->FCamAnimeData.frame = 60;
  work->FCamAnimeData.targetBindOffFlag = TRUE;
  work->FCamAnimeData.cameraAreaOffFlag = TRUE;
  FCAM_ANIME_SetAnimeData( work->FCamAnimeWork, &work->FCamAnimeData );
  FCAM_ANIME_SetupCamera( work->FCamAnimeWork );
  FCAM_PARAM_GetCurrentParam( camera, &work->FCamAnimeData.endParam );
  work->FCamAnimeData.startParam = work->FCamAnimeData.endParam;
  work->FCamAnimeData.startParam.length -= (100 << FX32_SHIFT);
  FCAM_ANIME_SetCameraStartParam( work->FCamAnimeWork );
  FCAM_ANIME_StartAnime( work->FCamAnimeWork );
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラの演出を終了する ( あなぬけのヒモ )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void FinishCameraAnime_Ananukenohimo( EVENT_WORK* work )
{
  FCAM_ANIME_RecoverCamera( work->FCamAnimeWork );
  FCAM_ANIME_DeleteWork( work->FCamAnimeWork );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の演出タスクを登録する ( あなぬけのヒモ )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void RegisterPlayerEffectTask_Ananukenohimo( EVENT_WORK* work )
{
  FIELD_TASK_MAN* taskMan = FIELDMAP_GetTaskManager( work->fieldmap ); 
  FIELD_TASK* rot;

  rot = FIELD_TASK_PlayerRotate_SpeedDown( work->fieldmap, 60, 8 );
  FIELD_TASK_MAN_AddTask( taskMan, rot, NULL );
}

//------------------------------------------------------------------------------------------
/**
 * @brief フェードインイベントを取得する ( あなぬけのヒモ )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static GMEVENT* GetFadeInEvent_Ananukenohimo( EVENT_WORK* work )
{
  GAMESYS_WORK*  gameSystem = work->gsys;
  FIELDMAP_WORK* fieldmap   = work->fieldmap;
  GMEVENT* event;

  // 季節が変化
  if( work->season_change_flag ) {
    event = EVENT_SeasonIn_Callback( 
        gameSystem, fieldmap, work->prev_season, work->now_season,
        CallbackFunc_bySeasonDisp_Ananukenohimo, work ); // 季節フェード
  }
  // 季節そのまま
  else {
    event = EVENT_FieldFadeIn_White( gameSystem, fieldmap,  FIELD_FADE_NO_WAIT ); // ホワイトフェード
  }

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 季節表示からのコールバック関数
 *
 * @param wk EVENT_WORK*
 */
//------------------------------------------------------------------------------------------
static void CallbackFunc_bySeasonDisp_Ananukenohimo( void* wk )
{
  EVENT_WORK* work = wk;

  RegisterPlayerEffectTask_Ananukenohimo( work );
  StartCameraAnime_Ananukenohimo( work );
}


//========================================================================================== 
/**
 * @brief 登場イベント処理関数( あなをほる )
 */
//========================================================================================== 
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Anawohoru( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work       = (EVENT_WORK*)wk;
  GAMESYS_WORK*   gameSystem = work->gsys;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  FIELD_CAMERA*   camera     = FIELDMAP_GetFieldCamera( fieldmap );
  FIELD_TASK_MAN* taskMan    = FIELDMAP_GetTaskManager( fieldmap ); 

  switch( *seq ) {
  case 0:
    // カメラのトレース処理停止リクエスト発行
    FIELD_CAMERA_StopTraceRequest( camera );
    (*seq)++;
    break;
  case 1:
    // カメラのトレース処理終了待ち
    if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ) { (*seq)++; }
    break;

  case 2:
    // 季節表示が有効な場合は, コールバック関数で処理する
    if( work->season_change_flag == FALSE ) {
      RegisterPlayerEffectTask_Anawohoru( work ); // 自機の演出タスクを登録
      StartCameraAnime_Anawohoru( work ); // カメラ演出開始
    }
    (*seq)++;
    break;

  case 3:
    // 画面フェードイン開始
    GMEVENT_CallEvent( event, GetFadeInEvent_Anawohoru( work ) );
    (*seq)++;
    break;

  case 4:
    // タスクの終了待ち
    if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) { (*seq)++; }
    break;

  case 5:
    FinishCameraAnime_Anawohoru( work ); // カメラ演出終了
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラの演出を開始する ( あなをほる )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void StartCameraAnime_Anawohoru( EVENT_WORK* work )
{
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  work->FCamAnimeWork = FCAM_ANIME_CreateWork( work->fieldmap );
  work->FCamAnimeData.frame = 60;
  work->FCamAnimeData.targetBindOffFlag = TRUE;
  work->FCamAnimeData.cameraAreaOffFlag = TRUE;
  FCAM_ANIME_SetAnimeData( work->FCamAnimeWork, &work->FCamAnimeData );
  FCAM_ANIME_SetupCamera( work->FCamAnimeWork );
  FCAM_PARAM_GetCurrentParam( camera, &work->FCamAnimeData.endParam );
  work->FCamAnimeData.startParam = work->FCamAnimeData.endParam;
  work->FCamAnimeData.startParam.length -= (100 << FX32_SHIFT);
  FCAM_ANIME_SetCameraStartParam( work->FCamAnimeWork );
  FCAM_ANIME_StartAnime( work->FCamAnimeWork );
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラの演出を終了する ( あなをほる )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void FinishCameraAnime_Anawohoru( EVENT_WORK* work )
{
  FCAM_ANIME_RecoverCamera( work->FCamAnimeWork );
  FCAM_ANIME_DeleteWork( work->FCamAnimeWork );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の演出タスクを登録する ( あなをほる )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void RegisterPlayerEffectTask_Anawohoru( EVENT_WORK* work )
{
  FIELD_TASK_MAN* taskMan = FIELDMAP_GetTaskManager( work->fieldmap ); 
  FIELD_TASK* rot;

  rot = FIELD_TASK_PlayerRotate_SpeedDown( work->fieldmap, 60, 8 );
  FIELD_TASK_MAN_AddTask( taskMan, rot, NULL );
}

//------------------------------------------------------------------------------------------
/**
 * @brief フェードインイベントを取得する ( あなをほる )
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static GMEVENT* GetFadeInEvent_Anawohoru( EVENT_WORK* work )
{
  GAMESYS_WORK*  gameSystem = work->gsys;
  FIELDMAP_WORK* fieldmap   = work->fieldmap;
  GMEVENT* event;

  // 季節が変化
  if( work->season_change_flag ) {
    event = EVENT_SeasonIn_Callback( 
        gameSystem, fieldmap, work->prev_season, work->now_season,
        CallbackFunc_bySeasonDisp_Anawohoru, work ); // 季節フェード
  }
  // 季節そのまま
  else {
    event = EVENT_FieldFadeIn_White( gameSystem, fieldmap,  FIELD_FADE_NO_WAIT ); // ホワイトフェード
  }

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 季節表示からのコールバック関数
 *
 * @param wk EVENT_WORK*
 */
//------------------------------------------------------------------------------------------
static void CallbackFunc_bySeasonDisp_Anawohoru( void* wk )
{
  EVENT_WORK* work = wk;

  RegisterPlayerEffectTask_Anawohoru( work ); // 自機の演出タスクを登録
  StartCameraAnime_Anawohoru( work ); // カメラ演出開始
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( テレポート )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Teleport( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work       = (EVENT_WORK*)wk;
  GAMESYS_WORK*   gameSystem = work->gsys;
  FIELDMAP_WORK*  fieldmap   = work->fieldmap;
  FIELD_CAMERA*   camera     = FIELDMAP_GetFieldCamera( fieldmap );
  FIELD_TASK_MAN* taskMan    = FIELDMAP_GetTaskManager( fieldmap ); 

  switch( *seq ) {
  case 0:
    // カメラのトレース処理停止リクエスト発行
    FIELD_CAMERA_StopTraceRequest( camera );
    (*seq)++;
    break;
  case 1:
    // カメラのトレース処理終了待ち
    if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ) { (*seq)++; }
    break;

  case 2:
    // タスクの追加
    { 
      FIELD_TASK* rot;
      rot  = FIELD_TASK_PlayerRotate_SpeedDown( fieldmap, 60, 8 );
      FIELD_TASK_MAN_AddTask( taskMan, rot, NULL );
    }
    // カメラ演出開始
    work->FCamAnimeWork = FCAM_ANIME_CreateWork( fieldmap );
    work->FCamAnimeData.frame = 60;
    work->FCamAnimeData.targetBindOffFlag = TRUE;
    work->FCamAnimeData.cameraAreaOffFlag = TRUE;
    FCAM_ANIME_SetAnimeData( work->FCamAnimeWork, &work->FCamAnimeData );
    FCAM_ANIME_SetupCamera( work->FCamAnimeWork );
    FCAM_PARAM_GetCurrentParam( camera, &work->FCamAnimeData.endParam );
    work->FCamAnimeData.startParam = work->FCamAnimeData.endParam;
    work->FCamAnimeData.startParam.length -= (100 << FX32_SHIFT);
    FCAM_ANIME_SetCameraStartParam( work->FCamAnimeWork );
    FCAM_ANIME_StartAnime( work->FCamAnimeWork );
    (*seq)++;
    break;

  case 3:
    // 画面フェードイン開始
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeIn_Black( gameSystem, fieldmap,  FIELD_FADE_NO_WAIT ) );
    (*seq)++;
    break;

  case 4:
    // タスクの終了待ち
    if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) { 
      // 煙エフェクト表示
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
      MMDL*         mmdl   = FIELD_PLAYER_GetMMdl( player );
      FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( fieldmap );
      FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
      (*seq)++; 
    }
    break;

  case 5:
    FCAM_ANIME_RecoverCamera( work->FCamAnimeWork );
    FCAM_ANIME_DeleteWork( work->FCamAnimeWork );
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( ワープ )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Warp( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*    work     = (EVENT_WORK*)wk;
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  FIELD_CAMERA*  camera   = FIELDMAP_GetFieldCamera( fieldmap );

  switch( *seq ) {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    { // 主人公の描画オフセットの初期設定(画面外にいるように設定)
      VecFx32 moveOffset;
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
      MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
      VEC_Set( &moveOffset, 0, 10<<FX32_SHIFT, 0);
      MMDL_SetVectorDrawOffsetPos( mmdl, &moveOffset );
    }
    { // フェードイン
      GMEVENT* fadeInEvent;
      fadeInEvent = EVENT_FieldFadeIn_Black( work->gsys, fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fadeInEvent );
    }
    { // タスクの追加
      FIELD_TASK* rotTask;
      FIELD_TASK* moveTask;
      FIELD_TASK_MAN* taskMan;
      VecFx32 moveVec;
      VEC_Set( &moveVec, 0, 150<<FX32_SHIFT, 0 );
      rotTask = FIELD_TASK_PlayerRotate( fieldmap, 24, 3 );
      moveTask = FIELD_TASK_TransDrawOffset( fieldmap, -24, &moveVec );
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, rotTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, moveTask, NULL );
    }
    (*seq)++;
    break;
  case 1:
    { // タスクの終了待ち
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) { (*seq)++; }
    }
    break;
  case 2:
    // 砂埃を出す
    {
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
      MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
      FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
      FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
    }
    (*seq)++;
    break;
  case 3:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    (*seq)++;
    break;
  case 4: 
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( ユニオン入室 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_UnionIn( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(work->fieldmap);
  MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    { 
      MMDL_SetAcmd(player_mmdl, AC_WARP_DOWN);
    }
    { // フェードイン
      GMEVENT* fadeInEvent;
      fadeInEvent = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fadeInEvent );
    }
    (*seq)++;
    break;
  case 1:
    if(MMDL_CheckEndAcmd(player_mmdl) == TRUE){
      MMDL_EndAcmd(player_mmdl);
      (*seq)++;
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    (*seq)++;
    break;
  case 3: 
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 

