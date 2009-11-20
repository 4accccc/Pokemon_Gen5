////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(自機の回転処理)
 * @file   field_task_player_rot.c
 * @author obata
 * @date   2009.11.18
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <math.h>     // sqrt
#include "fieldmap.h"
#include "field_player.h"
#include "field_task.h"
#include "field_task_player_rot.h"


//==========================================================================================
// ■タスクワーク
//==========================================================================================
typedef struct
{
  FIELDMAP_WORK* fieldmap; // 動作対象のフィールドマップ
  u16            frame;     // 動作フレーム数
  u16            endFrame;  // 最終フレーム数
  u8             rotateNum; // 回転回数

} TASK_WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static u16 CalcPlayerDir( float frame, float max_frame, float rotate_num );
static u16 CalcPlayerDir_SpeedUp( float frame, float max_frame, float rotate_num );
static u16 CalcPlayerDir_SpeedDown( float frame, float max_frame, float rotate_num );
static FIELD_TASK_RETVAL RotatePlayer( void *wk );
static FIELD_TASK_RETVAL RotatePlayer_SpeedUp( void *wk );
static FIELD_TASK_RETVAL RotatePlayer_SpeedDown( void *wk );


//------------------------------------------------------------------------------------------
/**
 * @brief 自機の等速回転タスクを作成する
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_PlayerRotate( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), RotatePlayer );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap  = fieldmap;
  work->frame     = 0;
  work->endFrame  = frame;
  work->rotateNum = rot_num;

  return task;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の加速回転タスクを作成する
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_PlayerRotate_SpeedUp( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), RotatePlayer_SpeedUp );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap  = fieldmap;
  work->frame     = 0;
  work->endFrame  = frame;
  work->rotateNum = rot_num;

  return task;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の減速回転タスクを作成する
 *
 * @param fieldmap タスクを追加するフィールドマップ
 * @param frame    動作フレーム数
 * @param rot_num  回転回数
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_PlayerRotate_SpeedDown( FIELDMAP_WORK* fieldmap, int frame, int rot_num )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), RotatePlayer_SpeedDown );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap  = fieldmap;
  work->frame     = 0;
  work->endFrame  = frame;
  work->rotateNum = rot_num;

  return task;
}


//========================================================================================== 
// ■非公開関数
//========================================================================================== 

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


//========================================================================================== 
// ■タスク処理関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @breif 自機の等速回転処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL RotatePlayer( void *wk )
{
  TASK_WORK*      work = wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16          anm_cmd = CalcPlayerDir( work->frame, work->endFrame, work->rotateNum );

  // 動作モデルの向きを更新
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  work->frame++;
  if( work->endFrame < work->frame )
  {
    return FIELD_TASK_RETVAL_FINISH;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @breif 自機の加速回転処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL RotatePlayer_SpeedUp( void *wk )
{
  TASK_WORK*      work = wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16          anm_cmd = CalcPlayerDir_SpeedUp( work->frame, work->endFrame, work->rotateNum );

  // 動作モデルの向きを更新
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  work->frame++;
  if( work->endFrame < work->frame )
  {
    return FIELD_TASK_RETVAL_FINISH;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @breif 自機の減速回転処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL RotatePlayer_SpeedDown( void *wk )
{
  TASK_WORK*      work = wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  u16          anm_cmd = CalcPlayerDir_SpeedDown( work->frame, work->endFrame, work->rotateNum );

  // 動作モデルの向きを更新
  MMDL_SetAcmd( mmdl, anm_cmd );

  // 指定フレームが経過したら, タスク終了
  work->frame++;
  if( work->endFrame < work->frame )
  {
    return FIELD_TASK_RETVAL_FINISH;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
} 
