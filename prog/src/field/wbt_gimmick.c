//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_gimmick.c
 *	@brief  WBT会場演出ギミック
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// system
#include "fieldmap.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/arc_def.h"
#include "arc/fieldmap/wbt_gmk.naix"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "wbt_gimmick.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define EXPOBJ_UNIT_IDX     (0)
#define WBT_TMP_ASSIGN_ID   (1)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック一時ワーク
//=====================================
typedef struct {

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ

  // 発動フラグ
  BOOL      trg_vision;     // 背景ビジョン
  BOOL      trg_effect;     // エフェクト

} WBT_GMK_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_DUMMY_MDL = 0,      // ダミー
  RESID_DUMMY_ICA,          // ダミー
};

// OBJインデックス
enum {
  OBJ_DUMMY = 0,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  // ダミー
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_dummy_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_dummy_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_3d_table[] =
{
  { RESID_DUMMY_ICA, 0 }, // nsbca
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  // ダミー
  {
    RESID_DUMMY_MDL,        // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_DUMMY_MDL,        // テクスチャリソースID
    anm_3d_table,           // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table),   // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

//=============================================================================
/**
 *          static関数宣言
 */
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, WBT_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(WBT_GMK_TMP) );

  p_wk->exp_obj   = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  // ギミックは発動させない
  p_wk->trg_vision = FALSE;
  p_wk->trg_effect = FALSE;

  // EXPOBJ初期化
  EXPOBJ_ObjSet( p_wk );

  ICHI_Printf("gimmick init\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, WBT_TMP_ASSIGN_ID);

  ICHI_Printf("gimmick end\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 背景ビジョン開始
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_VisionInit( FIELDMAP_WORK *p_fieldmap, u8 type )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  ICHI_Printf("背景ビジョン開始 %d \n", type);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 背景ビジョン変更
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_VisionChange( FIELDMAP_WORK *p_fieldmap, u8 type )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  ICHI_Printf("背景ビジョン変更 %d \n", type);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 通路エフェクト開始
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_EffectStart( FIELDMAP_WORK *p_fieldmap, u8 type, u8 pos )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  ICHI_Printf("通路エフェクト開始 %d pos:%d \n", type, pos);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 優勝エフェクト開始
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_WinEffectStart( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  ICHI_Printf("優勝エフェクト開始\n");
}


//=============================================================================
/**
 *        static関数
 */
//=============================================================================
//=============================================================================
/**
 *        EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk )
{

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, EXPOBJ_UNIT_IDX );

  //=====================================
  // ダミーの設定
  //
  {
    u16     obj_idx = OBJ_DUMMY;    // OBJインデックス
    VecFx32 pos     = { 0, 0, 0 };

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );

    // 座標
    status->trans = pos;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}

