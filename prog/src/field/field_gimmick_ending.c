//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_ending.c
 *	@brief  エンディング
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.12.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/rezi.naix"

// mine
#include "field_gimmick_ending.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GIMMICK_UNIT_IDX        (0)
#define GIMMICK_TMP_ASSIGN_ID   (1)

// 座標
#define MAP_SIZE          (512 << FX32_SHIFT )
#define MAP_HALF_SIZE     (256 << FX32_SHIFT )
#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

// 座標
#define POS_X             (14 * FIELD_CONST_GRID_FX32_SIZE )
#define POS_Y             ( 0 )
#define POS_Z             ( 7 * FIELD_CONST_GRID_FX32_SIZE )

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ

} ENDING_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_DOOR_MDL,       // ドア
  RESID_DOOR_ANM_ICA,   //
};

// OBJインデックス
enum{
  OBJ_DOOR,             // ドア

  OBJ_NUM,
};


// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_door_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_door_01_nsbca, GFL_G3D_UTIL_RESARC },
};


// ANIME
static const GFL_G3D_UTIL_ANM anm_door[] =
{
  { RESID_DOOR_ANM_ICA, 0 },
};
#define ANM_DOOR_NUM  ( NELEMS(anm_door) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_DOOR
  {
    RESID_DOOR_MDL,   // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_DOOR_MDL,   // テクスチャリソースID
    anm_door,         // アニメテーブル(複数指定のため)
    ANM_DOOR_NUM,     // アニメリソース数
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
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( ENDING_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( ENDING_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( ENDING_GIMMICK_TMP *p_wk );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * @bried       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void ENDING_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  ENDING_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(ENDING_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );
  OS_Printf("エンディングギミックセットアップ\n");
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void ENDING_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  ENDING_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void ENDING_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  ENDING_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  花火再生
 *
 *	@param	GAMESYS_WORK *gsys
 */
//-----------------------------------------------------------------------------
void ENDING_GIMMICK_FireWorks( GAMESYS_WORK *gsys )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  ENDING_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( ENDING_GIMMICK_TMP *p_wk )
{
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // OBJ_DOOR
  {
    u16 obj_idx = OBJ_DOOR;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, POS_X, POS_Y, POS_Z );

    // カリング
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<ANM_DOOR_NUM; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, FALSE );
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( ENDING_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( ENDING_GIMMICK_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}

//=============================================================================
/**
 *          ANIME
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;
  int anm_num = ANM_DOOR_NUM;

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );

    // 有効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, TRUE );
    // 頭だし
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, 0 );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定
  }
}

