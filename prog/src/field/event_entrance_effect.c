//============================================================================================
/**
 * @file event_entrance_effect.c
 * @brief
 * @date  2009.06.13
 * @author  tamada GAME FREAK inc.
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h" 
#include "sound/pm_sndsys.h"      // for PMSND_PlaySE
#include "field/field_sound.h"    // for FIELD_SOUND_ChangePlayZoneBGM
#include "field/field_const.h"
#include "field/eventdata_type.h"

#include "fieldmap.h"
#include "field_player.h"
#include "field_buildmodel.h"

#include "entrance_camera.h"
#include "event_entrance_effect.h" 
#include "event_mapchange.h"        // for MAPCHANGE_setPlayerVanish
#include "event_fieldmap_control.h" // for EVENT_FieldFadeOut
#include "event_fldmmdl_control.h"  // for EVENT_PlayerOneStepAnime



//============================================================================================
// ■定数
//============================================================================================
#define MAX_SEQ_LENGTH       (10) // 一連のシーケンスの最大数
#define ZERO_CAMERA_WAIT_IN  (15) // カメラのゼロフレーム切り替え演出の待ち時間 ( 入る時 )
#define ZERO_CAMERA_WAIT_OUT (15) // カメラのゼロフレーム切り替え演出の待ち時間 ( 出る時 )

// ドアから出てくるイベントのシーケンス番号
enum {
  SEQ_DOOROUT_INIT = 0,
  SEQ_DOOROUT_EFFECT_START,
  SEQ_DOOROUT_OPEN_ANIME_START,
  SEQ_DOOROUT_OPEN_ANIME_WAIT,
  SEQ_DOOROUT_PLAYER_STEP,
  SEQ_DOOROUT_CAMERA_ACT_START,
  SEQ_DOOROUT_CLOSE_ANIME_START,
  SEQ_DOOROUT_CLOSE_ANIME_WAIT,
  SEQ_DOOROUT_END,
  // 以下, 実行時の判定で遷移するシーケンス
  SEQ_DOOROUT_CAMERA_TRACE_WAIT,
  SEQ_DOOROUT_SOUND_LOAD_WAIT,
  SEQ_DOOROUT_WAIT_FOR_CAMERA_ACT_START,
};

// ドアに入るイベントのシーケンス番号
enum {
  SEQ_DOORIN_INIT = 0,
  SEQ_DOORIN_CAMERA_ACT_START,
  SEQ_DOORIN_CAMERA_ACT_WAIT,
  SEQ_DOORIN_OPEN_ANIME_START,
  SEQ_DOORIN_OPEN_ANIME_WAIT,
  SEQ_DOORIN_PLAYER_STEP,
  SEQ_DOORIN_BGM_STAND_BY,
  SEQ_DOORIN_FADEOUT,
  SEQ_DOORIN_END,
  // 以下, 実行時の判定で遷移するシーケンス
  SEQ_DOORIN_WAIT_FOR_PLAYER_ONESTEP,
};


//============================================================================================
// ■イベントワーク
//============================================================================================
typedef struct {

  // 以下はワーク生成時にセットされる
  GAMESYS_WORK *  gameSystem;      //<<<ゲームシステムへのポインタ
  GAMEDATA *      gameData;        //<<<ゲームデータへのポインタ
  FIELDMAP_WORK * fieldmap;        //<<<フィールドマップへのポインタ
  FIELD_SOUND *   fieldSound;      //<<<フィールドサウンドへのポインタ
  LOCATION        loc_req;         //<<<次のマップを指すロケーション指定 ( BGMフェード処理の係数 )
  VecFx32         doorPos;         //<<<ドアを検索する場所
  BOOL            cameraAnimeFlag; //<<<カメラアニメーションの有無
  BOOL            seasonDispFlag;  //<<<四季表示を行うかどうか
  u8              startSeason;     //<<<最初に表示する季節 ( 四季表示を行う場合のみ有効 )
  u8              endSeason;       //<<<最後に表示する季節 ( 四季表示を行う場合のみ有効 )
  FIELD_FADE_TYPE fadeType;        //<<<季節表示を行わない場合のフェードタイプ
  ECAM_WORK*      ECamWork;        //<<<カメラ演出ワーク
  ECAM_PARAM      ECamParam;       //<<<カメラ演出パラメータ
  EXIT_TYPE       exitType;        //<<<出入り口タイプ
  u16             count;           //<<<カウンタ
  u8              playerDir;       //<<<自機の向き
  FIELD_BMODEL *  doorBM;          //<<<ドアの配置モデル
  u8              seq[ MAX_SEQ_LENGTH ]; //<<<シーケンス
  u8              seqPos;                //<<<現在のシーケンス位置

} FIELD_DOOR_ANIME_WORK;


//============================================================================================
// ■index
//============================================================================================
static void SetupDoorOutEventSeq( FIELD_DOOR_ANIME_WORK* work ); // ドアから出てくるイベントのシーケンスの流れを決定する
static GMEVENT_RESULT ExitEvent_DoorOut( GMEVENT * event, int *seq, void * wk ); // ドアから出てくるイベント処理関数

static void SetupDoorInEventSeq( FIELD_DOOR_ANIME_WORK* work ); // ドアに入るイベントのシーケンスの流れを決定する
static GMEVENT_RESULT ExitEvent_DoorIn(GMEVENT * event, int *seq, void * wk); // ドアに入るイベント処理関数

static u8 GetPlayerDir( FIELDMAP_WORK* fieldmap ); // 自機の向きを取得する
static void GetPlayerFrontPos( FIELDMAP_WORK * fieldmap, VecFx32 * pos ); // 一歩前進した位置の座標を取得する
static u8 GetCurrentSeq( const FIELD_DOOR_ANIME_WORK* work ); // 現在のシーケンスを取得する
static u8 GetNextSeq( const FIELD_DOOR_ANIME_WORK* work ); // 次のシーケンスを取得する
static void FinishCurrentSeq( FIELD_DOOR_ANIME_WORK* work ); // 現在のシーケンスを終了する
static void SearchDoorBM( FIELD_DOOR_ANIME_WORK* work ); // ドアを探し、配置モデルを生成する
static void DeleteDoorBM( FIELD_DOOR_ANIME_WORK* work ); // 生成したドアの配置モデルを破棄する
static BOOL CheckDoorAnimeFinish( const FIELD_DOOR_ANIME_WORK* work ); // ドアアニメの完了をチェックする
static BOOL CheckDoorExist( const FIELD_DOOR_ANIME_WORK* work ); // ドアが存在するかどうかをチェックする
static void StartDoorOpen( const FIELD_DOOR_ANIME_WORK* work ); // ドアオープンを開始する
static void StartDoorClose( const FIELD_DOOR_ANIME_WORK* work ); // ドアクローズを開始する
static void StartFadeInEvent( GMEVENT* event, const FIELD_DOOR_ANIME_WORK* work ); // 画面のフェードインイベントを呼び出す
static void StartFadeOutEvent( GMEVENT* event, const FIELD_DOOR_ANIME_WORK* work ); // 画面のフェードアウトイベントを呼び出す


//--------------------------------------------------------------------------------------------
/**
 * @brief ドアからでてくる一連の演出イベントを生成する
 *
 * @param gameSystem
 * @param fieldmap
 * @param cameraAnimeFlag カメラアニメーションの有無
 * @param seasonDispFlag  四季表示を行うかどうか
 * @param startSeason     最初に表示する季節 ( 四季表示を行う場合のみ有効 )
 * @param endSeason       最後に表示する季節 ( 四季表示を行う場合のみ有効 )
 * @param fadeType        季節表示を行わない場合のフェードタイプ
 * @param exitType        出入り口タイプ
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT * EVENT_FieldDoorOutAnime ( GAMESYS_WORK * gameSystem, FIELDMAP_WORK * fieldmap, 
                                    BOOL cameraAnimeFlag,
                                    BOOL seasonDispFlag, u8 startSeason, u8 endSeason,
                                    FIELD_FADE_TYPE fadeType,
                                    EXIT_TYPE exitType )
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * work;

  // イベントを生成
  event = GMEVENT_Create( 
      gameSystem, NULL, ExitEvent_DoorOut, sizeof(FIELD_DOOR_ANIME_WORK) ); 

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem );
  work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  work->fieldSound = GAMEDATA_GetFieldSound( work->gameData );
  GetPlayerFrontPos(fieldmap, &work->doorPos);
  work->cameraAnimeFlag = cameraAnimeFlag;
  work->seasonDispFlag  = seasonDispFlag;
  work->startSeason     = startSeason;
  work->endSeason       = endSeason;
  work->fadeType        = fadeType;
  work->doorBM          = NULL;
  work->exitType        = exitType;
  work->count           = 0;
  work->playerDir       = GetPlayerDir( fieldmap );
  work->seqPos          = 0;

  // カメラ演出のセットアップパラメータを作成
  work->ECamWork = ENTRANCE_CAMERA_CreateWork( fieldmap );
  work->ECamParam.exitType  = exitType;
  work->ECamParam.situation = ECAM_SITUATION_OUT;
  work->ECamParam.oneStep   = ECAM_ONESTEP_ON;

  // ドアを検索
  SearchDoorBM( work );

  // シーケンスの流れを決定
  SetupDoorOutEventSeq( work ); 
  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアから出てくるイベントのシーケンスの流れを決定する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void SetupDoorOutEventSeq( FIELD_DOOR_ANIME_WORK* work )
{
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  int idx = 0;

  // ドアがある場合
  if( CheckDoorExist(work) ) {
    switch( GetPlayerDir(fieldmap) ) {
    default: GF_ASSERT(0);
    case DIR_UP: // 上向き
    case DIR_DOWN: // 下向き
      work->seq[idx++] = SEQ_DOOROUT_INIT;
      work->seq[idx++] = SEQ_DOOROUT_EFFECT_START;
      work->seq[idx++] = SEQ_DOOROUT_CAMERA_ACT_START;
      work->seq[idx++] = SEQ_DOOROUT_OPEN_ANIME_START;
      work->seq[idx++] = SEQ_DOOROUT_OPEN_ANIME_WAIT;
      work->seq[idx++] = SEQ_DOOROUT_PLAYER_STEP;
      work->seq[idx++] = SEQ_DOOROUT_CLOSE_ANIME_START;
      work->seq[idx++] = SEQ_DOOROUT_CLOSE_ANIME_WAIT;
      work->seq[idx++] = SEQ_DOOROUT_END; 
      break;
    case DIR_LEFT: // 左向き
    case DIR_RIGHT: // 右向き
      work->seq[idx++] = SEQ_DOOROUT_INIT;
      work->seq[idx++] = SEQ_DOOROUT_EFFECT_START;
      work->seq[idx++] = SEQ_DOOROUT_OPEN_ANIME_START;
      work->seq[idx++] = SEQ_DOOROUT_OPEN_ANIME_WAIT;
      work->seq[idx++] = SEQ_DOOROUT_PLAYER_STEP;
      work->seq[idx++] = SEQ_DOOROUT_CAMERA_ACT_START;
      work->seq[idx++] = SEQ_DOOROUT_CLOSE_ANIME_START;
      work->seq[idx++] = SEQ_DOOROUT_CLOSE_ANIME_WAIT;
      work->seq[idx++] = SEQ_DOOROUT_END; 
      break;
    }
  }
  // ドアがない場合
  else {
    // 特殊出入口
    if( EXIT_TYPE_IsSpExit(work->exitType) ) {
      work->seq[idx++] = SEQ_DOOROUT_INIT;
      work->seq[idx++] = SEQ_DOOROUT_EFFECT_START;
      work->seq[idx++] = SEQ_DOOROUT_PLAYER_STEP;
      work->seq[idx++] = SEQ_DOOROUT_CAMERA_ACT_START;
      work->seq[idx++] = SEQ_DOOROUT_END; 
    }
    // 通常出入口
    else {
      switch( GetPlayerDir(fieldmap) ) {
      default: GF_ASSERT(0);
      case DIR_UP: // 上向き
      case DIR_DOWN: // 下向き
        work->seq[idx++] = SEQ_DOOROUT_INIT;
        work->seq[idx++] = SEQ_DOOROUT_EFFECT_START;
        work->seq[idx++] = SEQ_DOOROUT_CAMERA_ACT_START;
        work->seq[idx++] = SEQ_DOOROUT_PLAYER_STEP;
        work->seq[idx++] = SEQ_DOOROUT_END; 
        break;
      case DIR_LEFT: // 左向き
      case DIR_RIGHT: // 右向き
        work->seq[idx++] = SEQ_DOOROUT_INIT;
        work->seq[idx++] = SEQ_DOOROUT_EFFECT_START;
        work->seq[idx++] = SEQ_DOOROUT_PLAYER_STEP;
        work->seq[idx++] = SEQ_DOOROUT_CAMERA_ACT_START;
        work->seq[idx++] = SEQ_DOOROUT_END; 
        break;
      }
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアから出てくるイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorOut( GMEVENT * event, int *seq, void * wk )
{ 
	FIELD_DOOR_ANIME_WORK * work       = wk;
	GAMESYS_WORK *          gameSystem = work->gameSystem;
  GAMEDATA *              gameData   = work->gameData;
  FIELDMAP_WORK *         fieldmap   = work->fieldmap;
  FIELD_SOUND *           fieldSound = work->fieldSound;
  FIELD_CAMERA *          camera     = FIELDMAP_GetFieldCamera( fieldmap ); 
  FIELD_TASK_MAN *   task_man   = FIELDMAP_GetTaskManager( fieldmap );

  switch( *seq ) {
  // イベント初期設定
  case SEQ_DOOROUT_INIT:
    // 自機を消す
    MAPCHANGE_setPlayerVanish( fieldmap, TRUE );

    // トレースシステムが無効
    if( FIELD_CAMERA_CheckTraceSys( camera ) == FALSE ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    // トレースシステムが有効
    else {
      FIELD_CAMERA_StopTraceRequest( camera ); // カメラのトレース機能をOFF
      *seq = SEQ_DOOROUT_CAMERA_TRACE_WAIT;
    }
    break;

  // カメラのトレース処理完了待ち
  case SEQ_DOOROUT_CAMERA_TRACE_WAIT:
    // トレース処理が完了
    if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

  // 演出開始
  case SEQ_DOOROUT_EFFECT_START:
    ENTRANCE_CAMERA_Setup( work->ECamWork, &work->ECamParam ); // カメラ演出をセットアップ
    FSND_PlayStartBGM( fieldSound ); // BGM 再生開始
    StartFadeInEvent( event, work ); // 画面フェードイン開始
    *seq = SEQ_DOOROUT_SOUND_LOAD_WAIT;
    break;

  // サウンドの読み込み待ち
  case SEQ_DOOROUT_SOUND_LOAD_WAIT:
    if( PMSND_IsLoading() ) {
      OS_Printf( "WARNING!!: BGMの読み込みが間に合っていません!\n" ); // 警告!!
    }
    else {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

  // ドアオープン開始
  case SEQ_DOOROUT_OPEN_ANIME_START:
    StartDoorOpen( work );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

  // ドアオープン待ち
  case SEQ_DOOROUT_OPEN_ANIME_WAIT:
    if( CheckDoorAnimeFinish(work) == TRUE ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

  // 自機出現、一歩移動アニメ
  case SEQ_DOOROUT_PLAYER_STEP:
    MAPCHANGE_setPlayerVanish( fieldmap, FALSE );
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime( gameSystem, fieldmap ) );

    // カメラ演出がゼロフレーム切り替え
    if( ENTRANCE_CAMERA_IsAnimeExist( work->ECamWork ) &&
        ENTRANCE_CAMERA_IsZeroFrameAnime( work->ECamWork ) ) {
      *seq = SEQ_DOOROUT_WAIT_FOR_CAMERA_ACT_START;
    }
    else {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

  // ドアクローズ開始
  case SEQ_DOOROUT_CLOSE_ANIME_START:
    StartDoorClose( work );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

  // カメラのゼロフレーム切り替え待ち
  case SEQ_DOOROUT_WAIT_FOR_CAMERA_ACT_START:
    if( ZERO_CAMERA_WAIT_OUT < work->count++ ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

  // カメラ演出開始
  case SEQ_DOOROUT_CAMERA_ACT_START:
    ENTRANCE_CAMERA_Start( work->ECamWork );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

  // ドアクローズ待ち
  case SEQ_DOOROUT_CLOSE_ANIME_WAIT:
    if( CheckDoorAnimeFinish(work) == TRUE ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break; 

  // イベント終了
  case SEQ_DOOROUT_END:
    ENTRANCE_CAMERA_Recover( work->ECamWork );
    ENTRANCE_CAMERA_DeleteWork( work->ECamWork );
    DeleteDoorBM( work );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief ドアに入る一連の演出イベントを生成する
 *
 * @param gameSystem
 * @param fieldmap
 * @param loc             次のマップを指定
 * @param cameraAnimeFlag カメラアニメーションの有無
 * @param seasonDispFlag  四季表示を行うかどうか
 * @param fadeType        四季表示を行わない場合のフェードタイプ
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* EVENT_FieldDoorInAnime ( 
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, const LOCATION* loc, 
    BOOL cameraAnimeFlag, BOOL seasonDispFlag, FIELD_FADE_TYPE fadeType,
    EXIT_TYPE exitType )
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * work;

  // イベントを生成 
  event = GMEVENT_Create( 
      gameSystem, NULL, ExitEvent_DoorIn, sizeof(FIELD_DOOR_ANIME_WORK) ); 

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem );
  work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  work->fieldSound = GAMEDATA_GetFieldSound( work->gameData );
  work->loc_req    = *loc;
  GetPlayerFrontPos(fieldmap, &work->doorPos);
  work->cameraAnimeFlag = cameraAnimeFlag;
  work->seasonDispFlag  = seasonDispFlag;
  work->fadeType        = fadeType;
  work->doorBM          = NULL;
  work->exitType        = exitType;
  work->count           = 0;
  work->playerDir       = GetPlayerDir( fieldmap );
  work->seqPos          = 0;

  // カメラ演出のセットアップパラメータを作成
  work->ECamWork = ENTRANCE_CAMERA_CreateWork( fieldmap );
  work->ECamParam.exitType  = exitType;
  work->ECamParam.situation = ECAM_SITUATION_IN;
  work->ECamParam.oneStep   = ECAM_ONESTEP_ON;

  // ドアを検索
  SearchDoorBM( work );

  // シーケンスの流れを決定
  SetupDoorInEventSeq( work ); 

  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアに入るイベントのシーケンスの流れを決定する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void SetupDoorInEventSeq( FIELD_DOOR_ANIME_WORK* work )
{
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  int idx = 0;

  // ドアがある場合
  if( CheckDoorExist(work) ) {
    work->seq[idx++] = SEQ_DOORIN_INIT;
    work->seq[idx++] = SEQ_DOORIN_OPEN_ANIME_START;
    work->seq[idx++] = SEQ_DOORIN_CAMERA_ACT_START;
    work->seq[idx++] = SEQ_DOORIN_CAMERA_ACT_WAIT;
    work->seq[idx++] = SEQ_DOORIN_PLAYER_STEP;
    work->seq[idx++] = SEQ_DOORIN_BGM_STAND_BY;
    work->seq[idx++] = SEQ_DOORIN_FADEOUT;
    work->seq[idx++] = SEQ_DOORIN_END; 
  }
  // ドアがない場合
  else {
    work->seq[idx++] = SEQ_DOORIN_INIT;
    work->seq[idx++] = SEQ_DOORIN_CAMERA_ACT_START;
    work->seq[idx++] = SEQ_DOORIN_CAMERA_ACT_WAIT;
    work->seq[idx++] = SEQ_DOORIN_PLAYER_STEP;
    work->seq[idx++] = SEQ_DOORIN_BGM_STAND_BY;
    work->seq[idx++] = SEQ_DOORIN_FADEOUT;
    work->seq[idx++] = SEQ_DOORIN_END; 
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアに入るイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorIn( GMEVENT * event, int *seq, void * wk )
{ 
  FIELD_DOOR_ANIME_WORK * work       = wk;
  GAMESYS_WORK *          gameSystem = work->gameSystem;
  GAMEDATA *              gameData   = work->gameData;
  FIELDMAP_WORK *         fieldmap   = work->fieldmap;
  FIELD_SOUND *           fieldSound = work->fieldSound; 
  FIELD_TASK_MAN *        task_man   = FIELDMAP_GetTaskManager( fieldmap );

  switch( *seq ) { 
  case SEQ_DOORIN_INIT:
    ENTRANCE_CAMERA_Setup( work->ECamWork, &work->ECamParam ); // カメラ演出をセットアップ
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

    // カメラ演出開始
  case SEQ_DOORIN_CAMERA_ACT_START: 
    ENTRANCE_CAMERA_Start( work->ECamWork );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

    // ドアオープン開始
  case SEQ_DOORIN_OPEN_ANIME_START:
    StartDoorOpen( work );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

    // カメラ演出待ち
  case SEQ_DOORIN_CAMERA_ACT_WAIT:
    if( FIELD_TASK_MAN_IsAllTaskEnd( task_man ) ) { 
      // カメラ演出がゼロフレーム切り替え
      if( ENTRANCE_CAMERA_IsAnimeExist( work->ECamWork ) &&
          ENTRANCE_CAMERA_IsZeroFrameAnime( work->ECamWork ) ) {
        *seq = SEQ_DOORIN_WAIT_FOR_PLAYER_ONESTEP;
      }
      else {
        *seq = GetNextSeq( work );
        FinishCurrentSeq( work );
      }
    }
    break;

    // ドアオープン待ち
  case SEQ_DOORIN_OPEN_ANIME_WAIT:
    if( CheckDoorAnimeFinish(work) == TRUE ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

    // カメラのゼロフレーム切り替え待ち
  case SEQ_DOORIN_WAIT_FOR_PLAYER_ONESTEP:
    if( ZERO_CAMERA_WAIT_IN < work->count++ ) {
      *seq = GetNextSeq( work );
      FinishCurrentSeq( work );
    }
    break;

    // 自機の一歩移動アニメ開始
  case SEQ_DOORIN_PLAYER_STEP:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime( gameSystem, fieldmap ) );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

    // BGM再生準備
  case SEQ_DOORIN_BGM_STAND_BY:
    FSND_StandByNextMapBGM( fieldSound, gameData, work->loc_req.zone_id );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

    // 画面フェード開始
  case SEQ_DOORIN_FADEOUT:
    StartFadeOutEvent( event, work );
    *seq = GetNextSeq( work );
    FinishCurrentSeq( work );
    break;

    // イベント終了
  case SEQ_DOORIN_END:
    if( FIELD_TASK_MAN_IsAllTaskEnd( task_man ) ) {
      ENTRANCE_CAMERA_Recover( work->ECamWork );
      ENTRANCE_CAMERA_DeleteWork( work->ECamWork );
      DeleteDoorBM( work );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//============================================================================================
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief 自機向きを取得する
 *
 * @param fieldmap
 *
 * @return 自機の向き ( DIR_xxxx )
 */
//--------------------------------------------------------------------------------------------
static u8 GetPlayerDir( FIELDMAP_WORK* fieldmap )
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  return FIELD_PLAYER_GetDir( player );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 自機の一歩先の座標を取得する
 *
 * @param fieldmap
 * @param pos 取得した座標の格納先
 */
//--------------------------------------------------------------------------------------------
static void GetPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16 dir;

  dir = MMDL_GetDirDisp( fmmdl );

  FIELD_PLAYER_GetDirPos( player, dir, pos );
} 

//--------------------------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを取得する 
 *
 * @param work
 * 
 * @return 現在のシーケンス
 */
//--------------------------------------------------------------------------------------------
static u8 GetCurrentSeq( const FIELD_DOOR_ANIME_WORK* work )
{
  GF_ASSERT( work->seqPos < MAX_SEQ_LENGTH );

  return work->seq[ work->seqPos ];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 次のシーケンスを取得する 
 *
 * @param work
 * 
 * @return 次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static u8 GetNextSeq( const FIELD_DOOR_ANIME_WORK* work )
{
  u8 nextPos;

  nextPos = work->seqPos + 1;
  GF_ASSERT( nextPos < MAX_SEQ_LENGTH );

  return work->seq[ nextPos ];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを終了する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void FinishCurrentSeq( FIELD_DOOR_ANIME_WORK* work )
{
  work->seqPos++;
  work->count = 0;
  GF_ASSERT( work->seqPos < MAX_SEQ_LENGTH );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアを探し、配置モデルを生成する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void SearchDoorBM( FIELD_DOOR_ANIME_WORK* work )
{
  FLDMAPPER* fldmapper;
  FIELD_BMODEL_MAN* BModelMan;

  GF_ASSERT( work->doorBM == NULL );

  fldmapper = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
  BModelMan = FLDMAPPER_GetBuildModelManager( fldmapper );
  work->doorBM = FIELD_BMODEL_Create_Search( BModelMan, BM_SEARCH_ID_DOOR, &work->doorPos );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 生成したドアの配置モデルを破棄する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void DeleteDoorBM( FIELD_DOOR_ANIME_WORK* work )
{
  if( work->doorBM ) {
    FIELD_BMODEL_Delete( work->doorBM );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアアニメが終了したかどうかを判定する
 *
 * @param work イベントワーク
 *
 * @return ドアアニメが終了している場合 TRUE
 *         そうでなければ FALSE
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckDoorAnimeFinish( const FIELD_DOOR_ANIME_WORK* work )
{ 
  // ドアが存在しない
  if( CheckDoorExist( work ) == FALSE ) {
    GF_ASSERT(0);
    return TRUE;
  }

  // アニメが終わっていない
  if( FIELD_BMODEL_WaitCurrentAnime( work->doorBM ) == FALSE ) { return FALSE; }

  // SE 再生中
  if( FIELD_BMODEL_CheckCurrentSE( work->doorBM ) == TRUE ) { return FALSE; } 

  // アニメ終了
  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアが存在するかどうかをチェックする
 *
 * @param work イベントワーク
 *
 * @return ドアが存在する場合 TRUE
 *          そうでなければ FALSE
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckDoorExist( const FIELD_DOOR_ANIME_WORK* work )
{
  if( work->doorBM == NULL ) { 
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアを開くアニメを開始する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void StartDoorOpen( const FIELD_DOOR_ANIME_WORK* work )
{
  u16 seNo;

  // ドアが存在しない
  if( CheckDoorExist( work ) == FALSE ) {
    GF_ASSERT(0);
    return;
  }

  // アニメ開始
  FIELD_BMODEL_StartAnime( work->doorBM, BMANM_INDEX_DOOR_OPEN );

  // SEを再生
  if( FIELD_BMODEL_GetCurrentSENo( work->doorBM, &seNo ) ) {
    PMSND_PlaySE( seNo );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ドアを閉じるアニメを開始する
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void StartDoorClose( const FIELD_DOOR_ANIME_WORK* work )
{
  u16 seNo;

  // ドアが存在しない
  if( CheckDoorExist( work ) == FALSE ) {
    GF_ASSERT(0);
    return;
  }

  // アニメ開始
  FIELD_BMODEL_StartAnime( work->doorBM, BMANM_INDEX_DOOR_CLOSE );

  // SEを再生
  if( FIELD_BMODEL_GetCurrentSENo( work->doorBM, &seNo ) ) {
    PMSND_PlaySE( seNo );
  }
} 

//--------------------------------------------------------------------------------------------
/**
 * @brief フェードインを開始する
 *
 * @param event 現在のイベント
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void StartFadeInEvent( GMEVENT* event, const FIELD_DOOR_ANIME_WORK* work )
{
  GMEVENT* fadeEvent;

  if( work->seasonDispFlag ) { 
    // 季節フェード
    fadeEvent = EVENT_FieldFadeIn_Season( 
        work->gameSystem, work->fieldmap, work->startSeason, work->endSeason );
  }
  else { 
    // 基本フェード
    fadeEvent = EVENT_FieldFadeIn( 
        work->gameSystem, work->fieldmap, work->fadeType, FIELD_FADE_WAIT, TRUE, 0, 0 );
  }

  GMEVENT_CallEvent( event, fadeEvent );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief フェードアウトを開始する
 *
 * @param event 現在のイベント
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void StartFadeOutEvent( GMEVENT* event, const FIELD_DOOR_ANIME_WORK* work )
{
  GMEVENT* fadeEvent;

  if( work->seasonDispFlag ) { 
    // 輝度フェード
    fadeEvent = EVENT_FieldFadeOut_Black( 
        work->gameSystem, work->fieldmap, FIELD_FADE_WAIT );
  }
  else { 
    // 基本フェード
    fadeEvent = EVENT_FieldFadeOut( 
        work->gameSystem, work->fieldmap, work->fadeType, FIELD_FADE_WAIT );
  }

  GMEVENT_CallEvent( event, fadeEvent );
}
