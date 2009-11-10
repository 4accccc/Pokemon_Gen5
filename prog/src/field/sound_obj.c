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
static void AdjustPosition( SOUNDOBJ* sobj );


//========================================================================================
/**
 * @brief 音源オブジェクト
 */
//========================================================================================
struct _SOUNDOBJ
{
  HEAPID             heapID;  // 使用するヒープID
  FIELDMAP_WORK*   fieldmap;  // フィールドマップ
  ISS_3DS_SYS*    iss3dsSys;  // 登録先3Dサウンドシステム
  u8          iss3dsUnitIdx;  // 自身の3Dサウンドユニットの管理インデックス
  ICA_ANIME*       icaAnime;  // アニメーションデータ
  GFL_G3D_OBJSTATUS* status;  // 操作対象ステータス
};


//=========================================================================================
// ■作成・破棄
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェクトを作成する
 *
 * @param fieldmap 表示先フィールドマップ
 * @param iss_sys  登録先3Dサウンドシステム
 * @param status   操作対象ステータス
 */
//-----------------------------------------------------------------------------------------
SOUNDOBJ* SOUNDOBJ_Create( 
    FIELDMAP_WORK* fieldmap, ISS_3DS_SYS* iss_sys, GFL_G3D_OBJSTATUS* status )
{
  SOUNDOBJ* sobj;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // インスタンス生成
  sobj = (SOUNDOBJ*)GFL_HEAP_AllocMemory( heap_id, sizeof(SOUNDOBJ) );

  // インスタンス初期化
  sobj->heapID        = heap_id;
  sobj->fieldmap      = fieldmap;
  sobj->iss3dsSys     = iss_sys;
  sobj->iss3dsUnitIdx = ISS_3DS_SYS_AddUnit( iss_sys );
  sobj->icaAnime      = NULL;
  sobj->status        = status;
  return sobj;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェクトを破棄する
 *
 * @param sobj 破棄するオブジェクト
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_Delete( SOUNDOBJ* sobj )
{
  // 3Dサウンドユニットを破棄
  ISS_3DS_SYS_DeleteUnit( sobj->iss3dsSys, sobj->iss3dsUnitIdx );

  // アニメーションデータを破棄
  if( sobj->icaAnime )
  {
    ICA_ANIME_Delete( sobj->icaAnime );
  }

  // インスタンスを破棄
  GFL_HEAP_FreeMemory( sobj );
}


//=========================================================================================
// ■初期化関数
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションを登録する
 *
 * @param arc_id       設定するアニメーションデータのアーカイブID
 * @param dat_id       設定するアニメーションデータのアーカイブ内データID
 * @param buf_interval ストリーミング間隔
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_SetAnime( SOUNDOBJ* sobj, ARCID arc_id, ARCDATID dat_id, int buf_interval )
{
  // 登録済みデータを破棄
  if( sobj->icaAnime )
  {
    ICA_ANIME_Delete( sobj->icaAnime );
  }

  // アニメーション作成
  sobj->icaAnime = ICA_ANIME_CreateStreamingAlloc( 
                             sobj->heapID, arc_id, dat_id, buf_interval );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 3Dサウンドユニットの設定をファイルからロードする
 *
 * @param arc_id       設定するデータのアーカイブID
 * @param dat_id       設定するデータのアーカイブ内データID
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_Set3DSUnitStatus( SOUNDOBJ* sobj, ARCID arc_id, ARCDATID dat_id )
{
  ISS_3DS_UNIT* unit = ISS_3DS_SYS_GetUnit( sobj->iss3dsSys, sobj->iss3dsUnitIdx );
  ISS_3DS_UNIT_Load( unit, arc_id, dat_id, sobj->heapID );
}


//=========================================================================================
// ■動作管理
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションを進める
 *
 * @param sobj  更新対象オブジェクト
 * @param frame 進めるフレーム数
 *
 * @return ループしたら TRUE
 */
//-----------------------------------------------------------------------------------------
BOOL SOUNDOBJ_IncAnimeFrame( SOUNDOBJ* sobj, fx32 frame )
{
  BOOL loop = FALSE;

  // アニメーション未登録なら, 何もしない
  if( sobj->icaAnime == NULL )
  {
    return FALSE;
  }

  // アニメーションを更新
  loop = ICA_ANIME_IncAnimeFrame( sobj->icaAnime, frame );

  // 位置を更新
  AdjustPosition( sobj );

  // ループしたかどうかを返す
  return loop;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションフレーム数を設定する
 *
 * @param sobj  更新対象オブジェクト
 * @param frame フレーム数を指定
 */
//-----------------------------------------------------------------------------------------
void SOUNDOBJ_SetAnimeFrame( SOUNDOBJ* sobj, fx32 frame )
{
  // アニメーション未登録なら, 何もしない
  if( sobj->icaAnime == NULL )
  {
    return;
  }

  // アニメーションを更新
  ICA_ANIME_SetAnimeFrame( sobj->icaAnime, frame );

  // 位置を更新
  AdjustPosition( sobj );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーションフレーム数を取得する
 *
 * @param sobj 取得対象オブジェクト
 *
 * @return 現在のアニメーションフレーム数
 */
//-----------------------------------------------------------------------------------------
u32 SOUNDOBJ_GetAnimeFrame( SOUNDOBJ* sobj ) 
{
  // アニメーション未登録なら, 何もしない
  if( sobj->icaAnime == NULL )
  {
    return 0;
  }

  // 現在のアニメーションフレーム数を返す
  return ICA_ANIME_GetNowFrame( sobj->icaAnime );
}


//=========================================================================================
// ■取得
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief ユニットが操作するトラックのビットマスクを取得する
 *
 * @param sobj 取得対象オブジェクト
 *
 * @return 指定ユニットが操作するトラックビットマスク
 */
//-----------------------------------------------------------------------------------------
u16 SOUNDOBJ_GetTrackBit( const SOUNDOBJ* sobj )
{
  ISS_3DS_UNIT* unit;
  u16 track_bit;

  // 3Dサウンドユニットを取得
  unit = ISS_3DS_SYS_GetUnit( sobj->iss3dsSys, sobj->iss3dsUnitIdx );

  // トラックマスクを取得
  track_bit = ISS_3DS_UNIT_GetTrackBit( unit );
  return track_bit;
}


//=========================================================================================
// ■非公開関数の定義
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 拡張オブジェ・音源オブジェの位置をアニメーションに合わせる
 *
 * @param sobj 更新するオブジェクト
 */
//-----------------------------------------------------------------------------------------
static void AdjustPosition( SOUNDOBJ* sobj )
{
  VecFx32 pos;
  ISS_3DS_UNIT* unit;

  // 位置を取得
  ICA_ANIME_GetTranslate( sobj->icaAnime, &pos );

  // 3Dステータスを設定
  VEC_Set( &sobj->status->trans, pos.x, pos.y, pos.z );

  // 音源位置を合わせる
  unit = ISS_3DS_SYS_GetUnit( sobj->iss3dsSys, sobj->iss3dsUnitIdx );
  ISS_3DS_UNIT_SetPos( unit, &pos );
}
