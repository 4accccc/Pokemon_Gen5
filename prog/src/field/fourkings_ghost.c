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
#include "system/wipe.h"

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

// 階段横の炎座標
#define FIRE_B_1_X        (0 << FX32_SHIFT )
#define FIRE_B_1_Y        (0 << FX32_SHIFT )
#define FIRE_B_1_Z        (-8 << FX32_SHIFT )
#define FIRE_B_2_X        (0 << FX32_SHIFT )
#define FIRE_B_2_Y        (40 << FX32_SHIFT )
#define FIRE_B_2_Z        (-48 << FX32_SHIFT )
#define FIRE_B_3_X        (0 << FX32_SHIFT )
#define FIRE_B_3_Y        (80 << FX32_SHIFT )
#define FIRE_B_3_Z        (-88 << FX32_SHIFT )
#define FIRE_B_4_X        (0 << FX32_SHIFT )
#define FIRE_B_4_Y        (120 << FX32_SHIFT )
#define FIRE_B_4_Z        (-128 << FX32_SHIFT )
#define FIRE_B_NUM        (4)   // FIRE_Bの数

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

  // FOGを無効にするためだけのフラグ
  // ギミックの初期化でなく、動作でやらないといけない
  BOOL                fog_off;        // TRUEでFOG無効にする
} FOURKINGS_GHOST_TMP;

//-------------------------------------
///	炎アニメ再生用イベントワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  int                 id;             // 炎IDX
} FIRE_EVENT_WORK;

//-------------------------------------
///	本落ちるイベント用ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  int                 wait;           // 暗転までのウェイト値
} BOOK_FALL_EVENT_WORK;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_BOOK_MDL = 0,       // 本モデル
  RESID_BOOK_01_ANM_ICA,    // 本アニメ(動いている)
  RESID_BOOK_01_ANM_ITP,    // 本アニメ(動いている)
  RESID_BOOK_02_ANM_ICA,    // 本アニメ(落ちる)
  RESID_BOOK_02_ANM_ITP,    // 本アニメ(動いている)
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
  OBJ_FIRE_B_1,             // 炎
  OBJ_FIRE_B_2,             // 炎
  OBJ_FIRE_B_3,             // 炎
  OBJ_FIRE_B_4,             // 炎
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
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_book01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_book02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_GHOST, NARC_fourkings_ghost_book02_nsbtp, GFL_G3D_UTIL_RESARC },
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
  { RESID_BOOK_01_ANM_ITP, 0 },
  { RESID_BOOK_02_ANM_ICA, 0 },
  { RESID_BOOK_02_ANM_ITP, 0 },
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
  // OBJ_FIRE_B_1
  {
    RESID_FIRE_B_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_B_MDL,     // テクスチャリソースID
    anm_fire_b,           // アニメテーブル(複数指定のため)
    ANM_FIRE_B_NUM,       // アニメリソース数
  },
  // OBJ_FIRE_B_2
  {
    RESID_FIRE_B_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_B_MDL,     // テクスチャリソースID
    anm_fire_b,           // アニメテーブル(複数指定のため)
    ANM_FIRE_B_NUM,       // アニメリソース数
  },
  // OBJ_FIRE_B_3
  {
    RESID_FIRE_B_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_B_MDL,     // テクスチャリソースID
    anm_fire_b,           // アニメテーブル(複数指定のため)
    ANM_FIRE_B_NUM,       // アニメリソース数
  },
  // OBJ_FIRE_B_4
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
static BOOL ANM_BOOK_CheckEndLoop( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_BOOK_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_BOOK_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_BOOK_FallDown( FOURKINGS_GHOST_TMP *p_wk );
// FIRE_A
static void ANM_FIRE_A_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_FIRE_A_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_FIRE_A_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
// FIRE_B
static void ANM_FIRE_B_Play( FOURKINGS_GHOST_TMP *p_wk, const int id );
static BOOL ANM_FIRE_B_CheckEnd( FOURKINGS_GHOST_TMP *p_wk, const int id );
static void ANM_FIRE_B_PlayLoop( FOURKINGS_GHOST_TMP *p_wk, const int id );
// LAMP
static void ANM_LAMP_PlayShake( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_LAMP_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_LAMP_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_LAMP_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
// LIGHT
static void ANM_LIGHT_Play( FOURKINGS_GHOST_TMP *p_wk );
// MIST
static void ANM_MIST_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_MIST_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_MIST_Play( FOURKINGS_GHOST_TMP *p_wk );
// NOFOG
static void ANM_NOFOG_Play( FOURKINGS_GHOST_TMP *p_wk );
static BOOL ANM_NOFOG_CheckEnd( FOURKINGS_GHOST_TMP *p_wk );
static void ANM_NOFOG_PlayLoop( FOURKINGS_GHOST_TMP *p_wk );

// 霧アニメイベント
static GMEVENT * MIST_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT MistEvent( GMEVENT* event, int* seq, void* wk );
// 炎アニメイベント
static GMEVENT * FIRE_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const int id );
static GMEVENT_RESULT FireEventA( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT FireEventB( GMEVENT* event, int* seq, void* wk );
// 本落ちる→暗転イベント
static GMEVENT * BOOK_FALL_CreateEvent( GAMESYS_WORK* gsys, int wait );
static GMEVENT_RESULT BookFallEvent( GMEVENT* event, int* seq, void* wk );
// ランプアニメイベント
static GMEVENT * LAMP_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT LampEvent( GMEVENT* event, int* seq, void* wk );


//-------------------------------------
///	セーブ情報
//=====================================
static void SAVE_SetPos1Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetPos1Flag( const FOURKINGS_SV_WORK *p_wk );
static void SAVE_SetPos2Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetPos2Flag( const FOURKINGS_SV_WORK *p_wk );

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
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = FIRE_CreateEvent( NULL, gsys, 0);
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("炎イベント\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = BOOK_FALL_CreateEvent( gsys, 10 );
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("本落ちるイベント\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = LAMP_CreateEvent( NULL, gsys);
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("霧イベント\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      FIELD_FOG_WORK  *fog  = FIELDMAP_GetFieldFog( p_wk->p_fieldmap );
      FIELD_FOG_SetFlag( fog, FALSE );
      FIELD_FOG_Reflect( fog );
      ICHI_Printf("fog 無効 \n");
    }
  }
#endif

  // FOGを無効
  // セットアップで行うと天気で上書きされてしまうため
  // 動作関数で行う
  if( p_wk->fog_off )
  {
    FIELD_FOG_WORK  *fog = FIELDMAP_GetFieldFog( p_wk->p_fieldmap );

    // 天気タスクで変更されているので、実際にOFFになるまでフラグは変えない
    if( FIELD_FOG_GetFlag( fog ) == FALSE )
    {
      p_wk->fog_off = FALSE;
    }

		FIELD_FOG_SetFlag( fog, FALSE );
    ICHI_Printf("fog 無効 \n");
  }

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//=============================================================================
/**
 *          スクリプトコマンド関係
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  もや晴れるアニメイベント作成
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* FOURKINGS_GHOST_MistAnm( GAMESYS_WORK *gsys )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );

  // イベント作成
  event = MIST_CreateEvent( NULL, gsys );

  // 下段のPOSイベントを踏んだ
  if( event != NULL )
  {
    SAVE_SetPos1Flag( p_wk->p_sv_wk, TRUE );
  }

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	id  0:fire_aの炎再生
 *	            1～4:fire_bの炎再生
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* FOURKINGS_GHOST_FireAnm( GAMESYS_WORK *gsys, int id )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );

  // イベント作成
  event = FIRE_CreateEvent( NULL, gsys, id );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  本が落ちる→暗転までのイベント
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	wait  暗転までのウェイト値
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* FOURKINGS_GHOST_BookFall( GAMESYS_WORK *gsys, int wait )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );

  // イベント作成
  event = BOOK_FALL_CreateEvent( gsys, wait );

  // 上段のPOSイベントを踏んだ
  if( event != NULL )
  {
    SAVE_SetPos2Flag( p_wk->p_sv_wk, TRUE );
  }

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランプアニメ再生イベント
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* FOURKINGS_GHOST_LampAnm( GAMESYS_WORK *gsys )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_GHOST_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID );

  // イベント作成
  event = LAMP_CreateEvent( NULL, gsys );

  return event;
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
  const BOOL pos1_flag = SAVE_GetPos1Flag( p_wk->p_sv_wk );
  const BOOL pos2_flag = SAVE_GetPos2Flag( p_wk->p_sv_wk );

  // セーブ状況別の状態
  // 何もPOS踏んでない
  // ⇒本ループ・炎非表示・ランプゆれ・もや表示・フォグ非表示
  // 下段POSふんだ
  // ⇒本ループ・炎表示・ランプゆれ・もや非表示・フォグ非表示
  // 上段POSふんだ
  // ⇒本おちてる・炎非表示・ランプ点灯・もや非表示・フォグ表示
  //

  // 何も踏んでいない
  if( pos1_flag == FALSE && pos2_flag == FALSE )
  {
    // ライト表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, OBJ_LIGHT, FALSE );

    ANM_BOOK_PlayLoop( p_wk );      // 本ループ
    ANM_LAMP_PlayShake( p_wk );     // ランプゆれ
    ANM_MIST_PlayLoop( p_wk );      // もや表示
    p_wk->fog_off = TRUE;           // FOG無効
  }
  // 下段POS踏んだ
  else if( pos1_flag == TRUE && pos2_flag == FALSE )
  {
    // ライト表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, OBJ_LIGHT, FALSE );
    ANM_BOOK_PlayLoop( p_wk );      // 本ループ
    ANM_LAMP_PlayShake( p_wk );     // ランプゆれ
   
    ANM_FIRE_A_PlayLoop( p_wk );    // 炎
    ANM_FIRE_B_PlayLoop( p_wk, 0 ); // 炎
    ANM_FIRE_B_PlayLoop( p_wk, 1 ); // 炎
    ANM_FIRE_B_PlayLoop( p_wk, 2 ); // 炎
    ANM_FIRE_B_PlayLoop( p_wk, 3 ); // 炎
    p_wk->fog_off = TRUE;           // FOG無効
  }
  // 上段POS踏んだ
  else if( pos2_flag == TRUE )
  {
    ANM_BOOK_FallDown( p_wk );      // 本おちている
    ANM_LAMP_PlayLoop( p_wk );      // ランプ点灯
    ANM_NOFOG_PlayLoop( p_wk );     // フォグ表示
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
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FIRE_B_1, ANM_FIRE_B_NUM );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FIRE_B_2, ANM_FIRE_B_NUM );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FIRE_B_3, ANM_FIRE_B_NUM );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FIRE_B_4, ANM_FIRE_B_NUM );

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

  // 炎Bだけ座標を設定する
  {
    int i;
    VecFx32 pos[ FIRE_B_NUM ] = {
      { FIRE_B_1_X, FIRE_B_1_Y, FIRE_B_1_Z },
      { FIRE_B_2_X, FIRE_B_2_Y, FIRE_B_2_Z },
      { FIRE_B_3_X, FIRE_B_3_Y, FIRE_B_3_Z },
      { FIRE_B_4_X, FIRE_B_4_Y, FIRE_B_4_Z },
    };


    for( i=0; i<FIRE_B_NUM; i++ )
    {
      int obj_idx = OBJ_FIRE_B_1 + i;
      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx );

      // 座標
      status->trans.x = pos[i].x + MAP_HALF_SIZE;
      status->trans.y = pos[i].y;
      status->trans.z = pos[i].z + MAP_HALF_SIZE;
    }
  }
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
 *          1 回転アニメ(loop)
 *          2 落ちるアニメ
 *          3 落ちるアニメ
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
  ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 3 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  本回ってるアニメ1周期終わりチェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_BOOK_CheckEndLoop( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_BOOK;

  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, 0 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
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
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
  ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 3, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  本落ちるアニメ終了チェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk ギミックワーク 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_BOOK_CheckEnd( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_BOOK;

  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, 2 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
}
//----------------------------------------------------------------------------
/**
 *	@brief  本落ちてる状態で表示
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_BOOK_FallDown( FOURKINGS_GHOST_TMP *p_wk )
{
  fx32 last_frame;
  u16 obj_idx = OBJ_BOOK;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, 2 );

  FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, 2, TRUE );
  // 最終フレーム取得
  last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  // ラストフレームにする
  FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, 2, last_frame );
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

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

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
 *	@param  id  0～3    炎インデックス
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_B_Play( FOURKINGS_GHOST_TMP *p_wk, const int id )
{
  int i;
  u16 obj_idx;

  GF_ASSERT( id < FIRE_B_NUM );

  // どの炎OBJ
  obj_idx = OBJ_FIRE_B_1 + id;

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
 *	@param  id  0～3    炎インデックス
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_FIRE_B_CheckEnd( FOURKINGS_GHOST_TMP *p_wk, const int id )
{
  int i;
  u16 obj_idx;

  GF_ASSERT( id < FIRE_B_NUM );

  // どの炎OBJ
  obj_idx = OBJ_FIRE_B_1 + id;

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
 *	@param  id  0～3    炎インデックス
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_B_PlayLoop( FOURKINGS_GHOST_TMP *p_wk, const int id )
{
  int i;
  u16 obj_idx;

  GF_ASSERT( id < FIRE_B_NUM );

  // どの炎OBJ
  obj_idx = OBJ_FIRE_B_1 + id;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

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
static void ANM_LIGHT_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_LIGHT;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
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
 *	@brief  FOG無効点灯アニメ再生
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_NOFOG_Play( FOURKINGS_GHOST_TMP *p_wk )
{
  u16 obj_idx = OBJ_NOFOG;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 3 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  FOG無効点灯アニメ再生終了チェック
 *
 *	@param	FOURKINGS_GHOST_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_NOFOG_CheckEnd( FOURKINGS_GHOST_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_NOFOG;

  for( i=0; i<=1; i++ )
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

  ANM_Play( p_wk->exp_obj, obj_idx, 2, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 3, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
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


//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメ再生イベント作成
 *
 *	@param	GMEVENT* parent 親イベント
 *	@param	gsys            ゲームシステム
 *	@param  id              炎ID
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * FIRE_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const int id )
{
  GMEVENT*          event;
  FIRE_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント生成
  if( id == 0 )
  {
    event = GMEVENT_Create( gsys, parent, FireEventA, sizeof(FIRE_EVENT_WORK) );
  }
  else
  {
    event = GMEVENT_Create( gsys, parent, FireEventB, sizeof(FIRE_EVENT_WORK) );
  }

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  work->p_fieldmap  = p_fieldmap;
  work->id          = id;

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメイベント関数  FIRE_A版
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT FireEventA( GMEVENT* event, int* seq, void* wk )
{
  FIRE_EVENT_WORK *p_wk = (FIRE_EVENT_WORK*)wk;
  FOURKINGS_GHOST_TMP  *p_gmk_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID);

  switch(*seq){
  case 0:
    ANM_FIRE_A_Play( p_gmk_wk );
    ANM_LIGHT_Play( p_gmk_wk );
    (*seq)++;
    break;
  case 1:
    // アニメ終了チェック
    if( ANM_FIRE_A_CheckEnd( p_gmk_wk ) )
    {
      (*seq)++;
    }
    break;
  case 2:
    ANM_FIRE_A_PlayLoop( p_gmk_wk );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメイベント関数  FIRE_B版
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT FireEventB( GMEVENT* event, int* seq, void* wk )
{
  FIRE_EVENT_WORK *p_wk = (FIRE_EVENT_WORK*)wk;
  FOURKINGS_GHOST_TMP  *p_gmk_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID);

  // -1 はOBJIDXを0はじまりにするため
  switch(*seq){
  case 0:
    ANM_FIRE_B_Play( p_gmk_wk, p_wk->id - 1 );
    (*seq)++;
    break;
  case 1:
    // アニメ終了チェック
    if( ANM_FIRE_B_CheckEnd( p_gmk_wk, p_wk->id - 1 ) )
    {
      (*seq)++;
    }
    break;
  case 2:
    ANM_FIRE_B_PlayLoop( p_gmk_wk, p_wk->id - 1 );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  本落ちるイベント作成
 *
 *	@param	GMEVENT* parent 親イベント
 *	@param	gsys            ゲームシステム
 *	@param  wait            暗転までのウェイト
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * BOOK_FALL_CreateEvent( GAMESYS_WORK* gsys, int wait )
{
  GMEVENT*          event;
  BOOK_FALL_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, BookFallEvent, sizeof(BOOK_FALL_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  work->p_fieldmap  = p_fieldmap;
  work->wait        = wait;

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  本落ちるイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT BookFallEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  BOOK_FALL_EVENT_WORK *p_wk = (BOOK_FALL_EVENT_WORK*)wk;
  FOURKINGS_GHOST_TMP  *p_gmk_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID);


  // 処理シーケンス
  enum {
    SEQ_BOOK_FALL,      // 本落ちるアニメ
    SEQ_BOOK_FALL_WAIT, // 本落ちるアニメ終了待ち
    SEQ_WAIT,           // 暗転待ち
    SEQ_FADE_OUT,       // フェードアウト
    SEQ_FADE_OUT_WAIT,  // フェードアウト待ち
    SEQ_FADE_IN,        // フェードイン
    SEQ_FADE_IN_WAIT,   // フェードイン待ち
    SEQ_END,
  };
  
  switch( *seq ){
  // 本落ちるアニメ
  case SEQ_BOOK_FALL:
    if( ANM_BOOK_CheckEndLoop( p_gmk_wk ) )
    {
      ANM_BOOK_Play( p_gmk_wk );
      *seq = SEQ_BOOK_FALL_WAIT;
    }
    break;
  // 本落ちるアニメ終了待ち
  case SEQ_BOOK_FALL_WAIT:
    if( ANM_BOOK_CheckEnd( p_gmk_wk ) )
    {
      *seq = SEQ_WAIT;
    }
    break;
  // 暗転待ち
  case SEQ_WAIT:
    {
      p_wk->wait--;
      if( p_wk->wait <= 0 )
      {
        *seq = SEQ_FADE_OUT;
      }
    }
    break;
  // フェードアウト
  case SEQ_FADE_OUT:
    WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
        WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, FIELDMAP_GetHeapID(p_wk->p_fieldmap) );
    //GFL_FADE_SetFadeSpeed( 10 );
		//GMEVENT_CallEvent(event, EVENT_FieldFadeOut_Black(gsys, p_wk->p_fieldmap, FIELD_FADE_WAIT));
    *seq = SEQ_FADE_OUT_WAIT;
    p_wk->wait = 60;
    break;
  // フェードアウト待ち
  case SEQ_FADE_OUT_WAIT:
    {
      if( WIPE_SYS_EndCheck() == FALSE )
      {
        break;
      }
      p_wk->wait--;
      if( p_wk->wait <= 0 )
      {
        // FOG有効にする
        {
          FIELD_FOG_WORK  *fog  = FIELDMAP_GetFieldFog( p_wk->p_fieldmap );
          FIELD_FOG_SetFlag( fog, TRUE );
          FIELD_FOG_Reflect( fog );
          ICHI_Printf("fog 有効 \n");
        }
        // ライトを専用にする
        {
        }
        // fog無効のモデル表示
        FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_GHOST_UNIT_IDX, OBJ_NOFOG, FALSE );
        *seq = SEQ_FADE_IN;
      }
    }
    break;
  // フェードイン
  case SEQ_FADE_IN:
    WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
        WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, FIELDMAP_GetHeapID(p_wk->p_fieldmap) );
		//GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, p_wk->p_fieldmap, FIELD_FADE_BLACK, FIELD_FADE_WAIT, FALSE, 0, 0 ));
    *seq = SEQ_FADE_IN_WAIT;
    break;
  // フェードイン待ち
  case SEQ_FADE_IN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      *seq = SEQ_END;
    }
    break;
  // 終了
  case SEQ_END:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランプアニメイベント作成
 *
 *	@param	parent  親イベント
 *	@param	gsys    ゲームシステム
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * LAMP_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*  event;
  // イベント生成
  event = GMEVENT_Create( gsys, parent, LampEvent, 0 );

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランプアニメイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT LampEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  FOURKINGS_GHOST_TMP  *p_gmk_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_GHOST_TMP_ASSIGN_ID);

  switch(*seq){
  case 0:
    // ランプ点灯アニメ
    ANM_LAMP_Play( p_gmk_wk );
    // NOFOG
    ANM_NOFOG_Play( p_gmk_wk );
    (*seq)++;
    break;

  case 1:
    // アニメ終了チェック
    if( ANM_LAMP_CheckEnd(p_gmk_wk) && ANM_NOFOG_CheckEnd(p_gmk_wk) )
    {
      (*seq)++;
    }
    break;
  case 2:
    ANM_LAMP_PlayLoop( p_gmk_wk );
    ANM_NOFOG_PlayLoop( p_gmk_wk );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *          セーブ情報
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント1起動フラグを設定
 *
 *	@param	p_wk  セーブワーク
 *	@param	flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static void SAVE_SetPos1Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag )
{
  p_wk->pos1_flag = flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント1起動フラグを取得
 *
 *	@param	p_wk  セーブワーク
 *
 *	@return flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetPos1Flag( const FOURKINGS_SV_WORK *p_wk )
{
  return p_wk->pos1_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント2起動フラグを設定
 *
 *	@param	p_wk  セーブワーク
 *	@param	flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static void SAVE_SetPos2Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag )
{
  p_wk->pos2_flag = flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント2起動フラグを取得
 *
 *	@param	p_wk  セーブワーク
 *
 *	@return flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetPos2Flag( const FOURKINGS_SV_WORK *p_wk )
{
  return p_wk->pos2_flag;
}

