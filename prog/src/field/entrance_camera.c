///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  特殊出入り口のカメラ演出データ
 * @file   entrance_camera.c
 * @author obata
 * @date   2010.02.16
 */
///////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "entrance_camera.h"

#include "fieldmap.h"
#include "field_task.h"  
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"
#include "field_task_target_pos.h"

#include "field/field_const.h"     // for FIELD_CONST_xxxx
#include "field/eventdata_type.h"  // for EXIT_TYPE_xxxx
#include "arc/arc_def.h"           // for ARCID_xxxx
#include "../../resource/fldmapdata/entrance_camera/entrance_camera.naix"  // for NARC_xxxx



//=============================================================================
// ■定数
//=============================================================================
#define DEBUG_PRINT_ENABLE    // デバッグ出力スイッチ
#define DEBUG_PRINT_TARGET (3)// デバッグ出力先

#define ZERO_FRAME 
//--------------------------------------------
// 通常出入り口のカメラ動作 ( ゼロフレーム版 )
//--------------------------------------------
#ifdef ZERO_FRAME 
// 上ドアへの出入り
#define U_DOOR_FRAME  (10)
#define U_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 左ドアへの出入り
#define L_DOOR_FRAME  (0)
#define L_DOOR_PITCH  (0x1fa1)
#define L_DOOR_YAW    (0x4000)
#define L_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 右ドアへの出入り
#define R_DOOR_FRAME  (0)
#define R_DOOR_PITCH  (0x1fa1)
#define R_DOOR_YAW    (0xc051)
#define R_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
//-------------------------------------
// 通常出入り口のカメラ動作 ( 通常版 )
//-------------------------------------
#else 
// 上ドアへの出入り
#define U_DOOR_FRAME  (10)
#define U_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 左ドアへの出入り
#define L_DOOR_FRAME  (20)
#define L_DOOR_PITCH  (0x1fa1)
#define L_DOOR_YAW    (0x0e0f)
#define L_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 右ドアへの出入り
#define R_DOOR_FRAME  (20)
#define R_DOOR_PITCH  (0x1fa1)
#define R_DOOR_YAW    (0xf2cc)
#define R_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
#endif



//=============================================================================
// ■エントランス・カメラ動作データ読み込み用ワーク
//=============================================================================
typedef struct {

  u32 exitType;        // 出入り口タイプ
  u32 pitch;           // ピッチ
  u32 yaw;             // ヨー
  u32 length;          // 距離
  u32 manualTargetFlag;// ターゲット座標が有効かどうか
  u32 targetPosX;      // ターゲット座標 x
  u32 targetPosY;      // ターゲット座標 y
  u32 targetPosZ;      // ターゲット座標 z
  u32 targetOffsetX;   // ターゲットオフセット x
  u32 targetOffsetY;   // ターゲットオフセット y
  u32 targetOffsetZ;   // ターゲットオフセット z
  u32 frame;           // 動作フレーム数
  u32 validFlag_IN;    // 進入時に有効なデータかどうか
  u32 validFlag_OUT;   // 退出時に有効なデータかどうか

} ECAM_LOAD_DATA; 


//=============================================================================
// ■カメラパラメータ
//=============================================================================
typedef struct {

  u16     pitch;        // ピッチ
  u16     yaw;          // ヨー
  fx32    length;       // 距離
  VecFx32 targetPos;    // ターゲット座標
  VecFx32 targetOffset; // ターゲットオフセット

} CAMERA_PARAM;


//=============================================================================
// ■エントランス・カメラ動作データ
//=============================================================================
typedef struct {

  u32 frame; // 動作フレーム数

  CAMERA_PARAM startParam; // 開始パラメータ
  CAMERA_PARAM endParam;   // 最終パラメータ

  BOOL validFlag_IN;  // 進入時に有効なデータかどうか
  BOOL validFlag_OUT; // 退出時に有効なデータかどうか

} ANIME_DATA; 


//=============================================================================
// ■エントランス・カメラ操作ワーク 
//=============================================================================
struct _ECAM_WORK {

  FIELDMAP_WORK* fieldmap;
  FIELD_CAMERA*  camera;

  ECAM_PARAM param;     // 演出パラメータ
  ANIME_DATA animeData; // アニメーションデータ

  // カメラ復帰用データ
  BOOL              recoveryValidFlag; // 復帰データが有効かどうか
  FIELD_CAMERA_MODE initialCameraMode; // カメラモード
  BOOL              cameraAreaFlag;    // カメラエリアの動作フラグ

};



//=============================================================================
// ■private functions
//=============================================================================
// 生成・破棄・初期化
static ECAM_WORK* CreateWork( HEAPID heapID ); // ワークを生成する
static void DeleteWork( ECAM_WORK* work ); // ワークを破棄する
static void InitWork( ECAM_WORK* work, FIELDMAP_WORK* fieldmap ); // ワークを初期化する
// セットアップ
static void ECamSetup_IN( ECAM_WORK* work ); // 通常出入口へ入る演出をセットアップする
static void ECamSetup_OUT( ECAM_WORK* work ); // 通常出入口から出る演出をセットアップする
static void ECamSetup_SP_IN( ECAM_WORK* work ); // 特殊出入口へ入る演出をセットアップする
static void ECamSetup_SP_OUT( ECAM_WORK* work ); // 特殊出入口から出る演出をセットアップする
// アニメーションデータ
static void LoadSpData( ECAM_LOAD_DATA* dest, EXIT_TYPE exitType ); // 特殊出入り口のカメラ動作データを読み込む
// カメラの準備・復帰
static void SetupCamera( ECAM_WORK* work ); // カメラの設定を変更する
static void RecoverCamera( const ECAM_WORK* work ); // カメラの設定を復帰する
static void AdjustCameraAngle( FIELD_CAMERA* camera ); // カメラアングルを再計算する
static void SetCurrentCameraTargetPos( FIELD_CAMERA* camera ); // ターゲット座標を現在の実効値に設定する
static void SetupCameraTargetPos( FIELD_CAMERA* camera, const VecFx32* targetPos ); // ターゲット座標を変更し, カメラに反映させる
static void GetCurrentCameraParam( const FIELD_CAMERA* camera, CAMERA_PARAM* dest ); // カメラの現在のパラメータを取得する
// カメラアニメーション
static void SetCameraParam( const ECAM_WORK* work ); // カメラを即時反映する
static void StartCameraAnime( const ECAM_WORK* work ); // カメラのアニメーションを開始する
// 自機
static void GetOneStepAfterPos( const ECAM_WORK* work, VecFx32* dest ); // 一歩先の座標を取得する
static u16 GetPlayerDir( const ECAM_WORK* work ); // 自機の向きを取得する
// アクセッサ
static FIELDMAP_WORK* GetFieldmap( const ECAM_WORK* work ); // フィールドマップを取得する
static FIELD_CAMERA* GetCamera( const ECAM_WORK* work ); // カメラを取得する
static const ANIME_DATA* GetAnimeData( const ECAM_WORK* work ); // アニメーションデータを取得する
static const ECAM_PARAM* GetECamParam( const ECAM_WORK* work ); // 演出パラメータを取得する
static void SetECamParam( ECAM_WORK* work, const ECAM_PARAM* param ); // 演出パラメータを設定する
static EXIT_TYPE GetExitType( const ECAM_WORK* work ); // 出入り口タイプを取得する
static BOOL CheckSpExit( const ECAM_WORK* work ); // 特殊な出入り口かどうかをチェックする
static BOOL CheckOneStep( const ECAM_WORK* work ); // 自機の一歩移動の有無をチェックする
static BOOL CheckCameraAnime( const ECAM_WORK* work ); // シチュエーションに合ったアニメーションの有無をチェックする



//=============================================================================
// ■public functions
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief ワークを生成する
 *
 * @param fieldmap
 *
 * @return 生成したワーク
 */
//-----------------------------------------------------------------------------
ECAM_WORK* ENTRANCE_CAMERA_CreateWork( FIELDMAP_WORK* fieldmap )
{
  ECAM_WORK* work;

  work = CreateWork( FIELDMAP_GetHeapID(fieldmap) ); // 生成
  InitWork( work, fieldmap ); // 初期化

  return work;
}

//-----------------------------------------------------------------------------
/**
 * @brief ワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void ENTRANCE_CAMERA_DeleteWork( ECAM_WORK* work )
{
  DeleteWork( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief 演出をセットアップする
 *
 * @param work
 * @param param 演出パラメータ
 */
//----------------------------------------------------------------------------- 
void ENTRANCE_CAMERA_Setup( ECAM_WORK* work, const ECAM_PARAM* param )
{
  // 演出パラメータを設定
  SetECamParam( work, param );

  // 特殊出入り口
  if( CheckSpExit(work) ) {
    switch( param->situation ) {
    case ECAM_SITUATION_IN:  ECamSetup_SP_IN( work );  break;
    case ECAM_SITUATION_OUT: ECamSetup_SP_OUT( work ); break;
    default:                 GF_ASSERT(0);             break;
    }
  }
  // 通常出入り口
  else {
    switch( param->situation ) {
    case ECAM_SITUATION_IN:  ECamSetup_IN( work );  break;
    case ECAM_SITUATION_OUT: ECamSetup_OUT( work ); break;
    default:                 GF_ASSERT(0);          break;
    }
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 演出を開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void ENTRANCE_CAMERA_Start( ECAM_WORK* work )
{
  const ECAM_PARAM* param = GetECamParam( work );
  const ANIME_DATA* anime = GetAnimeData( work );

  // 有効なアニメーションがない
  if( CheckCameraAnime(work) == FALSE ) { return; }

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
 * @brief 演出によって操作したカメラを復帰する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void ENTRANCE_CAMERA_Recover( ECAM_WORK* work )
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
 * @brief ワークを生成する
 *
 * @param heapID
 */
//-----------------------------------------------------------------------------
static ECAM_WORK* CreateWork( HEAPID heapID )
{
  ECAM_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(ECAM_WORK) );

#ifdef DEBUG_PRINT_ENABLE
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: CreateWork\n" );
#endif

  return work;
}

//-----------------------------------------------------------------------------
/**
 * @brief ワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DeleteWork( ECAM_WORK* work )
{
  GFL_HEAP_FreeMemory( work );

#ifdef DEBUG_PRINT_ENABLE
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: DeleteWork\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ワークを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void InitWork( ECAM_WORK* work, FIELDMAP_WORK* fieldmap )
{
  // ゼロクリア
  GFL_STD_MemClear( work, sizeof(ECAM_WORK) );

  // 初期化
  work->fieldmap = fieldmap;
  work->camera   = FIELDMAP_GetFieldCamera( fieldmap );
  work->recoveryValidFlag = FALSE;
  work->cameraAreaFlag    = FALSE;

#ifdef DEBUG_PRINT_ENABLE
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: InitWork\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 通常出入口へ入る演出をセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void ECamSetup_IN( ECAM_WORK* work )
{
  ANIME_DATA*   anime      = &work->animeData;
  CAMERA_PARAM* startParam = &anime->startParam;
  CAMERA_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 endPos;

  // アニメの有無を決定
  switch( playerDir ) {
  case DIR_UP:
  case DIR_LEFT:
  case DIR_RIGHT:
    anime->validFlag_IN  = TRUE;
    anime->validFlag_OUT = FALSE; 
    break;
  case DIR_DOWN:
    anime->validFlag_IN  = FALSE;
    anime->validFlag_OUT = FALSE; 
    return; // 以下の処理は不要
  default: 
    GF_ASSERT(0); 
    break; 
  }

  // 自機の最終座標を決定
  if( CheckOneStep(work) ) {
    GetOneStepAfterPos( work, &endPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &endPos );
  }

  SetupCamera( work ); // カメラの設定を変更
  AdjustCameraAngle( work->camera ); // カメラアングルを再計算
  SetCurrentCameraTargetPos( work->camera ); // ターゲット座標を初期化

  // 開始パラメータ ( = 現在値 )
  GetCurrentCameraParam( work->camera, startParam );

  // 最終パラメータ
  {
    VecFx32 ofs = {0, 0, 0};

    GetCurrentCameraParam( work->camera, endParam );
    endParam->targetPos = endPos;
    endParam->targetOffset = ofs;

    switch( playerDir ) {
    case DIR_UP:    
      endParam->length = U_DOOR_LENGTH; 
      break;
    case DIR_LEFT:  
      endParam->pitch = L_DOOR_PITCH;
      endParam->yaw = L_DOOR_YAW;    
      endParam->length = L_DOOR_LENGTH; 
      break;
    case DIR_RIGHT: 
      endParam->pitch = R_DOOR_PITCH;
      endParam->yaw = R_DOOR_YAW;    
      endParam->length = R_DOOR_LENGTH; 
      break;
    case DIR_DOWN: 
      break;
    default: 
      GF_ASSERT(0); 
      break;
    }
  }

  // アニメフレーム数
  switch( playerDir ) {
  case DIR_UP:    anime->frame = U_DOOR_FRAME; break;
  case DIR_DOWN:  anime->frame = 0;            break;
  case DIR_LEFT:  anime->frame = L_DOOR_FRAME; break;
  case DIR_RIGHT: anime->frame = R_DOOR_FRAME; break;
  default: GF_ASSERT(0); break;
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 通常出入口から出る演出をセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void ECamSetup_OUT( ECAM_WORK* work )
{
  ANIME_DATA*   anime      = &work->animeData;
  CAMERA_PARAM* startParam = &anime->startParam;
  CAMERA_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 stepPos;

  // アニメの有無を決定
  switch( playerDir ) {
  case DIR_DOWN:
  case DIR_LEFT:
  case DIR_RIGHT:
    anime->validFlag_IN  = FALSE;
    anime->validFlag_OUT = TRUE; 
    break;
  case DIR_UP:
    anime->validFlag_IN  = FALSE;
    anime->validFlag_OUT = FALSE; 
    return; // 以下の処理は不要
  default: 
    GF_ASSERT(0); 
    break; 
  }

  // 開始時のターゲット座標
  if( CheckOneStep(work) ) {
    GetOneStepAfterPos( work, &stepPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &stepPos );
  }

  // アニメフレーム数
  switch( playerDir ) {
  case DIR_UP:    anime->frame = 0;            break;
  case DIR_DOWN:  anime->frame = U_DOOR_FRAME; break;
  case DIR_LEFT:  anime->frame = R_DOOR_FRAME; break;
  case DIR_RIGHT: anime->frame = L_DOOR_FRAME; break;
  default: GF_ASSERT(0); break;
  }

  // 自機を一歩移動させた状態にカメラを更新
  if( CheckOneStep(work) ) {
    SetupCameraTargetPos( camera, &stepPos );
  }

  SetupCamera( work ); // カメラの設定を変更
  AdjustCameraAngle( camera ); // カメラアングルを再計算

  // 開始パラメータ
  {
    GetCurrentCameraParam( camera, startParam );

    switch( playerDir ) {
    case DIR_UP:
      break;
    case DIR_DOWN:  
      startParam->length = U_DOOR_LENGTH; 
      break;
    case DIR_LEFT:  
      endParam->pitch = R_DOOR_PITCH;
      startParam->yaw = R_DOOR_YAW;    
      startParam->length = R_DOOR_LENGTH; 
      break;
    case DIR_RIGHT: 
      endParam->pitch = L_DOOR_PITCH;
      startParam->yaw = L_DOOR_YAW;    
      startParam->length = L_DOOR_LENGTH; 
      break;
    default: GF_ASSERT(0); break;
    }
    startParam->targetPos = stepPos;
    VEC_Set( &startParam->targetOffset, 0, 0, 0 );
  }

  // 最終パラメータ ( = 現在値 )
  GetCurrentCameraParam( camera, endParam );

  // 開始カメラ設定
  if( anime->validFlag_OUT ) {
    FIELD_CAMERA_SetAnglePitch( camera, startParam->pitch );
    FIELD_CAMERA_SetAngleYaw( camera, startParam->yaw );
    FIELD_CAMERA_SetAngleLen( camera, startParam->length );
    FIELD_CAMERA_SetTargetPos( camera, &startParam->targetPos );
    FIELD_CAMERA_SetTargetOffset( camera, &startParam->targetOffset );
  } 

}

//-----------------------------------------------------------------------------
/**
 * @brief 特殊出入口へ入る演出をセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void ECamSetup_SP_IN( ECAM_WORK* work )
{
  ANIME_DATA*   anime      = &work->animeData;
  CAMERA_PARAM* startParam = &anime->startParam;
  CAMERA_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 stepPos;
  ECAM_LOAD_DATA loadData;

  // 演出データをロード
  LoadSpData( &loadData, GetExitType(work) );

  // 演出の有無を決定
  anime->validFlag_IN  = loadData.validFlag_IN;
  anime->validFlag_OUT = loadData.validFlag_OUT;
  if( anime->validFlag_IN == FALSE ) { 
    return; // 以下の処理は不要
  }

  // 一歩移動後の座標
  if( CheckOneStep(work) ) {
    GetOneStepAfterPos( work, &stepPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &stepPos );
  }

  SetupCamera( work ); // カメラの設定を変更
  AdjustCameraAngle( camera ); // カメラアングルを再計算
  SetCurrentCameraTargetPos( camera ); // ターゲット座標を初期化

  // 開始パラメータ ( = 現在値 )
  GetCurrentCameraParam( camera, startParam );

  // 最終パラメータ
  anime->frame = loadData.frame;
  endParam->pitch  = loadData.pitch;
  endParam->yaw    = loadData.yaw;
  endParam->length = loadData.length << FX32_SHIFT;
  VEC_Set( &endParam->targetOffset,
      loadData.targetOffsetX << FX32_SHIFT,
      loadData.targetOffsetY << FX32_SHIFT,
      loadData.targetOffsetZ << FX32_SHIFT );
  if( loadData.manualTargetFlag ) {
    VEC_Set( &endParam->targetPos,
        loadData.targetPosX << FX32_SHIFT,
        loadData.targetPosY << FX32_SHIFT,
        loadData.targetPosZ << FX32_SHIFT );
  }
  else {
    endParam->targetPos = stepPos;
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 特殊出入口から出る演出をセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void ECamSetup_SP_OUT( ECAM_WORK* work )
{
  ANIME_DATA*   anime      = &work->animeData;
  CAMERA_PARAM* startParam = &anime->startParam;
  CAMERA_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 stepPos;
  ECAM_LOAD_DATA loadData;

  // 演出データをロード
  LoadSpData( &loadData, GetExitType(work) );

  // 演出の有無を決定
  anime->validFlag_IN  = loadData.validFlag_IN;
  anime->validFlag_OUT = loadData.validFlag_OUT;
  if( anime->validFlag_OUT == FALSE ) { 
    return; // 以下の処理は不要
  }

  // 開始時のターゲット座標
  if( CheckOneStep(work) ) {
    GetOneStepAfterPos( work, &stepPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &stepPos );
  }

  // 自機を一歩移動させた状態にカメラを更新
  if( CheckOneStep(work) ) {
    SetupCameraTargetPos( camera, &stepPos );
  }

  SetupCamera( work );
  AdjustCameraAngle( camera );

  // 最終パラメータ ( = 現在値 )
  GetCurrentCameraParam( camera, endParam );

  // 開始パラメータ
  anime->frame = loadData.frame;
  startParam->pitch  = loadData.pitch;
  startParam->yaw    = loadData.yaw;
  startParam->length = loadData.length << FX32_SHIFT;
  VEC_Set( &startParam->targetOffset,
      loadData.targetOffsetX << FX32_SHIFT,
      loadData.targetOffsetY << FX32_SHIFT,
      loadData.targetOffsetZ << FX32_SHIFT );
  if( loadData.manualTargetFlag ) {
    VEC_Set( &startParam->targetPos,
        loadData.targetPosX << FX32_SHIFT,
        loadData.targetPosY << FX32_SHIFT,
        loadData.targetPosZ << FX32_SHIFT );
  }
  else {
    startParam->targetPos = stepPos;
  }

  // 開始カメラ設定
  if( anime->validFlag_OUT ) {
    FIELD_CAMERA_SetAnglePitch( camera, startParam->pitch );
    FIELD_CAMERA_SetAngleYaw( camera, startParam->yaw );
    FIELD_CAMERA_SetAngleLen( camera, startParam->length );
    FIELD_CAMERA_SetTargetPos( camera, &startParam->targetPos );
    FIELD_CAMERA_SetTargetOffset( camera, &startParam->targetOffset );
  } 
}

//-----------------------------------------------------------------------------
/**
 * @brief 特殊出入り口のカメラ動作データを読み込む
 *
 * @param dest     読み込んだデータの格納先
 * @param exitType 特殊出入口タイプ ( EXIT_TYPE_SPxx )
 */
//-----------------------------------------------------------------------------
static void LoadSpData( ECAM_LOAD_DATA* dest, EXIT_TYPE exitType )
{
  u32 datID;

  // EXIT_TYPE_SPxx以外はダメ
  GF_ASSERT( EXIT_TYPE_SP1 <= exitType );
  GF_ASSERT( exitType < EXIT_TYPE_MAX );

  // 読み込むデータを決定
  datID = exitType - EXIT_TYPE_SP1;

  // 読み込み
  GFL_ARC_LoadData( dest, ARCID_ENTRANCE_CAMERA, datID );

#ifdef DEBUG_PRINT_ENABLE
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: LoadSpData\n" );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - exitType         = %d\n", dest->exitType );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - pitch            = 0x%x\n", dest->pitch );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - yaw              = 0x%x\n", dest->yaw );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - length           = 0x%x\n", dest->length );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - manualTargetFlag = %d\n", dest->manualTargetFlag );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetPosX       = 0x%x\n", dest->targetPosX );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetPosY       = 0x%x\n", dest->targetPosY );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetPosZ       = 0x%x\n", dest->targetPosZ );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetOffsetX    = 0x%x\n", dest->targetOffsetX );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetOffsetY    = 0x%x\n", dest->targetOffsetY );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetOffsetZ    = 0x%x\n", dest->targetOffsetZ );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - frame            = %d\n", dest->frame );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - validFlag_IN     = %d\n", dest->validFlag_IN );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - validFlag_OUT    = %d\n", dest->validFlag_OUT );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラの現在のパラメータを取得する
 *
 * @param camera
 * @param dest   取得したパラメータの格納先
 */
//-----------------------------------------------------------------------------
static void GetCurrentCameraParam( const FIELD_CAMERA* camera, CAMERA_PARAM* dest )
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
 * @brief カメラ操作の準備をする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetupCamera( ECAM_WORK* work )
{
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  FIELD_CAMERA*  camera   = work->camera;

  // レールシステムのカメラ制御を停止
  if( FIELDMAP_GetBaseSystemType( fieldmap ) == FLDMAP_BASESYS_RAIL ) {
    FLDNOGRID_MAPPER* NGMapper = FIELDMAP_GetFldNoGridMapper( fieldmap );
    FLDNOGRID_MAPPER_SetRailCameraActive( NGMapper, FALSE );
  }

  // ターゲットのバインドをOFF
  FIELD_CAMERA_FreeTarget( camera );

  // カメラエリアを無効化
  work->cameraAreaFlag = FIELD_CAMERA_GetCameraAreaActive( camera );
  FIELD_CAMERA_SetCameraAreaActive( camera, FALSE );

  // カメラモードを変更
  work->initialCameraMode = FIELD_CAMERA_GetMode( camera ); 
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

  // フラグセット
  work->recoveryValidFlag = TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラの復帰をする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void RecoverCamera( const ECAM_WORK* work )
{
  FIELDMAP_WORK* fieldmap = work->fieldmap;
  FIELD_CAMERA*  camera   = work->camera;

  GF_ASSERT( work->recoveryValidFlag );

  // カメラエリアを復帰
  FIELD_CAMERA_SetCameraAreaActive( camera, work->cameraAreaFlag );

  // ターゲットを元に戻す
  FIELD_CAMERA_BindDefaultTarget( camera );

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
 * @brief ターゲット座標を変更し, カメラに反映させる
 *
 * @param camera
 * @param targetPos ターゲット座標
 */
//-----------------------------------------------------------------------------
static void SetupCameraTargetPos( FIELD_CAMERA* camera, const VecFx32* targetPos )
{
  FIELD_CAMERA_FreeTarget( camera ); // ターゲットを外す
  FIELD_CAMERA_SetTargetPos( camera, targetPos ); // 一時的にターゲット座標を設定
  FIELD_CAMERA_Main( camera, 0 ); // ターゲットの変更をカメラに反映させる
  FIELD_CAMERA_BindDefaultTarget( camera ); // ターゲットを元に戻す
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラを即時反映する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetCameraParam( const ECAM_WORK* work )
{
  FIELD_CAMERA*       camera   = work->camera;
  const ANIME_DATA*   anime    = &(work->animeData); 
  const CAMERA_PARAM* endParam = &(anime->endParam);

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
static void StartCameraAnime( const ECAM_WORK* work )
{
  FIELDMAP_WORK*      fieldmap   = work->fieldmap;
  FIELD_CAMERA*       camera     = work->camera;
  const ANIME_DATA*   anime      = &(work->animeData); 
  const CAMERA_PARAM* startParam = &(anime->startParam);
  const CAMERA_PARAM* endParam   = &(anime->endParam);

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
 * @brief 一歩先の座標を取得する
 *
 * @param work
 * @param dest 取得した座標の格納先
 */
//----------------------------------------------------------------------------- 
static void GetOneStepAfterPos( const ECAM_WORK* work, VecFx32* dest )
{
  FIELD_PLAYER* player;
  u16 playerDir;
  VecFx32 now_pos, way_vec, step_pos;

  // 自機データを取得
  player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  playerDir = FIELD_PLAYER_GetDir( player );

  // 一歩前進した座標を算出
  FIELD_PLAYER_GetPos( player, &now_pos );
  FIELD_PLAYER_GetDirWay( player, playerDir, &way_vec );
  VEC_MultAdd( FIELD_CONST_GRID_FX32_SIZE, &way_vec, &now_pos, &step_pos );

  // y 座標を補正
  {
    MMDL* mmdl;
    fx32 height;

    mmdl = FIELD_PLAYER_GetMMdl( player );
    MMDL_GetMapPosHeight( mmdl, &step_pos, &height );
    step_pos.y = height;
  }

  // 座標を返す
  *dest = step_pos;
}

//----------------------------------------------------------------------------- 
/**
 * @brief 自機の向きを取得する
 *
 * @param work
 */
//----------------------------------------------------------------------------- 
static u16 GetPlayerDir( const ECAM_WORK* work )
{
  FIELD_PLAYER* player;

  player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  return FIELD_PLAYER_GetDir( player );
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
static FIELDMAP_WORK* GetFieldmap( const ECAM_WORK* work )
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
static FIELD_CAMERA* GetCamera( const ECAM_WORK* work )
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
static const ANIME_DATA* GetAnimeData( const ECAM_WORK* work )
{
  return &(work->animeData);
}

//----------------------------------------------------------------------------- 
/**
 * @brief 演出パラメータを取得する
 *
 * @param work
 *
 * @return 演出パラメータ
 */
//----------------------------------------------------------------------------- 
static const ECAM_PARAM* GetECamParam( const ECAM_WORK* work )
{
  return &(work->param);
}

//----------------------------------------------------------------------------- 
/**
 * @brief 演出パラメータを設定する
 *
 * @param work
 * @param param 設定するパラメータ
 */
//----------------------------------------------------------------------------- 
static void SetECamParam( ECAM_WORK* work, const ECAM_PARAM* param )
{
  work->param = *param;
}

//----------------------------------------------------------------------------- 
/**
 * @brief 出入り口タイプを取得する
 *
 * @param work
 *
 * @return 出入り口タイプ
 */
//----------------------------------------------------------------------------- 
static EXIT_TYPE GetExitType( const ECAM_WORK* work )
{
  const ECAM_PARAM* param;

  param = GetECamParam( work );

  return param->exitType;
}

//----------------------------------------------------------------------------- 
/**
 * @brief 特殊な出入り口かどうかをチェックする
 *
 * @param work
 *
 * @return 特殊な出入り口の場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------- 
static BOOL CheckSpExit( const ECAM_WORK* work )
{
  const ECAM_PARAM* param;

  param = GetECamParam( work );

  if( EXIT_TYPE_SP1 <= param->exitType ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//----------------------------------------------------------------------------- 
/**
 * @brief 自機の一歩移動の有無をチェックする
 *
 * @param work
 *
 * @return 自機の一歩移動演出がある場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------- 
static BOOL CheckOneStep( const ECAM_WORK* work )
{
  const ECAM_PARAM* param;

  param = GetECamParam( work );

  return (param->oneStep == ECAM_ONESTEP_ON );
}

//----------------------------------------------------------------------------- 
/**
 * @brief シチュエーションに合ったアニメーションの有無をチェックする
 *
 * @param work
 *
 * @return 有効なアニメーションデータが存在する場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------- 
static BOOL CheckCameraAnime( const ECAM_WORK* work )
{
  const ECAM_PARAM* param;
  const ANIME_DATA* anime;

  param = GetECamParam( work );
  anime = GetAnimeData( work );

  if( param->situation == ECAM_SITUATION_IN ) {
    return anime->validFlag_IN;
  }
  else {
    return anime->validFlag_OUT;
  }
}












