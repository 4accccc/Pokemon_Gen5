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

#include "field/field_const.h"

#include "fieldmap.h"
#include "field_player.h"
#include "field_buildmodel.h"

#include "event_mapchange.h"        //MAPCHANGE_setPlayerVanish
#include "event_fieldmap_control.h" //EVENT_FieldFadeOut
#include "event_fldmmdl_control.h"  //EVENT_PlayerOneStepAnime
#include "event_entrance_effect.h"

#include "event_camera_act.h" 

#include "sound/pm_sndsys.h"      //PMSND_PlaySE
#include "field/field_sound.h"    // for FIELD_SOUND_ChangePlayZoneBGM


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
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

  // 以下はシーケンス動作中にセットされる
  FIELD_BMODEL * ctrl;

} FIELD_DOOR_ANIME_WORK;


//============================================================================================
//============================================================================================
static void GetPlayerFrontPos( FIELDMAP_WORK * fieldmap, VecFx32 * pos );
static BOOL CheckDoorAnimeFinish( const FIELD_DOOR_ANIME_WORK* work );
static BOOL CheckDoorExist( const FIELD_DOOR_ANIME_WORK* work );
static void StartDoorOpen( const FIELD_DOOR_ANIME_WORK* work );
static void StartDoorClose( const FIELD_DOOR_ANIME_WORK* work );



//------------------------------------------------------------------
/**
 * @brief ドアから出てくるイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorOut( GMEVENT * event, int *seq, void * wk )
{
  enum {
    SEQ_DOOROUT_INIT = 0,
    SEQ_DOOROUT_CAMERA_TRACE_WAIT,
    SEQ_DOOROUT_FADEIN,
    SEQ_DOOROUT_CAMERA_ACT_START,
    SEQ_DOOROUT_SOUND_LOAD_WAIT,
    SEQ_DOOROUT_OPENANIME_START,
    SEQ_DOOROUT_OPENANIME_WAIT,
    SEQ_DOOROUT_PLAYER_STEP,
    SEQ_DOOROUT_CLOSEANIME_START,
    SEQ_DOOROUT_CLOSEANIME_WAIT,
    SEQ_DOOROUT_END,
  };

	FIELD_DOOR_ANIME_WORK * work       = wk;
	GAMESYS_WORK *          gameSystem = work->gameSystem;
  GAMEDATA *              gameData   = work->gameData;
  FIELDMAP_WORK *         fieldmap   = work->fieldmap;
  FIELD_SOUND *           fieldSound = work->fieldSound;

  FLDMAPPER *        fldmapper  = FIELDMAP_GetFieldG3Dmapper( fieldmap );
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager( fldmapper );

  switch( *seq ) {
  // イベント初期設定
  case SEQ_DOOROUT_INIT:
    // フェードインする前に, カメラの初期設定を行う
    if( work->cameraAnimeFlag ) {
      EVENT_CAMERA_ACT_PrepareForDoorOut( fieldmap );
    }
    // 自機を消す
    MAPCHANGE_setPlayerVanish( fieldmap, TRUE );
    // カメラのトレース機能をOFF
    FIELD_CAMERA_StopTraceRequest( FIELDMAP_GetFieldCamera( fieldmap ) );
    *seq = SEQ_DOOROUT_CAMERA_TRACE_WAIT;
    break;

  // カメラのトレース処理完了待ち
  case SEQ_DOOROUT_CAMERA_TRACE_WAIT:
    // トレース処理が完了
    if( FIELD_CAMERA_CheckTrace( FIELDMAP_GetFieldCamera( fieldmap ) ) == FALSE ) {
      FSND_PlayStartBGM( fieldSound ); // BGM 再生開始
      *seq = SEQ_DOOROUT_FADEIN;
    }
    break;

  // 画面フェードイン開始
  case SEQ_DOOROUT_FADEIN:
    if( work->seasonDispFlag ) { 
      // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else { 
      // 基本フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn( gameSystem, fieldmap, work->fadeType, FIELD_FADE_WAIT, TRUE, 0, 0 ) );
    }
    *seq = SEQ_DOOROUT_SOUND_LOAD_WAIT;
    break;

  // サウンドの読み込み待ち
  case SEQ_DOOROUT_SOUND_LOAD_WAIT:
    if( PMSND_IsLoading() != TRUE ) {
      *seq = SEQ_DOOROUT_OPENANIME_START;
    }
    break;

  // ドアオープン開始
  case SEQ_DOOROUT_OPENANIME_START:
    work->ctrl = FIELD_BMODEL_Create_Search( bmodel_man, BM_SEARCH_ID_DOOR, &work->doorPos );
    if( CheckDoorExist( work ) == TRUE ) {
      StartDoorOpen( work );
    }
    *seq = SEQ_DOOROUT_CAMERA_ACT_START;
    break;

  // カメラ演出開始
  case SEQ_DOOROUT_CAMERA_ACT_START:
    if( work->cameraAnimeFlag ) {
      EVENT_CAMERA_ACT_CallDoorOutEvent( event, gameSystem, fieldmap );
    }
    // ドアがない場合
    if( CheckDoorExist(work) == FALSE ) {
      *seq = SEQ_DOOROUT_PLAYER_STEP; 
    }
    // ドアがある場合
    else {
      *seq = SEQ_DOOROUT_OPENANIME_WAIT; 
    }
    break;

  // ドアオープン待ち
  case SEQ_DOOROUT_OPENANIME_WAIT:
    if( CheckDoorAnimeFinish(work) == TRUE ) {
      *seq = SEQ_DOOROUT_PLAYER_STEP;
    }
    break;

  // 自機出現、一歩移動アニメ
  case SEQ_DOOROUT_PLAYER_STEP:
    MAPCHANGE_setPlayerVanish( fieldmap, FALSE );
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime( gameSystem, fieldmap ) );
    *seq = SEQ_DOOROUT_CLOSEANIME_START;
    break;

  // ドアクローズ開始
  case SEQ_DOOROUT_CLOSEANIME_START:
    // ドアがない場合
    if( CheckDoorExist(work) == FALSE ) {
      *seq = SEQ_DOOROUT_END;
    }
    // ドアがある場合
    else {
      StartDoorClose( work );
      *seq = SEQ_DOOROUT_CLOSEANIME_WAIT;
    }
    break;

  // ドアクローズ待ち
  case SEQ_DOOROUT_CLOSEANIME_WAIT:
    // ドアアニメが終了
    if( CheckDoorAnimeFinish(work) == TRUE ) {
      *seq = SEQ_DOOROUT_END;
    }
    break; 

  // イベント終了
  case SEQ_DOOROUT_END:
    EVENT_CAMERA_ACT_ResetCameraParameter( fieldmap );  // カメラの設定をデフォルトに戻す
    FIELD_BMODEL_Delete( work->ctrl );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
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
 *
 * @return 生成したイベント
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldDoorOutAnime ( GAMESYS_WORK * gameSystem, FIELDMAP_WORK * fieldmap, 
                                    BOOL cameraAnimeFlag,
                                    BOOL seasonDispFlag, u8 startSeason, u8 endSeason,
                                    FIELD_FADE_TYPE fadeType )
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
  work->ctrl            = NULL;

  return event;
}

//------------------------------------------------------------------
/**
 * @brief ドアに入るイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT ExitEvent_DoorIn(GMEVENT * event, int *seq, void * wk)
{
  enum {
    SEQ_DOORIN_OPENANIME_START = 0,
    SEQ_DOORIN_CAMERA_ACT,
    SEQ_DOORIN_OPENANIME_WAIT,
    SEQ_DOORIN_PLAYER_ONESTEP,
    SEQ_DOORIN_BGM_STAND_BY,
    SEQ_DOORIN_FADEOUT,
    SEQ_DOORIN_END,
  };

	FIELD_DOOR_ANIME_WORK * work = wk;
	GAMESYS_WORK  * gameSystem = work->gameSystem;
  GAMEDATA * gameData = GAMESYSTEM_GetGameData( gameSystem );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gameSystem );

  FLDMAPPER * fldmapper = FIELDMAP_GetFieldG3Dmapper(fieldmap);
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager(fldmapper);

  switch (*seq)
  { 
  case SEQ_DOORIN_OPENANIME_START:
    work->ctrl = FIELD_BMODEL_Create_Search( bmodel_man, BM_SEARCH_ID_DOOR, &work->doorPos );
    if (work->ctrl)
    {
      u16 seNo;
      FIELD_BMODEL_StartAnime( work->ctrl, BMANM_INDEX_DOOR_OPEN );
      if( FIELD_BMODEL_GetCurrentSENo( work->ctrl, &seNo) )
      {
        PMSND_PlaySE( seNo );
      }
    }
    *seq = SEQ_DOORIN_CAMERA_ACT;
    //*seq = SEQ_DOORIN_PLAYER_ONESTEP;
    break;

  case SEQ_DOORIN_CAMERA_ACT: 
    if( work->cameraAnimeFlag )
    {
      EVENT_CAMERA_ACT_CallDoorInEvent( event, gameSystem, fieldmap ); 
    }
    if (work->ctrl == NULL)
    { /* エラーよけ、ドアがない場合 */
      *seq = SEQ_DOORIN_PLAYER_ONESTEP;
    } 
    else
    {
      *seq = SEQ_DOORIN_OPENANIME_WAIT;
    }
    break;

  case SEQ_DOORIN_OPENANIME_WAIT:
    if( CheckDoorAnimeFinish(work) == TRUE ) {
      *seq = SEQ_DOORIN_PLAYER_ONESTEP;
    }
    break;

  case SEQ_DOORIN_PLAYER_ONESTEP:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gameSystem, fieldmap) );
    *seq = SEQ_DOORIN_BGM_STAND_BY;
    break;

  case SEQ_DOORIN_BGM_STAND_BY:
    { // BGM再生準備
      u16 nowZoneID = FIELDMAP_GetZoneID( fieldmap );
      FIELD_SOUND* fieldSound = GAMEDATA_GetFieldSound( gameData );
      FSND_StandByNextMapBGM( fieldSound, gameData, work->loc_req.zone_id );
    }
    *seq = SEQ_DOORIN_FADEOUT;
    break;

  case SEQ_DOORIN_FADEOUT:
    // 画面フェードアウト
    if( work->seasonDispFlag )
    { // 輝度フェード
      GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Black(gameSystem, fieldmap, FIELD_FADE_WAIT) );
    }
    else
    { // 基本フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeOut(gameSystem, fieldmap, work->fadeType, FIELD_FADE_WAIT) );
    }
    *seq = SEQ_DOORIN_END;
    break;

  case SEQ_DOORIN_END:
    if( work->cameraAnimeFlag )
    {
      EVENT_CAMERA_ACT_ResetCameraParameter( fieldmap );  // カメラの設定をデフォルトに戻す
    }
    FIELD_BMODEL_Delete( work->ctrl );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
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
//------------------------------------------------------------------
GMEVENT* EVENT_FieldDoorInAnime ( 
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, const LOCATION* loc, 
    BOOL cameraAnimeFlag, BOOL seasonDispFlag, FIELD_FADE_TYPE fadeType )
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
  work->ctrl            = NULL;

  return event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 自機の一歩先の座標を取得する
 *
 * @param fieldmap
 * @param pos 取得した座標の格納先
 */
//------------------------------------------------------------------
static void GetPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  MMDL *         fmmdl  = FIELD_PLAYER_GetMMdl( player );
  u16 dir;

  dir = MMDL_GetDirDisp( fmmdl );

  FIELD_PLAYER_GetDirPos( player, dir, pos );
} 

//------------------------------------------------------------------
/**
 * @brief ドアアニメが終了したかどうかを判定する
 *
 * @param work イベントワーク
 *
 * @return ドアアニメが終了している場合 TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------
static BOOL CheckDoorAnimeFinish( const FIELD_DOOR_ANIME_WORK* work )
{ 
  // ドアが存在しない
  if( CheckDoorExist( work ) == FALSE ) {
    GF_ASSERT(0);
    return TRUE;
  }

  // アニメが終わっていない
  if( FIELD_BMODEL_WaitCurrentAnime( work->ctrl ) == FALSE ) { return FALSE; }

  // SE 再生中
  if( FIELD_BMODEL_CheckCurrentSE( work->ctrl ) == TRUE ) { return FALSE; } 

  // アニメ終了
  return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief ドアが存在するかどうかをチェックする
 *
 * @param work イベントワーク
 *
 * @return ドアが存在する場合 TRUE
 *          そうでなければ FALSE
 */
//------------------------------------------------------------------
static BOOL CheckDoorExist( const FIELD_DOOR_ANIME_WORK* work )
{
  if( work->ctrl == NULL ) { 
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//------------------------------------------------------------------
/**
 * @brief ドアを開くアニメを開始する
 *
 * @param work
 */
//------------------------------------------------------------------
static void StartDoorOpen( const FIELD_DOOR_ANIME_WORK* work )
{
  u16 seNo;

  // ドアが存在しない
  if( CheckDoorExist( work ) == FALSE ) {
    GF_ASSERT(0);
    return;
  }

  // アニメ開始
  FIELD_BMODEL_StartAnime( work->ctrl, BMANM_INDEX_DOOR_OPEN );

  // SEを再生
  if( FIELD_BMODEL_GetCurrentSENo( work->ctrl, &seNo ) ) {
    PMSND_PlaySE( seNo );
  }
}

//------------------------------------------------------------------
/**
 * @brief ドアを閉じるアニメを開始する
 *
 * @param work
 */
//------------------------------------------------------------------
static void StartDoorClose( const FIELD_DOOR_ANIME_WORK* work )
{
  u16 seNo;

  // ドアが存在しない
  if( CheckDoorExist( work ) == FALSE ) {
    GF_ASSERT(0);
    return;
  }

  // アニメ開始
  FIELD_BMODEL_StartAnime( work->ctrl, BMANM_INDEX_DOOR_CLOSE );

  // SEを再生
  if( FIELD_BMODEL_GetCurrentSENo( work->ctrl, &seNo ) ) {
    PMSND_PlaySE( seNo );
  }
}

















