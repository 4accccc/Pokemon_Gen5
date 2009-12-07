//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケセン回復アニメーションイベント
 * @file   event_pc_recovery.c
 * @author obata_toshihiro
 * @date   2009.09.18
 */
//////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_pc_recovery.h"
#include "pc_recovery_anime.naix"
#include "sound/pm_sndsys.h"
#include "field_buildmodel.h"
#include "../../../resource/fldmapdata/build_model/output/buildmodel_indoor.naix"
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "field_sound.h"  // for FIELD_SOUND_xxxx


//========================================================================================
// ■定数
//========================================================================================
#define BALLSET_WAIT     (15) // ボールセット間隔
#define ANIME_START_WAIT (10) // ボールセット後の回復アニメーション開始待ち時間
#define PCMACHINE_SEARCH_RANGE (FIELD_CONST_GRID_SIZE * 5)  // PCマシン検索範囲

// イベント・シーケンス
typedef enum {
  SEQ_BALLSET,      // ボール設置
  SEQ_WAIT,         // 待ち
  SEQ_RECOV_ANIME,  // 回復アニメーション
  SEQ_EXIT,         // イベント終了
} SEQID;

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
VecFx32 ball_offset[BALL_NUM] = 
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
typedef struct
{
  HEAPID                  heapID;  // ヒープID
  FIELDMAP_WORK*        fieldmap;  // フィールドマップ
  VecFx32             machinePos;  // 回復マシンの座標
  u8                  pokemonNum;  // 手持ちポケモンの数
  u8                  setBallNum;  // マシンにセットしたボールの数
  u16                   seqCount;  // シーケンス動作フレームカウンタ
  FIELD_BMODEL_MAN*        bmMan;  // 配置モデルマネージャ
  FIELD_BMODEL* ballBM[BALL_NUM];  // ボールの配置モデル

} RECOVERY_WORK;


//========================================================================================
// ■プロトタイプ宣言
//========================================================================================
static GMEVENT_RESULT EVENT_FUNC_PcRecoveryAnime( GMEVENT* event, int* seq, void* work );
static void SetupEvent( 
    RECOVERY_WORK* work, GAMESYS_WORK* gsys, VecFx32* mechine_pos, u8 pokemon_num );
static void ExitEvent( RECOVERY_WORK* work );
static void ChangeSequence( RECOVERY_WORK* work, int* seq, SEQID next_seq );
static void SetMonsterBall( RECOVERY_WORK* work );
static void StartRecoveryAnime( RECOVERY_WORK* work );
static void StopRecoveryAnime( RECOVERY_WORK* work );
static BOOL IsRecoveryAnimeEnd( RECOVERY_WORK* work );


//========================================================================================
// ■外部公開関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief ポケセン回復アニメーションイベントを作成する
 *
 * @param gsys        ゲームシステム
 * @param parent      親イベント
 * @param machine_pos 回復マシンの座標
 * @param pokemon_num 手持ちポケモンの数
 *
 * @return 作成した回復アニメイベント
 */
//----------------------------------------------------------------------------------------
GMEVENT* EVENT_PcRecoveryAnime( 
    GAMESYS_WORK* gsys, GMEVENT* parent, VecFx32* machine_pos, u8 pokemon_num )
{
  GMEVENT* event;
  RECOVERY_WORK* work;

  // イベント作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_PcRecoveryAnime, sizeof(RECOVERY_WORK) );

  // イベントワーク初期化
  work = (RECOVERY_WORK*)GMEVENT_GetEventWork( event );
  SetupEvent( work, gsys, machine_pos, pokemon_num ); 
  return event;
}


//========================================================================================
// ■非公開関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief イベント制御関数
 *
 * @param event イベント
 * @param seq   シーケンス
 * @param work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//----------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_PcRecoveryAnime( GMEVENT* event, int* seq, void* work )
{
  RECOVERY_WORK* rw = (RECOVERY_WORK*)work;

  switch( *seq )
  {
  case SEQ_BALLSET: // ボールセット
    // 一定タイミングでボールをセット
    if( rw->seqCount % BALLSET_WAIT == 0 )
    {
      SetMonsterBall( rw );

      // 全ボールをセット完了
      if( rw->pokemonNum <= rw->setBallNum )
      {
        ChangeSequence( rw, seq, SEQ_WAIT );
      }
    }
    break;
  case SEQ_WAIT:  // ボールセット後の待ち
    if( ANIME_START_WAIT < rw->seqCount )
    {
        StartRecoveryAnime( rw );
        ChangeSequence( rw, seq, SEQ_RECOV_ANIME );
    }
    break;
  case SEQ_RECOV_ANIME: // アニメーション再生
    if( IsRecoveryAnimeEnd( rw ) )
    {
        StopRecoveryAnime( rw );
        ChangeSequence( rw, seq, SEQ_EXIT );
    }
    break;
  case SEQ_EXIT:  // イベント終了
    ExitEvent( rw );
    return GMEVENT_RES_FINISH;
  } 
  rw->seqCount++;
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------------------
/**
 * @breif イベント初期化処理
 *
 * @param work        イベントワーク
 * @param gsys        ゲームシステム
 * @param machine_pos 回復マシンの座標
 * @param pokemon_num 手持ちポケモンの数
 */
//----------------------------------------------------------------------------------------
static void SetupEvent( 
    RECOVERY_WORK* work, GAMESYS_WORK* gsys, VecFx32* machine_pos, u8 pokemon_num )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLDMAPPER*   g3d_mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );

  // イベントワーク初期化
  work->heapID     = heap_id;
  work->fieldmap   = fieldmap;
  work->pokemonNum = pokemon_num;
  work->setBallNum = 0;
  VEC_Set( &work->machinePos, machine_pos->x, machine_pos->y, machine_pos->z );
  work->bmMan = FLDMAPPER_GetBuildModelManager( g3d_mapper );

  // 念のため例外処理
  if( BALL_NUM < work->pokemonNum ) work->pokemonNum = BALL_NUM;

  // ボールの配置モデルを作成
  {
    int i;
    BMODEL_ID bmodel_id = NARC_output_buildmodel_indoor_mball01_nsbmd;
    GFL_G3D_OBJSTATUS status;
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );

    for( i=0; i<BALL_NUM; i++ )
    {
      VEC_Add( machine_pos, &ball_offset[i], &status.trans );
      work->ballBM[i] = FIELD_BMODEL_Create_Direct( work->bmMan, bmodel_id, &status );
    }
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief イベント終了処理
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void ExitEvent( RECOVERY_WORK* work )
{
  int i;

  // ボールの配置モデルを破棄
  { 
    // 配置したモデルを解除
    for( i=0; i<work->setBallNum; i++ )
    {
      FIELD_BMODEL_MAN_releaseBuildModel( work->bmMan, work->ballBM[i] );
    }
    // 作成したモデルを削除
    for( i=0; i<BALL_NUM; i++ )
    {
      FIELD_BMODEL_Delete( work->ballBM[i] );
    }
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief シーケンスを変更する
 *
 * @param work     イベントワーク
 * @param seq      イベントシーケンスワーク
 * @param next_seq 次のシーケンス
 */
//----------------------------------------------------------------------------------------
static void ChangeSequence( RECOVERY_WORK* work, int* seq, SEQID next_seq )
{
  // シーケンスを更新し, カウンタ初期化
  *seq = next_seq;
  work->seqCount = 0;
}

//----------------------------------------------------------------------------------------
/**
 * @brief モンスターボールを回復マシンにセットする
 *
 * @param work  イベントワーク
 */
//----------------------------------------------------------------------------------------
static void SetMonsterBall( RECOVERY_WORK* work )
{
  int index;  // セットするボール番号

  // すでに全ボールのセットが完了済みなら, 何もしない
  if( work->pokemonNum <= work->setBallNum ) return;

  // 配置モデルを登録
  index = work->setBallNum; 
  FIELD_BMODEL_MAN_EntryBuildModel( work->bmMan, work->ballBM[index] );

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
static void StartRecoveryAnime( RECOVERY_WORK* work )
{
  int i;

  // ボール
  for( i=0; i<work->setBallNum; i++ )
  {
    FIELD_BMODEL_SetAnime( work->ballBM[i], 0, BMANM_REQ_START );
  }

  // マシン
  {
    G3DMAPOBJST**  objst;
    u32           objnum;
    FLDHIT_RECT     rect;
    VecFx32          pos;
    FIELD_PLAYER* player;

    // 自機の周囲を検索
    player = FIELDMAP_GetFieldPlayer( work->fieldmap );
    FIELD_PLAYER_GetPos( player, &pos );
    rect.top    = pos.z - (PCMACHINE_SEARCH_RANGE << FX32_SHIFT);
    rect.bottom = pos.z + (PCMACHINE_SEARCH_RANGE << FX32_SHIFT);
    rect.left   = pos.x - (PCMACHINE_SEARCH_RANGE << FX32_SHIFT);
    rect.right  = pos.x + (PCMACHINE_SEARCH_RANGE << FX32_SHIFT);
    objst = FIELD_BMODEL_MAN_CreateObjStatusList( work->bmMan, &rect, 
                                                  BM_SEARCH_ID_PCMACHINE, &objnum );
    // 発見した配置モデルのアニメ再生開始
    if( 0 < objnum )
    {
      G3DMAPOBJST_setAnime( work->bmMan, objst[0], 0, BMANM_REQ_START );
      G3DMAPOBJST_setAnime( work->bmMan, objst[0], 1, BMANM_REQ_START );
    }
    // 後始末
    GFL_HEAP_FreeMemory( objst );
  }

  // ME再生
  {
    GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( work->fieldmap );
    GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
    FIELD_SOUND*  fsnd = GAMEDATA_GetFieldSound( gdata );
    FIELD_SOUND_PushPlayJingleBGM( fsnd, SEQ_ME_ASA );
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief 回復アニメーション終了処理
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void StopRecoveryAnime( RECOVERY_WORK* work )
{
  int i;

  // ボールのアニメーション終了
  for( i=0; i<work->setBallNum; i++ )
  {
    FIELD_BMODEL_SetAnime( work->ballBM[i], 0, BMANM_REQ_STOP );
  }

  // BGM復帰
  {
    GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( work->fieldmap );
    GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
    FIELD_SOUND*  fsnd = GAMEDATA_GetFieldSound( gdata );
    FIELD_SOUND_PopBGM( fsnd );
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
static BOOL IsRecoveryAnimeEnd( RECOVERY_WORK* work )
{
  int i;

  // 全ボールのアニメーションが終了しているかどうか
  for( i=0; i<work->setBallNum; i++ )
  {
    BOOL stop = FIELD_BMODEL_GetAnimeStatus( work->ballBM[i], 0 );
    if( stop != TRUE )
    {
      return FALSE;
    }
  }
  return TRUE;
} 
