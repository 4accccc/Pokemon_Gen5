////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フィールドマップTCB( 自機の移動 )
 * @file   fieldmap_tcb_move_player.c
 * @author obata
 * @date   2009.08.25
 *
 */
////////////////////////////////////////////////////////////////////////
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "fieldmap_tcb.h"
#include "fieldmap.h"
#include "field_player.h"


//==========================================================================================
/**
 * @brief TCBワークエリア
 */
//==========================================================================================
typedef struct
{
  FIELDMAP_WORK* pFieldmap; // 動作対象のフィールドマップ
  u16            frame;     // 動作フレーム数
  u16            endFrame;  // 最大フレーム数
  float          moveDist;  // 移動距離
}
MOVE_WORK;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
void InitWork( MOVE_WORK* work, FIELDMAP_WORK* fieldmap, u16 end_frame, float move_dist );
void DeleteTask( GFL_TCB* tcb );

void TCB_FUNC_DisappearPlayer_LinearUp( GFL_TCB* tcb, void* work );
void TCB_FUNC_AppearPlayer_LinearDown( GFL_TCB* tcb, void* work );
void TCB_FUNC_AppearPlayer_Fall( GFL_TCB* tcb, void* work );

void CalcDrawOffset_LinearUp( VecFx32* p_vec, float frame, float max_frame, float max_y );
void CalcDrawOffset_LinearDown( VecFx32* p_vec, float frame, float max_frame, float max_y );
void CalcDrawOffset_Fall( VecFx32* p_vec, float frame, float max_frame, float max_y );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 上昇退場 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_DisappearPlayer_LinearUp( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  MOVE_WORK*     work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( MOVE_WORK ) );

  // TCBワーク初期化
  InitWork( work, fieldmap, frame, dist );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_DisappearPlayer_LinearUp, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 降下登場 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_AppearPlayer_LinearDown( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  MOVE_WORK*     work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( MOVE_WORK ) );

  // TCBワーク初期化
  InitWork( work, fieldmap, frame, dist );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_AppearPlayer_LinearDown, work, 0 );

  // 主人公の向きを設定
  {
    FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
    MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
    MMDL_SetAcmd( mmdl, AC_DIR_D );
  }
} 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 落下登場 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_AppearPlayer_Fall( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  MOVE_WORK*     work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( MOVE_WORK ) );

  // TCBワーク初期化
  InitWork( work, fieldmap, frame, dist );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_AppearPlayer_Fall, work, 0 );

  // 主人公の向きを設定
  {
    FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
    MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
    MMDL_SetAcmd( mmdl, AC_DIR_D );
  }
} 


//========================================================================================== 
/**
 * @brief 非公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスク・ワークを初期化する
 *
 * @param work      初期化するワーク
 * @param fieldmap  動作対象のフィールドマップ
 * @param end_frame 最終フレーム数
 * @param move_dist 移動距離
 */
//------------------------------------------------------------------------------------------
void InitWork( MOVE_WORK* work, FIELDMAP_WORK* fieldmap, u16 end_frame, float move_dist )
{
  work->pFieldmap = fieldmap;
  work->frame     = 0;
  work->endFrame  = end_frame;
  work->moveDist  = move_dist;
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを破棄する
 *
 * @param tcb 破棄するタスク
 */
//------------------------------------------------------------------------------------------
void DeleteTask( GFL_TCB* tcb )
{
  // TCBワークを破棄
  GFL_HEAP_FreeMemory( GFL_TCB_GetWork( tcb ) );

  // タスクを破棄
  GFL_TCB_DeleteTask( tcb );
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の上昇 )
 */
//------------------------------------------------------------------------------------------
void TCB_FUNC_DisappearPlayer_LinearUp( GFL_TCB* tcb, void* work )
{
  MOVE_WORK*   tcbwork = work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32       offset;

  // 自機の描画オフセットを更新
  CalcDrawOffset_LinearUp( &offset, tcbwork->frame, tcbwork->endFrame, tcbwork->moveDist );
  MMDL_SetVectorDrawOffsetPos( mmdl, &offset );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->endFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の降下 )
 */
//------------------------------------------------------------------------------------------
void TCB_FUNC_AppearPlayer_LinearDown( GFL_TCB* tcb, void* work )
{
  MOVE_WORK*   tcbwork = work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32       offset;

  // 自機の描画オフセットを更新
  CalcDrawOffset_LinearDown( &offset, tcbwork->frame, tcbwork->endFrame, tcbwork->moveDist );
  MMDL_SetVectorDrawOffsetPos( mmdl, &offset );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->endFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 落下登場 )
 */
//------------------------------------------------------------------------------------------
void TCB_FUNC_AppearPlayer_Fall( GFL_TCB* tcb, void* work )
{
  MOVE_WORK*   tcbwork = work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32       offset;

  // 自機の描画オフセットを更新
  CalcDrawOffset_Fall( &offset, tcbwork->frame, tcbwork->endFrame, tcbwork->moveDist );
  MMDL_SetVectorDrawOffsetPos( mmdl, &offset );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->endFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief y座標の描画オフセットを算出する( 上昇 )
 *
 * @param p_vec     計算結果の格納先
 * @param frame     現在のフレーム数
 * @param max_frame 最大フレーム数
 * @param max_y     最大オフセット
 */
//------------------------------------------------------------------------------------------
void CalcDrawOffset_LinearUp( VecFx32* p_vec, float frame, float max_frame, float max_y )
{
  // 線形補完
  float x = frame;
  float a = max_y / max_frame;
  float y = a * x;

  p_vec->x = 0;
  p_vec->y = FX_F32_TO_FX32( y );
  p_vec->z = 0;
}

//------------------------------------------------------------------------------------------
/**
 * @brief y座標の描画オフセットを算出する( 降下 ) 
 *
 * @param p_vec     計算結果の格納先
 * @param frame     現在のフレーム数
 * @param max_frame 最大フレーム数
 * @param max_y     最大オフセット
 */
//------------------------------------------------------------------------------------------
void CalcDrawOffset_LinearDown( VecFx32* p_vec, float frame, float max_frame, float max_y )
{
  // 線形補完
  float x = frame;
  float a = max_y / max_frame;
  float y = max_y - (a * x);

  p_vec->x = 0;
  p_vec->y = FX_F32_TO_FX32( y );
  p_vec->z = 0;
} 

//------------------------------------------------------------------------------------------
/**
 * @brief y座標の描画オフセットを算出する( 落下 ) 
 *
 * @param p_vec     計算結果の格納先
 * @param frame     現在のフレーム数
 * @param max_frame 最大フレーム数
 * @param max_y     最大オフセット
 */
//------------------------------------------------------------------------------------------
void CalcDrawOffset_Fall( VecFx32* p_vec, float frame, float max_frame, float max_y )
{
  // y = ax^2
  float x = frame;
  float a = max_y / (max_frame * max_frame);
  float y = max_y - (a * x * x);

  p_vec->x = 0;
  p_vec->y = FX_F32_TO_FX32( y );
  p_vec->z = 0;
} 
