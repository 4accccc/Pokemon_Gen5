//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_ghost.c
 *	@brief  四天王部屋:ゴースト
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
#include "arc/fieldmap/fourkings_ghost.naix"

// mine
#include "fourkings_sv.h"
#include "fourkings_ghost.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FOURKINGS_GHOST_UNIT_IDX        (0)
#define FOURKINGS_GHOST_TMP_ASSIGN_ID   (1)

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
} FOURKINGS_GHOST_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_BOOK_MDL = 0,       // 本モデル
  RESID_BOOK_01_ANM_ICA,    // 本アニメ(動いている)
  RESID_BOOK_02_ANM_ICA,    // 本アニメ(落ちる)
  RESID_FIRE_A_MDL,         // 炎モデル
  RESID_FIRE_A_01_ANM_ICA,  // 炎アニメ(点火)
  RESID_FIRE_A_01_ANM_IMA,  // 炎アニメ(点火)
  RESID_FIRE_A_01_ANM_ITA,  // 炎アニメ(点火)
  RESID_FIRE_A_01_ANM_ITP,  // 炎アニメ(点火)
  RESID_FIRE_A_01_ANM_IVA,  // 炎アニメ(点火)
  RESID_FIRE_A_02_ANM_ICA,  // 炎アニメ(常時)
  RESID_FIRE_A_02_ANM_IMA,  // 炎アニメ(常時)
  RESID_FIRE_A_02_ANM_ITP,  // 炎アニメ(常時)
  RESID_FIRE_A_02_ANM_IVA,  // 炎アニメ(常時)
  RESID_FIRE_B_MDL,         // 炎モデル(複数個版)
  RESID_FIRE_B_01_ANM_ICA,  // 炎アニメ(点火)
  RESID_FIRE_B_01_ANM_IMA,  // 炎アニメ(点火)
  RESID_FIRE_B_01_ANM_ITA,  // 炎アニメ(点火)
  RESID_FIRE_B_01_ANM_ITP,  // 炎アニメ(点火)
  RESID_FIRE_B_01_ANM_IVA,  // 炎アニメ(点火)
  RESID_FIRE_B_02_ANM_ICA,  // 炎アニメ(常時)
  RESID_FIRE_B_02_ANM_IMA,  // 炎アニメ(常時)
  RESID_FIRE_B_02_ANM_ITP,  // 炎アニメ(常時)
  RESID_FIRE_B_02_ANM_IVA,  // 炎アニメ(常時)
  RESID_LAMP_MDL,           // ランプモデル
  RESID_LAMP_01_ANM_ICA,    // ランプアニメ(常時)
  RESID_LAMP_01_ANM_ITP,    // ランプアニメ(常時)
  RESID_LAMP_02_ANM_ICA,    // ランプアニメ(点火)
  RESID_LAMP_02_ANM_ITP,    // ランプアニメ(点火)
  RESID_LAMP_03_ANM_ICA,    // ランプアニメ(常時)
  RESID_LAMP_03_ANM_ITP,    // ランプアニメ(常時)
  RESID_LIGHT_MDL,          // 照らすライトモデル
  RESID_LIGHT_ANM_ICA,      // 照らすライトアニメ(常時)
  RESID_LIGHT_ANM_ITA,      // 照らすライトアニメ(常時)
  RESID_MIST_MDL,           // 霧モデル
  RESID_MIST_01_ANM_ICA,    // 霧アニメ(常時)
  RESID_MIST_01_ANM_IMA,    // 霧アニメ(常時)
  RESID_MIST_01_ANM_ITA,    // 霧アニメ(常時)
  RESID_MIST_02_ANM_ICA,    // 霧アニメ(霧が晴れる)
  RESID_MIST_02_ANM_IMA,    // 霧アニメ(霧が晴れる)
  RESID_MIST_02_ANM_ITA,    // 霧アニメ(霧が晴れる)
  RESID_NOFOG_MDL,          // フォグ無効モデル
  RESID_NOFOG_01_ANM_ICA,   // フォグ無効アニメ(ランプ点火時)
  RESID_NOFOG_01_ANM_ITA,   // フォグ無効アニメ(ランプ点火時)
  RESID_NOFOG_02_ANM_ICA,   // フォグ無効アニメ(常時)
  RESID_NOFOG_02_ANM_ITA,   // フォグ無効アニメ(常時)
};

// OBJインデックス
enum {
  OBJ_BOOK,                 // 本
  OBJ_FIRE_A,               // 炎
  OBJ_FIRE_B,               // 炎
  OBJ_LAMP,                 // ランプ
  OBJ_LIGHT,                // ライト
  OBJ_MIST,                 // 霧
  OBJ_NOFOG,                // フォグ無効

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_book01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_book01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_book02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_01_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_01_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_02_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_a_02_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_01_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_01_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_02_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_fire_b_02_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp02_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp03_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_lamp03_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_light01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_light01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_light01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist01_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_mist02_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_nofog01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_nofog01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_nofog01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_nofog02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_nofog02_nsbta, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_book[] =
{
  { RESID_BOOK_01_ANM_ICA, 0 },
  { RESID_BOOK_02_ANM_ICA, 0 },
};
#define ANM_BOOK_NUM  ( NELEMS(anm_book) )

static const GFL_G3D_UTIL_ANM anm_fire_a[] =
{
  { RESID_FIRE_A_01_ANM_ICA, 0 },
  { RESID_FIRE_A_01_ANM_IMA, 0 },
  { RESID_FIRE_A_01_ANM_ITA, 0 },
  { RESID_FIRE_A_01_ANM_ITP, 0 },
  { RESID_FIRE_A_01_ANM_IVA, 0 },
  { RESID_FIRE_A_02_ANM_ICA, 0 },
  { RESID_FIRE_A_02_ANM_IMA, 0 },
  { RESID_FIRE_A_02_ANM_ITP, 0 },
  { RESID_FIRE_A_02_ANM_IVA, 0 },
};
#define ANM_FIRE_A_NUM  ( NELEMS(anm_fire_a) )

static const GFL_G3D_UTIL_ANM anm_fire_b[] =
{
  { RESID_FIRE_B_01_ANM_ICA, 0 },
  { RESID_FIRE_B_01_ANM_IMA, 0 },
  { RESID_FIRE_B_01_ANM_ITA, 0 },
  { RESID_FIRE_B_01_ANM_ITP, 0 },
  { RESID_FIRE_B_01_ANM_IVA, 0 },
  { RESID_FIRE_B_02_ANM_ICA, 0 },
  { RESID_FIRE_B_02_ANM_IMA, 0 },
  { RESID_FIRE_B_02_ANM_ITP, 0 },
  { RESID_FIRE_B_02_ANM_IVA, 0 },
};
#define ANM_FIRE_B_NUM  ( NELEMS(anm_fire_b) )

static const GFL_G3D_UTIL_ANM anm_lamp[] =
{
  { RESID_LAMP_01_ANM_ICA, 0 },
  { RESID_LAMP_01_ANM_ITP, 0 },
  { RESID_LAMP_02_ANM_ICA, 0 },
  { RESID_LAMP_02_ANM_ITP, 0 },
  { RESID_LAMP_03_ANM_ICA, 0 },
  { RESID_LAMP_03_ANM_ITP, 0 },
};
#define ANM_LAMP_NUM  ( NELEMS(anm_lamp) )

static const GFL_G3D_UTIL_ANM anm_light[] =
{
  { RESID_LIGHT_ANM_ICA, 0 },
  { RESID_LIGHT_ANM_ITA, 0 },
};
#define ANM_LIGHT_NUM  ( NELEMS(anm_light) )

static const GFL_G3D_UTIL_ANM anm_mist[] =
{
  { RESID_MIST_01_ANM_ICA, 0 },
  { RESID_MIST_01_ANM_IMA, 0 },
  { RESID_MIST_01_ANM_ITA, 0 },
  { RESID_MIST_02_ANM_ICA, 0 },
  { RESID_MIST_02_ANM_IMA, 0 },
  { RESID_MIST_02_ANM_ITA, 0 },
};
#define ANM_MIST_NUM  ( NELEMS(anm_mist) )

static const GFL_G3D_UTIL_ANM anm_nofog[] =
{
  { RESID_NOFOG_01_ANM_ICA, 0 },
  { RESID_NOFOG_01_ANM_ITA, 0 },
  { RESID_NOFOG_02_ANM_ICA, 0 },
  { RESID_NOFOG_02_ANM_ITA, 0 },
};
#define ANM_NOFOG_NUM  ( NELEMS(anm_nofog) )


// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_BOOK
  {
    RESID_BOOK_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_BOOK_MDL,       // テクスチャリソースID
    anm_book,             // アニメテーブル(複数指定のため)
    ANM_BOOK_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_A
  {
    RESID_FIRE_A_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_A_MDL,     // テクスチャリソースID
    anm_fire_a,           // アニメテーブル(複数指定のため)
    ANM_FIRE_A_NUM,       // アニメリソース数
  },
  // OBJ_FIRE_B
  {
    RESID_FIRE_B_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_B_MDL,     // テクスチャリソースID
    anm_fire_b,           // アニメテーブル(複数指定のため)
    ANM_FIRE_B_NUM,       // アニメリソース数
  },
  // OBJ_LAMP
  {
    RESID_LAMP_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_LAMP_MDL,       // テクスチャリソースID
    anm_lamp,             // アニメテーブル(複数指定のため)
    ANM_LAMP_NUM,         // アニメリソース数
  },
  // OBJ_LIGHT
  {
    RESID_LIGHT_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_MDL,      // テクスチャリソースID
    anm_light,            // アニメテーブル(複数指定のため)
    ANM_LIGHT_NUM,        // アニメリソース数
  },
  // OBJ_MIST
  {
    RESID_MIST_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_MIST_MDL,       // テクスチャリソースID
    anm_mist,             // アニメテーブル(複数指定のため)
    ANM_MIST_NUM,         // アニメリソース数
  },
  // OBJ_NOFOG
  {
    RESID_NOFOG_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_NOFOG_MDL,      // テクスチャリソースID
    anm_nofog,            // アニメテーブル(複数指定のため)
    ANM_NOFOG_NUM,        // アニメリソース数
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

// INIT
// フィールド入りなおすたびに、セーブ状態による初期化
static void INIT_FieldRecover( FOURKINGS_GHOST_TMP *p_wk );

// EXPOBJ
static void EXPOBJ_ObjSet( FOURKINGS_GHOST_TMP *p_wk );
static void EXPOBJ_ObjDell( FOURKINGS_GHOST_TMP *p_wk );
static void EXPOBJ_ObjMain( FOURKINGS_GHOST_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// BOOK
static void ANM_BOOK_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_BOOK_Play( FOURKINGS_GHOST_TMP *p_wk );
// FIRE_A
static void ANM_FIRE_A_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_FIRE_A_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_FIRE_A_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
// FIRE_B
static void ANM_FIRE_B_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_FIRE_B_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_FIRE_B_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
// LAMP
static void ANM_LAMP_PlayShake( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_LAMP_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_LAMP_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_LAMP_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
// LIGHT
static void ANM_LIGHT_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
// MIST
static void ANM_MIST_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_MIST_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_MIST_Play( FOURKINGS_GHOST_TMP *p_wk );
// NOFOG
static void ANM_NOFOG_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );

// 霧アニメイベント
static GMEVENT * MIST_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT MistEvent( GMEVENT* event, int* seq, void* wk );

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
void FOURKINGS_GHOST_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  // 汎用ワーク確保
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(FOURKINGS_GHOST_TMP) );

  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->p_sv_wk     = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_C09R0101 );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // セーブ状態別初期化
  INIT_FieldRecover( p_wk );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_GHOST_End(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_GHOST_Move(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = MIST_CreateEvent( NULL, gsys);
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("霧イベント\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
    }
  }
#endif

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
 *	@brief  セーブ状態による初期化
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void INIT_FieldRecover( FOURKINGS_GHOST_TMP *p_wk )
{
  //const BOOL pos1_flag = SAVE_GetPos1Flag( p_wk->p_sv_wk );
  //const BOOL pos2_flag = SAVE_GetPos2Flag( p_wk->p_sv_wk );

  ANM_MIST_PlayLoop( p_wk );
  ANM_BOOK_PlayLoop( p_wk );
  //ANM_LIGHT_PlayLoop( p_wk );
  ANM_LAMP_PlayShake( p_wk );

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
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( FOURKINGS_GHOST_TMP *p_wk )
{
  ICHI_Printf("resCount %d \n", g3Dutil_Setup.resCount);
  ICHI_Printf("objCount %d \n", g3Dutil_Setup.objCount);
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, FOURKINGS_GHOST_UNIT_IDX);

  //=====================================
  // 本
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_BOOK, ANM_BOOK_NUM );

  //=====================================
  // 炎
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FIRE_A, ANM_FIRE_A_NUM );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FIRE_B, ANM_FIRE_B_NUM );

  //=====================================
  // ランプ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LAMP, ANM_LAMP_NUM );

  //=====================================
  // ライト
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT, ANM_LIGHT_NUM );

  //=====================================
  // 霧
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_MIST, ANM_MIST_NUM );

  //=====================================
  // フォグ無効
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_NOFOG, ANM_NOFOG_NUM );

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( FOURKINGS_GHOST_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( FOURKINGS_GHOST_TMP *p_wk )
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
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, MAP_HALF_SIZE,0,MAP_HALF_SIZE );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, TRUE );
  // 非表示
  FLD_EXP_OBJ_SetVanish( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, TRUE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, i, FALSE );
  }
}

//=============================================================================
/**
 *          アニメ管理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param	anm_idx   アニメインデックス
 *	@param	loop      ループ再生フラグ(TRUEでループ
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ停止
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param	anm_idx   アニメインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//=============================================================================
/**
 *          本
 *
 *          アニメインデックス
 *          0 回転アニメ(loop)
 *          1 落ちるアニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  本回ってるアニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_BOOK_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_BOOK;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  本落ちるアニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_BOOK_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_BOOK;

  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
}
//=============================================================================
/**
 *          炎(１つだけ用)
 *
 *          アニメインデックス
 *          0～4  点火アニメ
 *          5～8  常時アニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  炎点火アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_A_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FIRE_A;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<=4; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
  for( i=5; i<=8; i++ )
  {
    ANM_Stop( p_wk->exp_obj, obj_idx, i );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  炎点火アニメ終了チェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_FIRE_A_CheckEnd( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FIRE_A;

  for( i=0; i<=4; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, i );
    // 終了していないアニメを発見
    if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
    {
      return FALSE;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  炎常時アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_A_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FIRE_A;

  for( i=0; i<=4; i++ )
  {
    ANM_Stop( p_wk->exp_obj, obj_idx, i );
  }
  for( i=5; i<=8; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, TRUE );
  }
}

//=============================================================================
/**
 *          炎(複数個用)
 *
 *          アニメインデックス
 *          0～4  点火アニメ
 *          5～8  常時アニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  炎点火アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_B_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FIRE_B;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<=4; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
  for( i=5; i<=8; i++ )
  {
    ANM_Stop( p_wk->exp_obj, obj_idx, i );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  炎点火アニメ終了チェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_FIRE_B_CheckEnd( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FIRE_B;

  for( i=0; i<=4; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, i );
    // 終了していないアニメを発見
    if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
    {
      return FALSE;
    }
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  炎常時アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_B_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FIRE_B;

  for( i=0; i<=4; i++ )
  {
    ANM_Stop( p_wk->exp_obj, obj_idx, i );
  }
  for( i=5; i<=8; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, TRUE );
  }
}

//=============================================================================
/**
 *          ランプ
 *
 *          アニメインデックス
 *          0,1 揺れるアニメ(loop)
 *          2,3 点火アニメ
 *          4,5 点火後常時アニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ランプ揺れるアニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_LAMP_PlayShake( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_LAMP;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 3 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 4 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 5 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランプ点火アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_LAMP_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_LAMP;

  ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 3, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 4 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 5 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランプ点火アニメ終了チェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_LAMP_CheckEnd( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_LAMP;

  for( i=2; i<=3; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, i );
    // 終了していないアニメを発見
    if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
    {
      return FALSE;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランプ常時アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_LAMP_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_LAMP;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 4, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 5, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 3 );
}

//=============================================================================
/**
 *          ライト
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ライトアニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_LIGHT_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_LIGHT;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
}

//=============================================================================
/**
 *          霧
 *
 *          アニメインデックス
 *          0～2  常時アニメ(loop)
 *          3～5  霧が晴れるアニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  霧常時アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_MIST_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_MIST;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 2, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 3 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 4 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 5 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  霧常時アニメの1周期終了チェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_MIST_CheckEnd( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_MIST;

  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, 2 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
}
//----------------------------------------------------------------------------
/**
 *	@brief  霧晴れるアニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_MIST_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_MIST;

  ANM_Play( p_wk->exp_obj, obj_idx, 3, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 4, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 5, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
}

//=============================================================================
/**
 *          FOG無効板ポリ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  FOG無効常時アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_NOFOG_PlayLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_NOFOG;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 3 );
}

//=============================================================================
/**
 *          イベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  霧アニメイベント作成
 *
 *	@param	parent  親イベント
 *	@param	gsys    ゲームシステム
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * MIST_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*  event;
  // イベント生成
  event = GMEVENT_Create( gsys, parent, MistEvent, 0 );

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  霧アニメイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MistEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  FOURKINGS_GHOST_TMP  *p_gmk_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID);

  switch(*seq){
  case 0:
    if( ANM_MIST_CheckEnd(p_gmk_wk) )
    {
      (*seq)++;
    }
    break;
  case 1:
    // 霧晴れるアニメ
    ANM_MIST_Play( p_gmk_wk );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}


