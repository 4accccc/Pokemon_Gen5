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

#include "event_entrance_effect.h" // for EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos
#include "field_camera_anime.h"
#include "field_rail.h"
#include "fieldmap.h"

#include "field/field_const.h"     // for FIELD_CONST_xxxx
#include "field/eventdata_type.h"  // for EXIT_TYPE_xxxx
#include "arc/arc_def.h"           // for ARCID_xxxx



//=============================================================================
// ■定数
//=============================================================================
//#define DEBUG_PRINT_ON    // デバッグ出力スイッチ
#define DEBUG_PRINT_TARGET (3)// デバッグ出力先

//#define ZERO_FRAME 
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
#define L_DOOR_YAW    (0x1089)
#define L_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 右ドアへの出入り
#define R_DOOR_FRAME  (0)
#define R_DOOR_PITCH  (0x1fa1)
#define R_DOOR_YAW    (0xf047)
#define R_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
//-------------------------------------
// 通常出入り口のカメラ動作 ( 通常版 )
//-------------------------------------
#else 
// 上ドアへの出入り
#define U_DOOR_FRAME  (10)
#define U_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 左ドアへの出入り
#define L_DOOR_FRAME  (10)
#define L_DOOR_PITCH  (0x1fa1)
#define L_DOOR_YAW    (0x0a09)//(0x1089)
#define L_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
// 右ドアへの出入り
#define R_DOOR_FRAME  (10)
#define R_DOOR_PITCH  (0x1fa1)
#define R_DOOR_YAW    (0xf5f7)//(0xf047)
#define R_DOOR_LENGTH (0x00b4 << FX32_SHIFT) 
#endif



//=============================================================================
// ■エントランス・カメラ動作データ読み込み用ワーク
//=============================================================================
typedef struct {

  u32 exitType;          // 出入り口タイプ
  u32 pitch;             // ピッチ
  u32 yaw;               // ヨー
  u32 length;            // 距離
  u32 manualTargetFlag;  // ターゲット座標が有効かどうか
  u32 targetPosX;        // ターゲット座標 x
  u32 targetPosY;        // ターゲット座標 y
  u32 targetPosZ;        // ターゲット座標 z
  u32 targetOffsetX;     // ターゲットオフセット x
  u32 targetOffsetY;     // ターゲットオフセット y
  u32 targetOffsetZ;     // ターゲットオフセット z
  u32 frame;             // 動作フレーム数
  u32 validFlag_IN;      // 進入時に有効なデータかどうか
  u32 validFlag_OUT;     // 退出時に有効なデータかどうか
  u32 targetBindOffFlag; // ターゲットのバインドをOFFにするかどうか
  u32 cameraAreaOffFlag; // カメラエリアをOFFにするかどうか

} ECAM_LOAD_DATA; 


//=============================================================================
// ■エントランス・カメラ操作ワーク 
//=============================================================================
struct _ECAM_WORK {

  FIELDMAP_WORK* fieldmap;
  FIELD_CAMERA*  camera;
  FCAM_ANIME_DATA animeData;
  FCAM_ANIME_WORK* animeWork;
  ECAM_PARAM param;     // 演出パラメータ
  BOOL setupFlag;       // 演出のセットアップが完了済みかどうか
  BOOL validFlag_IN;    // 進入時に有効なデータかどうか
  BOOL validFlag_OUT;   // 退出時に有効なデータかどうか
};



//=============================================================================
// ■private functions
//=============================================================================
// 生成・破棄・初期化
static ECAM_WORK* CreateWork( HEAPID heapID ); // ワークを生成する
static void DeleteWork( ECAM_WORK* work ); // ワークを破棄する
static void InitWork( ECAM_WORK* work, FIELDMAP_WORK* fieldmap ); // ワークを初期化する
static void CreateAnimeWork( ECAM_WORK* work, HEAPID heapID ); // カメラ操作ワークを生成する
static void DeleteAnimeWork( ECAM_WORK* work ); // カメラ操作ワークを破棄する
// セットアップ
static void ECamSetup_IN( ECAM_WORK* work ); // 通常出入口へ入る演出をセットアップする
static void ECamSetup_OUT( ECAM_WORK* work ); // 通常出入口から出る演出をセットアップする
static void ECamSetup_SP_IN( ECAM_WORK* work ); // 特殊出入口へ入る演出をセットアップする
static void ECamSetup_SP_OUT( ECAM_WORK* work ); // 特殊出入口から出る演出をセットアップする
// アニメーションデータ
static void LoadSpData( ECAM_LOAD_DATA* dest, EXIT_TYPE exitType ); // 特殊出入り口のカメラ動作データを読み込む
// カメラの準備・復帰
static void SetupCameraTargetPos( FIELD_CAMERA* camera, const VecFx32* targetPos ); // ターゲット座標を変更し, カメラに反映させる
// アクセッサ
static FIELDMAP_WORK* GetFieldmap( const ECAM_WORK* work ); // フィールドマップを取得する
static FIELD_CAMERA* GetCamera( const ECAM_WORK* work ); // カメラを取得する
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
  HEAPID heapID;
  ECAM_WORK* work;

  heapID = FIELDMAP_GetHeapID( fieldmap );

  // ワークを生成
  work = CreateWork( heapID );

  // ワークを初期化
  InitWork( work, fieldmap );
  CreateAnimeWork( work, heapID );

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
  DeleteAnimeWork( work );
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
  FCAM_ANIME_SetAnimeData( work->animeWork, &work->animeData );

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

  // セットアップ完了
  work->setupFlag = TRUE;
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
  // 有効なアニメーションがない
  if( CheckCameraAnime(work) == FALSE ) { return; }

  FCAM_ANIME_StartAnime( work->animeWork );
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
  FCAM_ANIME_RecoverCamera( work->animeWork );
}

//-----------------------------------------------------------------------------
/**
 * @brief 演出の有無をチェックする
 *
 * @param work
 *
 * @return 有効な演出がある場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------
BOOL ENTRANCE_CAMERA_IsAnimeExist( const ECAM_WORK* work )
{
  // セットアップ後でないとダメ
  GF_ASSERT( work->setupFlag );

  return CheckCameraAnime(work);
}

//-----------------------------------------------------------------------------
/**
 * @brief 演出が０フレームかどうかをチェックする
 *
 * @param work
 *
 * @return ０フレーム演出データなら TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------
BOOL ENTRANCE_CAMERA_IsZeroFrameAnime( const ECAM_WORK* work )
{
  const FCAM_ANIME_DATA* anime = &work->animeData;

  if( anime->frame == 0 ) { return TRUE; } 
  return FALSE;
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

#ifdef DEBUG_PRINT_ON
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

#ifdef DEBUG_PRINT_ON
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
  work->setupFlag = FALSE;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: InitWork\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを生成する
 *
 * @parma work
 * @param heapID
 */
//-----------------------------------------------------------------------------
static void CreateAnimeWork( ECAM_WORK* work, HEAPID heapID )
{
  GF_ASSERT( work->animeWork == NULL );
  GF_ASSERT( work->fieldmap );

  work->animeWork = FCAM_ANIME_CreateWork( work->fieldmap );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: CreateWork\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief カメラ操作ワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DeleteAnimeWork( ECAM_WORK* work )
{
  if( work->animeWork ) {
    FCAM_ANIME_DeleteWork( work->animeWork );
    work->animeWork = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: DeleteAnimeWork\n" );
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
  FCAM_ANIME_DATA*   anime      = &work->animeData;
  FCAM_PARAM* startParam = &anime->startParam;
  FCAM_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 endPos;

  // アニメの有無を決定
  switch( playerDir ) {
  case DIR_LEFT:
  case DIR_RIGHT:
    work->validFlag_IN  = TRUE;
    work->validFlag_OUT = FALSE; 
    break;
  case DIR_UP:
  case DIR_DOWN:
    work->validFlag_IN  = FALSE;
    work->validFlag_OUT = FALSE; 
    break;
  default: 
    GF_ASSERT(0); 
    break; 
  }

  if( work->validFlag_IN == FALSE ) { 
    return; // 以下の処理は不要
  }

  // ターゲットオフセット・カメラエリアは操作しない
  anime->cameraAreaOffFlag = FALSE;
  anime->targetBindOffFlag = FALSE;

  // 自機の最終座標を決定
  if( CheckOneStep(work) ) {
    EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos( work->fieldmap, &endPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &endPos );
  }

  // カメラをセットアップ
  FCAM_ANIME_SetupCamera( work->animeWork );

  // 開始パラメータ ( = 現在値 )
  FCAM_PARAM_GetCurrentParam( work->camera, startParam );

  // 最終パラメータ
  {
    VecFx32 ofs = {0, 0, 0};

    FCAM_PARAM_GetCurrentParam( work->camera, endParam );
    //endParam->targetPos = endPos;
    //endParam->targetOffset = ofs;

    switch( playerDir ) {
    case DIR_UP:    endParam->length = U_DOOR_LENGTH; break;
    case DIR_LEFT:  endParam->yaw = L_DOOR_YAW;    break;
    case DIR_RIGHT: endParam->yaw = R_DOOR_YAW;    break;
    case DIR_DOWN:  break;
    default:        GF_ASSERT(0); break;
    }
  }

  // アニメフレーム数
  switch( playerDir ) {
  case DIR_UP:    anime->frame = 0; break;
  case DIR_DOWN:  anime->frame = 0; break;
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
  FCAM_ANIME_DATA*   anime      = &work->animeData;
  FCAM_PARAM* startParam = &anime->startParam;
  FCAM_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 stepPos;

  // アニメの有無を決定
  switch( playerDir ) {
  case DIR_DOWN:
  case DIR_LEFT:
  case DIR_RIGHT:
    work->validFlag_IN  = FALSE;
    work->validFlag_OUT = TRUE; 
    break;
  case DIR_UP:
    work->validFlag_IN  = FALSE;
    work->validFlag_OUT = FALSE; 
    break;
  default: 
    GF_ASSERT(0); 
    break; 
  }

  if( work->validFlag_OUT == FALSE ) { 
    return; // 以下の処理は不要
  }

  // ターゲットオフセット・カメラエリアは操作しない
  anime->cameraAreaOffFlag = FALSE;
  anime->targetBindOffFlag = FALSE;

  // 開始時のターゲット座標
  if( CheckOneStep(work) ) {
    EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos( work->fieldmap, &stepPos );
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

  // カメラをセットアップ
  FCAM_ANIME_SetupCamera( work->animeWork );

  // 開始パラメータ
  {
    FCAM_PARAM_GetCurrentParam( work->camera, startParam );

    switch( playerDir ) {
    case DIR_UP:
      break;
    case DIR_DOWN:  
      startParam->length = U_DOOR_LENGTH; 
      break;
    case DIR_LEFT:  
      startParam->yaw = R_DOOR_YAW;    
      break;
    case DIR_RIGHT: 
      startParam->yaw = L_DOOR_YAW;    
      break;
    default: GF_ASSERT(0); break;
    }
    //startParam->targetPos = stepPos;
    //VEC_Set( &startParam->targetOffset, 0, 0, 0 );
  }

  // 最終パラメータ ( = 現在値 )
  FCAM_PARAM_GetCurrentParam( work->camera, endParam );

  // 開始カメラ設定
  if( work->validFlag_OUT ) {
    FCAM_ANIME_SetCameraStartParam( work->animeWork );
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
  FCAM_ANIME_DATA*   anime      = &work->animeData;
  FCAM_PARAM* startParam = &anime->startParam;
  FCAM_PARAM* endParam   = &anime->endParam;
  FIELD_CAMERA* camera     = GetCamera( work );
  FIELD_PLAYER* player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16           playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 stepPos;
  ECAM_LOAD_DATA loadData;

  // 演出データをロード
  LoadSpData( &loadData, GetExitType(work) );
  anime->cameraAreaOffFlag = loadData.cameraAreaOffFlag;
  anime->targetBindOffFlag = loadData.targetBindOffFlag;

  // 演出の有無を決定
  work->validFlag_IN  = loadData.validFlag_IN;
  work->validFlag_OUT = loadData.validFlag_OUT;
  if( work->validFlag_IN == FALSE ) { 
    return; // 以下の処理は不要
  }

  // 一歩移動後の座標
  if( CheckOneStep(work) ) {
    EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos( work->fieldmap, &stepPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &stepPos );
  }

  // カメラをセットアップ
  FCAM_ANIME_SetupCamera( work->animeWork );

  // 開始パラメータ ( = 現在値 )
  FCAM_PARAM_GetCurrentParam( work->camera, startParam );

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
  FCAM_ANIME_DATA* anime      = &work->animeData;
  FCAM_PARAM*      startParam = &anime->startParam;
  FCAM_PARAM*      endParam   = &anime->endParam;
  FIELD_CAMERA*    camera     = GetCamera( work );
  FIELD_PLAYER*    player     = FIELDMAP_GetFieldPlayer( work->fieldmap );
  u16              playerDir  = FIELD_PLAYER_GetDir( player );

  VecFx32 stepPos;
  ECAM_LOAD_DATA loadData;

  // 演出データをロード
  LoadSpData( &loadData, GetExitType(work) );
  anime->cameraAreaOffFlag = loadData.cameraAreaOffFlag;
  anime->targetBindOffFlag = loadData.targetBindOffFlag;

  // 演出の有無を決定
  work->validFlag_IN  = loadData.validFlag_IN;
  work->validFlag_OUT = loadData.validFlag_OUT;
  if( work->validFlag_OUT == FALSE ) { 
    return; // 以下の処理は不要
  }

  // 開始時のターゲット座標
  if( CheckOneStep(work) ) {
    EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos( work->fieldmap, &stepPos );
  }
  else {
    FIELD_PLAYER_GetPos( player, &stepPos );
  }

  // 自機を一歩移動させた状態にカメラを更新
  if( anime->targetBindOffFlag && CheckOneStep(work) ) {
    SetupCameraTargetPos( camera, &stepPos );
  }

  // カメラをセットアップ
  FCAM_ANIME_SetupCamera( work->animeWork );

  // 最終パラメータ ( = 現在値 )
  FCAM_PARAM_GetCurrentParam( work->camera, endParam );

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
  if( work->validFlag_OUT ) {
    FCAM_ANIME_SetCameraStartParam( work->animeWork );
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

#ifdef DEBUG_PRINT_ON
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
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - cameraAreaOffFlag   = %d\n", dest->cameraAreaOffFlag );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "ENTRANCE-CAMERA: - targetBindOffFlag   = %d\n", dest->targetBindOffFlag );
#endif
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

  return EXIT_TYPE_IsSpExit( param->exitType );
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

  param = GetECamParam( work );

  if( param->situation == ECAM_SITUATION_IN ) {
    return work->validFlag_IN;
  }
  else {
    return work->validFlag_OUT;
  }
} 
