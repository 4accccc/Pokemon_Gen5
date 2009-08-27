////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フィールドマップTCB( 自機の回転 )
 * @file   fieldmap_tcb_rot_player.c
 * @author obata
 * @date   2009.08.25
 *
 */
////////////////////////////////////////////////////////////////////////
#include <math.h>     // sqrt
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "fieldmap_tcb.h"
#include "fieldmap.h"
#include "field_player.h"


//==========================================================================================
/**
 * @brief TCBワーク
 */
//==========================================================================================
typedef struct
{
  u16  frame;     // 動作フレーム数
  u16  maxFrame;  // 最大フレーム数
  u8   rotateNum; // 回転回数
  FIELDMAP_WORK* pFieldmap; // 動作対象のフィールドマップ
}
TCB_WORK_ROT_HERO;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
static void DeleteTask( GFL_TCB* tcb );

static void TCB_FUNC_RotPlayer_Accel( GFL_TCB* tcb, void* work );
static void TCB_FUNC_RotPlayer_Decel( GFL_TCB* tcb, void* work );

static u16 CalcRotateDir_Accel( float frame, float max_frame, float rotate_num );
static u16 CalcRotateDir_Decel( float frame, float max_frame, float rotate_num );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 自機回転タスクを追加する
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_ROT_PLAYER_AddTask_SpeedUp( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*      tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TCB_WORK_ROT_HERO* work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TCB_WORK_ROT_HERO ) );
  GFL_TCB*            tcb;

  // TCBワーク初期化
  work->frame     = 0;
  work->maxFrame  = frame;
  work->rotateNum = rot_num;
  work->pFieldmap = fieldmap;

  // TCBを追加
  tcb = GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotPlayer_Accel, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機回転タスクを追加する
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_ROT_PLAYER_AddTask_SlowDown( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*      tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TCB_WORK_ROT_HERO* work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TCB_WORK_ROT_HERO ) );
  GFL_TCB*            tcb;

  // TCBワーク初期化
  work->frame     = 0;
  work->maxFrame  = frame;
  work->rotateNum = rot_num;
  work->pFieldmap = fieldmap;

  // TCBを追加
  tcb = GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotPlayer_Decel, work, 0 );
}


//========================================================================================== 
/*
 * @brief 非公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを破棄する
 *
 * @param tcb 破棄するタスク
 */
//------------------------------------------------------------------------------------------
static void DeleteTask( GFL_TCB* tcb )
{
  // TCBワークを破棄
  GFL_HEAP_FreeMemory( GFL_TCB_GetWork( tcb ) );

  // タスクを破棄
  GFL_TCB_DeleteTask( tcb );
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の加速回転 )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_RotPlayer_Accel( GFL_TCB* tcb, void* work )
{
  TCB_WORK_ROT_HERO* tcbwork = work;
  FIELD_PLAYER*       player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*                 mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16                anm_cmd = CalcRotateDir_Accel( tcbwork->frame, tcbwork->maxFrame, tcbwork->rotateNum );

  // 動作モデルの向きを更新
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->maxFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の減速回転 )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_RotPlayer_Decel( GFL_TCB* tcb, void* work )
{
  TCB_WORK_ROT_HERO* tcbwork = work;
  FIELD_PLAYER*       player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*                 mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16                anm_cmd = CalcRotateDir_Decel( tcbwork->frame, tcbwork->maxFrame, tcbwork->rotateNum );

  // 動作モデルの向きを更新
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->maxFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
} 

//------------------------------------------------------------------------------------------
 /**
  * @brief 回転時の向きを算出する
  *
  * @param frame      現在のフレーム数
  * @param max_frame  最大フレーム数
  * @param rotate_num 全回転数
  *
  * @return 動作モデルのアニメーションコマンド( AC_DIR_D など )
  */
//------------------------------------------------------------------------------------------
static u16 CalcRotateDir_Accel( float frame, float max_frame, float rotate_num )
{
  u16 acmd_list[] =
  {
    AC_DIR_D,
    AC_DIR_R,
    AC_DIR_U,
    AC_DIR_L,
  };

  float x     = frame;
  float x_max = max_frame;
  float y_max = 4 * rotate_num;
  float a     = y_max / ( x_max * x_max );
  float y     = a * x * x;
  int anime   = (int)y % 4;

  return acmd_list[ anime ];
} 

//------------------------------------------------------------------------------------------
 /**
  * @brief 回転時の向きを算出する
  *
  * @param frame      現在のフレーム数
  * @param max_frame  最大フレーム数
  * @param rotate_num 全回転数
  *
  * @return 動作モデルのアニメーションコマンド( AC_DIR_D など )
  */
//------------------------------------------------------------------------------------------
static u16 CalcRotateDir_Decel( float frame, float max_frame, float rotate_num )
{
  u16 acmd_list[] =
  {
    AC_DIR_D,
    AC_DIR_R,
    AC_DIR_U,
    AC_DIR_L,
  };

  float x     = frame;
  float x_max = max_frame;
  float y_max = 4 * rotate_num;
  float a     = (y_max * y_max) / x_max;
  float y     = sqrt(a * x);
  int anime   = (int)y % 4;

  return acmd_list[ anime ];
}
