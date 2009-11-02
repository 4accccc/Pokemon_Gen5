////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケセン回復アニメーションイベント
 * @file   event_pc_recovery.c
 * @author obata_toshihiro
 * @date   2009.09.18
 */
//////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_pc_recovery.h"
#include "pc_recovery_anime.naix"
#include "sound/pm_sndsys.h"


//==============================================================================
// 定数
//==============================================================================
//#define PC_RECOVERY_DEBUG_ON  // デバッグ出力の有無
#define MAX_BALL_NUM (6)  // 最大手持ちポケモン数
#define BALLSET_WAIT (15) // ボールセット待ち時間
#define ANIME_START_WAIT (10) // ボールセット後の回復アニメーション開始待ち時間

// イベント・シーケンス
typedef enum
{
  SEQ_BALLSET,      // ボール設置
  SEQ_WAIT,         // 待ち
  SEQ_RECOV_ANIME,  // 回復アニメーション
  SEQ_EXIT,         // イベント終了
} SEQID;

//------------------
// 拡張オブジェクト
//------------------
// ユニット番号
#define EXOBJUNITID (1)

// オブジェクト番号
typedef enum
{
  EXOBJ_BALL_0,
  EXOBJ_BALL_1,
  EXOBJ_BALL_2,
  EXOBJ_BALL_3,
  EXOBJ_BALL_4,
  EXOBJ_BALL_5,
  EXOBJ_NUM
} EXOBJID;

// リソーステーブル
GFL_G3D_UTIL_RES res_table[] = 
{
  // ボールのモデル
  {ARCID_PC_RECOVERY, NARC_pc_recovery_anime_pc_mb_nsbmd, GFL_G3D_UTIL_RESARC},
  // ボールのアニメーション
  {ARCID_PC_RECOVERY, NARC_pc_recovery_anime_pc_mb_nsbtp, GFL_G3D_UTIL_RESARC},
};
// アニメーションテーブル
GFL_G3D_UTIL_ANM ball_anm_table[] = 
{
  {1, 0},
};
// オブジェクト設定テーブル
GFL_G3D_UTIL_OBJ obj_table[] = 
{
  {0, 0, 0, ball_anm_table, NELEMS(ball_anm_table)},
  {0, 0, 0, ball_anm_table, NELEMS(ball_anm_table)},
  {0, 0, 0, ball_anm_table, NELEMS(ball_anm_table)},
  {0, 0, 0, ball_anm_table, NELEMS(ball_anm_table)},
  {0, 0, 0, ball_anm_table, NELEMS(ball_anm_table)},
  {0, 0, 0, ball_anm_table, NELEMS(ball_anm_table)},
};
// 3Dセットアップテーブル
GFL_G3D_UTIL_SETUP setup = 
{
  res_table, NELEMS(res_table), obj_table, NELEMS(obj_table)
};

// ボール設置座標 (回復マシンからのオフセット)
VecFx32 ball_offset[] = 
{
  {  -4*FX32_ONE,   14*FX32_ONE,  -3*FX32_ONE},
  {   4*FX32_ONE,   14*FX32_ONE,  -3*FX32_ONE},
  {   9*FX32_ONE,   14*FX32_ONE,   4*FX32_ONE},
  {   4*FX32_ONE,   14*FX32_ONE,   8*FX32_ONE},
  {  -4*FX32_ONE,   14*FX32_ONE,   8*FX32_ONE},
  {  -9*FX32_ONE,   14*FX32_ONE,   4*FX32_ONE},
};
// ボールのオブジェクト番号
EXOBJID ball_obj_id[] = 
{
  EXOBJ_BALL_0,
  EXOBJ_BALL_1,
  EXOBJ_BALL_2,
  EXOBJ_BALL_3,
  EXOBJ_BALL_4,
  EXOBJ_BALL_5,
};

//==============================================================================
// イベントワーク
//==============================================================================
typedef struct
{
  HEAPID           heapID;  // ヒープID
  FIELDMAP_WORK* fieldmap;  // フィールドマップ
  VecFx32      machinePos;  // 回復マシンの座標
  u8           pokemonNum;  // 手持ちポケモンの数
  u8           setBallNum;  // マシンにセットしたボールの数
  u16            seqCount;  // シーケンス動作フレームカウンタ

} RECOVERY_WORK;


//==============================================================================
// プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT EVENT_FUNC_PcRecoveryAnime( GMEVENT* event, int* seq, void* work );
static void SetupEvent( 
    RECOVERY_WORK* work, GAMESYS_WORK* gsys, VecFx32* mechine_pos, u8 pokemon_num );
static void ExitEvent( RECOVERY_WORK* work );
static void ChangeSequence( RECOVERY_WORK* work, int* seq, SEQID next_seq );
static void SetMonsterBall( RECOVERY_WORK* work );
static void StartRecoveryAnime( RECOVERY_WORK* work );
static void StopRecoveryAnime( RECOVERY_WORK* work );
static BOOL IsRecoveryAnimeEnd( RECOVERY_WORK* work );
static void PlayRecoveryAnime( RECOVERY_WORK* work );


//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
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


//==============================================================================
/**
 * @brief イベント制御関数
 *
 * @param event イベント
 * @param seq   シーケンス
 * @param work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//==============================================================================
static GMEVENT_RESULT EVENT_FUNC_PcRecoveryAnime( GMEVENT* event, int* seq, void* work )
{
  RECOVERY_WORK* rw = (RECOVERY_WORK*)work;

#ifdef PC_RECOVERY_DEBUG_ON
  switch( *seq )
  {
  case SEQ_BALLSET:     OBATA_Printf( "SEQ_BALLSET\n" );     break;
  case SEQ_WAIT:        OBATA_Printf( "SEQ_BALLSET\n" );     break;
  case SEQ_RECOV_ANIME: OBATA_Printf( "SEQ_RECOV_ANIME\n" ); break;
  case SEQ_EXIT:        OBATA_Printf( "SEQ_EXIT\n" );        break;
  }
#endif


  switch( *seq )
  {
  case SEQ_BALLSET:
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
  case SEQ_WAIT:
    if( ANIME_START_WAIT < rw->seqCount )
    {
        StartRecoveryAnime( rw );
        ChangeSequence( rw, seq, SEQ_RECOV_ANIME );
    }
    break;
  case SEQ_RECOV_ANIME:
    PlayRecoveryAnime( rw );
    if( IsRecoveryAnimeEnd( rw ) )
    {
        StopRecoveryAnime( rw );
        ChangeSequence( rw, seq, SEQ_EXIT );
    }
    break;
  case SEQ_EXIT:
    ExitEvent( rw );
    return GMEVENT_RES_FINISH;
  } 
  rw->seqCount++;
  return GMEVENT_RES_CONTINUE;
}


//==============================================================================
/**
 * @breif イベント初期化処理
 *
 * @param work        イベントワーク
 * @param gsys        ゲームシステム
 * @param machine_pos 回復マシンの座標
 * @param pokemon_num 手持ちポケモンの数
 */
//==============================================================================
static void SetupEvent( 
    RECOVERY_WORK* work, GAMESYS_WORK* gsys, VecFx32* machine_pos, u8 pokemon_num )
{
  FIELDMAP_WORK*        fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID                 heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cont = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // イベントワーク初期化
  work->heapID     = heap_id;
  work->fieldmap   = fieldmap;
  work->pokemonNum = pokemon_num;
  work->setBallNum = 0;
  VEC_Set( &work->machinePos, machine_pos->x, machine_pos->y, machine_pos->z );

  // 念のため例外処理
  if( MAX_BALL_NUM < work->pokemonNum ) work->pokemonNum = MAX_BALL_NUM;

  // 拡張オブジェクトユニット追加
  FLD_EXP_OBJ_AddUnit( exobj_cont, &setup, EXOBJUNITID );
}

//==============================================================================
/**
 * @brief イベント終了処理
 *
 * @param work イベントワーク
 */
//==============================================================================
static void ExitEvent( RECOVERY_WORK* work )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cont = FIELDMAP_GetExpObjCntPtr( work->fieldmap );

  // 拡張オブジェクトユニットを破棄
  FLD_EXP_OBJ_DelUnit( exobj_cont, EXOBJUNITID );
}

//==============================================================================
/**
 * @brief シーケンスを変更する
 *
 * @param work     イベントワーク
 * @param seq      イベントシーケンスワーク
 * @param next_seq 次のシーケンス
 */
//==============================================================================
static void ChangeSequence( RECOVERY_WORK* work, int* seq, SEQID next_seq )
{
  // シーケンスを更新し, カウンタ初期化
  *seq = next_seq;
  work->seqCount = 0;
}

//==============================================================================
/**
 * @brief モンスターボールを回復マシンにセットする
 *
 * @param work  イベントワーク
 */
//==============================================================================
static void SetMonsterBall( RECOVERY_WORK* work )
{
  int index;// セットするボール番号
  GFL_G3D_OBJSTATUS* status;
  FLD_EXP_OBJ_CNT_PTR exobj_cont;

  // すでに全ボールのセットが完了済みなら, 何もしない
  if( work->pokemonNum <= work->setBallNum ) return;

  // ボールをセット
  index      = work->setBallNum; 
  exobj_cont = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
  status     = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cont, EXOBJUNITID, ball_obj_id[index] );
  VEC_Add( &work->machinePos, &ball_offset[index], &status->trans );
  VEC_Set( &status->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status->rotate );

  // セットしたボールの数をインクリメント
  work->setBallNum++;

  // SE再生
  PMSND_PlaySE( SEQ_SE_BOWA1 );

#ifdef PC_RECOVERY_DEBUG_ON
  OBATA_Printf( "SetMonsterBall\n" );
#endif
}

//==============================================================================
/**
 * @brief 回復アニメーション開始処理
 *
 * @param work イベントワーク
 */
//==============================================================================
static void StartRecoveryAnime( RECOVERY_WORK* work )
{
  int i;

  // ボールのアニメーション開始
  for( i=0; i<work->setBallNum; i++ )
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cont = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
    EXP_OBJ_ANM_CNT_PTR anm_cont = FLD_EXP_OBJ_GetAnmCnt( exobj_cont, EXOBJUNITID, i, 0 );
    FLD_EXP_OBJ_ChgAnmStopFlg( anm_cont, FALSE );
    FLD_EXP_OBJ_ValidCntAnm( exobj_cont, EXOBJUNITID, ball_obj_id[i], 0, TRUE );
  }

  // ME再生
  PMSND_PauseBGM( TRUE );
  PMSND_PushBGM();
  PMSND_PlayBGM( SEQ_ME_ASA );

#ifdef PC_RECOVERY_DEBUG_ON
  OBATA_Printf( "StartRecoveryAnime\n" );
#endif
}

//==============================================================================
/**
 * @brief 回復アニメーション終了処理
 *
 * @param work イベントワーク
 */
//==============================================================================
static void StopRecoveryAnime( RECOVERY_WORK* work )
{
  int i;

  // ボールのアニメーション終了
  for( i=0; i<work->setBallNum; i++ )
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cont = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
    EXP_OBJ_ANM_CNT_PTR anm_cont = FLD_EXP_OBJ_GetAnmCnt( exobj_cont, EXOBJUNITID, i, 0 );
    FLD_EXP_OBJ_ChgAnmStopFlg( anm_cont, FALSE );
  }

  // BGM再生再開
  PMSND_PopBGM();
  PMSND_PauseBGM( FALSE );

#ifdef PC_RECOVERY_DEBUG_ON
  OBATA_Printf( "StopRecoveryAnime\n" );
#endif
}

//==============================================================================
/**
 * @brief 回復アニメーション終了チェック
 *
 * @param work イベントワーク
 *
 * @param アニメーション終了時は TRUE
 */
//==============================================================================
static BOOL IsRecoveryAnimeEnd( RECOVERY_WORK* work )
{
  int i;

  // 全ボールのアニメーションが終了しているかどうか
  for( i=0; i<work->setBallNum; i++ )
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cont = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
    EXP_OBJ_ANM_CNT_PTR anm_cont = FLD_EXP_OBJ_GetAnmCnt( exobj_cont, EXOBJUNITID, i, 0 );
    if( FLD_EXP_OBJ_ChkAnmEnd( anm_cont ) == FALSE )
    {
      return FALSE;
    }
  }
  return TRUE;
}

//==============================================================================
/**
 * @brief 回復アニメーション再生処理
 *
 * @param work イベントワーク
 */
//==============================================================================
static void PlayRecoveryAnime( RECOVERY_WORK* work )
{
  // ボールアニメーション再生
  FLD_EXP_OBJ_CNT_PTR exobj_cont = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
  FLD_EXP_OBJ_PlayAnime( exobj_cont );

  
#ifdef PC_RECOVERY_DEBUG_ON
  {
    fx32 frame = FLD_EXP_OBJ_GetObjAnmFrm( exobj_cont, EXOBJUNITID, 0, 0 );
    OBATA_Printf( "anmframe = %d\n", frame>>FX32_SHIFT );
  }
#endif
}
