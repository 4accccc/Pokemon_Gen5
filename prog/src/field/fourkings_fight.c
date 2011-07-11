//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_fight.c
 *	@brief  四天王部屋:格闘
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.20
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
#include "arc/fieldmap/fourkings_fight.naix"

// mine
#include "fourkings_sv.h"
#include "fourkings_fight.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FOURKINGS_FIGHT_UNIT_IDX        (0)
#define FOURKINGS_FIGHT_TMP_ASSIGN_ID   (1)

// 座標
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
  FOURKINGS_SV_WORK   *p_sv_wk;       // セーブワーク
} FOURKINGS_FIGHT_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び純
enum {
  RESID_DOOR_MDL = 0,       // ドアモデル    
  RESID_DOOR_ANM_ICA,       // ドアアニメ    
  RESID_LIGHT_MDL,          // ライトモデル
  RESID_LIGHT_ANM_ICA,      // ライトアニメ
  RESID_LIGHT_ANM_ITA,      // ライトアニメ
  RESID_LIGHT_ANM_ITP,      // ライトアニメ
  RESID_STAGE_MDL,          // ステージモデル
  RESID_STAIR_MDL,          // 階段モデル
  RESID_STAIR_01_ANM_ICA,   // 階段アニメ
  RESID_STAIR_02_ANM_ICA,   // 階段アニメ
};

// OBJインデックス
enum {
  OBJ_DOOR,                 // ドア
  OBJ_LIGHT,                // ライト
  OBJ_STAGE,                // ステージ
  OBJ_STAIR,                // 階段

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_door01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_door01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stage_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stair01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stair01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stair02_nsbca, GFL_G3D_UTIL_RESARC },
};


// ANIME
static const GFL_G3D_UTIL_ANM anm_door[] =
{
  { RESID_DOOR_ANM_ICA, 0 },
};
#define ANM_DOOR_NUM  ( NELEMS(anm_door) )

static const GFL_G3D_UTIL_ANM anm_light[] =
{
  { RESID_LIGHT_ANM_ICA, 0 },
  { RESID_LIGHT_ANM_ITA, 0 },
  { RESID_LIGHT_ANM_ITP, 0 },
};
#define ANM_LIGHT_NUM  ( NELEMS(anm_light) )

static const GFL_G3D_UTIL_ANM anm_stair[] =
{
  { RESID_STAIR_01_ANM_ICA, 0 },
  { RESID_STAIR_02_ANM_ICA, 0 },
};
#define ANM_STAIR_NUM  ( NELEMS(anm_stair) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_DOOR
  {
    RESID_DOOR_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_DOOR_MDL,       // テクスチャリソースID
    anm_door,             // アニメテーブル(複数指定のため)
    ANM_DOOR_NUM,         // アニメリソース数
  },
  // OBJ_LIGHT
  {
    RESID_LIGHT_MDL,       // モデルリソースID
    0,                     // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_MDL,       // テクスチャリソースID
    anm_light,             // アニメテーブル(複数指定のため)
    ANM_LIGHT_NUM,         // アニメリソース数
  },
  // OBJ_STAGE
  {
    RESID_STAGE_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_STAGE_MDL,      // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_STAIR
  {
    RESID_STAIR_MDL,       // モデルリソースID
    0,                     // モデルデータID(リソース内部INDEX)
    RESID_STAIR_MDL,       // テクスチャリソースID
    anm_stair,             // アニメテーブル(複数指定のため)
    ANM_STAIR_NUM,         // アニメリソース数
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
static void EXPOBJ_ObjSet( FOURKINGS_FIGHT_TMP *p_wk );
static void EXPOBJ_ObjDell( FOURKINGS_FIGHT_TMP *p_wk );
static void EXPOBJ_ObjMain( FOURKINGS_FIGHT_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num );

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
void FOURKINGS_FIGHT_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  // 汎用ワーク確保
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(FOURKINGS_FIGHT_TMP) );

  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->p_sv_wk     = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_C09R0301 );

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
void FOURKINGS_FIGHT_End(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_FIGHT_Move(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );
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
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( FOURKINGS_FIGHT_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, FOURKINGS_FIGHT_UNIT_IDX);

  //=====================================
  // ドア
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_DOOR, ANM_DOOR_NUM );

  //=====================================
  // ライト
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT, ANM_LIGHT_NUM );

  //=====================================
  // ステージ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_STAGE, 0 );

  //=====================================
  // 階段
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_STAIR, ANM_STAIR_NUM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( FOURKINGS_FIGHT_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( FOURKINGS_FIGHT_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ初期設定
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj 3D拡張OBJポインタ
 *	@param  u16 obj_idx OBJインデックス
 *	@param	int anm_num アニメ数
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num )
{
  int i;
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, MAP_HALF_SIZE,0,MAP_HALF_SIZE );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, TRUE );
  // 非表示
  FLD_EXP_OBJ_SetVanish( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, TRUE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i, FALSE );
  }
}


