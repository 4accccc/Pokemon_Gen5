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
  FIELDMAP_WORK* pFieldmap; // 動作対象のフィールドマップ
  u16            frame;     // 動作フレーム数
  u16            endFrame;  // 最終フレーム数
  u8             rotateNum; // 回転回数
}
TASK_WORK;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
static void InitWork( TASK_WORK* work, FIELDMAP_WORK* fieldmap, u16 end_frame, u8 rot_num );
static void DeleteTask( GFL_TCB* tcb );

static void TCB_FUNC_RotPlayer( GFL_TCB* tcb, void* work );
static void TCB_FUNC_RotPlayer_SpeedUp( GFL_TCB* tcb, void* work );
static void TCB_FUNC_RotPlayer_SpeedDown( GFL_TCB* tcb, void* work );

static u16 CalcPlayerDir( float frame, float max_frame, float rotate_num );
static u16 CalcPlayerDir_SpeedUp( float frame, float max_frame, float rotate_num );
static u16 CalcPlayerDir_SpeedDown( float frame, float max_frame, float rotate_num );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 自機の等速回転 )
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_RotatePlayer( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  HEAPID     heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS* tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TASK_WORK*    work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TASK_WORK ) );

  // TCBワーク初期化
  InitWork( work, fieldmap, frame, rot_num );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotPlayer, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 自機の加速回転 )
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_RotatePlayer_SpeedUp( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  HEAPID     heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS* tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TASK_WORK*    work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TASK_WORK ) );

  // TCBワーク初期化
  InitWork( work, fieldmap, frame, rot_num );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotPlayer_SpeedUp, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 自機の減速回転 )
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_RotatePlayer_SpeedDown( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  GFL_TCBSYS*      tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  TASK_WORK* work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( TASK_WORK ) );

  // TCBワーク初期化
  work->frame     = 0;
  work->endFrame  = frame;
  work->rotateNum = rot_num;
  work->pFieldmap = fieldmap;

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotPlayer_SpeedDown, work, 0 );
}


//========================================================================================== 
/*
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
 * @param rot_num   回転回数
 */
//------------------------------------------------------------------------------------------
static void InitWork( TASK_WORK* work, FIELDMAP_WORK* fieldmap, u16 end_frame, u8 rot_num )
{
  work->pFieldmap = fieldmap;
  work->frame     = 0;
  work->endFrame  = end_frame;
  work->rotateNum = rot_num;
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを破棄する
 *
 * @param tcb 破棄するタスク
 */
//------------------------------------------------------------------------------------------
static void DeleteTask( GFL_TCB* tcb )
{
  // タスク・ワークを破棄
  GFL_HEAP_FreeMemory( GFL_TCB_GetWork( tcb ) );

  // タスクを破棄
  GFL_TCB_DeleteTask( tcb );
}

//------------------------------------------------------------------------------------------
/**
 * @breif タスク実行関数( 自機の等速回転 )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_RotPlayer( GFL_TCB* tcb, void* work )
{
  TASK_WORK* tcbwork = work;
  FIELD_PLAYER*       player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*                 mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16                anm_cmd = CalcPlayerDir( tcbwork->frame, tcbwork->endFrame, tcbwork->rotateNum );

  // 動作モデルの向きを更新
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->endFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の加速回転 )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_RotPlayer_SpeedUp( GFL_TCB* tcb, void* work )
{
  TASK_WORK*   tcbwork = work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16          anm_cmd;

  // 動作モデルの向きを更新
  anm_cmd = CalcPlayerDir_SpeedUp( tcbwork->frame, tcbwork->endFrame, tcbwork->rotateNum );
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->endFrame < tcbwork->frame )
  {
    DeleteTask( tcb );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif TCB実行関数( 自機の減速回転 )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_RotPlayer_SpeedDown( GFL_TCB* tcb, void* work )
{
  TASK_WORK*   tcbwork = work;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( tcbwork->pFieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16          anm_cmd;
  
  // 動作モデルの向きを更新
  anm_cmd = CalcPlayerDir_SpeedDown( tcbwork->frame, tcbwork->endFrame, tcbwork->rotateNum );
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  tcbwork->frame++;
  if( tcbwork->endFrame < tcbwork->frame )
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
static u16 CalcPlayerDir( float frame, float max_frame, float rotate_num )
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
  float a     = y_max / x_max;
  float y     = a * x;
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
static u16 CalcPlayerDir_SpeedUp( float frame, float max_frame, float rotate_num )
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
static u16 CalcPlayerDir_SpeedDown( float frame, float max_frame, float rotate_num )
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
