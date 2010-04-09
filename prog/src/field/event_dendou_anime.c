//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  殿堂入り登録アニメーションイベント
 * @file   event_dendou_anime.c
 * @author obata_toshihiro
 * @date   2010.04.07
 */
//////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "event_dendou_anime.h"
#include "fieldmap.h"
#include "field_buildmodel.h"
#include "field_sound.h"  // for FIELD_SOUND_xxxx

#include "gamesystem/game_event.h" 
#include "sound/pm_sndsys.h"
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE

#include "pc_recovery_anime.naix"
#include "../../../resource/fldmapdata/build_model/output/buildmodel_indoor.naix"


//========================================================================================
// ■定数
//========================================================================================
#define BALLSET_WAIT      (20) // ボールセット間隔
#define ANIME_START_WAIT  (10) // ボールセット後の回復アニメーション開始待ち時間
#define ANIME_FINISH_WAIT (60) // ボールセット後の回復アニメーション終了待ち時間
#define MACHINE_SEARCH_RANGE (FIELD_CONST_GRID_SIZE * 5)  // マシン検索範囲

// ボール番号
typedef enum {
  BALL_1,
  BALL_2,
  BALL_3,
  BALL_4,
  BALL_5,
  BALL_6,
  BALL_NUM
} BALL_INDEX;

// ボール設置座標 (回復マシンからのオフセット)
static const VecFx32 ball_offset[BALL_NUM] = 
{
  {  -4*FX32_ONE,   14*FX32_ONE,  -3*FX32_ONE},
  {   4*FX32_ONE,   14*FX32_ONE,  -3*FX32_ONE},
  {   9*FX32_ONE,   14*FX32_ONE,   4*FX32_ONE},
  {   4*FX32_ONE,   14*FX32_ONE,   8*FX32_ONE},
  {  -4*FX32_ONE,   14*FX32_ONE,   8*FX32_ONE},
  {  -9*FX32_ONE,   14*FX32_ONE,   4*FX32_ONE},
};

//========================================================================================
// ■イベントワーク
//========================================================================================
typedef struct {

  HEAPID            heapID;           // ヒープID
  FIELDMAP_WORK*    fieldmap;         // フィールドマップ
  VecFx32           machinePos;       // 回復マシンの座標
  u8                pokemonNum;       // 手持ちポケモンの数
  u8                setBallNum;       // マシンにセットしたボールの数
  u16               seqCount;         // シーケンス動作フレームカウンタ
  FIELD_BMODEL_MAN* bmMan;            // 配置モデルマネージャ
  FIELD_BMODEL*     ballBM[BALL_NUM]; // ボールの配置モデル
  G3DMAPOBJST*      machineObjSt;     // 回復マシン配置モデル
  FIELD_BMODEL* machineBM;

} EVENT_WORK;


//========================================================================================
// ■プロトタイプ宣言
//========================================================================================
static GMEVENT_RESULT DendouMachineEvent( GMEVENT* event, int* seq, void* wk );
static void SetupEvent( EVENT_WORK* work, GAMESYS_WORK* gsys );
static void ExitEvent( EVENT_WORK* work );
static void SetMonsterBall( EVENT_WORK* work );
static void StartRecoveryAnime( EVENT_WORK* work );
static void StopRecoveryAnime( EVENT_WORK* work );
static BOOL IsRecoveryAnimeEnd( EVENT_WORK* work );
static int GetPokemonNum_withoutEgg( GAMESYS_WORK* gameSystem );


//========================================================================================
// ■外部公開関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief 殿堂入りアニメイベントを生成する
 *
 * @param gsys
 * @param parent
 *
 * @return 殿堂入りアニメイベント
 */
//----------------------------------------------------------------------------------------
GMEVENT* EVENT_DendouAnime( GAMESYS_WORK* gsys, GMEVENT* parent )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // イベントを生成
  event = GMEVENT_Create( gsys, parent, DendouMachineEvent, sizeof(EVENT_WORK) );

  // イベントワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  SetupEvent( work, gsys );

  return event;
}


//========================================================================================
// ■非公開関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief 殿堂入り登録アニメーションイベント
 *
 * @param event
 * @param seq
 * @param wk
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//----------------------------------------------------------------------------------------
static GMEVENT_RESULT DendouMachineEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq ) {
  case 0: 
    if( work->seqCount % BALLSET_WAIT == 0 ) {
      // ボールを設置
      SetMonsterBall( work ); 
      // すべてのボールを設置完了
      if( work->pokemonNum <= work->setBallNum ) {
        (*seq)++; 
        work->seqCount = 0;
      }
    }
    break;

  case 1:  
    if( ANIME_START_WAIT < work->seqCount ) {
      StartRecoveryAnime( work ); // アニメ開始
      (*seq)++;
      work->seqCount = 0;
    }
    break;

  case 2: 
    if( IsRecoveryAnimeEnd( work ) ) {
      (*seq)++;
    }
    break;

  case 3:  
    ExitEvent( work );
    return GMEVENT_RES_FINISH;
  }
  work->seqCount++;
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------------------
/**
 * @breif イベント初期化処理
 *
 * @param work
 * @param gsys
 */
//----------------------------------------------------------------------------------------
static void SetupEvent( EVENT_WORK* work, GAMESYS_WORK* gsys )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMAPPER*     mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );

  // イベントワーク初期化
  work->heapID       = FIELDMAP_GetHeapID( fieldmap );
  work->fieldmap     = fieldmap;
  work->pokemonNum   = GetPokemonNum_withoutEgg( gsys );
  work->setBallNum   = 0;
  work->bmMan        = FLDMAPPER_GetBuildModelManager( mapper );
  work->machineObjSt = NULL;

  // 念のため例外処理
  if( BALL_NUM < work->pokemonNum ) { work->pokemonNum = BALL_NUM; }

  // 回復マシンの座標を取得
  {
    FIELD_PLAYER* player;
    VecFx32 pos;
    G3DMAPOBJST** objst;
    u32 objnum;
    FLDHIT_RECT rect;
    player = FIELDMAP_GetFieldPlayer( work->fieldmap );
    FIELD_PLAYER_GetPos( player, &pos );
    rect.top    = pos.z - (MACHINE_SEARCH_RANGE << FX32_SHIFT);
    rect.bottom = pos.z + (MACHINE_SEARCH_RANGE << FX32_SHIFT);
    rect.left   = pos.x - (MACHINE_SEARCH_RANGE << FX32_SHIFT);
    rect.right  = pos.x + (MACHINE_SEARCH_RANGE << FX32_SHIFT);
    objst = FIELD_BMODEL_MAN_CreateObjStatusList( work->bmMan, &rect, 
                                                  BM_SEARCH_ID_EVENT, &objnum );
    if( objst ) {
      work->machineObjSt = objst[0];
      G3DMAPOBJST_getPos( objst[0], &work->machinePos );
    }
    GFL_HEAP_FreeMemory( objst ); 
  }

  if( work->machineObjSt ) {
    work->machineBM = FIELD_BMODEL_Create( work->bmMan, work->machineObjSt ); 
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief イベント終了処理
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void ExitEvent( EVENT_WORK* work )
{
  int i;

#if 0
  // ボールの配置モデルを破棄
  for( i=0; i<work->pokemonNum; i++ ) {
    FIELD_BMODEL_Delete( work->ballBM[i] );
  }
#endif

  if( work->machineObjSt ) {
    FIELD_BMODEL_Delete( work->machineBM );
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief モンスターボールを回復マシンにセットする
 *
 * @param work  イベントワーク
 */
//----------------------------------------------------------------------------------------
static void SetMonsterBall( EVENT_WORK* work )
{
  int index;  // セットするボール番号

  // すでに全ボールのセットが完了済みなら, 何もしない
  if( work->pokemonNum <= work->setBallNum ) { return; }

  // 配置モデルを登録
  index = work->setBallNum; 

  {
    BMODEL_ID bmodel_id = NARC_output_buildmodel_indoor_mball01_nsbmd;
    GFL_G3D_OBJSTATUS status;
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );
    VEC_Add( &work->machinePos, &ball_offset[index], &status.trans );
    work->ballBM[index] = FIELD_BMODEL_Create_Direct( work->bmMan, bmodel_id, &status );
  }

  // セットしたボールの数をインクリメント
  work->setBallNum++;

  // SE再生
  PMSND_PlaySE( SEQ_SE_BOWA1 ); 
}

//----------------------------------------------------------------------------------------
/**
 * @brief 回復アニメーション開始処理
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void StartRecoveryAnime( EVENT_WORK* work )
{
  int i;

  // ボール
  for( i=0; i<work->setBallNum; i++ ) {
    FIELD_BMODEL_SetAnime( work->ballBM[i], 0, BMANM_REQ_LOOP );
  }

  // マシン
  if( work->machineObjSt ) {
    FIELD_BMODEL_StartAnime( work->machineBM, 0 );
  } 
}

//----------------------------------------------------------------------------------------
/**
 * @brief 回復アニメーション終了処理
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void StopRecoveryAnime( EVENT_WORK* work )
{
  int i;

  // ボールのアニメーション終了
  for( i=0; i<work->setBallNum; i++ ) {
    FIELD_BMODEL_SetAnime( work->ballBM[i], 0, BMANM_REQ_STOP );
  } 
}

//----------------------------------------------------------------------------------------
/**
 * @brief 回復アニメーション終了チェック
 *
 * @param work イベントワーク
 *
 * @param アニメーション終了時は TRUE
 */
//----------------------------------------------------------------------------------------
static BOOL IsRecoveryAnimeEnd( EVENT_WORK* work )
{
#if 0
  // 全ボールのアニメーションが終了しているかどうか
  for( i=0; i<work->setBallNum; i++ )
  {
    BOOL stop = FIELD_BMODEL_GetAnimeStatus( work->ballBM[i], 0 );
    if( stop != TRUE ) { return FALSE; }
  }
  return TRUE;
#endif

  // マシンのアニメで判定
  if( work->machineObjSt ) {
    return FIELD_BMODEL_WaitCurrentAnime( work->machineBM );
  }
  else {
    return TRUE;
  }
} 

//----------------------------------------------------------------------------------------
/**
 * @brief タマゴを除いた手持ちポケモンの数を取得する
 *
 * @param gameSystem
 *
 * @return 手持ちポケモンの数
 */
//----------------------------------------------------------------------------------------
static int GetPokemonNum_withoutEgg( GAMESYS_WORK* gameSystem )
{
  GAMEDATA*  gameData = GAMESYSTEM_GetGameData( gameSystem );
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( gameData );
  return PokeParty_GetPokeCountNotEgg( party );
}
