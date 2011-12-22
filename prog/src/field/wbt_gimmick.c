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
#include "system/font2tex.h"

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// script
#include "script_def.h"

// arc
#include "arc/arc_def.h"
#include "arc/fieldmap/wbt_gmk.naix"
#include "arc/face_gra.naix"
#include "font/font.naix"
#include "message.naix"

// sound
#include "sound/pm_sndsys.h"

// tool
#include "wbt_tool.h"
#include "tr_tool/trtype_def.h"

// debug
#include "debug/debug_str_conv.h"

// mine
#include "wbt_gimmick.h"


//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define WBT_DEBUG_MENU
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define EXPOBJ_UNIT_IDX         (0)
#define EXPOBJ_UNIT_IDX_EFFECT  (2)

#define WBT_TMP_ASSIGN_ID   (1)

#define VISION_X  (368 << FX32_SHIFT)
#define VISION_Y  (0)
#define VISION_Z  (160 << FX32_SHIFT)
#define EFFECT_X  (256 << FX32_SHIFT)
#define EFFECT_X_MARGIN   (EFFECT_X*2)    // 差調整
#define EFFECT_Y  (0)
#define EFFECT_Z  (256 << FX32_SHIFT)

#define RES_E_NUM (4)   // 片方の通路エフェクトのリソース総数

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック一時ワーク
//=====================================
typedef struct {
  HEAPID              heapID;         // ヒープID

  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  PLAYER_WORK         *p_player_wk;

  // ユニット追加フラグ
  BOOL      is_unit_effect;     // EXPOBJ_UNIT_IDX_EFFECT

  // 発動フラグ
  BOOL      move_e_win;         // 優勝エフェクト

  // trtype
  u16       trtype1;
  u16       trtype2;

  // 顔グラフィックテクスチャ転送用
  GFL_BMP_DATA* img_bitmap;
  NNSG2dCharacterData *chr;
  NNSG2dPaletteData   *plt;
  void *chr_buf;
  void *pal_buf;

#ifdef WBT_DEBUG_MENU
  STRBUF* trtype_name;
  GFL_MSGDATA *trtype_msg;
#endif

} WBT_GMK_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
// naixの並び順と同じ
enum {
  RESID_V_VS_MDL = 0,       // VS
  RESID_V_VS_ITA,           // VS
  RESID_V_VS_ICA,           // VS
  RESID_V_VS_NAME_MDL,      // VS 名前のところ
  RESID_V_WIN_MDL,          // 優勝
  RESID_V_WIN_ITA,          // 優勝
  RESID_V_WIN_ICA,          // 優勝
  RESID_V_WIN_NAME_MDL,     // 優勝 名前のところ
  RESID_V_LOGO_MDL,         // ロゴ
  RESID_V_LOGO_ITA,         // ロゴ

  // 優勝エフェクト
  RESID_E_WIN_A_MDL,        // 優勝 開始
  RESID_E_WIN_A_ICA,        // 優勝 開始
  RESID_E_WIN_A_ITA,        // 優勝 開始
  RESID_E_WIN_B_MDL,        // 優勝 ループ
  RESID_E_WIN_B_ICA,        // 優勝 ループ
  RESID_E_WIN_B_ITA,        // 優勝 ループ

  // 照明
  RESID_LIGHT_ON_L_MDL,     // 照明ON左
  RESID_LIGHT_ON_R_MDL,     // 照明ON右
  RESID_LIGHT_OFF_L_MDL,    // 照明OFF左
  RESID_LIGHT_OFF_R_MDL,    // 照明OFF右

  // 爆破エフェクト
  RESID_E_VS1_L_MDL,        // 対戦前用爆破エフェクト 左
  RESID_E_VS1_L_ICA,        // 対戦前用爆破エフェクト 左
  RESID_E_VS1_L_IMA,        // 対戦前用爆破エフェクト 左
  RESID_E_VS1_L_ITA,        // 対戦前用爆破エフェクト 左
  RESID_E_VS1_R_MDL,        // 対戦前用爆破エフェクト 右
  RESID_E_VS1_R_ICA,        // 対戦前用爆破エフェクト 右
  RESID_E_VS1_R_IMA,        // 対戦前用爆破エフェクト 右
  RESID_E_VS1_R_ITA,        // 対戦前用爆破エフェクト 右
  RESID_E_VS2_MDL,          // 対戦後用爆破エフェクト
  RESID_E_VS2_ITA,          // 対戦後用爆破エフェクト
  RESID_E_VS3_MDL,          // 決勝戦対戦前用爆破エフェクト
  RESID_E_VS3_ICA,          // 決勝戦対戦前用爆破エフェクト
  RESID_E_VS3_IMA,          // 決勝戦対戦前用爆破エフェクト
  RESID_E_VS3_ITA,          // 決勝戦対戦前用爆破エフェクト

  // ※ここから上の順番は変えない

  // エフェクト
  RESID_E_NORMAL_L_MDL,     // ノーマル
  RESID_E_NORMAL_L_IMA,     // ノーマル
  RESID_E_NORMAL_L_ICA,     // ノーマル
  RESID_E_NORMAL_L_ITA,     // ノーマル
  RESID_E_NORMAL_R_MDL,     // ノーマル
  RESID_E_NORMAL_R_IMA,     // ノーマル
  RESID_E_NORMAL_R_ICA,     // ノーマル
  RESID_E_NORMAL_R_ITA,     // ノーマル
  // 本当は↑のように書くと分かりやすいが、長くなるので
  // ↓の記述にする。意味は同じ。
  RESID_E_FIRE,             // 炎
  RESID_E_WATER = RESID_E_FIRE + (RES_E_NUM*2),   // 水
  RESID_E_ELECT = RESID_E_WATER + (RES_E_NUM*2),  // 電気
  RESID_E_GRASS = RESID_E_ELECT + (RES_E_NUM*2),  // 草
  RESID_E_PSYCHC = RESID_E_GRASS + (RES_E_NUM*2), // エスパー
  RESID_E_FIGHT = RESID_E_PSYCHC + (RES_E_NUM*2), // 格闘
  RESID_E_POISON = RESID_E_FIGHT + (RES_E_NUM*2), // 毒
  RESID_E_GROUND = RESID_E_POISON + (RES_E_NUM*2),// 地面
  RESID_E_FLYING = RESID_E_GROUND + (RES_E_NUM*2),// 飛行
  RESID_E_DRAGON = RESID_E_FLYING + (RES_E_NUM*2),// ドラゴン
  RESID_E_BUG = RESID_E_DRAGON + (RES_E_NUM*2),   // 虫
  RESID_E_ROCK = RESID_E_BUG + (RES_E_NUM*2),     // 岩
  RESID_E_GHOST = RESID_E_ROCK + (RES_E_NUM*2),   // ゴースト
  RESID_E_ICE = RESID_E_GHOST + (RES_E_NUM*2),    // 氷
  RESID_E_STEEL = RESID_E_ICE + (RES_E_NUM*2),    // 鋼
  RESID_E_DARK = RESID_E_STEEL + (RES_E_NUM*2),   // 悪
  RESID_E_HRS = RESID_E_DARK + (RES_E_NUM*2),     // ホドモエ/レンタル/シャッフル
  RESID_E_WCSL = RESID_E_HRS + (RES_E_NUM*2),     // WCS/地方リーダーズ
  RESID_E_WM = RESID_E_WCSL + (RES_E_NUM*2),      // ワールド/マスター
  RESID_E_CHAMP1_L_MDL = RESID_E_WM + (RES_E_NUM*2),// チャンピオンズ
  RESID_E_CHAMP1_L_ICA,     // チャンピオンズ
  RESID_E_CHAMP1_R_MDL,     // チャンピオンズ
  RESID_E_CHAMP1_R_ICA,     // チャンピオンズ
  RESID_E_CHAMP2_L_MDL,     // チャンピオンズ
  RESID_E_CHAMP2_L_IMA,     // チャンピオンズ
  RESID_E_CHAMP2_L_ICA,     // チャンピオンズ
  RESID_E_CHAMP2_L_ITA,     // チャンピオンズ
  RESID_E_CHAMP2_R_MDL,     // チャンピオンズ
  RESID_E_CHAMP2_R_IMA,     // チャンピオンズ
  RESID_E_CHAMP2_R_ICA,     // チャンピオンズ
  RESID_E_CHAMP2_R_ITA,     // チャンピオンズ
};

// リソース指定の基準値
#define RESID_E_START_MDL   ( RESID_E_NORMAL_L_MDL )
#define RESID_E_START_IMA   ( RESID_E_NORMAL_L_IMA )
#define RESID_E_START_ICA   ( RESID_E_NORMAL_L_ICA )
#define RESID_E_START_ITA   ( RESID_E_NORMAL_L_ITA )

// OBJインデックス
// ユニット別
enum {
  OBJ_V_VS = 0,       // VS
  OBJ_V_VS_NAME,      // VS 名前
  OBJ_V_WIN,          // 優勝ビジョン
  OBJ_V_WIN_NAME,     // 優勝ビジョン 名前
  OBJ_V_LOGO,         // ロゴ
  OBJ_E_WIN_START,    // 優勝エフェクト開始
  OBJ_E_WIN_LOOP,     // 優勝エフェクトループ
  OBJ_LIGHT_ON_L,     // 照明ON左
  OBJ_LIGHT_ON_R,     // 照明ON右
  OBJ_LIGHT_OFF_L,    // 照明OFF左
  OBJ_LIGHT_OFF_R,    // 照明OFF右
  OBJ_V_VS1_L,        // 対戦前用爆破エフェクト 左
  OBJ_V_VS1_R,        // 対戦前用爆破エフェクト 右
  OBJ_V_VS2,          // 対戦後用爆破エフェクト
  OBJ_V_VS3,          // 決勝戦対戦前用爆破エフェクト

  OBJ_V_MAX,
};

// 通路エフェクト
enum {
  OBJ_EFFECT = 0,     // エフェクト
  OBJ_CHAMP,          // チャンピオンズ用(オブジェが2つある為)

  OBJ_EFFECT_MAX,
};


static const GFL_G3D_UTIL_RES res_3d_table[] = 
{
  // VS   ビジョン
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_name_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  // 優勝 ビジョン
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_name_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD

  // ロゴ
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_all_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_all_nsbta, GFL_G3D_UTIL_RESARC }, // ITA

  // 優勝 エフェクト
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_a_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_a_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_a_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_b_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_b_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_b_nsbta, GFL_G3D_UTIL_RESARC }, // ITA

  // 照明
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_light_on_l_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_light_on_r_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_light_off_l_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_light_off_r_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD

  // 爆破エフェクト
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_l_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_l_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_l_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_l_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_r_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_r_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_r_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs1_r_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs2_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs2_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs3_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs3_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs3_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_vs3_nsbta, GFL_G3D_UTIL_RESARC }, // ITA

};

// ANIME
static const GFL_G3D_UTIL_ANM anm_v_vs_table[] =
{
  { RESID_V_VS_ITA, 0 }, // nsbta
  { RESID_V_VS_ICA, 0 }, // nsbca
};
#define ANM_VS_NUM  ( NELEMS(anm_v_vs_table) )

static const GFL_G3D_UTIL_ANM anm_v_win_table[] =
{
  { RESID_V_WIN_ITA, 0 }, // nsbta
  { RESID_V_WIN_ICA, 0 }, // nsbca
};
#define ANM_VS_WIN_NUM  ( NELEMS(anm_v_win_table) )

static const GFL_G3D_UTIL_ANM anm_v_logo_table[] =
{
  { RESID_V_LOGO_ITA, 0 }, // nsbta
};
#define ANM_LOGO_NUM  ( NELEMS(anm_v_logo_table) )

static const GFL_G3D_UTIL_ANM anm_e_win_a_table[] =
{
  { RESID_E_WIN_A_ICA, 0 }, // nsbca
  { RESID_E_WIN_A_ITA, 0 }, // nsbta
};
#define ANM_WIN_A_NUM  ( NELEMS(anm_e_win_a_table) )

static const GFL_G3D_UTIL_ANM anm_e_win_b_table[] =
{
  { RESID_E_WIN_B_ICA, 0 }, // nsbca
  { RESID_E_WIN_B_ITA, 0 }, // nsbta
};
#define ANM_WIN_B_NUM  ( NELEMS(anm_e_win_b_table) )

static const GFL_G3D_UTIL_ANM anm_v_vs1_l[] =
{
  { RESID_E_VS1_L_ICA, 0 }, // nsbca
  { RESID_E_VS1_L_IMA, 0 }, // nsbma
  { RESID_E_VS1_L_ITA, 0 }, // nsbta
};
#define ANM_V_VS1_L_NUM  ( NELEMS(anm_v_vs1_l) )

static const GFL_G3D_UTIL_ANM anm_v_vs1_r[] =
{
  { RESID_E_VS1_R_ICA, 0 }, // nsbca
  { RESID_E_VS1_R_IMA, 0 }, // nsbma
  { RESID_E_VS1_R_ITA, 0 }, // nsbta
};
#define ANM_V_VS1_R_NUM  ( NELEMS(anm_v_vs1_r) )

static const GFL_G3D_UTIL_ANM anm_v_vs2[] =
{
  { RESID_E_VS2_ITA, 0 }, // nsbta
};
#define ANM_V_VS2_NUM  ( NELEMS(anm_v_vs2) )

static const GFL_G3D_UTIL_ANM anm_v_vs3[] =
{
  { RESID_E_VS3_ICA, 0 }, // nsbca
  { RESID_E_VS3_IMA, 0 }, // nsbma
  { RESID_E_VS3_ITA, 0 }, // nsbta
};
#define ANM_V_VS3_NUM  ( NELEMS(anm_v_vs3) )


// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  // OBJ_V_VS
  {
    RESID_V_VS_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_VS_MDL,       // テクスチャリソースID
    anm_v_vs_table,         // アニメテーブル(複数指定のため)
    ANM_VS_NUM,           // アニメリソース数
  },
  // OBJ_V_VS_NAME
  {
    RESID_V_VS_NAME_MDL,  // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_VS_NAME_MDL,  // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_V_WIN
  {
    RESID_V_WIN_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_WIN_MDL,      // テクスチャリソースID
    anm_v_win_table,      // アニメテーブル(複数指定のため)
    ANM_VS_WIN_NUM,       // アニメリソース数
  },
  // OBJ_V_WIN_NAME
  {
    RESID_V_WIN_NAME_MDL, // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_WIN_NAME_MDL, // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_V_LOGO
  {
    RESID_V_LOGO_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_LOGO_MDL,     // テクスチャリソースID
    anm_v_logo_table,     // アニメテーブル(複数指定のため)
    ANM_LOGO_NUM,         // アニメリソース数
  },
  // OBJ_E_WIN_START
  {
    RESID_E_WIN_A_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_WIN_A_MDL,    // テクスチャリソースID
    anm_e_win_a_table,    // アニメテーブル(複数指定のため)
    ANM_WIN_A_NUM,        // アニメリソース数
  },
  // OBJ_E_WIN_LOOP
  {
    RESID_E_WIN_B_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_WIN_B_MDL,    // テクスチャリソースID
    anm_e_win_b_table,    // アニメテーブル(複数指定のため)
    ANM_WIN_B_NUM,        // アニメリソース数
  },
  // OBJ_LIGHT_ON_L
  {
    RESID_LIGHT_ON_L_MDL, // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_ON_L_MDL, // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_LIGHT_ON_R
  {
    RESID_LIGHT_ON_R_MDL, // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_ON_R_MDL, // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_LIGHT_OFF_L
  {
    RESID_LIGHT_OFF_L_MDL,// モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_OFF_L_MDL,// テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_LIGHT_OFF_R
  {
    RESID_LIGHT_OFF_R_MDL,// モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_OFF_R_MDL,// テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_V_VS1_L
  {
    RESID_E_VS1_L_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_VS1_L_MDL,    // テクスチャリソースID
    anm_v_vs1_l,          // アニメテーブル(複数指定のため)
    ANM_V_VS1_L_NUM,      // アニメリソース数
  },
  // OBJ_V_VS1_R
  {
    RESID_E_VS1_R_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_VS1_R_MDL,    // テクスチャリソースID
    anm_v_vs1_r,          // アニメテーブル(複数指定のため)
    ANM_V_VS1_R_NUM,      // アニメリソース数
  },
  // OBJ_V_VS2
  {
    RESID_E_VS2_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_VS2_MDL,      // テクスチャリソースID
    anm_v_vs2,            // アニメテーブル(複数指定のため)
    ANM_V_VS2_NUM,        // アニメリソース数
  },
  // OBJ_V_VS3
  {
    RESID_E_VS3_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_VS3_MDL,      // テクスチャリソースID
    anm_v_vs3,            // アニメテーブル(複数指定のため)
    ANM_V_VS3_NUM,        // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				//リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				//オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

//-------------------------------------
///	テクスチャデータ
//=====================================
typedef enum
{
  TEX_INDEX_LEFT,     // 左上の名前
  TEX_INDEX_RIGHT,    // 右下の名前
  TEX_INDEX_WIN,      // 優勝者の名前

  TEX_INDEX_MAX,
}TEX_INDEX;
static char* tex_name[ TEX_INDEX_MAX ] =
{
  "wbt_name01",
  "wbt_name02",
  "wbt_name03",
};
static char* plt_name[ TEX_INDEX_MAX ] =
{
  "wbt_name01_pl",
  "wbt_name02_pl",
  "wbt_name03_pl",
};

static char* kao_tex_name[ TEX_INDEX_MAX ] =
{
  "wbt_v_kao1",
  "wbt_v_kao2",
  "wbt_v_kao1",
};

static char* kao_plt_name[ TEX_INDEX_MAX ] =
{
  "wbt_v_kao1_pl",
  "wbt_v_kao2_pl",
  "wbt_v_kao1_pl",
};
#define TEX_W   (4)     // テクスチャ幅(キャラ)
#define TEX_H   (4)     // テクスチャ幅(キャラ)

//=============================================================================
/**
 *          static関数宣言
 */
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, VecFx32 pos );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// 爆破エフェクト
static void ANM_VS1_L_Play( WBT_GMK_TMP *p_wk );
static void ANM_VS1_R_Play( WBT_GMK_TMP *p_wk );
static void ANM_VS2_Play( WBT_GMK_TMP *p_wk );
static void ANM_VS3_Play( WBT_GMK_TMP *p_wk );


// 背景ビジョン
static void VISION_Setup( WBT_GMK_TMP *p_wk );
static void VISION_SetupName( WBT_GMK_TMP *p_wk, u8 type, STRBUF *strbuf1, STRBUF *strbuf2, u16 trtype1, u16 trtype2 );
static BOOL VISION_Font2Tex( WBT_GMK_TMP *p_wk, TEX_INDEX tex_index, STRBUF *strbuf, HEAPID heapID );
static void VISION_Tex2Trgra( WBT_GMK_TMP *p_wk, TEX_INDEX tex_index, u16 trtype );
static void TexTransToGra( WBT_GMK_TMP *p_wk,
                           GFL_G3D_RES* p_res, 
                           const char* tex_name,
                           const char* plt_name,
                           u16 trtype,
                           HEAPID heapID );

static void* GetTexStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictTexData* pData);
static void* GetPlttStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictPlttData* pData);
static void SetResName(NNSG3dResName *outName,const char *inName);
static void* GetTexStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name);
static void* GetPlttStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name);

// 通路エフェクト
// チャンピオンズの設定
static void EFFECT_ChampInit( WBT_GMK_TMP *p_wk, u8 pos );

// task
// タスクワーク
typedef struct {
  u16           seq;
  WBT_GMK_TMP   *p_gmk_wk;    // ギミックワーク
} VISION_TASK_WORK;
// VSビジョン演出タスク
static void VISION_VS_TASK_Add( WBT_GMK_TMP *p_wk );
static FIELD_TASK_RETVAL VisionVsTaskFunc( void* wk );
// 優勝ビジョン演出タスク
static void VISION_WIN_TASK_Add( WBT_GMK_TMP *p_wk );
static FIELD_TASK_RETVAL VisionWinTaskFunc( void* wk );

// チャンピオンズエフェクト再生タスク
static void CHAMP_TASK_Add( WBT_GMK_TMP *p_wk );
static FIELD_TASK_RETVAL ChampTaskFunc( void* wk );


#ifdef WBT_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int  debug_trface;
static int  debug_wbt_trtype;

#endif // WBT_DEBUG_MENU

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

  p_wk->heapID      = HEAPID_FIELDMAP;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->p_player_wk = GAMEDATA_GetMyPlayerWork( p_gdata );

  p_wk->is_unit_effect= FALSE;

  // ギミックは発動させない
  p_wk->move_e_win  = FALSE;

  // EXPOBJ初期化
  EXPOBJ_ObjSet( p_wk );

#ifdef WBT_DEBUG_MENU
  p_wk->trtype_name = GFL_STR_CreateBuffer( BUFLEN_TRAINER_TYPE_NAME, HEAPID_FIELDMAP );
  p_wk->trtype_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                     NARC_message_trtype_dat, HEAPID_FIELDMAP );
  DEBUGMENU_Init( p_wk, p_fieldmap, HEAPID_FIELDMAP );
#endif // WBT_DEBUG_MENU
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

#ifdef WBT_DEBUG_MENU
  DEBUGMENU_Exit();
  GFL_MSG_Delete( p_wk->trtype_msg );
  GFL_STR_DeleteBuffer( p_wk->trtype_name );
#endif // WBT_DEBUG_MENU

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

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GAMEDATA   *gdata = GAMESYSTEM_GetGameData( gsys );
      STRBUF * strbuf1;
      STRBUF * strbuf2;   // プレイヤー名専用
      MYSTATUS* myStatus;
      myStatus = GAMEDATA_GetMyStatus( gdata );
      strbuf1  = MyStatus_CreateNameString( myStatus, p_wk->heapID );
      strbuf2  = MyStatus_CreateNameString( myStatus, p_wk->heapID );

      OS_Printf("デバッグ VSビジョン\n");
      WBT_GIMMICK_VisionChange( p_fieldmap, SCR_WBT_VISION_VS, strbuf1, strbuf2, debug_wbt_trtype, debug_wbt_trtype );

      GFL_STR_DeleteBuffer( strbuf1 );
      GFL_STR_DeleteBuffer( strbuf2 );
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 背景ビジョン変更
 *	        優勝ビジョンなど
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 *	@param  strbuf1     対戦者名1
 *	@param  strbuf2     対戦者名2
 *	@param  trtype1     対戦者トレーナータイプ1
 *	@param  trtype2     対戦者トレーナータイプ2
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_VisionChange( FIELDMAP_WORK *p_fieldmap, u8 type, STRBUF *strbuf1, STRBUF *strbuf2, u16 trtype1, u16 trtype2 )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // 既存の表示ビジョンを非表示
  {
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS_NAME, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN_NAME, TRUE );
    //FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_LOGO, TRUE );
  }

  // ビジョン別設定
  switch( type ){
  case SCR_WBT_VISION_VS:
    GF_ASSERT( strbuf2 );     // VSの場合は2つ名前を使用
    /* break through */
  case SCR_WBT_VISION_WIN_PLAYER:
  case SCR_WBT_VISION_WIN_ENEMY:
    GF_ASSERT( strbuf1 );

    // 名前張り付けのビジョンなら
    // リソースは読み込まない
    VISION_SetupName( p_wk, type, strbuf1, strbuf2, trtype1, trtype2 );
    break;

  default:
    // トーナメントロゴのビジョン
    VISION_Setup( p_wk );

    break;
  }

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

  GFL_G3D_UTIL_RES    res_e_table[RES_E_NUM] = {0};
  GFL_G3D_UTIL_ANM    anm_e_table[RES_E_NUM-1] = {0};
  GFL_G3D_UTIL_OBJ    obj_e_table = {0};
  GFL_G3D_UTIL_SETUP  setup = {0};
  u8      res_cnt = 0;
  u8      anm_cnt = 0;
  // リソースIDの開始位置テーブル
  // ※並び替え不可
  const   res_start_tbl[ RES_E_NUM ] = {
    RESID_E_START_MDL,    // モデル
    RESID_E_START_IMA,    // IMA
    RESID_E_START_ICA,    // ICA
    RESID_E_START_ITA,    // ITA
  };

  // チャンピオンズのみ設定が違う
  if( type == SCR_WBT_EFFECT_CHAMP )
  {
    EFFECT_ChampInit( p_wk, pos );
    ICHI_Printf("チャンピオンズ設定\n");
    return;
  }

  // RES
  for( res_cnt=0; res_cnt<RES_E_NUM; res_cnt++ )
  {
    res_e_table[ res_cnt ].arcive  = ARCID_WBT_GIMMICK_DATA;
    // 1種類のエフェクトで片方 RES_E_NUM個ある 両方でRES_E_NUM*2
    res_e_table[ res_cnt ].datID   = res_start_tbl[ res_cnt ] + (type*RES_E_NUM*2) + (pos*RES_E_NUM);
    res_e_table[ res_cnt ].arcType = GFL_G3D_UTIL_RESARC;
  }

  // ANIME
  anm_e_table[ anm_cnt ].anmresID = 1;  // IMA
  anm_e_table[ anm_cnt ].anmdatID = 0;
  anm_cnt++;
  anm_e_table[ anm_cnt ].anmresID = 2;  // ICA
  anm_e_table[ anm_cnt ].anmdatID = 0;
  anm_cnt++;
  anm_e_table[ anm_cnt ].anmresID = 3;  // ITA
  anm_e_table[ anm_cnt ].anmdatID = 0;
  anm_cnt++;

  // OBJ
  obj_e_table.mdlresID = 0;
  obj_e_table.mdldatID = 0;
  obj_e_table.texresID = 0;
  obj_e_table.anmTbl   = anm_e_table;
  obj_e_table.anmCount = anm_cnt;

  // セットアップデータ
  setup.resTbl  = res_e_table;
  setup.resCount= res_cnt;
  setup.objTbl  = &obj_e_table;
  setup.objCount= 1;


  //必要なリソースの用意
  // 仮にすでにリソース設定された後なら削除
  if( p_wk->is_unit_effect )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT );
  }
  FLD_EXP_OBJ_AddUnit( p_wk->exp_obj, &setup, EXPOBJ_UNIT_IDX_EFFECT );
  p_wk->is_unit_effect = TRUE;    // 作成した

  //=====================================
  // Effectの設定
  {
    u16     obj_idx = OBJ_EFFECT;    // OBJインデックス
    u8      i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx );

    // 座標
    status->trans.x = EFFECT_X + (pos*EFFECT_X_MARGIN); // 右側は、ずれているので調整
    status->trans.y = EFFECT_Y;
    status->trans.z = EFFECT_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<RES_E_NUM-1; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

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

  // 優勝のアニメ再生
  {
    u16     obj_idx = OBJ_E_WIN_START;    // OBJインデックス
    u8      i;

    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<ANM_WIN_A_NUM; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }
  // 一応ループの方を非表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, TRUE );

  p_wk->move_e_win = TRUE;

  ICHI_Printf("優勝エフェクト開始\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  照明を設定
 *
 *	@param	*p_fieldmap フィールドワークポインタ
 *	@param	on_off  ON/OFFフラグ  0:OFF 1:ON
 *	@param	pos     SCR_WBT_EFFECT_POS_L:左 SCR_WBT_EFFECT_POS_R:右
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_LightSet( FIELDMAP_WORK *p_fieldmap, u8 on_off, u8 pos )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );
  int i;

#if 0
  // 最初に全て非表示
  for( i=OBJ_LIGHT_ON_L; i<=OBJ_LIGHT_OFF_R; i++ )
  {
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, TRUE );
  }
#endif

  // 指定したものを表示
  {
    u16 obj_idx_vanish_true;  // 非表示
    u16 obj_idx_vanish_false; // 表示
    if( pos == SCR_WBT_EFFECT_POS_L )
    {
      obj_idx_vanish_true = ( on_off == 0 ) ? OBJ_LIGHT_ON_L : OBJ_LIGHT_OFF_L;
      obj_idx_vanish_false = ( on_off == 0 ) ? OBJ_LIGHT_OFF_L : OBJ_LIGHT_ON_L;
    }
    else
    {
      obj_idx_vanish_true = ( on_off == 0 ) ? OBJ_LIGHT_ON_R : OBJ_LIGHT_OFF_R;
      obj_idx_vanish_false = ( on_off == 0 ) ? OBJ_LIGHT_OFF_R : OBJ_LIGHT_ON_R;
    }

    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx_vanish_false, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx_vanish_true, TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  爆破エフェクト再生
 *
 *	@param	*p_fieldmap フィールドマップワーク
 *	@param	type  種類 ※script_def.hのSCR_WBT_VS???定義
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_Explosion( FIELDMAP_WORK *p_fieldmap, u8 type )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  switch( type ){
  case SCR_WBT_VS1_L:
    ANM_VS1_L_Play( p_wk );
    break;
  case SCR_WBT_VS1_R:
    ANM_VS1_R_Play( p_wk );
    break;
  case SCR_WBT_VS2:
    ANM_VS1_L_Play( p_wk );
    ANM_VS1_R_Play( p_wk );
    ANM_VS2_Play( p_wk );
    break;
  case SCR_WBT_VS3:
    ANM_VS3_Play( p_wk );
    break;
  default:
    GF_ASSERT(0);
  }
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
  int i;
  VecFx32 vision_pos = { VISION_X, VISION_Y, VISION_Z };
  VecFx32 effect_pos = { EFFECT_X, EFFECT_Y, EFFECT_Z };
  VecFx32 effect_pos_r = { EFFECT_X+EFFECT_X_MARGIN, EFFECT_Y, EFFECT_Z };

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, EXPOBJ_UNIT_IDX);

  //=====================================
  // VSの設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_VS, ANM_VS_NUM, vision_pos );

  //=====================================
  // VS 名前の設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_VS_NAME, 0, vision_pos );
  

  //=====================================
  // WINの設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_WIN, ANM_VS_WIN_NUM, vision_pos );
  
  //=====================================
  // WIN 名前の設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_WIN_NAME, 0, vision_pos );
  

  //=====================================
  // LOGOの設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_LOGO, ANM_LOGO_NUM, vision_pos );
  

  //=====================================
  // WIN エフェクトの設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_E_WIN_START, ANM_WIN_A_NUM, effect_pos );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_E_WIN_LOOP, ANM_WIN_B_NUM, effect_pos );
  // WIN エフェクトはカリングしない
  FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_START, FALSE );
  FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, FALSE );

  //=====================================
  // 照明の設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT_ON_L, 0, effect_pos );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT_ON_R, 0, effect_pos_r );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT_OFF_L, 0, effect_pos );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT_OFF_R, 0, effect_pos_r );

  //=====================================
  // 爆破エフェクトの設定
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_VS1_L, ANM_V_VS1_L_NUM, effect_pos );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_VS1_R, ANM_V_VS1_R_NUM, effect_pos );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_VS2, ANM_V_VS2_NUM, effect_pos );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_V_VS3, ANM_V_VS3_NUM, effect_pos );

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

  if( p_wk->is_unit_effect )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT );
  }
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


  // 優勝エフェクト
  if( p_wk->move_e_win == TRUE )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_START, 0 );
    // アニメ終了したら、ループ用に切り替え
    if( FLD_EXP_OBJ_ChkAnmEnd( exp_anm ) == TRUE )
    {
      u8 i;
      // 開始エフェクトは消す
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_START, TRUE );

      // ループエフェクトは表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, FALSE );
      // ループエフェクトアニメ設定
      for( i=0; i<ANM_WIN_B_NUM; i++ )
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, i );
        FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, i, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
      }

      // フラグOFF
      p_wk->move_e_win = FALSE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJ１つの初期設定
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
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, EXPOBJ_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, pos.x, pos.y, pos.z );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
  // 非表示
  FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i, FALSE );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx );

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//=============================================================================
/**
 *          爆破エフェクトのアニメ管理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  対戦前用爆破エフェクト左 アニメ再生
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_VS1_L_Play( WBT_GMK_TMP *p_wk )
{
  u16 obj_idx = OBJ_V_VS1_L;

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦前用爆破エフェクト右 アニメ再生
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_VS1_R_Play( WBT_GMK_TMP *p_wk )
{
  u16 obj_idx = OBJ_V_VS1_R;

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦後用爆破エフェクト アニメ再生
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_VS2_Play( WBT_GMK_TMP *p_wk )
{
  u16 obj_idx = OBJ_V_VS2;

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  決勝戦対戦前用爆破エフェクト アニメ再生
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_VS3_Play( WBT_GMK_TMP *p_wk )
{
  u16 obj_idx = OBJ_V_VS3;

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
}


//=============================================================================
/**
 *        背景ビジョン
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンセットアップ
 *
 *	@param	WBT_GMK_TMP *p_wk   ギミックワーク
 */
//-----------------------------------------------------------------------------
static void VISION_Setup( WBT_GMK_TMP *p_wk )
{
  //=====================================
  // LOGOの設定
  u16 obj_idx = OBJ_V_LOGO;    // OBJインデックス
  int i;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );
  // アニメ再生
  for( i=0; i<ANM_LOGO_NUM; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i, TRUE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンセットアップ
 *	        名前貼り付け有り
 *
 *	@param	WBT_GMK_TMP *p_wk   ギミックワーク
 *	@param	type                ビジョン種類
 *	@param	strbuf              対戦相手名
 */
//-----------------------------------------------------------------------------
static void VISION_SetupName( WBT_GMK_TMP *p_wk, u8 type, STRBUF *strbuf1, STRBUF *strbuf2, u16 trtype1, u16 trtype2 )
{
  u16   obj_idx;    // OBJインデックス
  HEAPID heapID = GFL_HEAP_LOWID( p_wk->heapID );

  // 3DモデルオブジェID判定
  // 名前貼り付けビジョンは
  // VSか優勝だけ
  switch( type ){
  case SCR_WBT_VISION_WIN_PLAYER:
  case SCR_WBT_VISION_WIN_ENEMY:
    obj_idx = OBJ_V_WIN;
    // 演出タスク作成
    VISION_WIN_TASK_Add( p_wk );
    break;
  case SCR_WBT_VISION_VS:
    obj_idx = OBJ_V_VS;
    // 演出タスク作成
    VISION_VS_TASK_Add( p_wk );
    break;
  }

  // 引数を保存
  p_wk->trtype1 = trtype1;
  p_wk->trtype2 = trtype2;

  // 名前の設定
  // と顔グラの設定(テクスチャ置き換え)
  {
    BOOL ret;
    STRBUF  *str_player;

    str_player = MyStatus_CreateNameString( &p_wk->p_player_wk->mystatus, heapID );

    switch( obj_idx ){
    case OBJ_V_VS:
      {
        // 名前
        ret = VISION_Font2Tex( p_wk, TEX_INDEX_LEFT, strbuf1, heapID );
        GF_ASSERT(ret);
        ret = VISION_Font2Tex( p_wk, TEX_INDEX_RIGHT, strbuf2, heapID );
        GF_ASSERT(ret);
        // 顔グラはtask内で。
      }
      break;

    case OBJ_V_WIN:
      {
        // 名前
        ret = VISION_Font2Tex( p_wk, TEX_INDEX_WIN, strbuf1, heapID );
        GF_ASSERT(ret);
        // 顔グラ
        VISION_Tex2Trgra( p_wk, TEX_INDEX_WIN, trtype1 );
      }
      break;
    }

    GFL_HEAP_FreeMemory( str_player );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンに名前貼り付け
 *
 *	@param	const GFL_G3D_RES* g3Dtex
 *	@param	char* tex_name
 *	@param	char* plt_name
 *	@param	STRBUF* str
 *	@param	xpos
 *	@param	ypos
 *	@param	heapID 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL VISION_Font2Tex( WBT_GMK_TMP *p_wk, TEX_INDEX tex_index, STRBUF *strbuf, HEAPID heapID )
{
  F2T_WORK      f2t_wk;
  BOOL          ret;
  PRINTSYS_LSB  lsb;

  GFL_FONT*     fontHandle;

  GFL_G3D_OBJ   *p_obj;
  GFL_G3D_RND   *p_rnd;
  GFL_G3D_RES   *p_res;
  u8            obj_idx;

  u32           len = 0;

  // どのオブジェのテクスチャに貼り付けるか
  switch( tex_index ){
  case TEX_INDEX_LEFT:
  case TEX_INDEX_RIGHT:
    obj_idx = OBJ_V_VS_NAME;
    break;

  case TEX_INDEX_WIN:
    obj_idx = OBJ_V_WIN_NAME;
    break;
  }

  // 文字幅計算
  {
    //フォントハンドル作成
    fontHandle = GFL_FONT_Create
      (ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID);
    len = PRINTSYS_GetStrWidth(strbuf, fontHandle, 0);
    ICHI_Printf("nameLen %d \n", len );
    GFL_FONT_Delete(fontHandle);
  }

  p_obj = FLD_EXP_OBJ_GetUnitObj( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );
  p_rnd = GFL_G3D_OBJECT_GetG3Drnd( p_obj );
  p_res = GFL_G3D_RENDER_GetG3DresTex( p_rnd );

  lsb = PRINTSYS_LSB_Make(1,2,0);

  // テクスチャに文字貼り付け
  // 76はテクスチャの見えてる範囲の幅
  ret = F2T_CopyString( p_res, tex_name[tex_index], NULL, plt_name[tex_index],
      strbuf, ((76/2) - (len/2)), 0, lsb, heapID, &f2t_wk );


  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンに顔グラを貼り付け
 *
 *	@param	WBT_GMK_TMP *p_wk
 *	@param	tex_index 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void VISION_Tex2Trgra( WBT_GMK_TMP *p_wk, TEX_INDEX tex_index, u16 trtype )
{
  u8            obj_idx;
  HEAPID heapID = GFL_HEAP_LOWID( p_wk->heapID );

  // どのオブジェのテクスチャに貼り付けるか
  switch( tex_index ){
  case TEX_INDEX_LEFT:
  case TEX_INDEX_RIGHT:
    obj_idx = OBJ_V_VS;
    break;

  case TEX_INDEX_WIN:
    obj_idx = OBJ_V_WIN;
    break;
  }

  p_wk->img_bitmap = GFL_BMP_Create(TEX_W, TEX_H, GFL_BMP_16_COLOR, heapID );

  // 転送
  {
    GFL_G3D_OBJ   *p_obj = FLD_EXP_OBJ_GetUnitObj( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );
    GFL_G3D_RND   *p_rnd = GFL_G3D_OBJECT_GetG3Drnd( p_obj );
    GFL_G3D_RES   *p_res = GFL_G3D_RENDER_GetG3DresTex( p_rnd );

    TexTransToGra( p_wk, p_res, kao_tex_name[tex_index], kao_plt_name[tex_index], trtype, heapID );
  }

  GFL_BMP_Delete( p_wk->img_bitmap );
  GFL_HEAP_FreeMemory( p_wk->chr_buf );
  GFL_HEAP_FreeMemory( p_wk->pal_buf );
}

//=============================================================================
/**
 *          TASK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  背景ビジョンVS画面タスク
 */
//-----------------------------------------------------------------------------
static void VISION_VS_TASK_Add( WBT_GMK_TMP *p_wk )
{
  FIELD_TASK        *task;
  FIELD_TASK_MAN    *taskMan;
  VISION_TASK_WORK  *work;
  HEAPID heapID = GFL_HEAP_LOWID( p_wk->heapID );

  taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heapID, sizeof(VISION_TASK_WORK), VisionVsTaskFunc );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq = 0;
  work->p_gmk_wk = p_wk;

  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
}
// タスク関数
static FIELD_TASK_RETVAL VisionVsTaskFunc( void* wk )
{
  VISION_TASK_WORK *p_wk = (VISION_TASK_WORK*)wk;
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_wk->p_gmk_wk->exp_obj;

  // 処理シーケンス
  enum{
    SEQ_TRANS1,
    SEQ_TRANS2,
    SEQ_START,
    SEQ_ANM_WAIT,
    SEQ_END,
  };

  switch( p_wk->seq ){
  case SEQ_TRANS1:
    {
      VISION_Tex2Trgra( p_wk->p_gmk_wk, TEX_INDEX_LEFT, p_wk->p_gmk_wk->trtype1 ); // 顔グラ
      p_wk->seq = SEQ_TRANS2;
    }
    break;
  case SEQ_TRANS2:
    {
      VISION_Tex2Trgra( p_wk->p_gmk_wk, TEX_INDEX_RIGHT, p_wk->p_gmk_wk->trtype2 ); // 顔グラ
      p_wk->seq = SEQ_START;
    }
    break;
  case SEQ_START:
    {
      // wbt_v_vs
      // itaをループ再生
      // icaを1回再生
      ANM_Play( exp_obj, OBJ_V_VS, 0, TRUE );
      ANM_Play( exp_obj, OBJ_V_VS, 1, FALSE );
      
      p_wk->seq = SEQ_ANM_WAIT;
    }
    break;
  case SEQ_ANM_WAIT:
    {
      // icaの終了チェック
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS, 1 );
      if( FLD_EXP_OBJ_ChkAnmEnd( exp_anm ) == TRUE )
      {
        // wbt_v_allを非表示
        FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_LOGO, TRUE );

        // wbt_v_vs_nameを表示
        FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS_NAME, FALSE );

        p_wk->seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    return FIELD_TASK_RETVAL_FINISH;
  default:
    GF_ASSERT(0);
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  背景ビジョン 優勝画面タスク
 */
//-----------------------------------------------------------------------------
static void VISION_WIN_TASK_Add( WBT_GMK_TMP *p_wk )
{
  FIELD_TASK        *task;
  FIELD_TASK_MAN    *taskMan;
  VISION_TASK_WORK  *work;
  HEAPID heapID = GFL_HEAP_LOWID( p_wk->heapID );

  taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heapID, sizeof(VISION_TASK_WORK), VisionWinTaskFunc );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq = 0;
  work->p_gmk_wk = p_wk;

  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
}

// タスク関数
static FIELD_TASK_RETVAL VisionWinTaskFunc( void* wk )
{
  VISION_TASK_WORK *p_wk = (VISION_TASK_WORK*)wk;
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_wk->p_gmk_wk->exp_obj;

  // 処理シーケンス
  enum{
    SEQ_START,
    SEQ_ANM_WAIT,
    SEQ_END,
  };

  switch( p_wk->seq ){
  case SEQ_START:
    {
      // wbt_v_win
      // itaをループ再生
      // icaを1回再生
      ANM_Play( exp_obj, OBJ_V_WIN, 0, TRUE );
      ANM_Play( exp_obj, OBJ_V_WIN, 1, FALSE );
      
      // wbt_v_allは非表示
      FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_LOGO, TRUE );

      p_wk->seq = SEQ_ANM_WAIT;
    }
    break;
  case SEQ_ANM_WAIT:
    {
      // icaの終了チェック
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN, 1 );
      if( FLD_EXP_OBJ_ChkAnmEnd( exp_anm ) == TRUE )
      {
        // wbt_v_win_nameを表示
        FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN_NAME, FALSE );

        p_wk->seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    return FIELD_TASK_RETVAL_FINISH;
  default:
    GF_ASSERT(0);
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}

static void* GetTexStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictTexData* pData)
{
	// オフセット値
	u32 offset = (pData->texImageParam & NNS_G3D_TEXIMAGE_PARAM_TEX_ADDR_MASK)<<3;
	offset += NNS_GfdGetTexKeyAddr( inResTex->texInfo.vramKey );
	return (void*)(offset);

}

static void* GetPlttStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictPlttData* pData)
{
	// オフセット値
	u32 offset = pData->offset<<3;
	offset += NNS_GfdGetPlttKeyAddr( inResTex->plttInfo.vramKey );
	return (void*)(offset);

}

static void SetResName(NNSG3dResName *outName,const char *inName)
{
	u8 i;
  int len = STD_StrLen(inName);

	//初期化
	for (i=0;i<4;i++){
		outName->val[i] = 0;
	}
	for (i=0;i<len;i++){
		outName->name[i] = inName[i];
	}
}

static void* GetTexStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name)
{
	NNSG3dResName tmpResName;
	const NNSG3dResDictTexData* pData;

	//テクスチャの実体をサーチ
	SetResName(&tmpResName, name);	//文字列をNNSG3dResNameに変換
	pData = NNS_G3dGetTexDataByName( inResTex, &tmpResName );

	if (pData == NULL){
		return NULL;
	}

	//アドレスを算出
	return GetTexStartVRAMAdrSub(inResTex, pData);
}

static void* GetPlttStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name)
{
	NNSG3dResName tmpResName;
	const NNSG3dResDictPlttData* pData;

	//テクスチャの実体をサーチ
	SetResName(&tmpResName, name);	//文字列をNNSG3dResNameに変換
	pData = NNS_G3dGetPlttDataByName( inResTex, &tmpResName );

	if (pData == NULL){
		return NULL;
	}

	//アドレスを算出
	return GetPlttStartVRAMAdrSub(inResTex, pData);
}


//----------------------------------------------------------------------------
/**
 *	@brief  テクスチャを転送する
 *
 *	@param	const GFL_G3D_RES* p_res
 *	@param	char* tex_name    テクスチャ名
 *
 *	@return オフセット値
 */
//-----------------------------------------------------------------------------
static void TexTransToGra( WBT_GMK_TMP *p_wk,
                           GFL_G3D_RES* p_res, 
                           const char* tex_name,
                           const char* plt_name,
                           u16 trtype,
                           HEAPID heapID )
{
  u32 tex_adr;
  u32 pal_adr;
  u32 pal_size;

  const u8 cw = TEX_W;     // 切り抜いた幅
  const u8 ch = TEX_H;     // 切り抜いた幅
  const u8 ncg_cw = 256/8;  // ncg画像の幅(キャラ単位)


  //リソースのＶＲＡＭアドレス取得
  {
    NNSG3dTexKey  texkey = GFL_G3D_GetTextureDataVramKey( p_res );
    NNSG3dPlttKey palkey = GFL_G3D_GetTexturePlttVramKey( p_res );
    {
      u8 * adr;
      NNSG3dResTex *resTex;
      NNSG3dResFileHeader* res_head = GFL_G3D_GetResourceFileHeader( p_res );
      resTex = NNS_G3dGetTex( res_head );
      adr = GetTexStartVRAMAdrByName(resTex, tex_name );
      tex_adr = (u32)adr;
      adr = GetPlttStartVRAMAdrByName(resTex, plt_name );
      pal_adr = (u32)adr;
      pal_size = (u32)resTex->plttInfo.sizePltt << 3;
    }

    ICHI_Printf("============================================\n");
    ICHI_Printf("txname = %s\n",tex_name);
    ICHI_Printf("plname = %s\n",plt_name);
    ICHI_Printf("key = %d %d\n",texkey, palkey);
    ICHI_Printf("adr = %x %x\n",tex_adr, pal_adr);
    ICHI_Printf("trtype = %d \n", trtype);
    ICHI_Printf("tex_size = %x \n", NNS_GfdGetTexKeySize(texkey));
    ICHI_Printf("pal_size = %x \n", pal_size);
    ICHI_Printf("============================================\n\n");
  }

  {
    p_wk->chr = NULL;
    p_wk->plt = NULL;
    p_wk->chr_buf = GFL_ARC_UTIL_LoadBGCharacter( ARCID_WBT_CUT_GRA, (trtype*2), TRUE, &p_wk->chr, heapID );
    p_wk->pal_buf = GFL_ARC_UTIL_LoadPalette( ARCID_WBT_CUT_GRA, (trtype*2+1), &p_wk->plt, heapID );

    ICHI_Printf("chr w = %d h = %d szByte = %d ", p_wk->chr->W,p_wk->chr->H,p_wk->chr->szByte);
    ICHI_Printf("pixelFmt=%d maping=%d charFmt=%d \n", p_wk->chr->pixelFmt,p_wk->chr->mapingType,p_wk->chr->characterFmt);
    ICHI_Printf("plt szByte = %d \n", p_wk->plt->szByte);
    ICHI_Printf("bmp w=%d h=%d size=%d \n", GFL_BMP_GetSizeX(p_wk->img_bitmap),GFL_BMP_GetSizeY(p_wk->img_bitmap), GFL_BMP_GetBmpDataSize(p_wk->img_bitmap));
    
    // メモ
    // キャラデータの幅は画像の幅と同じになってしまう。
    // g2dcvrt の-crオプションのXとWは無視される仕様だから。
    {
      u8	*src;
      u8  *dst;
      int x, y;

      // ncg画像をXでどこから切り取るか
      // 右端の4キャラを切り取りたい
      const u8 ncg_cw_offset = 224/8;


      src = p_wk->chr->pRawData;
      dst = GFL_BMP_GetCharacterAdrs( p_wk->img_bitmap );
      GFL_STD_MemClear32((void*)dst, cw*ch*0x20);

      for (y=0; y<ch; y++){
        for(x=0; x<cw; x++){
          GFL_STD_MemCopy32((void*)(&src[(y*ncg_cw+x+ncg_cw_offset)*0x20]), (void*)(&dst[(y*cw+x)*0x20]), 0x20);
          //GFL_STD_MemFill32((void*)(&dst[(y*cw+x)*0x20]), 0x11111111, 0x20);
        }
      }
    }
  }

  //GFL_BMP_Clear( p_wk->img_bitmap, 2 );

  // フォーマットデータをＢＭＰデータに変換
  GFL_BMP_DataConv_to_BMPformat( p_wk->img_bitmap, FALSE, heapID);
  //転送
  {
    BOOL rc;
    u16*	src;
		u32		dst;
    src = (u16*)GFL_BMP_GetCharacterAdrs( p_wk->img_bitmap );
		dst = tex_adr;
    
    ICHI_Printf("trans tex_addr %x\n", dst);
    ICHI_Printf("trans src_addr %x\n", src);
    rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, cw*ch*0x20);
    GF_ASSERT(rc);
    src = p_wk->plt->pRawData;
		dst = pal_adr;
    ICHI_Printf("trans pal_addr %x\n", dst);
    rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, 32);
    GF_ASSERT(rc);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  通路エフェクトのチャンピオンズのみリソースが違うので
 *	        初期化を分けた
 *
 *	@param	WBT_GMK_TMP *p_wk
 *	@param	pos 
 */
//-----------------------------------------------------------------------------
static void EFFECT_ChampInit( WBT_GMK_TMP *p_wk, u8 pos )
{
  const int champ_res_num = 2 + RES_E_NUM;

  GFL_G3D_UTIL_RES    res_e_table[champ_res_num] = {0};
  GFL_G3D_UTIL_ANM    anm_e_table1 = {0};
  GFL_G3D_UTIL_ANM    anm_e_table2[RES_E_NUM-1] = {0};
  GFL_G3D_UTIL_OBJ    obj_e_table[2] = {0};
  GFL_G3D_UTIL_SETUP  setup = {0};
  u8      res_cnt = 0;
  u8      anm_cnt = 0;
  // リソースIDの開始位置テーブル
  // ※並び替え不可
  const   res_start_tbl[ champ_res_num ] = {
    RESID_E_CHAMP1_L_MDL,    // モデル
    RESID_E_CHAMP1_L_ICA,    // ICA
    RESID_E_CHAMP2_L_MDL,    // モデル
    RESID_E_CHAMP2_L_IMA,    // IMA
    RESID_E_CHAMP2_L_ICA,    // ICA
    RESID_E_CHAMP2_L_ITA,    // ITA
  };

  // RES
  for( res_cnt=0; res_cnt<2; res_cnt++ )
  {
    res_e_table[ res_cnt ].arcive  = ARCID_WBT_GIMMICK_DATA;
    res_e_table[ res_cnt ].datID   = res_start_tbl[ res_cnt ] + (pos*2);
    res_e_table[ res_cnt ].arcType = GFL_G3D_UTIL_RESARC;
  }
  for( ; res_cnt<champ_res_num; res_cnt++ )
  {
    res_e_table[ res_cnt ].arcive  = ARCID_WBT_GIMMICK_DATA;
    res_e_table[ res_cnt ].datID   = res_start_tbl[ res_cnt ] + (pos*RES_E_NUM);
    res_e_table[ res_cnt ].arcType = GFL_G3D_UTIL_RESARC;
  }

  {
    int i;
    for( i=0; i<champ_res_num; i++ )
    {
      ICHI_Printf("rec_tbl[%d].datID %d \n", i, res_e_table[i].datID);
    }
    ICHI_Printf("res_cnt = %d \n", res_cnt);
  }

  // ANIME
  anm_e_table1.anmresID = 1;  // ICA
  anm_e_table1.anmdatID = 0;

  anm_e_table2[ anm_cnt ].anmresID = 3;  // IMA
  anm_e_table2[ anm_cnt ].anmdatID = 0;
  anm_cnt++;
  anm_e_table2[ anm_cnt ].anmresID = 4;  // ICA
  anm_e_table2[ anm_cnt ].anmdatID = 0;
  anm_cnt++;
  anm_e_table2[ anm_cnt ].anmresID = 5;  // ITA
  anm_e_table2[ anm_cnt ].anmdatID = 0;
  anm_cnt++;

  // OBJ_EFFECT
  obj_e_table[0].mdlresID = 0;
  obj_e_table[0].mdldatID = 0;
  obj_e_table[0].texresID = 0;
  obj_e_table[0].anmTbl   = &anm_e_table1;
  obj_e_table[0].anmCount = 1;
  // OBJ_CHAMP
  obj_e_table[1].mdlresID = 2;
  obj_e_table[1].mdldatID = 0;
  obj_e_table[1].texresID = 2;
  obj_e_table[1].anmTbl   = anm_e_table2;
  obj_e_table[1].anmCount = anm_cnt;

  // セットアップデータ
  setup.resTbl  = res_e_table;
  setup.resCount= res_cnt;
  setup.objTbl  = obj_e_table;
  setup.objCount= 2;

  //必要なリソースの用意
  // 仮にすでにリソース設定された後なら削除
  if( p_wk->is_unit_effect )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT );
  }
  FLD_EXP_OBJ_AddUnit( p_wk->exp_obj, &setup, EXPOBJ_UNIT_IDX_EFFECT );
  p_wk->is_unit_effect = TRUE;    // 作成した

  //=====================================
  // Effectの設定
  {
    u16     obj_idx = OBJ_EFFECT;    // OBJインデックス
    u8      i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx );

    // 座標
    status->trans.x = EFFECT_X + (pos*EFFECT_X_MARGIN); // 右側は、ずれているので調整
    status->trans.y = EFFECT_Y;
    status->trans.z = EFFECT_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, FALSE );

    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
      // 頭だし
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, 0, 0 );
    }
  }
  {
    u16     obj_idx = OBJ_CHAMP;    // OBJインデックス
    u8      i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx );

    // 座標
    status->trans.x = EFFECT_X + (pos*EFFECT_X_MARGIN); // 右側は、ずれているので調整
    status->trans.y = EFFECT_Y;
    status->trans.z = EFFECT_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<RES_E_NUM-1; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, i, FALSE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }

  // タスク追加
  CHAMP_TASK_Add( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  通路エフェクトのチャンピオンズ　タスク
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void CHAMP_TASK_Add( WBT_GMK_TMP *p_wk )
{
  FIELD_TASK        *task;
  FIELD_TASK_MAN    *taskMan;
  VISION_TASK_WORK  *work;
  HEAPID heapID = GFL_HEAP_LOWID( p_wk->heapID );

  taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heapID, sizeof(VISION_TASK_WORK), ChampTaskFunc );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq = 0;
  work->p_gmk_wk = p_wk;

  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
}
// タスク関数
static FIELD_TASK_RETVAL ChampTaskFunc( void* wk )
{
  VISION_TASK_WORK *p_wk = (VISION_TASK_WORK*)wk;
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_wk->p_gmk_wk->exp_obj;

  u8      i;

  // 処理シーケンス
  enum{
    SEQ_ANM_WAIT,
    SEQ_END,
  };

  switch( p_wk->seq ){
  case SEQ_ANM_WAIT:
    {
      // icaの終了チェック
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX_EFFECT, OBJ_EFFECT, 0 );
      if( FLD_EXP_OBJ_ChkAnmEnd( exp_anm ) == TRUE )
      {
        ICHI_Printf("champ1 アニメ終了\n");
        // 元の非表示
        //FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX_EFFECT, OBJ_EFFECT, TRUE );
        // 表示
        FLD_EXP_OBJ_SetVanish( exp_obj, EXPOBJ_UNIT_IDX_EFFECT, OBJ_CHAMP, FALSE );
        // アニメ再生
        for( i=0; i<RES_E_NUM-1; i++ )
        {
          EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX_EFFECT, OBJ_CHAMP, i );
          FLD_EXP_OBJ_ValidCntAnm( exp_obj, EXPOBJ_UNIT_IDX_EFFECT, OBJ_CHAMP, i, TRUE );
          FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
          FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
        }

        p_wk->seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    return FIELD_TASK_RETVAL_FINISH;
  default:
    GF_ASSERT(0);
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}


#ifdef WBT_DEBUG_MENU
//=============================================================================
/**
 *    デバッグメニュー
 */
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#define DEBUGMENU_WBT (2)

//-------------------------------------
///	static
//=====================================
// キー入力でデータ変更
static BOOL DebugWin_ChangeData( int *p_param, int min, int max );
// データの変更
static void DebugWin_Update_ChangeData( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeData( void *userWork, DEBUGWIN_ITEM *item );

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yにデバッグメニュー追加
 *
 *	@param	*p_wk       ギミックワーク
 *	@param	*p_fieldmap フィールドマップワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID )
{
  // デバッグメニューグループ追加
  DEBUGWIN_AddGroupToTop( DEBUGMENU_WBT, "WBT", heapID );

  // 
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeData,
                              DebugWin_Draw_ChangeData,
                              p_wk,
                              DEBUGMENU_WBT,
                              heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_WBT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ数値入力
 *
 *	@param	*p_param  変更する値のポインタ
 *	@param	min       入力する時の最小値
 *	@param	max       入力する時の最大値
 *
 *	@retval TRUE 入力あり
 *	@retval FALSE 入力なし
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_ChangeData( int *p_param, int min, int max )
{
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
  { 
    value = -10;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
  { 
    value = +10;
  }

  (*p_param) += value;
  (*p_param)  = MATH_CLAMP( (*p_param), min, max );

  if( value == 0 )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  顔グラフィック　更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeData( void *userWork, DEBUGWIN_ITEM *item )
{
  WBT_GMK_TMP *p_wk = userWork;
  int value = debug_trface;

  if( DebugWin_ChangeData( &value, 0, 77-1+2 ) )    // MOB+主人公分
  {
    u16 wbt_trtype;
    // 更新あり
    debug_trface = value;

    // 主人公はそのままだしたいので
    // それ以外をWBTトレーナーにする
    if( debug_trface > 1 )
    {
      WBTROM_TRDATA *wbtrom = (void*)WBTTOOL_CreateRomTrainerData( HEAPID_FIELDMAP );
      // WBTのトレーナータイプに変換
      wbt_trtype = WBTTOOL_GetRomTrainerBtlTrType( wbtrom, debug_trface - 2 );
      WBTTOOL_DeleteRomTrainerData( wbtrom );
      debug_wbt_trtype = wbt_trtype;
    }
    else
    {
      wbt_trtype = debug_trface;
      debug_wbt_trtype = wbt_trtype;
    }

    // トレーナータイプ名取得
    p_wk->trtype_name = GFL_MSG_CreateString( p_wk->trtype_msg, wbt_trtype );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  顔グラフィック　描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeData( void *userWork, DEBUGWIN_ITEM *item )
{
  WBT_GMK_TMP *p_wk = userWork;
  const STRCODE *cp_strcode = GFL_STR_GetStringCodePointer(p_wk->trtype_name);
  char str[BUFLEN_TRAINER_TYPE_NAME] = {0};

  DEB_STR_CONV_StrcodeToSjis( cp_strcode, str, BUFLEN_TRAINER_TYPE_NAME );

  DEBUGWIN_ITEM_SetNameV( item, "%s[%d]", str, debug_trface );
}

#endif // WBT_DEBUG_MENU

