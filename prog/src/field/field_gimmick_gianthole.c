//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_gianthole.c
 *	@brief  ジャイアントホールの対決
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/gianthole.naix"

// mine
#include "field_gimmick_gianthole.h"

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

} GIANTHOLE_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_BLIZZARD_01_MDL,      // こごえるせかい 前半
  RESID_BLIZZARD_01_ANM_ICA,  // こごえるせかい 前半
  RESID_BLIZZARD_01_ANM_ITA,  // こごえるせかい 前半
  RESID_BLIZZARD_02_MDL,      // こごえるせかい 後半
  RESID_BLIZZARD_02_ANM_ICA,  // こごえるせかい 後半
  RESID_BLIZZARD_02_ANM_ITA,  // こごえるせかい 後半
};

// OBJインデックス
enum{
  OBJ_BLIZZARD_01,            // こごえるせかい 前半
  OBJ_BLIZZARD_02,            // こごえるせかい 後半

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_02_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_02_nsbta, GFL_G3D_UTIL_RESARC },
};

// ANIME
// こごえるせかい 前半
static const GFL_G3D_UTIL_ANM anm_blizzard_01[] =
{
  { RESID_BLIZZARD_01_ANM_ICA, 0 },
  { RESID_BLIZZARD_01_ANM_ITA, 0 },
};
#define ANM_BLIZZARD_01_NUM  ( NELEMS(anm_blizzard_01) )

// こごえるせかい 後半
static const GFL_G3D_UTIL_ANM anm_blizzard_02[] =
{
  { RESID_BLIZZARD_02_ANM_ICA, 0 },
  { RESID_BLIZZARD_02_ANM_ITA, 0 },
};
#define ANM_BLIZZARD_02_NUM  ( NELEMS(anm_blizzard_02) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_BLIZZARD_01
  {
    RESID_BLIZZARD_01_MDL,  // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BLIZZARD_01_MDL,  // テクスチャリソースID
    anm_blizzard_01,        // アニメテーブル(複数指定のため)
    ANM_BLIZZARD_01_NUM,    // アニメリソース数
  },
  // OBJ_BLIZZARD_02
  {
    RESID_BLIZZARD_02_MDL,  // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BLIZZARD_02_MDL,  // テクスチャリソースID
    anm_blizzard_02,        // アニメテーブル(複数指定のため)
    ANM_BLIZZARD_02_NUM,    // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

// OBJとアニメ総数の対応テーブル
static const int anm_num_tbl[ OBJ_NUM ] = {
  ANM_BLIZZARD_01_NUM,
  ANM_BLIZZARD_02_NUM,
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( GIANTHOLE_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( GIANTHOLE_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( GIANTHOLE_GIMMICK_TMP *p_wk );

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
void GIANTHOLE_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GIANTHOLE_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(GIANTHOLE_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

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
void GIANTHOLE_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

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
void GIANTHOLE_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      OS_Printf("デバッグ こごえるせかい前半\n");

      // 表示設定
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_01, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_02, TRUE );

      ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_01 );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      OS_Printf("デバッグ こごえるせかい後半\n");

      // 表示設定
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_01, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_02, TRUE );

      ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_02 );
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	id  0:前半 1:後半
 */
//-----------------------------------------------------------------------------
void GIANTHOLE_GIMMICK_AnmStart( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  GF_ASSERT( id<2 );

  if( id == 0 )
  {
    ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_01 );
  }
  else
  {
    ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_02 );
  }
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
 *	@param	GIANTHOLE_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GIANTHOLE_GIMMICK_TMP *p_wk )
{
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // OBJ_BLIZZARD_01
  {
    u16 obj_idx = OBJ_BLIZZARD_01;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, MAP_HALF_SIZE, 0, MAP_HALF_SIZE );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_BLIZZARD_01_NUM; i++ )
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

  // OBJ_BLIZZARD_02
  {
    u16 obj_idx = OBJ_BLIZZARD_02;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, MAP_HALF_SIZE, 0, MAP_HALF_SIZE );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_BLIZZARD_02_NUM; i++ )
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
 *	@param	GIANTHOLE_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GIANTHOLE_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GIANTHOLE_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GIANTHOLE_GIMMICK_TMP *p_wk )
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
  int anm_num = anm_num_tbl[ obj_idx ];

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );

    // 有効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, TRUE );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定
  }
}

