#include <gflib.h>
#include "fieldmap.h"
#include "gamesystem/iss_3ds_sys.h"
#include "sound_obj.h"

#include "gamesystem/iss_3ds_unit.h"
#include "system/ica_anime.h"
#include "h03.naix"


//=========================================================================================
// ■非公開関数のプロトタイプ宣言
//=========================================================================================
static void AdjustPosition( SOUNDOBJ* soundObj );
static void AdjustDirection( SOUNDOBJ* soundObj );


//========================================================================================
// 音源オブジェクト
//========================================================================================
struct _SOUNDOBJ
{
  HEAPID             heapID;
  FIELDMAP_WORK*     fieldmap;
  ISS_3DS_SYS*       iss3dsSystem;
  ISS3DS_UNIT_INDEX  iss3dsUnitIdx;  // 自身の3Dサウンドユニットの管理インデックス
  ICA_ANIME*         icaAnime;       // アニメーションデータ
  GFL_G3D_OBJSTATUS* g3dObjStatus;   // 操作対象ステータス
};


//=========================================================================================
// ■作成・破棄
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェクトを作成する
 *
 * @param fieldmap
 * @param g3dObjStatus   操作対象3Dオブジェクトのステータス
 * @param iss3dsUnitIdx  登録する3Dサウンドユニットを指定
 * @param effectiveRange 音が届く距離
 * @param maxVolume      最大ボリューム
 */
//-----------------------------------------------------------------------------------------
SOUNDOBJ* SOUNDOBJ_Create( FIELDMAP_WORK* fieldmap, GFL_G3D_OBJSTATUS* g3dObjStatus,
                           ISS3DS_UNIT_INDEX iss3dsUnitIdx, fx32 effectiveRange, int maxVolume )
{
  SOUNDOBJ* soundObj;
  HEAPID heapID;
  GAMESYS_WORK* gameSystem;
  ISS_SYS* issSystem;
  ISS_3DS_SYS* iss3dsSystem;

  heapID       = FIELDMAP_GetHeapID( fieldmap );
  gameSystem   = FIELDMAP_GetGameSysWork( fieldmap );
  issSystem    = GAMESYSTEM_GetIssSystem( gameSystem );
  iss3dsSystem = ISS_SYS_GetIss3DSSystem( issSystem );

  // 生成
  soundObj = (SOUNDOBJ*)GFL_HEAP_AllocMemory( heapID, sizeof(SOUNDOBJ) );

  // 初期化
  soundObj->heapID        = heapID;
  soundObj->fieldmap      = fieldmap;
  soundObj->iss3dsSystem  = iss3dsSystem;
  soundObj->iss3dsUnitIdx = iss3dsUnitIdx;
  soundObj->icaAnime      = NULL;
  soundObj->g3dObjStatus  = g3dObjStatus;

  // 3Dサウンドユニット登録
  if( ISS_3DS_SYS_IsUnitRegistered( iss3dsSystem, iss3dsUnitIdx ) == FALSE )
  {
    ISS_3DS_SYS_RegisterUnit( iss3dsSystem, iss3dsUnitIdx, effectiveRange, maxVolume );
  }

  return soundObj;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ダミー音源オブジェクトを作成する
 *
 * @param fieldmap
 * @param g3dObjStatus 操作対象3Dオブジェクトのステータス
 *
 * ※BGMの操作を行わない音源オブジェクトを生成する。
 */
//-----------------------------------------------------------------------------------------
SOUNDOBJ* SOUNDOBJ_CreateDummy( FIELDMAP_WORK* fieldmap, GFL_G3D_OBJSTATUS* g3dObjStatus )
{
  SOUNDOBJ* soundObj;
  HEAPID heapID;

  heapID = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  soundObj = (SOUNDOBJ*)GFL_HEAP_AllocMemory( heapID, sizeof(SOUNDOBJ) );

  // 初期化
  soundObj->heapID       = heapID;
  soundObj->fieldmap     = fieldmap;
  soundObj->iss3dsSystem = NULL;
  soundObj->icaAnime     = NULL;
  soundObj->g3dObjStatus = g3dObjStatus;

  return soundObj;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェクトを破棄する
 *
 * @param soundObj
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_Delete( SOUNDOBJ* soundObj )
{
  // アニメーションデータ破棄
  if( soundObj->icaAnime )
  {
    ICA_ANIME_Delete( soundObj->icaAnime );
  }

  // 本体を破棄
  GFL_HEAP_FreeMemory( soundObj );
}


//=========================================================================================
// ■初期化関数
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションを登録する
 *
 * @param arcID       設定するアニメーションデータのアーカイブID
 * @param datID       設定するアニメーションデータのアーカイブ内データID
 * @param bufInterval ストリーミング間隔
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_SetAnime( SOUNDOBJ* soundObj, ARCID arcID, ARCDATID datID, int bufInterval )
{
  // 登録済みデータを破棄
  if( soundObj->icaAnime )
  {
    ICA_ANIME_Delete( soundObj->icaAnime );
  }

  // アニメーション作成
  soundObj->icaAnime = ICA_ANIME_CreateStreamingAlloc( soundObj->heapID, arcID, datID, bufInterval );
}


//=========================================================================================
// ■動作管理
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションを進める
 *
 * @param soundObj
 * @param frame    進めるフレーム数
 *
 * @return ループしたら TRUE
 */
//-----------------------------------------------------------------------------------------
BOOL SOUNDOBJ_IncAnimeFrame( SOUNDOBJ* soundObj, fx32 frame )
{
  BOOL loop = FALSE;

  // アニメーション未登録
  if( soundObj->icaAnime == NULL ){ return FALSE; }

  // アニメーションを更新
  loop = ICA_ANIME_IncAnimeFrame( soundObj->icaAnime, frame );

  // 位置を更新
  AdjustPosition( soundObj );
  AdjustDirection( soundObj );

  // ループしたかどうかを返す
  return loop;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションフレーム数を設定する
 *
 * @param soundObj
 * @param frame    設定するフレーム数を指定
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_SetAnimeFrame( SOUNDOBJ* soundObj, fx32 frame )
{
  // アニメーション未登録
  if( soundObj->icaAnime == NULL ){ return; }

  // アニメーションを更新
  ICA_ANIME_SetAnimeFrame( soundObj->icaAnime, frame );

  // 位置を更新
  AdjustPosition( soundObj );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションフレーム数を取得する
 *
 * @param soundObj
 *
 * @return 現在のアニメーションフレーム数
 */
//-----------------------------------------------------------------------------------------
fx32 SOUNDOBJ_GetAnimeFrame( SOUNDOBJ* soundObj ) 
{
  // アニメーション未登録
  if( soundObj->icaAnime == NULL ){ return 0; }

  // 現在のアニメーションフレーム数を返す
  return ICA_ANIME_GetNowFrame( soundObj->icaAnime );
}


//=========================================================================================
// ■非公開関数
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 拡張オブジェ・音源オブジェの位置をアニメーションに合わせる
 *
 * @param soundObj 更新するオブジェクト
 */
//-----------------------------------------------------------------------------------------
static void AdjustPosition( SOUNDOBJ* soundObj )
{
  VecFx32 pos;
  ISS_3DS_UNIT* unit;

  // アニメーション未登録
  if( soundObj->icaAnime == NULL ){ return; }

  // 位置を取得
  if( ICA_ANIME_GetTranslate( soundObj->icaAnime, &pos ) != TRUE ) { return; }

  // 3Dステータスを設定
  VEC_Set( &soundObj->g3dObjStatus->trans, pos.x, pos.y, pos.z );

  // 音源位置を合わせる
  if( soundObj->iss3dsSystem )
  {
    ISS_3DS_SYS_SetUnitPos( soundObj->iss3dsSystem, soundObj->iss3dsUnitIdx, &pos );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 拡張オブジェの向きをアニメーションに合わせる
 *
 * @param soundObj 更新するオブジェクト
 */
//-----------------------------------------------------------------------------------------
static void AdjustDirection( SOUNDOBJ* soundObj )
{
  VecFx32 vec;

  // アニメーション未登録
  if( soundObj->icaAnime == NULL ){ return; }

  // 回転角度を取得
  if( ICA_ANIME_GetRotate( soundObj->icaAnime, &vec ) != TRUE ) { return; }

  // 3Dステータスを設定
  {
    float x, y, z;
    u16 rx, ry, rz;
    x = FX_FX32_TO_F32( vec.x );
    y = FX_FX32_TO_F32( vec.y );
    z = FX_FX32_TO_F32( vec.z );
    while( x < 0 ) x += 360.0f;
    while( y < 0 ) y += 360.0f;
    while( z < 0 ) z += 360.0f;
    rx = x / 360.0f * 0xffff;
    ry = y / 360.0f * 0xffff;
    rz = z / 360.0f * 0xffff; 
    MTX_Identity33( &soundObj->g3dObjStatus->rotate );
    GFL_CALC3D_MTX_CreateRot( rx, ry, rz, &soundObj->g3dObjStatus->rotate );
  }
}
