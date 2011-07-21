//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_hide_ship_deck.c
 *	@brief  アジト船(甲板)ギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.21
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
#include "arc/fieldmap/hide_ship_deck.naix"

// script
#include "../../../resource/fldmapdata/flagwork/work_define.h"

// mine
#include "field_gimmick_hide_ship_deck.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define HIDE_SHIP_UNIT_IDX        (0)
#define HIDE_SHIP_TMP_ASSIGN_ID   (1)

#define GRID_HALF_SIZE  ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

// ハッチ座標
#define HATCH_1_X     ( 18 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE )   // HATCH1 X座標
#define HATCH_1_Y     ( 0 )   // HATCH1 Y座標
#define HATCH_1_Z     ( 19 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE )   // HATCH1 Z座標
#define HATCH_2_X     ( 18 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE )   // HATCH2 X座標
#define HATCH_2_Y     ( 0 )   // HATCH2 Y座標
#define HATCH_2_Z     ( 41 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE )   // HATCH2 Z座標

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

} HIDE_SHIP_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum{
  RESID_HATCH_MDL_01,     // ハッチモデル(閉まっている)
  RESID_HATCH_MDL_02,     // ハッチモデル(開いている)
};

// OBJインデックス
enum{
  OBJ_HATCH_1,
  OBJ_HATCH_2,

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_HIDE_SHIP_DECK, NARC_hide_ship_deck_hatch_off_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_DECK, NARC_hide_ship_deck_hatch_on_nsbmd, GFL_G3D_UTIL_RESARC },
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_HATCH_1
  {
    RESID_HATCH_MDL_01,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HATCH_MDL_01,   // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_HATCH_2
  {
    RESID_HATCH_MDL_02,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HATCH_MDL_02,   // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
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
static void EXPOBJ_ObjSet( HIDE_SHIP_TMP *p_wk );
static void EXPOBJ_ObjDell( HIDE_SHIP_TMP *p_wk );
static void EXPOBJ_ObjMain( HIDE_SHIP_TMP *p_wk );

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
void HIDE_SHIP_DECK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  // 汎用ワーク確保
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(HIDE_SHIP_TMP) );

  p_wk->exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap = p_fieldmap;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void HIDE_SHIP_DECK_End( FIELDMAP_WORK *p_fieldmap )
{
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void HIDE_SHIP_DECK_Move( FIELDMAP_WORK *p_fieldmap )
{
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
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
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( HIDE_SHIP_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, HIDE_SHIP_UNIT_IDX);

  //=====================================
  {
    int i;
    VecFx32 pos[ OBJ_NUM ] = { 0 };
    u16     rot[ OBJ_NUM ] = { 0 };

    // バージョン別で位置が反対
    if( GET_VERSION() == VERSION_B2 )
    {
      VEC_Set( &pos[0], HATCH_1_X, HATCH_1_Y, HATCH_1_Z );
      VEC_Set( &pos[1], HATCH_2_X, HATCH_2_Y, HATCH_2_Z );
      rot[0] = 0;
      rot[1] = 0x8000; // 下にあるハッチは180度回転させる
    }
    else
    {
      VEC_Set( &pos[0], HATCH_2_X, HATCH_2_Y, HATCH_2_Z );
      VEC_Set( &pos[1], HATCH_1_X, HATCH_1_Y, HATCH_1_Z );
      rot[0] = 0x8000; // 下にあるハッチは180度回転させる
      rot[1] = 0;
    }

    for( i=0; i<OBJ_NUM; i++ )
    {
      int obj_idx = i;

      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx );
      
      // 座標
      VEC_Set( &status->trans, pos[i].x, pos[i].y, pos[i].z );
      // 回転
      GFL_CALC3D_MTX_CreateRot( 0, rot[i], 0, &status->rotate );

      // カリングする
      FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, TRUE );
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, FALSE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( HIDE_SHIP_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( HIDE_SHIP_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}

