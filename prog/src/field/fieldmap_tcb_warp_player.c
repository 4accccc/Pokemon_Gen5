////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フィールドマップTCB( 自機のワープ移動 )
 * @file   fieldmap_tcb_wap_player.c
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
  u16   frame;    // 動作フレーム数
  u16   maxFrame; // 最大フレーム数
  float maxDist;  // 最大移動距離
  FIELDMAP_WORK* pFieldmap; // 動作対象のフィールドマップ
}
TCB_WORK_WARP;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
void DeleteTask( GFL_TCB* tcb );

void TCB_FUNC_WarpPlayer_DisappearUp( GFL_TCB* tcb, void* work );
void TCB_FUNC_WarpPlayer_AppearDown( GFL_TCB* tcb, void* work );
void TCB_FUNC_WarpPlayer_FallIn( GFL_TCB* tcb, void* work );

void CalcDrawOffset_LinearUp( VecFx32* p_vec, float frame, float max_frame, float max_y );
void CalcDrawOffset_LinearDown( VecFx32* p_vec, float frame, float max_frame, float max_y );
void CalcDrawOffset_FallIn( VecFx32* p_vec, float frame, float max_frame, float max_y );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

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
 * @brief 自機ワープタスクを追加する( 上昇による退場 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_WARP_PLAYER_AddTask_DisappearUp( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TCB_WORK_WARP* work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TCB_WORK_WARP ) );
  GFL_TCB*        tcb;

  // TCBワーク初期化
  work->frame     = 0;
  work->maxFrame  = frame;
  work->maxDist   = dist;
  work->pFieldmap = fieldmap;

  // TCBを追加
  tcb = GFL_TCB_AddTask( tcbsys, TCB_FUNC_WarpPlayer_DisappearUp, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機ワープタスクを追加する( 降下による登場 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_WARP_PLAYER_AddTask_AppearDown( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TCB_WORK_WARP* work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TCB_WORK_WARP ) );
  GFL_TCB*        tcb;

  // TCBワーク初期化
  work->frame     = 0;
  work->maxFrame  = frame;
  work->maxDist   = dist;
  work->pFieldmap = fieldmap;

  // TCBを追加
  tcb = GFL_TCB_AddTask( tcbsys, TCB_FUNC_WarpPlayer_AppearDown, work, 0 );
} 

//------------------------------------------------------------------------------------------
/**
 * @brief 自機ワープタスクを追加する( 落下による登場 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_WARP_PLAYER_AddTask_FallIn( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TCB_WORK_WARP* work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TCB_WORK_WARP ) );

  // TCBワーク初期化
  work->frame     = 0;
  work->maxFrame  = frame;
  work->maxDist   = dist;
  work->pFieldmap = fieldmap;

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_WarpPlayer_FallIn, work, 0 );

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
 * @breif TCB実行関数( 自機の上昇 )
 */
//------------------------------------------------------------------------------------------
void TCB_FUNC_WarpPlayer_DisappearUp( GFL_TCB* tcb, void* work )
{
  TCB_WORK_WARP* tcbwork = work;
  FIELD_PLAYER*   player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*             mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32         offset;

  // 自機の描画オフセットを更新
  CalcDrawOffset_LinearUp( &offset, tcbwork->frame, tcbwork->maxFrame, tcbwork->maxDist );
  MMDL_SetVectorDrawOffsetPos( mmdl, &offset );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->maxFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の降下 )
 */
//------------------------------------------------------------------------------------------
void TCB_FUNC_WarpPlayer_AppearDown( GFL_TCB* tcb, void* work )
{
  TCB_WORK_WARP* tcbwork = work;
  FIELD_PLAYER*   player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*             mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32         offset;

  // 自機の描画オフセットを更新
  CalcDrawOffset_LinearDown( &offset, tcbwork->frame, tcbwork->maxFrame, tcbwork->maxDist );
  MMDL_SetVectorDrawOffsetPos( mmdl, &offset );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->maxFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 落下登場 )
 */
//------------------------------------------------------------------------------------------
void TCB_FUNC_WarpPlayer_FallIn( GFL_TCB* tcb, void* work )
{
  TCB_WORK_WARP* tcbwork = work;
  FIELD_PLAYER*   player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*             mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32         offset;

  // 自機の描画オフセットを更新
  CalcDrawOffset_FallIn( &offset, tcbwork->frame, tcbwork->maxFrame, tcbwork->maxDist );
  MMDL_SetVectorDrawOffsetPos( mmdl, &offset );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->maxFrame < tcbwork->frame )
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
void CalcDrawOffset_FallIn( VecFx32* p_vec, float frame, float max_frame, float max_y )
{
  float x = frame;
  float a = max_y / (max_frame * max_frame);
  float y = max_y - (a * x * x);

  p_vec->x = 0;
  p_vec->y = FX_F32_TO_FX32( y );
  p_vec->z = 0;
} 
