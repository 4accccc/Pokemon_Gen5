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
#include "fldeff_iwakudaki.h"
#include "fldeff_gyoe.h"
#include "sound/pm_sndsys.h"
#include "field_task.h"
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_player_rot.h"
#include "field_task_player_trans.h"
#include "field_task_player_drawoffset.h"
#include "field_task_fade.h"
#include "field_task_wait.h"
#include "field_task_anahori_effect.h"
#include "field_buildmodel.h"
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "field_camera_anime.h"
#include "event_field_fade.h" // for EVENT_FieldFadeOut_xxxx


//==========================================================================================
// ■定数
//========================================================================================== 
#define ZOOM_IN_DIST   (180 << FX32_SHIFT)   // カメラズーム時の距離
#define ZOOM_IN_FRAME  (30)   // ズームインに掛かるフレーム数
#define ZOOM_OUT_FRAME (60)   // ズームアウトに掛かるフレーム数
#define ANAHORI_EFF_INTVL (6)  //「あなをほる」のエフェクト表示間隔


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct {

  GAMESYS_WORK*     gameSystem;
  FIELDMAP_WORK*    fieldmap;
  FIELD_CAMERA_MODE camera_mode; // イベント開始時のカメラモード
  int               frame;       // フレーム数カウンタ
  BOOL              teleport_f;  // テレポートとユニオンルーム退場が同じイベントなので、SE分岐のためフラグを持つ(100601 iwasawa)

  // 流砂イベント
  VecFx32       sandStreamPos; // 流砂中心部の位置
  FIELD_BMODEL* bmSandStream;  // 流砂の配置モデル

  // カメラアニメーション
  FCAM_ANIME_DATA  FCamAnimeData;
  FCAM_ANIME_WORK* FCamAnimeWork;

  // フェードアウト
  int  wait_count;         // フェードアウト開始待ちカウンタ
  BOOL season_change_flag; // 季節が変化したかどうか

} EVENT_WORK;


//==========================================================================================
// ■プロトタイプ宣言
//========================================================================================== 
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_FallInSand( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Ananukenohimo( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Anawohoru( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Teleport( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Warp( GMEVENT* event, int* seq, void* wk );

// あなぬけのヒモ
static void RegisterPlayerEffectTask_Ananukenohimo( EVENT_WORK* work ); // 自機の演出タスクを登録する
static void StartCameraAnime_Ananukenohimo( EVENT_WORK* work ); // カメラ演出を開始する
static void FinishCameraAnime_Ananukenohimo( EVENT_WORK* work ); // カメラ演出を終了する
static GMEVENT* GetFadeOutEvent_Ananukenohimo( EVENT_WORK* work ); // フェードアウトのイベントを取得する
// あなをほる
static void RegisterPlayerEffectTask_Anawohoru( EVENT_WORK* work ); // 自機の演出タスクを登録する
static void StartCameraAnime_Anawohoru( EVENT_WORK* work ); // カメラ演出を開始する
static void FinishCameraAnime_Anawohoru( EVENT_WORK* work ); // カメラ演出を終了する
static GMEVENT* GetFadeOutEvent_Anawohoru( EVENT_WORK* work ); // フェードアウトのイベントを取得する


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
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_FallInSand, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem   = gsys;
  work->fieldmap     = fieldmap;
  work->frame        = 0;
  work->bmSandStream = NULL;
  VEC_Set( &work->sandStreamPos, stream_pos->x, stream_pos->y, stream_pos->z );

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( あなぬけのヒモ )
 *
 * @param parent             親イベント
 * @param gsys               ゲームシステム
 * @param fieldmap           フィールドマップ
 * @param season_change_flag 季節が変化したかどうか
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_Ananukenohimo( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, BOOL season_change_flag )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_Ananukenohimo, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem         = gsys;
  work->fieldmap           = fieldmap;
  work->frame              = 0;
  work->wait_count         = 0;
  work->season_change_flag = season_change_flag;

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( あなをほる )
 *
 * @param parent             親イベント
 * @param gsys               ゲームシステム
 * @param fieldmap           フィールドマップ
 * @param season_change_flag 季節が変化したかどうか
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_Anawohoru( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, BOOL season_change_flag )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_Anawohoru, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem         = gsys;
  work->fieldmap           = fieldmap;
  work->frame              = 0;
  work->wait_count         = 0;
  work->season_change_flag = season_change_flag;

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( テレポート/ユニオンルームからの退場兼用 )
 *
 * @param parent     親イベント
 * @param gsys       ゲームシステム
 * @param fieldmap   フィールドマップ
 * @param teleport_f  テレポートでの呼び出しならTRUE 内部でSE分岐に使用
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_Teleport( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, BOOL teleport_f )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_Teleport, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem = gsys;
  work->fieldmap   = fieldmap;
  work->frame      = 0;
  work->teleport_f = teleport_f;

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベントを作成する( ワープ )
 *
 * @param parent     親イベント
 * @param gsys       ゲームシステム
 * @param fieldmap   フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_DISAPPEAR_Warp( GMEVENT* parent, 
                               GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_DISAPPEAR_Warp, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem = gsys;
  work->fieldmap   = fieldmap;
  work->frame      = 0;

  return event;
}


//========================================================================================== 
// ■非公開関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベント処理関数( 流砂 )
 */
//------------------------------------------------------------------------------------------
// 流砂のアニメ番号
#define SAND_ANM_IDX (0)  

// シーケンス番号
enum {
  SEQ_INIT,                       // 初期設定
  SEQ_SAND_ANIME_START,           // 流砂アニメ開始
  SEQ_PLAYER_DRAW_ANIME_START,    // 自機が引き込まれる移動開始
  SEQ_PLAYER_DRAW_ANIME_WAIT,     // 自機が引き込まれる移動終了待ち
  SEQ_PLAYER_SALLOW_ANIME_START,  // 自機が飲み込まれる演出開始
  SEQ_PLAYER_SALLOW_ANIME_WAIT,   // 自機が飲み込まれる演出終了
  SEQ_EXIT,                       // 終了処理
};

static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_FallInSand( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player );
  FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq ) {
  case SEQ_INIT:
    // カメラモードの設定
    work->camera_mode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    *seq = SEQ_SAND_ANIME_START;
    break;

  // 流砂アニメーション開始
  case SEQ_SAND_ANIME_START:
    // SE
    {
      SEPLAYER_ID player_id = PMSND_GetSE_DefaultPlayerID( SEQ_SE_FLD_15 );
      PMSND_PlaySE_byPlayerID( SEQ_SE_FLD_15, player_id );
    }
    // 配置モデルのアニメ再生
    {
      FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
      FIELD_BMODEL_MAN* taskMan = FLDMAPPER_GetBuildModelManager( mapper );
      G3DMAPOBJST** objst;
      u32 objnum;
      FLDHIT_RECT rect;
      // 配置モデル(流砂)を検索
      rect.left   = work->sandStreamPos.x - (FIELD_CONST_GRID_SIZE*2 << FX32_SHIFT);
      rect.right  = work->sandStreamPos.x + (FIELD_CONST_GRID_SIZE*2 << FX32_SHIFT);
      rect.top    = work->sandStreamPos.z - (FIELD_CONST_GRID_SIZE*2 << FX32_SHIFT);
      rect.bottom = work->sandStreamPos.z + (FIELD_CONST_GRID_SIZE*2 << FX32_SHIFT);
      objst = FIELD_BMODEL_MAN_CreateObjStatusList( taskMan, &rect, BM_SEARCH_ID_SANDSTREAM, &objnum ); 
      // 配置モデルを複製し, アニメーション再生
      if( objst[0] ) {
        work->bmSandStream = FIELD_BMODEL_Create( taskMan, objst[0] );        // 複製
        FIELD_BMODEL_SetAnime( work->bmSandStream, SAND_ANM_IDX, BMANM_REQ_START );  // 再生
      }
      else {
        work->bmSandStream = NULL;
      }
      GFL_HEAP_FreeMemory( objst );
    }
    *seq = SEQ_PLAYER_DRAW_ANIME_START;
    break;

  // 自機が引き込まれる移動開始
  case SEQ_PLAYER_DRAW_ANIME_START:
    // 自機移動タスク登録
    { 
      int animeFrame;
      // アニメーションフレーム数を決定
      { 
        VecFx32 playerPos;
        int playerGX, playerGZ;
        int sandStreamGX, sandStreamGZ;
        int distX, distZ;
        FIELD_PLAYER_GetPos( player, &playerPos );
        playerGX = FX32_TO_GRID( playerPos.x );
        playerGZ = FX32_TO_GRID( playerPos.z );
        sandStreamGX = FX32_TO_GRID( work->sandStreamPos.x );
        sandStreamGZ = FX32_TO_GRID( work->sandStreamPos.z );
        distX = playerGX - sandStreamGX;
        distZ = playerGZ - sandStreamGZ;
        if( distX < 0 ){ distX = -distX; }
        if( distZ < 0 ){ distZ = -distZ; }
        if( (distX == 0) && (distZ == 0) ){ animeFrame = 0; }
        else if( (distX < 2) && (distZ < 2) ){ animeFrame = 60; }
        else{ animeFrame = 120; }
      }
      // タスク登録
      if( 0 < animeFrame ) {
        FIELD_TASK* move;
        FIELD_TASK_MAN* taskMan;
        move = FIELD_TASK_TransPlayer( work->fieldmap, animeFrame, &work->sandStreamPos );
        taskMan = FIELDMAP_GetTaskManager( work->fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
        *seq = SEQ_PLAYER_DRAW_ANIME_WAIT;
      }
      else {
        *seq = SEQ_PLAYER_SALLOW_ANIME_START;
      }
    }
    work->frame = 0;
    break;

  // タスク終了待ち&砂埃
  case SEQ_PLAYER_DRAW_ANIME_WAIT:
    // 流砂アニメが終了したら再生する
    if( work->bmSandStream ) {
      if( FIELD_BMODEL_GetAnimeStatus( work->bmSandStream, SAND_ANM_IDX ) ) {
        FIELD_BMODEL_SetAnime( work->bmSandStream, SAND_ANM_IDX, BMANM_REQ_START );
      }
    }
    // 自機の向きを変える
    { 
      int key = GFL_UI_KEY_GetCont();
      if( key & PAD_KEY_UP )    { MMDL_SetAcmd( mmdl, AC_STAY_WALK_U_4F ); }
      if( key & PAD_KEY_DOWN )  { MMDL_SetAcmd( mmdl, AC_STAY_WALK_D_4F ); }
      if( key & PAD_KEY_LEFT )  { MMDL_SetAcmd( mmdl, AC_STAY_WALK_L_4F ); }
      if( key & PAD_KEY_RIGHT ) { MMDL_SetAcmd( mmdl, AC_STAY_WALK_R_4F ); }
    }
    // 砂埃
    if( work->frame++ % 10 == 0 ) {
      FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
    }
    // タスク終了チェック
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( work->fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) {
        *seq = SEQ_PLAYER_SALLOW_ANIME_START;
      }
    }
    break;

  // 自機が飲み込まれるアニメーション開始
  case SEQ_PLAYER_SALLOW_ANIME_START:
    // SE再生
    PMSND_PlaySE( SEQ_SE_FLD_147 );
    // 影を消す
    MMDL_OnMoveBit( mmdl, MMDL_MOVEBIT_SHADOW_VANISH );
    // タスク登録
    { 
      VecFx32 moveVec;
      FIELD_TASK* moveTask;
      FIELD_TASK* rotTask;
      FIELD_TASK* zoomTask;
      FIELD_TASK* fadeTask;
      FIELD_TASK_MAN* taskMan;
      VEC_Set( &moveVec, 0, -30<<FX32_SHIFT, 0 );
      moveTask = FIELD_TASK_TransDrawOffset( work->fieldmap, 34, &moveVec );
      rotTask  = FIELD_TASK_PlayerRotate( work->fieldmap, 34, 5 );
      zoomTask = FIELD_TASK_CameraLinearZoom( work->fieldmap, 30, ZOOM_IN_DIST );
      fadeTask = FIELD_TASK_Fade( work->fieldmap, GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
      taskMan = FIELDMAP_GetTaskManager( work->fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, moveTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, rotTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, zoomTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, fadeTask, zoomTask );
    }
    work->frame = 0;
    *seq = SEQ_PLAYER_SALLOW_ANIME_WAIT;
    break;

  // タスクの終了待ち
  case SEQ_PLAYER_SALLOW_ANIME_WAIT:
    { 
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( work->fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) { *seq = SEQ_EXIT; }
    }
    break;

  // 終了
  case SEQ_EXIT:
    // SE 停止
    {
      SEPLAYER_ID player_id = PMSND_GetSE_DefaultPlayerID( SEQ_SE_FLD_15 );
      PMSND_StopSE_byPlayerID( player_id );
    }
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->camera_mode );
    // 登録した配置モデルを消去
    if( work->bmSandStream ) {
      FIELD_BMODEL_Delete( work->bmSandStream );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//==========================================================================================
/**
 * @brief 退場イベント処理関数 ( あなぬけのヒモ )
 */
//==========================================================================================
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Ananukenohimo( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work    = (EVENT_WORK*)wk;
  FIELD_TASK_MAN* taskMan = FIELDMAP_GetTaskManager( work->fieldmap );

  switch( *seq ) {
  case 0:
    // 演出開始
    RegisterPlayerEffectTask_Ananukenohimo( work ); // 自機の演出タスクを登録
    StartCameraAnime_Ananukenohimo( work ); // カメラ演出開始
    (*seq)++;
    break;

  case 1:
    // フェードアウト開始待ち
    if( 35 < work->wait_count++ ) {
      GMEVENT_CallEvent( event, GetFadeOutEvent_Ananukenohimo( work ) );
      (*seq)++;
      break;
    }

  case 2:
    // 全演出タスクの終了待ち
    if( FIELD_TASK_MAN_IsAllTaskEnd( taskMan ) ) {
      PMSND_PlaySE( SEQ_SE_KAIDAN ); //「ザッザッザッ」
      (*seq)++;
    }
    break;
    
  case 3:
    FinishCameraAnime_Ananukenohimo( work ); // カメラ演出終了
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
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
  FIELDMAP_WORK*  fieldmap = work->fieldmap;
  FIELD_TASK_MAN* taskMan  = FIELDMAP_GetTaskManager( fieldmap );

  FIELD_TASK* rot_up;
  FIELD_TASK* rot;

  rot_up = FIELD_TASK_PlayerRotate_SpeedUp( fieldmap, 20, 3 );
  rot    = FIELD_TASK_PlayerRotate( fieldmap, 20, 5 );
  FIELD_TASK_MAN_AddTask( taskMan, rot_up, NULL );
  FIELD_TASK_MAN_AddTask( taskMan, rot, rot_up );
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラ演出を開始する ( あなぬけのヒモ )
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
  FCAM_PARAM_GetCurrentParam( camera, &work->FCamAnimeData.startParam );
  work->FCamAnimeData.endParam = work->FCamAnimeData.startParam;
  work->FCamAnimeData.endParam.length -= (100 << FX32_SHIFT);
  FCAM_ANIME_SetCameraStartParam( work->FCamAnimeWork );
  FCAM_ANIME_StartAnime( work->FCamAnimeWork );
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラ演出を終了する ( あなぬけのヒモ )
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
 * @brief フェードアウトのイベントを呼び出す ( あなぬけのヒモ )
 *
 * @param work
 *
 * @return フェードアウトイベント
 */
//------------------------------------------------------------------------------------------
static GMEVENT* GetFadeOutEvent_Ananukenohimo( EVENT_WORK* work )
{
  GAMESYS_WORK*  gameSystem = work->gameSystem;
  FIELDMAP_WORK* fieldmap   = work->fieldmap;
  GMEVENT* event;

  // 季節が変化
  if( work->season_change_flag ) { 
    event = EVENT_FieldFadeOut_Season( gameSystem, fieldmap, FIELD_FADE_WAIT ); // 季節フェード
  }
  else { 
    event = EVENT_FieldFadeOut_White( gameSystem, fieldmap, FIELD_FADE_WAIT ); // ホワイトアウト
  }

  return event;
}

//==========================================================================================
/**
 * @brief 退場イベント処理関数( あなをほる )
 */
//==========================================================================================
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Anawohoru( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work    = (EVENT_WORK*)wk;
  FIELD_TASK_MAN* taskMan = FIELDMAP_GetTaskManager( work->fieldmap );

  switch( *seq ) {
  case 0:
    RegisterPlayerEffectTask_Anawohoru( work ); // 自機の演出タスクを登録
    StartCameraAnime_Anawohoru( work ); // カメラ演出開始
    PMSND_PlaySE( SEQ_SE_FLD_80 ); // SE 再生
    (*seq)++;
    break;

  case 1:
    // フェードアウト開始待ち
    if( 35 < work->wait_count++ ) {
      GMEVENT_CallEvent( event, GetFadeOutEvent_Anawohoru( work ) );
      (*seq)++;
      break;
    }

  case 2:
    // 全タスク終了
    if( FIELD_TASK_MAN_IsAllTaskEnd( taskMan ) ) {
      PMSND_PlaySE( SEQ_SE_KAIDAN ); //「ザッザッザッ」
      (*seq)++;
    }
    break;

  case 3:
    FinishCameraAnime_Anawohoru( work ); // カメラ演出終了
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
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
  FIELDMAP_WORK*  fieldmap = work->fieldmap;
  FIELD_TASK_MAN* taskMan  = FIELDMAP_GetTaskManager( fieldmap );
  FIELD_PLAYER*   player   = FIELDMAP_GetFieldPlayer( fieldmap );
  MMDL*           mmdl     = FIELD_PLAYER_GetMMdl( player );

  FIELD_TASK* effect;
  FIELD_TASK* rot_up;
  FIELD_TASK* rot;

  effect = FIELD_TASK_AnahoriEffect( fieldmap, mmdl, 40, ANAHORI_EFF_INTVL );
  rot_up = FIELD_TASK_PlayerRotate_SpeedUp( fieldmap, 20, 4 );
  rot    = FIELD_TASK_PlayerRotate( fieldmap, 20, 5 );

  FIELD_TASK_MAN_AddTask( taskMan, effect, NULL );
  FIELD_TASK_MAN_AddTask( taskMan, rot_up, NULL );
  FIELD_TASK_MAN_AddTask( taskMan, rot, rot_up );
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラ演出を開始する ( あなをほる )
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
  FCAM_PARAM_GetCurrentParam( camera, &work->FCamAnimeData.startParam );
  work->FCamAnimeData.endParam = work->FCamAnimeData.startParam;
  work->FCamAnimeData.endParam.length -= (100 << FX32_SHIFT);
  FCAM_ANIME_SetCameraStartParam( work->FCamAnimeWork );
  FCAM_ANIME_StartAnime( work->FCamAnimeWork );
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラ演出を終了する ( あなをほる )
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
 * @brief フェードアウトのイベントを呼び出す ( あなをほる )
 *
 * @param work
 *
 * @return フェードアウトイベント
 */
//------------------------------------------------------------------------------------------
static GMEVENT* GetFadeOutEvent_Anawohoru( EVENT_WORK* work )
{
  GAMESYS_WORK*  gameSystem = work->gameSystem;
  FIELDMAP_WORK* fieldmap   = work->fieldmap;
  GMEVENT* event;

  // 季節が変化
  if( work->season_change_flag ) { 
    event = EVENT_FieldFadeOut_Season( gameSystem, fieldmap, FIELD_FADE_WAIT ); // 季節フェード
  }
  else { 
    event = EVENT_FieldFadeOut_White( gameSystem, fieldmap, FIELD_FADE_WAIT ); // ホワイトアウト
  }

  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベント処理関数( テレポート )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Teleport( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work     = (EVENT_WORK*)wk;
  FIELDMAP_WORK*  fieldmap = work->fieldmap;
  FIELD_PLAYER*   player   = FIELDMAP_GetFieldPlayer( work->fieldmap );
  FIELD_CAMERA*   camera   = FIELDMAP_GetFieldCamera( work->fieldmap );
  FIELD_TASK_MAN* taskMan  = FIELDMAP_GetTaskManager( fieldmap );

  switch( *seq )
  {
  case 0:
    // タスク登録
    {
      FIELD_TASK* rot_up;
      FIELD_TASK* rot;
      FIELD_TASK* wait;
      FIELD_TASK* fade_out;
      rot_up   = FIELD_TASK_PlayerRotate_SpeedUp( fieldmap, 20, 3 );
      rot      = FIELD_TASK_PlayerRotate( fieldmap, 20, 5 );
      wait     = FIELD_TASK_Wait( fieldmap, ZOOM_IN_FRAME );
      fade_out = FIELD_TASK_Fade( fieldmap, GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 1 );
      FIELD_TASK_MAN_AddTask( taskMan, rot_up, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, rot, rot_up );
      FIELD_TASK_MAN_AddTask( taskMan, wait, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, fade_out, wait );
    }
    // SE
    if( work->teleport_f ){
      PMSND_PlaySE( SEQ_SE_FLD_86 );  //テレポートは専用SE
    }else{
      PMSND_PlaySE( SEQ_SE_FLD_05 );  //その他は通常ワープ音
    }
    // カメラ演出開始
    work->FCamAnimeWork = FCAM_ANIME_CreateWork( fieldmap );
    work->FCamAnimeData.frame = 60;
    work->FCamAnimeData.targetBindOffFlag = TRUE;
    work->FCamAnimeData.cameraAreaOffFlag = TRUE;
    FCAM_ANIME_SetAnimeData( work->FCamAnimeWork, &work->FCamAnimeData );
    FCAM_ANIME_SetupCamera( work->FCamAnimeWork );
    FCAM_PARAM_GetCurrentParam( camera, &work->FCamAnimeData.startParam );
    work->FCamAnimeData.endParam = work->FCamAnimeData.startParam;
    work->FCamAnimeData.endParam.length -= (100 << FX32_SHIFT);
    FCAM_ANIME_SetCameraStartParam( work->FCamAnimeWork );
    FCAM_ANIME_StartAnime( work->FCamAnimeWork );
    (*seq)++;
    break;

  case 1:
    // タスクの終了待ち
    if( FIELD_TASK_MAN_IsAllTaskEnd( taskMan ) ) { (*seq)++; }
    break;

  case 2:
    FCAM_ANIME_RecoverCamera( work->FCamAnimeWork );
    FCAM_ANIME_DeleteWork( work->FCamAnimeWork );
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 退場イベント処理関数( ワープ )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_DISAPPEAR_Warp( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*    work     = (EVENT_WORK*)wk;
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  FIELD_PLAYER*  player   = FIELDMAP_GetFieldPlayer( fieldmap );
  FIELD_CAMERA*  camera   = FIELDMAP_GetFieldCamera( fieldmap );

  switch( *seq ) {
  case 0:
    // カメラモードの設定
    work->camera_mode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    // タスク登録
    { 
      FIELD_TASK* rotTask;
      FIELD_TASK* fadeOutTask;
      FIELD_TASK* moveTask;
      FIELD_TASK_MAN* taskMan;
      VecFx32 moveVec;
      VEC_Set( &moveVec, 0, 150<<FX32_SHIFT, 0 );
      rotTask = FIELD_TASK_PlayerRotate( fieldmap, 24, 3 );
      fadeOutTask = FIELD_TASK_Fade( fieldmap, GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 1 );
      moveTask = FIELD_TASK_TransDrawOffset( fieldmap, 24, &moveVec );
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, rotTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, moveTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, fadeOutTask, moveTask );
    }
    // SE
    PMSND_PlaySE( SEQ_SE_FLD_05 );
    (*seq)++;
    break;
  case 1:
    // タスクの終了待ち
    { 
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) && !GFL_FADE_CheckFade() ) { 
        (*seq)++; 
      }
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->camera_mode );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
