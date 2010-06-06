///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドカメラ アニメーション
 * @file   field_camera_anime.c
 * @author obata
 * @date   2010.04.10
 */
///////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "field_camera_anime.h"

#include "fieldmap.h"
#include "field_task.h"  
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"
#include "field_task_target_pos.h"


//=============================================================================
// ■カメラ操作ワーク
//=============================================================================
struct _FCAM_ANIME_WORK {

  FIELDMAP_WORK*         fieldmap;
  FIELD_CAMERA*          camera;
  const FCAM_ANIME_DATA* pAnimeData;

  // カメラ復帰用データ
  BOOL              recoveryValidFlag;     // 復帰データが有効かどうか
  FIELD_CAMERA_MODE initialCameraMode;     // カメラモード
  BOOL              initialCameraAreaFlag; // カメラエリアの動作フラグ
  const VecFx32*    initialWatchTarget;
  BOOL              recoveryWatchTargetFlag;

};


//=============================================================================
// ■private functions
//=============================================================================
// 生成・破棄・初期化
static FCAM_ANIME_WORK* CreateWork( HEAPID heapID ); // カメラ操作ワークを生成する
static void DeleteWork( FCAM_ANIME_WORK* work ); // カメラ操作ワークを破棄する
static void InitWork( FCAM_ANIME_WORK* work, FIELDMAP_WORK* fieldmap ); // カメラ操作ワークを初期化する
// カメラの準備・復帰
static void SetupCamera( FCAM_ANIME_WORK* work ); // カメラの設定を変更する
static void SetupCameraMode( FCAM_ANIME_WORK* work ); // カメラモードの設定を変更する
static void RecoverCamera( const FCAM_ANIME_WORK* work ); // カメラの設定を復帰する
static void AdjustCameraAngle( FIELD_CAMERA* camera ); // カメラアングルを再計算する
static void SetCurrentCameraTargetPos( FIELD_CAMERA* camera ); // ターゲット座標を現在の実効値に設定する
static void GetCurrentCameraParam( const FIELD_CAMERA* camera, FCAM_PARAM* dest ); // カメラの現在のパラメータを取得する
// カメラアニメーション
static void SetCameraParam( const FCAM_ANIME_WORK* work ); // カメラを即時反映する
static void StartCameraAnime( const FCAM_ANIME_WORK* work ); // カメラのアニメーションを開始する
// アクセッサ
static FIELDMAP_WORK* GetFieldmap( const FCAM_ANIME_WORK* work ); // フィールドマップを取得する
static FIELD_CAMERA* GetCamera( const FCAM_ANIME_WORK* work ); // カメラを取得する
static const FCAM_ANIME_DATA* GetAnimeData( const FCAM_ANIME_WORK* work ); // アニメーションデータを取得する



//=============================================================================
// ■public functions
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief 現在のカメラパラメータを取得する
 *
 * @param camera
 * @param dest   取得したパラメータの格納先
 */
//-----------------------------------------------------------------------------
void FCAM_PARAM_GetCurrentParam( const FIELD_CAMERA* camera, FCAM_PARAM* dest )
{
  GetCurrentCameraParam( camera, dest );
}


//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを生成する
 *
 * @param fieldamp
 *
 * @return 生成したワーク
 */
//-----------------------------------------------------------------------------
FCAM_ANIME_WORK* FCAM_ANIME_CreateWork( FIELDMAP_WORK* fieldmap )
{
  FCAM_ANIME_WORK* work;

  work = CreateWork( FIELDMAP_GetHeapID(fieldmap) ); // 生成
  InitWork( work, fieldmap ); // 初期化
  return work;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void FCAM_ANIME_DeleteWork( FCAM_ANIME_WORK* work )
{
  DeleteWork( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションデータを設定する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void FCAM_ANIME_SetAnimeData( FCAM_ANIME_WORK* work, const FCAM_ANIME_DATA* data )
{
  work->pAnimeData = data;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラをセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void FCAM_ANIME_SetupCamera( FCAM_ANIME_WORK* work )
{ 
  SetupCameraMode( work );
  AdjustCameraAngle( work->camera );
  SetCurrentCameraTargetPos( work->camera ); // ターゲット座標を初期化
  SetupCamera( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラの状態を開始パラメータの設定にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void FCAM_ANIME_SetCameraStartParam( const FCAM_ANIME_WORK* work )
{
  FIELD_CAMERA* camera;
  const FCAM_ANIME_DATA* anime;
  const FCAM_PARAM* param;

  camera = GetCamera( work );
  anime = GetAnimeData( work );
  param = &(anime->startParam);

  FIELD_CAMERA_SetAnglePitch( camera, param->pitch );
  FIELD_CAMERA_SetAngleYaw( camera, param->yaw );
  FIELD_CAMERA_SetAngleLen( camera, param->length );
  FIELD_CAMERA_SetTargetPos( camera, &param->targetPos );
  FIELD_CAMERA_SetTargetOffset( camera, &param->targetOffset );
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void FCAM_ANIME_StartAnime( const FCAM_ANIME_WORK* work )
{
  const FCAM_ANIME_DATA* anime;

  anime = GetAnimeData( work );

  // 即時反映
  if( anime->frame == 0 ) {
    SetCameraParam( work );
  }
  // タスクを登録
  else {
    StartCameraAnime( work );
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションの終了をチェックする
 *
 * @param work
 *
 * @param アニメーションが終了している場合 TRUE
 *        そうでなければ FALSE
 */
//-----------------------------------------------------------------------------
BOOL FCAM_ANIME_IsAnimeFinished( const FCAM_ANIME_WORK* work )
{
  FIELD_TASK_MAN* taskMan;

  taskMan = FIELDMAP_GetTaskManager( work->fieldmap );

  if( FIELD_TASK_MAN_IsAllTaskEnd( taskMan ) ) { return TRUE; }
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラを復帰する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void FCAM_ANIME_RecoverCamera( const FCAM_ANIME_WORK* work )
{
  if( work->recoveryValidFlag ) {
    RecoverCamera( work );
  }
} 




//=============================================================================
// ■private functions
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを生成する
 *
 * @param heapID
 */
//-----------------------------------------------------------------------------
static FCAM_ANIME_WORK* CreateWork( HEAPID heapID )
{
  FCAM_ANIME_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(FCAM_ANIME_WORK) );

  return work;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DeleteWork( FCAM_ANIME_WORK* work )
{
  GFL_HEAP_FreeMemory( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void InitWork( FCAM_ANIME_WORK* work, FIELDMAP_WORK* fieldmap )
{
  // ゼロクリア
  GFL_STD_MemClear( work, sizeof(FCAM_ANIME_WORK) );

  // 初期化
  work->fieldmap = fieldmap;
  work->camera = FIELDMAP_GetFieldCamera( fieldmap );
  work->pAnimeData = NULL;
  work->recoveryValidFlag = FALSE;
  work->initialCameraAreaFlag = FALSE;
  work->initialWatchTarget = NULL;
  work->recoveryWatchTargetFlag = FALSE;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作の準備をする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetupCamera( FCAM_ANIME_WORK* work )
{
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  FIELD_CAMERA*  camera   = work->camera;
  const FCAM_ANIME_DATA* anime = GetAnimeData( work );

  // レールシステムのカメラ制御を停止
  if( FIELDMAP_GetBaseSystemType( fieldmap ) == FLDMAP_BASESYS_RAIL ) {
    FLDNOGRID_MAPPER* NGMapper = FIELDMAP_GetFldNoGridMapper( fieldmap );
    FLDNOGRID_MAPPER_SetRailCameraActive( NGMapper, FALSE );
  }

  // ターゲットのバインドをOFF
  if( anime->targetBindOffFlag ) {
    if( FIELD_CAMERA_GetWatchTarget( work->camera ) != NULL ) {
      work->recoveryWatchTargetFlag = TRUE;
      work->initialWatchTarget = FIELD_CAMERA_GetWatchTarget( camera );
      FIELD_CAMERA_FreeTarget( camera );
    }
  }

  // カメラエリアを無効化
  if( anime->cameraAreaOffFlag ) {
    work->initialCameraAreaFlag = FIELD_CAMERA_GetCameraAreaActive( camera );
    FIELD_CAMERA_SetCameraAreaActive( camera, FALSE );
  }

  // カメラシーンを停止
  {
    FLD_SCENEAREA* sceneArea;
    sceneArea = FIELDMAP_GetFldSceneArea( fieldmap );
    FLD_SCENEAREA_SetActiveFlag( sceneArea, FALSE );
  }

  // フラグセット
  work->recoveryValidFlag = TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラモードをセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetupCameraMode( FCAM_ANIME_WORK* work )
{
  FIELD_CAMERA* camera = work->camera;

  // カメラモードを変更
  work->initialCameraMode = FIELD_CAMERA_GetMode( camera ); 
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラの復帰をする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void RecoverCamera( const FCAM_ANIME_WORK* work )
{
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  FIELD_CAMERA*  camera   = work->camera;
  const FCAM_ANIME_DATA* anime = GetAnimeData( work );

  GF_ASSERT( work->recoveryValidFlag );

  // カメラシーンの再始動
  {
    FLD_SCENEAREA* sceneArea;
    sceneArea = FIELDMAP_GetFldSceneArea( fieldmap );
    FLD_SCENEAREA_SetActiveFlag( sceneArea, TRUE );
  }

  // カメラエリアを復帰
  if( anime->cameraAreaOffFlag ) {
    FIELD_CAMERA_SetCameraAreaActive( camera, work->initialCameraAreaFlag );
  }

  // ターゲットを元に戻す
  if( work->recoveryWatchTargetFlag ) {
    FIELD_CAMERA_BindTarget( camera, work->initialWatchTarget );
    //FIELD_CAMERA_BindDefaultTarget( camera );
  }

  // カメラモードを復帰
  FIELD_CAMERA_ChangeMode( camera, work->initialCameraMode );

  // レールシステムのカメラ制御を復帰
  if( FIELDMAP_GetBaseSystemType( fieldmap ) == FLDMAP_BASESYS_RAIL ) {
    FLDNOGRID_MAPPER* NGMapper = FIELDMAP_GetFldNoGridMapper( fieldmap );
    FLDNOGRID_MAPPER_SetRailCameraActive( NGMapper, TRUE );
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラアングルを再計算する
 *
 * @param camera
 */
//-----------------------------------------------------------------------------
static void AdjustCameraAngle( FIELD_CAMERA* camera )
{
  u16 pitch, yaw;
  fx32 length;
  VecFx32 targetPos, cameraPos;
  VecFx32 toCameraVec;

  // ターゲット → カメラ ベクトルを算出
  FIELD_CAMERA_GetCameraAreaAfterCameraPos( camera, &cameraPos );
  FIELD_CAMERA_GetCameraAreaAfterTargetPos( camera, &targetPos );
  VEC_Subtract( &cameraPos, &targetPos, &toCameraVec );

  // アングルを計算
  FIELD_CAMERA_CalcVecAngle( &toCameraVec, &pitch, &yaw, &length );

  // 求めたアングルをセット
  FIELD_CAMERA_SetAngleLen( camera, length );
  FIELD_CAMERA_SetAnglePitch( camera, pitch );
  FIELD_CAMERA_SetAngleYaw( camera, yaw );
}

//-----------------------------------------------------------------------------
/**
 * @brief ターゲット座標を現在の実効値に設定する
 *
 * @param camera
 */
//-----------------------------------------------------------------------------
static void SetCurrentCameraTargetPos( FIELD_CAMERA* camera )
{
  VecFx32 pos;

  // カメラエリアを考慮したターゲット座標を取得
  FIELD_CAMERA_GetCameraAreaAfterTargetPos( camera, &pos );

  // ターゲット座標に反映
  FIELD_CAMERA_SetTargetPos( camera, &pos );
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラの現在のパラメータを取得する
 *
 * @param camera
 * @param dest   取得したパラメータの格納先
 */
//-----------------------------------------------------------------------------
static void GetCurrentCameraParam( const FIELD_CAMERA* camera, FCAM_PARAM* dest )
{
  VecFx32 targetPos, cameraPos;
  VecFx32 toCameraVec;

  // アングルを計算 ( カメラエリアを考慮 )
  FIELD_CAMERA_GetCameraAreaAfterCameraPos( camera, &cameraPos );
  FIELD_CAMERA_GetCameraAreaAfterTargetPos( camera, &targetPos );
  VEC_Subtract( &cameraPos, &targetPos, &toCameraVec ); 
  FIELD_CAMERA_CalcVecAngle( 
      &toCameraVec, &(dest->pitch), &(dest->yaw), &(dest->length) );

  // ターゲット座標・ターゲットオフセットを取得 ( カメラエリアを考慮 )
  FIELD_CAMERA_GetCameraAreaAfterTargetPos( camera, &(dest->targetPos) );
  FIELD_CAMERA_GetTargetOffset( camera, &(dest->targetOffset) );
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラを即時反映する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetCameraParam( const FCAM_ANIME_WORK* work )
{
  FIELD_CAMERA*     camera   = work->camera;
  const FCAM_ANIME_DATA* anime    = GetAnimeData( work );
  const FCAM_PARAM* endParam = &(anime->endParam);

  GF_ASSERT( anime->frame == 0 ); // 即時反映データでなくてはダメ

  // カメラに反映させる
  FIELD_CAMERA_SetAnglePitch( camera, endParam->pitch );
  FIELD_CAMERA_SetAngleYaw( camera, endParam->yaw );
  FIELD_CAMERA_SetAngleLen( camera, endParam->length );
  FIELD_CAMERA_SetTargetPos( camera, &(endParam->targetPos) );
  FIELD_CAMERA_SetTargetOffset( camera, &(endParam->targetOffset) );
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラのアニメーションを開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void StartCameraAnime( const FCAM_ANIME_WORK* work )
{
  FIELDMAP_WORK*    fieldmap   = work->fieldmap;
  FIELD_CAMERA*     camera     = work->camera;
  const FCAM_ANIME_DATA* anime      = GetAnimeData( work );
  const FCAM_PARAM* startParam = &(anime->startParam);
  const FCAM_PARAM* endParam   = &(anime->endParam);

  GF_ASSERT( 0 < anime->frame ); // アニメ用データでなくてはダメ

  // タスクを登録
  {
    FIELD_TASK* pitch;
    FIELD_TASK* yaw;
    FIELD_TASK* length;
    FIELD_TASK* TPos;
    FIELD_TASK* TOfs;

    // タスクを生成
    {
      u32 frame  = anime->frame;
      pitch  = FIELD_TASK_CameraRot_Pitch( fieldmap, frame, endParam->pitch );
      yaw    = FIELD_TASK_CameraRot_Yaw( fieldmap, frame, endParam->yaw );
      length = FIELD_TASK_CameraLinearZoom( fieldmap, frame, endParam->length ); 
      TPos   = FIELD_TASK_CameraTargetMove( fieldmap, frame, &(startParam->targetPos), &(endParam->targetPos) );
      TOfs   = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &(endParam->targetOffset) );
    }

    // タスクを登録
    {
      FIELD_TASK_MAN* taskMan = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, pitch, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, yaw, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, length, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, TPos, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, TOfs, NULL );
    }
  }
}

//----------------------------------------------------------------------------- 
/**
 * @brief フィールドマップを取得する
 *
 * @param work
 *
 * @return FIELDMAP_WORK*
 */
//----------------------------------------------------------------------------- 
static FIELDMAP_WORK* GetFieldmap( const FCAM_ANIME_WORK* work )
{
  return work->fieldmap;
}

//----------------------------------------------------------------------------- 
/**
 * @brief カメラを取得する
 *
 * @param work
 *
 * @return FIELD_CAMERA* 
 */
//----------------------------------------------------------------------------- 
static FIELD_CAMERA* GetCamera( const FCAM_ANIME_WORK* work )
{
  return work->camera;
}

//----------------------------------------------------------------------------- 
/**
 * @brief アニメーションデータを取得する
 *
 * @param work
 *
 * @return アニメーションデータ
 */
//----------------------------------------------------------------------------- 
static const FCAM_ANIME_DATA* GetAnimeData( const FCAM_ANIME_WORK* work )
{
  GF_ASSERT( work->pAnimeData );

  return work->pAnimeData;
}
