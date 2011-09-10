//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_h06.c
 *	@brief  マリンチューブ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.09.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// field
#include "ev_time.h"

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/h06.naix"

// mine
#include "field_gimmick_h06.h"
#include "field_gimmick_h06_sv.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define H06_GIMMICK_UNIT_IDX        (0)
#define H06_GIMMICK_TMP_ASSIGN_ID   (1)

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
  H06_GIMMICK_SV_WORK *p_sv_wk;       // セーブワーク

} H06_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_BASU_A1_MDL,      // バスラオ赤 前半
  RESID_BASU_A1_ANM_ICA,  // バスラオ赤 前半
  RESID_BASU_A2_MDL,      // バスラオ赤 後半
  RESID_BASU_A2_ANM_ICA,  // バスラオ赤 後半
  RESID_BASU_B1_MDL,      // バスラオ青 前半
  RESID_BASU_B1_ANM_ICA,  // バスラオ青 前半
  RESID_BASU_B2_MDL,      // バスラオ青 後半
  RESID_BASU_B2_ANM_ICA,  // バスラオ青 後半
  RESID_HOERU_MDL,        // ホエルオー
  RESID_HOERU_ANM_ICA,    // ホエルオー
  RESID_HOERU_ANM_ITA,    // ホエルオー
  RESID_MAMAN_1_MDL,      // ママンボウ 前半
  RESID_MAMAN_1_ANM_ICA,  // ママンボウ 前半
  RESID_MAMAN_2_MDL,      // ママンボウ 後半
  RESID_MAMAN_2_ANM_ICA,  // ママンボウ 後半
  RESID_MANTA_1_MDL,      // マンタイン 前半
  RESID_MANTA_1_ANM_ICA,  // マンタイン 前半
  RESID_MANTA_1_ANM_ITA,  // マンタイン 前半
  RESID_MANTA_2_MDL,      // マンタイン 後半
  RESID_MANTA_2_ANM_ICA,  // マンタイン 後半
  RESID_MANTA_2_ANM_ITA,  // マンタイン 後半
  RESID_PURU_A1_MDL,      // プルリル♂ 前半
  RESID_PURU_A1_ANM_ICA,  // プルリル♂ 前半
  RESID_PURU_A2_MDL,      // プルリル♂ 後半
  RESID_PURU_A2_ANM_ICA,  // プルリル♂ 後半
  RESID_PURU_B1_MDL,      // プルリル♀ 前半
  RESID_PURU_B1_ANM_ICA,  // プルリル♀ 前半
  RESID_PURU_B2_MDL,      // プルリル♀ 後半
  RESID_PURU_B2_ANM_ICA,  // プルリル♀ 後半
};

// OBJインデックス
enum{
  OBJ_BASU_A1,            // バスラオ赤 前半
  OBJ_BASU_A2,            // バスラオ赤 後半
  OBJ_BASU_B1,            // バスラオ青 前半
  OBJ_BASU_B2,            // バスラオ青 後半
  OBJ_HOERU,              // ホエルオー
  OBJ_MAMAN_1,            // ママンボウ 前半
  OBJ_MAMAN_2,            // ママンボウ 後半
  OBJ_MANTA_1,            // マンタイン 前半
  OBJ_MANTA_2,            // マンタイン 後半
  OBJ_PURU_A1,            // プルリル♂ 前半
  OBJ_PURU_A2,            // プルリル♂ 後半
  OBJ_PURU_B1,            // プルリル♀ 前半
  OBJ_PURU_B2,            // プルリル♀ 後半

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_H06_GIMMICK, NARC_h06_basu_a1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_a1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_a2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_a2_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_b1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_b1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_b2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_basu_b2_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_hoeru_1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_hoeru_1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_hoeru_1_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_maman_1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_maman_1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_maman_2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_maman_2_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_manta_1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_manta_1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_manta_1_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_manta_2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_manta_2_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_manta_2_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_a1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_a1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_a2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_a2_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_b1_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_b1_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_b2_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_H06_GIMMICK, NARC_h06_puru_b2_nsbca, GFL_G3D_UTIL_RESARC },
};

// ANIME
// バスラオ赤 前半
static const GFL_G3D_UTIL_ANM anm_basu_a1[] =
{
  { RESID_BASU_A1_ANM_ICA, 0 },
};
#define ANM_BASU_A1_NUM  ( NELEMS(anm_basu_a1) )

// バスラオ赤 後半
static const GFL_G3D_UTIL_ANM anm_basu_a2[] =
{
  { RESID_BASU_A2_ANM_ICA, 0 },
};
#define ANM_BASU_A2_NUM  ( NELEMS(anm_basu_a2) )

// バスラオ青 前半
static const GFL_G3D_UTIL_ANM anm_basu_b1[] =
{
  { RESID_BASU_B1_ANM_ICA, 0 },
};
#define ANM_BASU_B1_NUM  ( NELEMS(anm_basu_b1) )

// バスラオ青 後半
static const GFL_G3D_UTIL_ANM anm_basu_b2[] =
{
  { RESID_BASU_B2_ANM_ICA, 0 },
};
#define ANM_BASU_B2_NUM  ( NELEMS(anm_basu_b2) )

// ホエルオー
static const GFL_G3D_UTIL_ANM anm_hoeru[] =
{
  { RESID_HOERU_ANM_ICA, 0 },
  { RESID_HOERU_ANM_ITA, 0 },
};
#define ANM_HOERU_NUM  ( NELEMS(anm_hoeru) )

// ママンボウ 前半
static const GFL_G3D_UTIL_ANM anm_maman_1[] =
{
  { RESID_MAMAN_1_ANM_ICA, 0 },
};
#define ANM_MAMAN_1_NUM  ( NELEMS(anm_maman_1) )

// ママンボウ 後半
static const GFL_G3D_UTIL_ANM anm_maman_2[] =
{
  { RESID_MAMAN_2_ANM_ICA, 0 },
};
#define ANM_MAMAN_2_NUM  ( NELEMS(anm_maman_2) )

// マンタイン 前半
static const GFL_G3D_UTIL_ANM anm_manta_1[] =
{
  { RESID_MANTA_1_ANM_ICA, 0 },
  { RESID_MANTA_1_ANM_ITA, 0 },
};
#define ANM_MANTA_1_NUM  ( NELEMS(anm_manta_1) )

// マンタイン 後半
static const GFL_G3D_UTIL_ANM anm_manta_2[] =
{
  { RESID_MANTA_2_ANM_ICA, 0 },
  { RESID_MANTA_2_ANM_ITA, 0 },
};
#define ANM_MANTA_2_NUM  ( NELEMS(anm_manta_2) )

// プルリル♂ 前半
static const GFL_G3D_UTIL_ANM anm_puru_a1[] =
{
  { RESID_PURU_A1_ANM_ICA, 0 },
};
#define ANM_PURU_A1_NUM  ( NELEMS(anm_puru_a1) )

// プルリル♂後半
static const GFL_G3D_UTIL_ANM anm_puru_a2[] =
{
  { RESID_PURU_A2_ANM_ICA, 0 },
};
#define ANM_PURU_A2_NUM  ( NELEMS(anm_puru_a2) )

// プルリル♀ 前半
static const GFL_G3D_UTIL_ANM anm_puru_b1[] =
{
  { RESID_PURU_B1_ANM_ICA, 0 },
};
#define ANM_PURU_B1_NUM  ( NELEMS(anm_puru_b1) )

// プルリル♀ 後半
static const GFL_G3D_UTIL_ANM anm_puru_b2[] =
{
  { RESID_PURU_B2_ANM_ICA, 0 },
};
#define ANM_PURU_B2_NUM  ( NELEMS(anm_puru_b2) )


// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_BASU_A1
  {
    RESID_BASU_A1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BASU_A1_MDL,      // テクスチャリソースID
    anm_basu_a1,            // アニメテーブル(複数指定のため)
    ANM_BASU_A1_NUM,        // アニメリソース数
  },
  // OBJ_BASU_A2
  {
    RESID_BASU_A1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BASU_A2_MDL,      // テクスチャリソースID
    anm_basu_a2,            // アニメテーブル(複数指定のため)
    ANM_BASU_A2_NUM,        // アニメリソース数
  },
  // OBJ_BASU_B1
  {
    RESID_BASU_B1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BASU_B1_MDL,      // テクスチャリソースID
    anm_basu_b1,            // アニメテーブル(複数指定のため)
    ANM_BASU_B1_NUM,        // アニメリソース数
  },
  // OBJ_BASU_B2
  {
    RESID_BASU_B2_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BASU_B2_MDL,      // テクスチャリソースID
    anm_basu_b2,            // アニメテーブル(複数指定のため)
    ANM_BASU_B2_NUM,        // アニメリソース数
  },
  // OBJ_HOERU
  {
    RESID_HOERU_MDL,        // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_HOERU_MDL,        // テクスチャリソースID
    anm_hoeru,              // アニメテーブル(複数指定のため)
    ANM_HOERU_NUM,          // アニメリソース数
  },
  // OBJ_MAMAN_1
  {
    RESID_MAMAN_1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_MAMAN_1_MDL,      // テクスチャリソースID
    anm_maman_1,            // アニメテーブル(複数指定のため)
    ANM_MAMAN_1_NUM,        // アニメリソース数
  },
  // OBJ_MAMAN_2
  {
    RESID_MAMAN_2_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_MAMAN_2_MDL,      // テクスチャリソースID
    anm_maman_2,            // アニメテーブル(複数指定のため)
    ANM_MAMAN_2_NUM,        // アニメリソース数
  },
  // OBJ_MANTA_1
  {
    RESID_MANTA_1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_MANTA_1_MDL,      // テクスチャリソースID
    anm_manta_1,            // アニメテーブル(複数指定のため)
    ANM_MANTA_1_NUM,        // アニメリソース数
  },
  // OBJ_MANTA_2
  {
    RESID_MANTA_2_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_MANTA_2_MDL,      // テクスチャリソースID
    anm_manta_2,            // アニメテーブル(複数指定のため)
    ANM_MANTA_2_NUM,        // アニメリソース数
  },
  // OBJ_PURU_A1
  {
    RESID_PURU_A1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PURU_A1_MDL,      // テクスチャリソースID
    anm_puru_a1,            // アニメテーブル(複数指定のため)
    ANM_PURU_A1_NUM,        // アニメリソース数
  },
  // OBJ_PURU_A2
  {
    RESID_PURU_A2_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PURU_A2_MDL,      // テクスチャリソースID
    anm_puru_a2,            // アニメテーブル(複数指定のため)
    ANM_PURU_A2_NUM,        // アニメリソース数
  },
  // OBJ_PURU_B1
  {
    RESID_PURU_B1_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PURU_B1_MDL,      // テクスチャリソースID
    anm_puru_b1,            // アニメテーブル(複数指定のため)
    ANM_PURU_B1_NUM,        // アニメリソース数
  },
  // OBJ_PURU_B2
  {
    RESID_PURU_B2_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PURU_B2_MDL,      // テクスチャリソースID
    anm_puru_b2,            // アニメテーブル(複数指定のため)
    ANM_PURU_B2_NUM,        // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

// OBJとアニメ数の対応テーブル
static const int anm_num_tbl[ OBJ_NUM ] = {
  ANM_BASU_A1_NUM,
  ANM_BASU_A2_NUM,
  ANM_BASU_B1_NUM,
  ANM_BASU_B2_NUM,
  ANM_HOERU_NUM,
  ANM_MAMAN_1_NUM,
  ANM_MAMAN_2_NUM,
  ANM_MANTA_1_NUM,
  ANM_MANTA_2_NUM,
  ANM_PURU_A1_NUM,
  ANM_PURU_A2_NUM,
  ANM_PURU_B1_NUM,
  ANM_PURU_B2_NUM,
};


//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// セーブデータから何をだすか決める
static void EXPOBJ_Init( H06_GIMMICK_TMP *p_wk );

// EXPOBJ
static void EXPOBJ_ObjSet( H06_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( H06_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( H06_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, VecFx32 pos );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );

//-------------------------------------
///	セーブ
//=====================================
static void SAVE_SaveAnmFrm( H06_GIMMICK_TMP *p_wk );
static void SAVE_LoadAnmFrm( H06_GIMMICK_TMP *p_wk );


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
void H06_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  H06_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, H06_GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(H06_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, H06_GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_sv_wk = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_H06 );

  ICHI_Printf("時間帯:%d\n", p_wk->p_sv_wk->time_zone);

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 出現ポケモン設定
  EXPOBJ_Init( p_wk );

  // セーブ復帰
  SAVE_LoadAnmFrm( p_wk );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void H06_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  H06_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, H06_GIMMICK_TMP_ASSIGN_ID );
  
  // セーブ保存
  SAVE_SaveAnmFrm( p_wk );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, H06_GIMMICK_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void H06_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  H06_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, H06_GIMMICK_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  セーブデータから何を出現させるか決める
 *
 *	@param	H06_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_Init( H06_GIMMICK_TMP *p_wk )
{
  switch( p_wk->p_sv_wk->time_zone ){
  case TIME_ZONE_AM:
    ANM_Play( p_wk->exp_obj, OBJ_BASU_A1 );
    ANM_Play( p_wk->exp_obj, OBJ_BASU_A2 );
    ANM_Play( p_wk->exp_obj, OBJ_BASU_B1 );
    ANM_Play( p_wk->exp_obj, OBJ_BASU_B2 );
    ANM_Play( p_wk->exp_obj, OBJ_MAMAN_1 );
    ANM_Play( p_wk->exp_obj, OBJ_MAMAN_2 );
    ANM_Play( p_wk->exp_obj, OBJ_MANTA_1 );
    ANM_Play( p_wk->exp_obj, OBJ_MANTA_2 );
    break;
  case TIME_ZONE_PM:
    ANM_Play( p_wk->exp_obj, OBJ_BASU_A1 );
    ANM_Play( p_wk->exp_obj, OBJ_BASU_A2 );
    ANM_Play( p_wk->exp_obj, OBJ_BASU_B1 );
    ANM_Play( p_wk->exp_obj, OBJ_BASU_B2 );
    ANM_Play( p_wk->exp_obj, OBJ_PURU_A1 );
    ANM_Play( p_wk->exp_obj, OBJ_PURU_A2 );
    ANM_Play( p_wk->exp_obj, OBJ_PURU_B1 );
    ANM_Play( p_wk->exp_obj, OBJ_PURU_B2 );
    break;
  case TIME_ZONE_RARE:
    ANM_Play( p_wk->exp_obj, OBJ_HOERU );
    break;
  default:
    GF_ASSERT(0);
  }
}

//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	H06_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( H06_GIMMICK_TMP *p_wk )
{
  // 6ブロック分の座標
  const VecFx32   pos[7] = {
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE },  // 0_0
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE + (MAP_SIZE * 1) },  // 0_1
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE + (MAP_SIZE * 2) },  // 0_2
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE + (MAP_SIZE * 3) },  // 0_3
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE + (MAP_SIZE * 4) },  // 0_4
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE + (MAP_SIZE * 5) },  // 0_5
    { MAP_HALF_SIZE, 0, MAP_HALF_SIZE + (MAP_SIZE * 6) },  // 0_6
  };

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, H06_GIMMICK_UNIT_IDX);

  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_BASU_A1, ANM_BASU_A1_NUM, pos[3] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_BASU_A2, ANM_BASU_A2_NUM, pos[1] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_BASU_B1, ANM_BASU_B1_NUM, pos[3] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_BASU_B2, ANM_BASU_B2_NUM, pos[1] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_HOERU,   ANM_HOERU_NUM,   pos[3] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_MAMAN_1, ANM_MAMAN_1_NUM, pos[4] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_MAMAN_2, ANM_MAMAN_2_NUM, pos[1] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_MANTA_1, ANM_MANTA_1_NUM, pos[4] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_MANTA_2, ANM_MANTA_2_NUM, pos[1] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_PURU_A1, ANM_PURU_A1_NUM, pos[3] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_PURU_A2, ANM_PURU_A2_NUM, pos[1] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_PURU_B1, ANM_PURU_B1_NUM, pos[3] );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_PURU_B2, ANM_PURU_B2_NUM, pos[1] );

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	H06_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( H06_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, H06_GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	H06_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( H06_GIMMICK_TMP *p_wk )
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
 *	@param  VecFx32 pos 座標
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, VecFx32 pos )
{
  int i;
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, pos.x, pos.y, pos.z );

  // カリング
  // ※しない設定
  FLD_EXP_OBJ_SetCulling( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, FALSE );
  // 非表示
  FLD_EXP_OBJ_SetVanish( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, TRUE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, i, FALSE );
  }
}

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
  FLD_EXP_OBJ_SetVanish( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, i );

    // 有効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, H06_GIMMICK_UNIT_IDX, obj_idx, i, TRUE );

    // ループ再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定
  }
}

//=============================================================================
/**
 *        セーブ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アニメの経過フレームを保存する
 *
 *	@param	H06_GIMMICK_TMP *p_wk   ギミックワーク
 */
//-----------------------------------------------------------------------------
static void SAVE_SaveAnmFrm( H06_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_wk->exp_obj;
  H06_GIMMICK_SV_WORK *p_sv = p_wk->p_sv_wk;

  int i;

  for( i=0; i<OBJ_NUM; i++ )
  {
    int anm_no;
    int anm_num = anm_num_tbl[i];

    for( anm_no=0; anm_no<anm_num; anm_no++ )
    {
      p_sv->anm_frm[ i+anm_no ] = FLD_EXP_OBJ_GetObjAnmFrm( exp_obj, H06_GIMMICK_UNIT_IDX, i, anm_no );
      ICHI_Printf("セーブ %d anmfrm %x \n", i, p_sv->anm_frm[i]);
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アニメの経過フレームを復帰する
 *
 *	@param	H06_GIMMICK_TMP *p_wk   ギミックワーク
 */
//-----------------------------------------------------------------------------
static void SAVE_LoadAnmFrm( H06_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_wk->exp_obj;
  H06_GIMMICK_SV_WORK *p_sv = p_wk->p_sv_wk;

  int i;

  for( i=0; i<OBJ_NUM; i++ )
  {
    int anm_no;
    int anm_num = anm_num_tbl[i];

    for( anm_no=0; anm_no<anm_num; anm_no++ )
    {
     FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, H06_GIMMICK_UNIT_IDX, i, anm_no, p_sv->anm_frm[ i+anm_no ] );
      ICHI_Printf("セーブ %d anmfrm %x \n", i, p_sv->anm_frm[i]);
    }
  }
}

