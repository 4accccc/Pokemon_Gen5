#include <gflib.h>
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "fieldmap_tcb.h"
#include "fieldmap.h"
#include "field_player.h"


//==============================================================================
/**
 * @brief TCBワーク
 */
//==============================================================================
typedef struct
{
  FIELDMAP_WORK* fieldmap;  // 動作対象フィールドマップ
  int               frame;  // 現在のフレーム数
  int            endFrame;  // 最終フレーム数
  VecFx32        startPos;  // 移動開始位置
  VecFx32          endPos;  // 最終位置
}
TCB_WORK;


//==============================================================================
/**
 * @brief プロトタイプ宣言
 */
//==============================================================================
static void TCB_FUNC_MovePlayer( GFL_TCB* tcb, void* work );
static void DeleteTask( GFL_TCB* tcb, TCB_WORK* work );


//------------------------------------------------------------------------------
/**
 * @brief プレイヤーの移動タスクを追加する
 *
 * @param fieldmap 動作対象フィールドマップ
 * @param frame    動作フレーム数
 * @param pos      移動先座標
 */
//------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_MovePlayer(
    FIELDMAP_WORK* fieldmap, int frame, VecFx32* pos )
{
  HEAPID       heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*   tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
  TCB_WORK*       work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof(TCB_WORK) );

  // TCBワーク初期化
  work->frame    = 0;
  work->endFrame = frame;
  work->fieldmap = fieldmap;
  FIELD_PLAYER_GetPos( player, &work->startPos );
  VEC_Set( &work->endPos, pos->x, pos->y, pos->z );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_MovePlayer, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief TCB動作関数
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_MovePlayer( GFL_TCB* tcb, void* work )
{
  TCB_WORK*    tcbwork = (TCB_WORK*)work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( tcbwork->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player );

  // 自機の座標を更新
  {
    VecFx32 v0, v1, v2;
    fx32 w1 = FX_F32_TO_FX32( (float)tcbwork->frame / (float)tcbwork->endFrame );
    fx32 w0 = FX32_ONE - w1;
    GFL_CALC3D_VEC_MulScalar( &tcbwork->startPos, w0, &v0 );
    GFL_CALC3D_VEC_MulScalar( &tcbwork->endPos,   w1, &v1 );
    VEC_Add( &v0, &v1, &v2 );
    FIELD_PLAYER_SetPos( player, &v2 );   // 移動
    MMDL_UpdateCurrentHeight( mmdl );   // 接地
  }

  // 一定時間動作したら自殺する
  if( tcbwork->endFrame < ++tcbwork->frame )
  {
    DeleteTask( tcb, tcbwork );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを破棄する
 *
 * @param tcb 破棄するタスク
 */
//------------------------------------------------------------------------------------------
static void DeleteTask( GFL_TCB* tcb, TCB_WORK* work )
{ 
  // TCBワークを破棄
  GFL_HEAP_FreeMemory( GFL_TCB_GetWork( tcb ) );

  // タスクを破棄
  GFL_TCB_DeleteTask( tcb );
}
