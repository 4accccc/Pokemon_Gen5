//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_water.c
 *	@brief  水ジム
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "savedata/gimmickwork.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl
#include "field_task_target_pos.h" // for FIELD_TASK_CameraTargetMove

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/gym_water.naix"
#include "../../../resource/fldmapdata/gimmick/gym_water/gym_water_local_def.h"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "gym_water.h"
#include "gym_water_se_def.h"
#include "gym_water_sv.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_WATER_DEBUG_MENU
#endif //PM_DEBUG


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_WATER_UNIT_IDX (0)
#define GYM_WATER_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define HASU_EX_OBJ_NULL (0xff)   // ハスOBJIDXなし
#define HASU_EX_OBJ_IDX_MINE  (0) // 拡張高さハスインデックス(自分)
#define HASU_EX_OBJ_IDX_OTHER (1) // 拡張高さハスインデックス(周り)

//-------------------------------------
///	ハスの上情報
//=====================================
typedef enum {
  EX_HEIGHT_RIDE_MINE,    // 自分のハスに乗っている( =HASU_EX_OBJ_IDX_MINE
  EX_HEIGHT_RIDE_OTHER,   // 周りのハスに乗っている( =HASU_EX_OBJ_IDX_OTHER
  EX_HEIGHT_RIDE_NO,      // 乗っていない

} EX_HEIGHT_RIDE_TYPE;


#define HASU_NUM   (11)   // ハスの個数

// ハス座標
#define HASU1_X    (10 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスA　X座標
#define HASU1_Z    (64 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスA　Z座標
#define HASU2_X    ( 7 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスB　X座標
#define HASU2_Z    (58 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスB　Z座標
#define HASU3_X    (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスC　X座標
#define HASU3_Z    (58 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスC　Z座標
#define HASU4_X    (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスD　X座標
#define HASU4_Z    (49 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスD　Z座標
#define HASU5_X    (22 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスE　X座標
#define HASU5_Z    (49 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスE　Z座標
#define HASU6_X    ( 6 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスF　X座標
#define HASU6_Z    (31 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスF　Z座標
#define HASU7_X    (12 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスG　X座標
#define HASU7_Z    (37 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスG　Z座標
#define HASU8_X    (22 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスH　X座標
#define HASU8_Z    (22 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスH　Z座標
#define HASU9_X    ( 7 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスI　X座標
#define HASU9_Z    (19 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスI　Z座標
#define HASU10_X   (10 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスJ　X座標
#define HASU10_Z   (13 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスJ　Z座標
#define HASU11_X   (19 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスK　X座標
#define HASU11_Z   (13 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスK　Z座標

// クリア後のハス座標
#define HASU1_CLEAR_X    (10 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスA　X座標
#define HASU1_CLEAR_Z    (64 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスA　Z座標
#define HASU2_CLEAR_X    (13 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスB　X座標
#define HASU2_CLEAR_Z    (58 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスB　Z座標
#define HASU3_CLEAR_X    (25 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスC　X座標
#define HASU3_CLEAR_Z    (58 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスC　Z座標
#define HASU4_CLEAR_X    (19 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスD　X座標
#define HASU4_CLEAR_Z    (52 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスD　Z座標
#define HASU5_CLEAR_X    (25 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスE　X座標
#define HASU5_CLEAR_Z    (49 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスE　Z座標
#define HASU6_CLEAR_X    ( 9 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスF　X座標
#define HASU6_CLEAR_Z    (37 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスF　Z座標
#define HASU7_CLEAR_X    (25 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスG　X座標
#define HASU7_CLEAR_Z    (37 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスG　Z座標
#define HASU8_CLEAR_X    (10 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスH　X座標
#define HASU8_CLEAR_Z    (25 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスH　Z座標
#define HASU9_CLEAR_X    ( 7 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスI　X座標
#define HASU9_CLEAR_Z    (10 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスI　Z座標
#define HASU10_CLEAR_X   (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスJ　X座標
#define HASU10_CLEAR_Z   (61 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスJ　Z座標
#define HASU11_CLEAR_X   (25 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスK　X座標
#define HASU11_CLEAR_Z   (13 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハスK　Z座標

#define HASU_ID_CLEAR   (3)   // 乗るとクリアとするハスID

// アニメ
#define HASU_ANM        (0)   // 常時たゆわう
#define HASU_ANM_MOVE   (1)   // 降りるとき

#define ANM_HIT_RIPPLE_MINE   (0)     // 衝突波紋(自分)
#define ANM_HIT_RIPPLE_OTHER  (1)     // 衝突波紋(相手)

// タスク
#define TASK_MAX        (8)   // 移動波紋6 + 衝突波紋2

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	ANIME用タスクワーク
//=====================================
typedef struct {
  BOOL                is_enable;    // 動作しているか
  FLD_EXP_OBJ_CNT_PTR exp_obj;
  u16                 obj_idx;
  u8                  anm_num;
} ANM_TASK_WORK;

//-------------------------------------
///	ジム内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  GYM_WATER_SV_WORK   *p_sv_wk;       // セーブワーク

  ANM_TASK_WORK       anm_task_wk[ TASK_MAX ];

  EX_HEIGHT_RIDE_TYPE now_hasu_ride;  // ハスに乗っているか
  BOOL                is_anm_hasu;    // ハス降りるアニメフラグ
  u16                 anm_hasu_obj;   // ↑のハスOBJ
  u8                  ex_height_idx;  // 拡張高さインデックス         
} GYM_WATER_TMP;

//-------------------------------------
///	ハス移動ワーク
//=====================================
typedef struct {

  // 移動用
  int               frame;  // 現在のフレーム数
  int            endFrame;  // 最終フレーム数
  VecFx32        startPos;  // 移動開始位置
  VecFx32          endPos;  // 最終位置

  // 上下ゆれ用
  s16              Height;  // ゆれ高さ
  u32                Sync;  // 1周期するのにかかるシンク数
  u32             NowSync;  // 現在のシンク
} HASU_MOVE_WORK;

//-------------------------------------
///	ハスイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機

  HASU_MOVE_WORK  move_wk;      // 移動ワーク

  u8              id;           // ハスID
  u16             obj_idx;      // OBJインデックス

  // 衝突用
  BOOL            is_collision; // ハスに衝突したか
  u16             other_obj_idx;// 衝突したハスOBJインデックス
  u16             other_move_distance;  // 衝突ハスの移動距離
  s16             wait_frame;   // 待ちフレーム

  FIELD_CAMERA*   camera;
  const VecFx32   *WatchTarget;

} HASU_EVENT_WORK;



//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_HASU_MDL = 0,     // ハス
  RESID_HASU_ANM,         // ハスたゆわう(常時)
  RESID_HASU_ANM_MOVE,    // ハス揺れる（降りるとき）

  RESID_WATER1_MDL,       // 衝突波紋(自機側)
  RESID_WATER1_ANM_IMA,   // 
  RESID_WATER1_ANM_ITA,   // 
  RESID_WATER2_MDL,       // 衝突波紋(相手側)
  RESID_WATER2_ANM_IMA,   // 
  RESID_WATER2_ANM_ITA,   // 
  RESID_WATER3_MDL,       // 移動波紋
  RESID_WATER3_ANM_IMA,   // 
  RESID_WATER3_ANM_ITA,   // 
  RESID_WATER4_MDL,       // 水しぶき
  RESID_WATER4_ANM_ITA,   // 
};

// OBJインデックス
enum {
  OBJ_HASU_1,   // ハスA
  OBJ_HASU_2,   // ハスB
  OBJ_HASU_3,   // ハスC
  OBJ_HASU_4,   // ハスD
  OBJ_HASU_5,   // ハスE
  OBJ_HASU_6,   // ハスF
  OBJ_HASU_7,   // ハスG
  OBJ_HASU_8,   // ハスH
  OBJ_HASU_9,   // ハスI
  OBJ_HASU_10,  // ハスJ
  OBJ_HASU_11,  // ハスK

  // 演出
  EFFECT_HIT_RIPPLE1,   // 衝突波紋(自分)
  EFFECT_HIT_RIPPLE2,   // 衝突波紋(相手)
  EFFECT_MOVE_RIPPLE1,  // 移動波紋
  EFFECT_MOVE_RIPPLE2,  // 移動波紋
  EFFECT_MOVE_RIPPLE3,  // 移動波紋
  EFFECT_MOVE_RIPPLE4,  // 移動波紋
  EFFECT_MOVE_RIPPLE5,  // 移動波紋
  EFFECT_MOVE_RIPPLE6,  // 移動波紋
  EFFECT_MOVE_RIPPLE_MAX, // 移動波紋 総数
  EFFECT_SPLASH = EFFECT_MOVE_RIPPLE_MAX, // 水しぶき

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_GYM_WATER, NARC_gym_water_hasu_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_WATER, NARC_gym_water_hasu_shake_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_GYM_WATER, NARC_gym_water_hasu_move_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_GYM_WATER, NARC_gym_water_water1_up_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_WATER, NARC_gym_water_water1_up_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_GYM_WATER, NARC_gym_water_water1_up_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_GYM_WATER, NARC_gym_water_water1_down_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_WATER, NARC_gym_water_water1_down_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_GYM_WATER, NARC_gym_water_water1_down_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_GYM_WATER, NARC_gym_water_water2_move_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_WATER, NARC_gym_water_water2_move_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_GYM_WATER, NARC_gym_water_water2_move_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_GYM_WATER, NARC_gym_water_water3_shibu_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_WATER, NARC_gym_water_water3_shibu_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_hasu[] =
{
  { RESID_HASU_ANM, 0 }, // nsbca
  { RESID_HASU_ANM_MOVE, 0 }, // nsbca
};
static const GFL_G3D_UTIL_ANM anm_water1[] =
{
  { RESID_WATER1_ANM_IMA, 0 }, // nsbma
  { RESID_WATER1_ANM_ITA, 0 }, // nsbta
};
static const GFL_G3D_UTIL_ANM anm_water2[] =
{
  { RESID_WATER2_ANM_IMA, 0 }, // nsbma
  { RESID_WATER2_ANM_ITA, 0 }, // nsbta
};
static const GFL_G3D_UTIL_ANM anm_water3[] =
{
  { RESID_WATER3_ANM_IMA, 0 }, // nsbma
  { RESID_WATER3_ANM_ITA, 0 }, // nsbta
};
static const GFL_G3D_UTIL_ANM anm_water4[] =
{
  { RESID_WATER4_ANM_ITA, 0 }, // nsbta
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  // ハス1
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス2
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス3
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス4
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス5
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス6
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス7
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス8
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス9
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス10
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },
  // ハス11
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    anm_hasu,             // アニメテーブル(複数指定のため)
    NELEMS(anm_hasu),     // アニメリソース数
  },

  // 衝突波紋(自分)
  {
    RESID_WATER1_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER1_MDL,     // テクスチャリソースID
    anm_water1,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water1),   // アニメリソース数
  },
  // 衝突波紋(相手)
  {
    RESID_WATER2_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER2_MDL,     // テクスチャリソースID
    anm_water2,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water2),   // アニメリソース数
  },
  // 移動波紋
  {
    RESID_WATER3_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER3_MDL,     // テクスチャリソースID
    anm_water3,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water3),   // アニメリソース数
  },
  // 移動波紋
  {
    RESID_WATER3_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER3_MDL,     // テクスチャリソースID
    anm_water3,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water3),   // アニメリソース数
  },
  // 移動波紋
  {
    RESID_WATER3_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER3_MDL,     // テクスチャリソースID
    anm_water3,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water3),   // アニメリソース数
  },
  // 移動波紋
  {
    RESID_WATER3_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER3_MDL,     // テクスチャリソースID
    anm_water3,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water3),   // アニメリソース数
  },
  // 移動波紋
  {
    RESID_WATER3_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER3_MDL,     // テクスチャリソースID
    anm_water3,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water3),   // アニメリソース数
  },
  // 移動波紋
  {
    RESID_WATER3_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER3_MDL,     // テクスチャリソースID
    anm_water3,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water3),   // アニメリソース数
  },
  // 水しぶき
  {
    RESID_WATER4_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_WATER4_MDL,     // テクスチャリソースID
    anm_water4,           // アニメテーブル(複数指定のため)
    NELEMS(anm_water4),   // アニメリソース数
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
 *          データ
 */
//=============================================================================
//-------------------------------------
///	ハスイベント起動位置パラメータ
//=====================================
typedef struct {
  u8    gx;       // 起動Xグリッド座標
  u8    gz;       // 起動Zグリッド座標
  u8    move_dir; // 移動方向
  u8    move_distance;  // 移動距離
} HASU_POS_PARAM;

static const HASU_POS_PARAM HasuPosParam[ HASU_POS_NUM ] = {
  { HASU_POS_0_X, HASU_POS_0_Z, HASU_POS_0_DIR, HASU_POS_0_MOVE },
  { HASU_POS_1_X, HASU_POS_1_Z, HASU_POS_1_DIR, HASU_POS_1_MOVE },
  { HASU_POS_2_X, HASU_POS_2_Z, HASU_POS_2_DIR, HASU_POS_2_MOVE },
  { HASU_POS_3_X, HASU_POS_3_Z, HASU_POS_3_DIR, HASU_POS_3_MOVE },
  { HASU_POS_4_X, HASU_POS_4_Z, HASU_POS_4_DIR, HASU_POS_4_MOVE },
  { HASU_POS_5_X, HASU_POS_5_Z, HASU_POS_5_DIR, HASU_POS_5_MOVE },
  { HASU_POS_6_X, HASU_POS_6_Z, HASU_POS_6_DIR, HASU_POS_6_MOVE },
  { HASU_POS_7_X, HASU_POS_7_Z, HASU_POS_7_DIR, HASU_POS_7_MOVE },
  { HASU_POS_8_X, HASU_POS_8_Z, HASU_POS_8_DIR, HASU_POS_8_MOVE },
  { HASU_POS_9_X, HASU_POS_9_Z, HASU_POS_9_DIR, HASU_POS_9_MOVE },
  { HASU_POS_10_X, HASU_POS_10_Z, HASU_POS_10_DIR, HASU_POS_10_MOVE },
  { HASU_POS_11_X, HASU_POS_11_Z, HASU_POS_11_DIR, HASU_POS_11_MOVE },
  { HASU_POS_12_X, HASU_POS_12_Z, HASU_POS_12_DIR, HASU_POS_12_MOVE },
  { HASU_POS_13_X, HASU_POS_13_Z, HASU_POS_13_DIR, HASU_POS_13_MOVE },
  { HASU_POS_14_X, HASU_POS_14_Z, HASU_POS_14_DIR, HASU_POS_14_MOVE },
  { HASU_POS_15_X, HASU_POS_15_Z, HASU_POS_15_DIR, HASU_POS_15_MOVE },
  { HASU_POS_16_X, HASU_POS_16_Z, HASU_POS_16_DIR, HASU_POS_16_MOVE },
  { HASU_POS_17_X, HASU_POS_17_Z, HASU_POS_17_DIR, HASU_POS_17_MOVE },
  { HASU_POS_18_X, HASU_POS_18_Z, HASU_POS_18_DIR, HASU_POS_18_MOVE },
  { HASU_POS_19_X, HASU_POS_19_Z, HASU_POS_19_DIR, HASU_POS_19_MOVE },
  { HASU_POS_20_X, HASU_POS_20_Z, HASU_POS_20_DIR, HASU_POS_20_MOVE },
  { HASU_POS_21_X, HASU_POS_21_Z, HASU_POS_21_DIR, HASU_POS_21_MOVE },
  { HASU_POS_22_X, HASU_POS_22_Z, HASU_POS_22_DIR, HASU_POS_22_MOVE },
  { HASU_POS_23_X, HASU_POS_23_Z, HASU_POS_23_DIR, HASU_POS_23_MOVE },
  { HASU_POS_24_X, HASU_POS_24_Z, HASU_POS_24_DIR, HASU_POS_24_MOVE },
  { HASU_POS_25_X, HASU_POS_25_Z, HASU_POS_25_DIR, HASU_POS_25_MOVE },
  { HASU_POS_26_X, HASU_POS_26_Z, HASU_POS_26_DIR, HASU_POS_26_MOVE },
  { HASU_POS_27_X, HASU_POS_27_Z, HASU_POS_27_DIR, HASU_POS_27_MOVE },
  { HASU_POS_28_X, HASU_POS_28_Z, HASU_POS_28_DIR, HASU_POS_28_MOVE },
  { HASU_POS_29_X, HASU_POS_29_Z, HASU_POS_29_DIR, HASU_POS_29_MOVE },
  { HASU_POS_30_X, HASU_POS_30_Z, HASU_POS_30_DIR, HASU_POS_30_MOVE },
  { HASU_POS_31_X, HASU_POS_31_Z, HASU_POS_31_DIR, HASU_POS_31_MOVE },
  { HASU_POS_32_X, HASU_POS_32_Z, HASU_POS_32_DIR, HASU_POS_32_MOVE },
  { HASU_POS_33_X, HASU_POS_33_Z, HASU_POS_33_DIR, HASU_POS_33_MOVE },
  { HASU_POS_34_X, HASU_POS_34_Z, HASU_POS_34_DIR, HASU_POS_34_MOVE },
  { HASU_POS_35_X, HASU_POS_35_Z, HASU_POS_35_DIR, HASU_POS_35_MOVE },
  { HASU_POS_36_X, HASU_POS_36_Z, HASU_POS_36_DIR, HASU_POS_36_MOVE },
};


//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( GYM_WATER_TMP *p_wk );
static void EXPOBJ_ObjDell( GYM_WATER_TMP *p_wk );
static void EXPOBJ_ObjMain( GYM_WATER_TMP *p_wk );


// HASU
static BOOL HASU_CheckPos( FLD_EXP_OBJ_CNT_PTR exp_obj, const fx32 x, const fx32 z, u16 *p_obj_idx );
static BOOL HASU_GetCenterPos( u8 id, int *p_gx, int *p_gz );
static void HASU_SetExHeight_DirMax4( GYM_WATER_TMP *p_wk );
static void HASU_InitExHeight( GYM_WATER_TMP *p_wk );
static void HASU_ExitExHeight( GYM_WATER_TMP *p_wk );

// HASUイベント
static GMEVENT * HASU_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, u8 id, u16 obj_idx );
static GMEVENT_RESULT HasuEvent( GMEVENT* event, int* seq, void* wk );

// HASU移動
static int  HASU_EVENT_GetMoveDistance( HASU_EVENT_WORK *p_wk, u8 id );
static void HASU_EVENT_MoveInit( HASU_EVENT_WORK *p_wk, int frame, VecFx32* pos );
static BOOL HASU_EVENT_MoveMain( HASU_EVENT_WORK *p_wk );
static BOOL HASU_EVENT_OtherMoveInit( HASU_EVENT_WORK *p_wk );
static BOOL HASU_EVENT_OtherMoveMain( HASU_EVENT_WORK *p_wk );

// ANIME
static void ANIME_HASU_LeaveStart( GYM_WATER_TMP *p_wk, EX_HEIGHT_RIDE_TYPE ride );
static BOOL ANIME_HASU_LeaveCheck( GYM_WATER_TMP *p_wk );
static void ANIME_HASU_LeaveEnd( GYM_WATER_TMP *p_wk );
static void ANIME_WATER_Splash( HASU_EVENT_WORK *p_wk );
static void ANIME_WATER_MoveRipple( HASU_EVENT_WORK *p_wk, const u16 hasu_obj_idx );
static void ANIME_WATER_HitRipple( HASU_EVENT_WORK *p_wk, const u16 hasu_obj_idx, const u8 anm_type );

// EX_HEIGHT 拡張高さ
static EX_HEIGHT_RIDE_TYPE EX_HEIGHT_CheckHasuRide( GYM_WATER_TMP *p_wk, const int gx, const int gz );
static void EX_HEIGHT_Reset( GYM_WATER_TMP *p_wk ); 
static void EX_HEIGHT_Set( GYM_WATER_TMP *p_wk, u8 id );
static void EX_HEIGHT_Set2( GYM_WATER_TMP *p_wk, u16 obj_idx );

// TASK追加
static void ANM_TASK_Add( GYM_WATER_TMP *p_wk, u16 obj_idx, u8 anm_num );
// task関数
static void tcb_CheckAnime( GFL_TCB *tcb, void* work );

//-------------------------------------
///	セーブ情報
//=====================================
static void SAVE_SetHasuPos( GYM_WATER_SV_WORK *p_wk, const fx32 x, const fx32 z, const int idx );
static void SAVE_GetHasuPos( GYM_WATER_SV_WORK *p_wk, fx32 *p_x, fx32 *p_z, const int idx );
static void SAVE_SetSaveFlag( GYM_WATER_SV_WORK *p_wk );
static BOOL SAVE_GetSaveFlag( GYM_WATER_SV_WORK *p_wk );
static void SAVE_SetObjIdx( GYM_WATER_SV_WORK *p_wk, int idx, const u8 obj_idx );
static u8   SAVE_GetObjIdx( GYM_WATER_SV_WORK *p_wk, int idx );
// ギミック外でも破棄されない
static void SAVE_SetClearFlag( GIMMICKWORK* p_gmkwk, const BOOL flag );
static BOOL SAVE_GetClearFlag( GIMMICKWORK* p_gmkwk );


#ifdef GYM_WATER_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int  debug_frame = HASU_MOVE_SPEED;
static s16  debug_height = HASU_SHAKE_HEIGHT;
static u32  debug_sync = HASU_SHAKE_SYNC;
static int  debug_ripple = WATER_MOVE_RIPPLE_INTERVAL;

#endif // GYM_WATER_DEBUG_MENU


//=============================================================================
/**
 *          外部公開関数
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief     セットアップ関数
 * @param	    p_fieldmap     フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_WATER_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_WATER_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_WATER_TMP));
  
  p_wk->exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap = p_fieldmap;
  p_wk->p_sv_wk = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_GYM_WATER );

  p_wk->now_hasu_ride = EX_HEIGHT_RIDE_NO;
  p_wk->ex_height_idx = 0;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 初回セーブフラグが立っている場合
  if( SAVE_GetSaveFlag( p_wk->p_sv_wk ) == TRUE )
  {
    HASU_InitExHeight( p_wk );
  }


#ifdef GYM_WATER_DEBUG_MENU
  DEBUGMENU_Init( p_wk, p_fieldmap, HEAPID_FIELDMAP );
#endif // GYM_WATER_DEBUG_MENU
}

//--------------------------------------------------------------
/**
 * @brief   解放関数
 * @param   p_fieldmap     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_WATER_End(FIELDMAP_WORK *p_fieldmap)
{
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );

#ifdef GYM_WATER_DEBUG_MENU
  DEBUGMENU_Exit();
#endif // GYM_WATER_DEBUG_MENU


  // ハス拡張高さの破棄
  HASU_ExitExHeight( p_wk );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_WATER_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @brief       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_WATER_Move(FIELDMAP_WORK *p_fieldmap)
{
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
      GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);
      SAVE_SetClearFlag( p_gmkwk, TRUE );
      ICHI_Printf("クリアフラグ TRUE \n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
      GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);
      SAVE_SetClearFlag( p_gmkwk, FALSE );
      ICHI_Printf("クリアフラグ FALSE \n");
    }
  }
#endif

  // ハス降りるアニメ中チェック
  if( p_wk->is_anm_hasu == TRUE )
  {
    // ※ハス降りるアニメ開始の後に呼ばないこと。
    // アニメフレームが0なので、終了とみなされてしまう
    if( ANIME_HASU_LeaveCheck( p_wk ) == TRUE )
    {
      // ANIME終了
      ANIME_HASU_LeaveEnd( p_wk );
    }
  }

  // ハスの上にいる場合は降りたチェック
  if( p_wk->now_hasu_ride != EX_HEIGHT_RIDE_NO )
  {
    EX_HEIGHT_RIDE_TYPE ret_ride;
    s16 gx, gy, gz;
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( cp_player );

    // 自機過去グリッド位置
    // ※過去座標でないと、一瞬拡張高さの消えた状態が出来てしまう
    gx = MMDL_GetOldGridPosX( mmdl);
    gz = MMDL_GetOldGridPosZ( mmdl);

    ret_ride = EX_HEIGHT_CheckHasuRide( p_wk, gx, gz );
    switch( ret_ride ){
    // 自分のハスに乗っている(HASU_EX_OBJ_IDX_MINE
    case EX_HEIGHT_RIDE_MINE:
      {
        // 周りのハスからきたか？
        if( p_wk->now_hasu_ride == EX_HEIGHT_RIDE_OTHER )
        {
          // ハスの上タイプ更新
          p_wk->now_hasu_ride = EX_HEIGHT_RIDE_MINE;

          // ハス降りるアニメ
          // ※拡張高さをリセットする前に呼ぶこと
          ANIME_HASU_LeaveStart( p_wk, EX_HEIGHT_RIDE_OTHER );

          // SE 再生
          PMSND_PlaySE( GYM_WATER_SE_GET_OFF );

          ICHI_Printf("周りのハスから自分のハスに移動\n");
        }
      }
      break;
    // 周りのハスに乗っている(HASU_EX_OBJ_IDX_OTHER
    case EX_HEIGHT_RIDE_OTHER:
      {
        // 自分のハスからきたか？
        if( p_wk->now_hasu_ride == EX_HEIGHT_RIDE_MINE )
        {
          // ハスの上タイプ更新
          p_wk->now_hasu_ride = EX_HEIGHT_RIDE_OTHER;

          // ハス降りるアニメ
          ANIME_HASU_LeaveStart( p_wk, EX_HEIGHT_RIDE_MINE );

          // SE 再生
          PMSND_PlaySE( GYM_WATER_SE_GET_OFF );

          ICHI_Printf("自分のハスから周りのハスに移動\n");
        }
      }
      break;
    // 乗っていない
    case EX_HEIGHT_RIDE_NO:
      {
        // 自分のハスからきたか？
        if( p_wk->now_hasu_ride == EX_HEIGHT_RIDE_MINE )
        {
          // ハス降りるアニメ
          // ※拡張高さをリセットする前に呼ぶこと
          ANIME_HASU_LeaveStart( p_wk, EX_HEIGHT_RIDE_MINE );
        }
        else
        {
          // ハス降りるアニメ
          // ※拡張高さをリセットする前に呼ぶこと
          ANIME_HASU_LeaveStart( p_wk, EX_HEIGHT_RIDE_OTHER );
        }

        // ハスの上タイプ更新
        p_wk->now_hasu_ride = EX_HEIGHT_RIDE_NO;

        // 拡張高さリセット
        EX_HEIGHT_Reset( p_wk );

        // SE 再生
        PMSND_PlaySE( GYM_WATER_SE_GET_OFF );

        ICHI_Printf("ハスから降りた\n");

      }
      break;
    default:
      GF_ASSERT(0);
    }
  }

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ハスイベントチェック
 *
 *	@param	*p_gamesys  ゲームシステムへのポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_WATER_CheckHasuEvent( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_WATER_TMP_ASSIGN_ID);
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );

  GMEVENT *event;
  s16 gx, gy, gz;
  u8  i;
  u16 obj_idx = 0;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &gx, &gy, &gz );

  for( i=0; i<HASU_POS_NUM; i++ )
  {
    // 起動位置チェック
    if( gx == HasuPosParam[i].gx && gz == HasuPosParam[i].gz )
    {
      // DIR方向にハスがあるか調べる
      fx32 x;
      fx32 z;
      int center_gx;
      int center_gz;

      // 起動位置のハスIDの中央グリッド座標を取得
      HASU_GetCenterPos( i, &center_gx, &center_gz );

      x = GRID_TO_FX32(center_gx);
      z = GRID_TO_FX32(center_gz);

      // 半グリッドずれを調整
      x += GRID_HALF_SIZE;
      z += GRID_HALF_SIZE;

      // ハス存在チェック
      if( HASU_CheckPos( p_wk->exp_obj, x, z, &obj_idx ) )
      {
        ICHI_Printf("はす %d イベント起動\n", i );

        event = HASU_CreateEvent( NULL, p_gamesys, i, obj_idx );
        return event;
      }
    }
  }

  return NULL;
}

//=============================================================================
/**
 *      static関数
 */
//=============================================================================
//=============================================================================
/**
 *      EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GYM_WATER_TMP *p_wk )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
  GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);

  u8 i;

  // クリアフラグ
  BOOL clear_flag = SAVE_GetClearFlag( p_gmkwk );

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GYM_WATER_UNIT_IDX);

  //=====================================
  // ハスの初期設定
  for( i=OBJ_HASU_1; i<OBJ_HASU_1+HASU_NUM; i++ )
  {
    VecFx32 pos[ HASU_NUM ] = {
      { HASU1_X, 0, HASU1_Z },
      { HASU2_X, 0, HASU2_Z },
      { HASU3_X, 0, HASU3_Z },
      { HASU4_X, 0, HASU4_Z },
      { HASU5_X, 0, HASU5_Z },
      { HASU6_X, 0, HASU6_Z },
      { HASU7_X, 0, HASU7_Z },
      { HASU8_X, 0, HASU8_Z },
      { HASU9_X, 0, HASU9_Z },
      { HASU10_X, 0, HASU10_Z },
      { HASU11_X, 0, HASU11_Z },
    };

    VecFx32 clear_pos[ HASU_NUM ] = {
      { HASU1_CLEAR_X, 0, HASU1_CLEAR_Z },
      { HASU2_CLEAR_X, 0, HASU2_CLEAR_Z },
      { HASU3_CLEAR_X, 0, HASU3_CLEAR_Z },
      { HASU4_CLEAR_X, 0, HASU4_CLEAR_Z },
      { HASU5_CLEAR_X, 0, HASU5_CLEAR_Z },
      { HASU6_CLEAR_X, 0, HASU6_CLEAR_Z },
      { HASU7_CLEAR_X, 0, HASU7_CLEAR_Z },
      { HASU8_CLEAR_X, 0, HASU8_CLEAR_Z },
      { HASU9_CLEAR_X, 0, HASU9_CLEAR_Z },
      { HASU10_CLEAR_X, 0, HASU10_CLEAR_Z },
      { HASU11_CLEAR_X, 0, HASU11_CLEAR_Z },
    };

    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    // 座標
    // セーブされている座標があればその座標
    // ないならデフォルト座標
    {
      fx32 save_x;
      fx32 save_z;
      // セーブデータから取得
      SAVE_GetHasuPos( p_wk->p_sv_wk,
          &save_x,
          &save_z,
          i );

      if( save_x == 0 && save_z == 0 )
      {
        if( clear_flag == FALSE )
        {
          // クリア前座標
          status->trans.x = pos[i].x;
          status->trans.z = pos[i].z;
        }
        else
        {
          // クリア後座標
          status->trans.x = clear_pos[i].x;
          status->trans.z = clear_pos[i].z;
        }
      }
      else
      {
        status->trans.x = save_x;
        status->trans.z = save_z;
      }
      status->trans.y = 0;
    }
    

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, FALSE );

    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;

      // 常時
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM );
      // アニメ有効
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM, TRUE );

      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定

      // 降りるとき
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE );
      // アニメ無効
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE, FALSE );

      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }

  //=====================================
  // 演出の初期設定
  for( i=EFFECT_HIT_RIPPLE1; i<EFFECT_MOVE_RIPPLE_MAX; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    VEC_Set( &status->trans, 0,0,0 );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0 );
      // IMA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, FALSE );
      // ITA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 1, FALSE );

      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }
  {
    u16 obj_idx = EFFECT_SPLASH;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    VEC_Set( &status->trans, 0,0,0 );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0 );
      // ITA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, FALSE );

      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GYM_WATER_TMP *p_wk )
{
  int i;

  //=====================================
  // ハスの初期設定
  for( i=OBJ_HASU_1; i<OBJ_HASU_1+HASU_NUM; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    // セーブデータ設定
    SAVE_SetHasuPos( p_wk->p_sv_wk,
        status->trans.x,
        status->trans.z,
        obj_idx );
  }
  // セーブしたフラグを立てる
  SAVE_SetSaveFlag( p_wk->p_sv_wk );


  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GYM_WATER_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GYM_WATER_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}


//=============================================================================
// ハスイベント
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハスイベント作成
 *
 *	@param	parent  親イベント
 *	@param	gsys    ゲームシステム
 *	@param	id      ハスID
 *	@param  obj_idx OBJインデックス
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * HASU_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, u8 id, u16 obj_idx )
{
  GMEVENT*          event;
  HASU_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER      *player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント生成
  event = GMEVENT_Create( gsys, parent, HasuEvent, sizeof(HASU_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = player;

  work->id          = id;
  work->obj_idx     = obj_idx;

  work->is_collision        = FALSE;
  work->other_obj_idx       = 0;
  work->other_move_distance = 0;
  work->wait_frame = 0;

  work->camera   = FIELDMAP_GetFieldCamera( p_fieldmap );

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハスイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT HasuEvent( GMEVENT* event, int* seq, void* wk )
{
  HASU_EVENT_WORK *p_wk = (HASU_EVENT_WORK*)wk;
  GYM_WATER_TMP  *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_WATER_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_START,            // イベント開始
    SEQ_JUMP,             // 飛び乗り
    SEQ_CAMERA_TRACE_STOP,// トレース終了
    SEQ_CAMERA_TRACE_STOP_WAIT,// トレース終了待ち
    SEQ_MOVE_INIT,        // ハス移動初期化
    SEQ_MOVE,             // ハス移動
    SEQ_MOVE_END,         // ハス移動終了(衝突エフェクト)
    SEQ_OTHER_MOVE_INIT,  // 衝突ハス移動初期化
    SEQ_OTHER_MOVE,       // 衝突ハス移動
    SEQ_EX_HEIGHT_SET,    // 拡張高さ設定
    SEQ_SAVE,             // セーブ
    SEQ_END,
  };

  switch( *seq ){
  // イベント開始
  case SEQ_START:
    {
      // 拡張高さリセット
      EX_HEIGHT_Reset( p_gym_wk );

      // 拡張高さセット
      EX_HEIGHT_Set( p_gym_wk, p_wk->id );

      *seq = SEQ_JUMP;
    }
    break;

  // 飛び乗り
  case SEQ_JUMP:
    {
      // アニメ終了待ち
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
      {
        // ジャンプアニメ
        u16   dir   = FIELD_PLAYER_GetDir( p_wk->player );
        MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
        MMDL_SetAcmd( mmdl, AC_JUMP_U_2G_16F + dir ); 

        // SE 再生
        //PMSND_PlaySE( GYM_WATER_SE_JUMP );

        *seq = SEQ_CAMERA_TRACE_STOP;
      }
    }
    break;

  // トレース終了
  case SEQ_CAMERA_TRACE_STOP:
    {
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) == FALSE )
      {
        break;
      }

      //トレース終了
      FIELD_CAMERA_StopTraceRequest(p_wk->camera);
      *seq = SEQ_CAMERA_TRACE_STOP_WAIT;
    }
    /* break through */

  // トレース終了待ち
  case SEQ_CAMERA_TRACE_STOP_WAIT:
    {
      if ( FIELD_CAMERA_CheckTrace(p_wk->camera) )
      {
        //break;
      }

      {
        //復帰パラメータをセットする
        FIELD_CAMERA_SetRecvCamParam(p_wk->camera);

        //カメラのターゲットを取得
        p_wk->WatchTarget = FIELD_CAMERA_GetWatchTarget(p_wk->camera);
        //バインド解除
        FIELD_CAMERA_FreeTarget(p_wk->camera);

        //補間移動はカメラ位置不定型のみのサポート
        FIELD_CAMERA_ChangeMode( p_wk->camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
      }

      *seq = SEQ_MOVE_INIT;
    }
    /* break through */
  
  // ハス移動初期化
  case SEQ_MOVE_INIT:
    {
      int move_distance = HASU_EVENT_GetMoveDistance( p_wk, p_wk->id );
      VecFx32 move_pos;
      int frame;
      // 実際の移動するのは3グリッド少ない
      move_distance -= 3;
      ICHI_Printf("移動距離 %d\n", move_distance);

      if( move_distance == 0 )
      {
        // 移動しないときは水しぶきのみだす
        // 水しぶき
        ANIME_WATER_Splash( p_wk );

        // SE 再生
        PMSND_PlaySE( GYM_WATER_SE_LAND );

        *seq = SEQ_EX_HEIGHT_SET;
        break;
      }

      // 移動後位置計算
      {
        FIELD_PLAYER_GetPos( p_wk->player, &move_pos );
        switch( HasuPosParam[ p_wk->id ].move_dir ){
        case DIR_UP:
          move_pos.z -= GRID_TO_FX32(move_distance);
          break;
        case DIR_DOWN:
          move_pos.z += GRID_TO_FX32(move_distance);
          break;
        case DIR_LEFT:
          move_pos.x -= GRID_TO_FX32(move_distance);
          break;
        case DIR_RIGHT:
          move_pos.x += GRID_TO_FX32(move_distance);
          break;
        default:
          GF_ASSERT(0);
        }
      }

      // フレーム数
#ifdef GYM_WATER_DEBUG_MENU
      frame = move_distance*debug_frame;
#else
      frame = move_distance*HASU_MOVE_SPEED;
#endif

      // ハス移動初期化
      HASU_EVENT_MoveInit( p_wk, frame, &move_pos );


      // カメラ移動
      {
        FIELD_TASK* cam;
        FIELD_TASK_MAN* taskMan;

        VecFx32 start_pos;    // 初期座標
        VecFx32 end_pos;      // 最終座標

        FIELD_PLAYER_GetPos( p_wk->player, &start_pos );
        end_pos = move_pos;

        cam = FIELD_TASK_CameraTargetMove( p_wk->p_fieldmap, frame, &start_pos, &end_pos );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, cam, NULL );
      }

      // 水しぶき
      ANIME_WATER_Splash( p_wk );

      // SE 再生
      PMSND_PlaySE( GYM_WATER_SE_LAND );

      *seq = SEQ_MOVE;

    }
    break;

  // ハス移動
  case SEQ_MOVE:
    {
      // ハス移動終了チェック
      if( HASU_EVENT_MoveMain( p_wk ) == TRUE )
      {
        // 衝突波紋(自分
        ANIME_WATER_HitRipple( p_wk, p_wk->obj_idx, ANM_HIT_RIPPLE_MINE );

        if( p_wk->is_collision == TRUE )
        {
          // 衝突波紋(相手
          ANIME_WATER_HitRipple( p_wk, p_wk->other_obj_idx, ANM_HIT_RIPPLE_OTHER );
          // 少しだけ待つ
          p_wk->wait_frame = 7;
        }

        

        // SE 再生
        PMSND_PlaySE( GYM_WATER_SE_HIT );

        *seq = SEQ_MOVE_END;
      }
    }
    break;

  // ハス移動終了(衝突エフェクト)
  case SEQ_MOVE_END:
    {
      // 少しだけ待つ
      if( p_wk->wait_frame-- == 0 )
      {
        if( p_wk->is_collision == TRUE )
        {
          // ハス衝突した
          *seq = SEQ_OTHER_MOVE_INIT;
        }
        else
        {
          *seq = SEQ_EX_HEIGHT_SET;
        }
      }
    }
    break;

  // 衝突ハス移動初期化
  case SEQ_OTHER_MOVE_INIT:
    {
      BOOL ret;
      ret = HASU_EVENT_OtherMoveInit( p_wk );

      // 移動有無で分岐
      if( ret ) { *seq = SEQ_OTHER_MOVE; }
      else      { *seq = SEQ_EX_HEIGHT_SET; }
    }
    break;

  // 衝突ハス移動
  case SEQ_OTHER_MOVE:
    {
      // ハス移動終了チェック
      if( HASU_EVENT_OtherMoveMain( p_wk ) == TRUE )
      {
        // 衝突波紋
        ANIME_WATER_HitRipple( p_wk, p_wk->other_obj_idx, ANM_HIT_RIPPLE_MINE );

        // SE 再生
        PMSND_PlaySE( GYM_WATER_SE_HIT );

        *seq = SEQ_EX_HEIGHT_SET;
      }
    }
    break;

  // 拡張高さ設定
  case SEQ_EX_HEIGHT_SET:
    {
      // 拡張高さリセット
      EX_HEIGHT_Reset( p_gym_wk );

      // 拡張高さ更新(自分のいるハス)
      EX_HEIGHT_Set2( p_gym_wk, p_wk->obj_idx );

      // 周りのハスを調べて、拡張高さ更新
      HASU_SetExHeight_DirMax4( p_gym_wk );

      // ハスの上フラグを立てる
      p_gym_wk->now_hasu_ride = EX_HEIGHT_RIDE_MINE;

      // カメラ
      {
        //カメラオフセットクリア
        {
          VecFx32 cam_ofs = {0,0,0};
          FIELD_CAMERA_SetCamPosOffset( p_wk->camera, &cam_ofs );
        }
        //復帰パラメータをクリアする
        FIELD_CAMERA_ClearRecvCamParam(p_wk->camera);

        //際バインド
        FIELD_CAMERA_BindTarget(p_wk->camera, p_wk->WatchTarget);
        //トレース再開
        FIELD_CAMERA_RestartTrace(p_wk->camera);
      }

      *seq = SEQ_SAVE;
    }
    break;

  // セーブ
  case SEQ_SAVE:
    {
      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_gym_wk->exp_obj, GYM_WATER_UNIT_IDX, p_wk->obj_idx );

      // ハス位置設定
      SAVE_SetHasuPos( p_gym_wk->p_sv_wk,
          status->trans.x,
          status->trans.z,
          p_wk->obj_idx );

      if( p_wk->is_collision == TRUE )
      {
        status = FLD_EXP_OBJ_GetUnitObjStatus( p_gym_wk->exp_obj, GYM_WATER_UNIT_IDX, p_wk->other_obj_idx );

        // ハス位置設定
        SAVE_SetHasuPos( p_gym_wk->p_sv_wk,
            status->trans.x,
            status->trans.z,
            p_wk->other_obj_idx );
      }

      // 自分のいるハスのOBJインデックスを保存
      SAVE_SetObjIdx( p_gym_wk->p_sv_wk, HASU_EX_OBJ_IDX_MINE, p_wk->obj_idx );
      // ※周りのハスのOBJインデックスはHASU_SetExHeight_DirMax4で設定している
      
      // セーブしたフラグを立てる
      SAVE_SetSaveFlag( p_gym_wk->p_sv_wk );

      // ハスIDがHASU_ID_CLEARはギミック解いたとなる
      if( p_wk->id == HASU_ID_CLEAR )
      {
        GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
        GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
        GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);

        // クリアフラグON
        SAVE_SetClearFlag( p_gmkwk, TRUE );
      }

      *seq = SEQ_END;
    }
    /* break throw */

  case SEQ_END:
    {
      return GMEVENT_RES_FINISH;
    }
    break;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *        HASU
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  指定位置にハスがあるかチェック
 *
 *	@param	exp_obj 拡張3Dオブジェポインタ
 *	@param	fx32 x  X座標
 *	@param	fx32 z  Z座標
 *	@param  *obj_idx  ハスがあった場合そのOBJインデックス
 *
 *	@return TRUEだと存在する
 */
//-----------------------------------------------------------------------------
static BOOL HASU_CheckPos( FLD_EXP_OBJ_CNT_PTR exp_obj, const fx32 x, const fx32 z, u16 *p_obj_idx )
{
  u8 i;

  for( i=OBJ_HASU_1; i<OBJ_HASU_1+HASU_NUM; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    if( status->trans.x == x && status->trans.z == z )
    {
      *p_obj_idx = obj_idx;

      // 存在する
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハスIDの中央位置を求める
 *
 *	@param	id      ハスID 
 *	@param	*p_gx   グリッドX座標を入れるポインタ
 *	@param	*p_gz   グリッドZ座標を入れるポインタ
 *
 *	@return 取得できたらTRUE
 */
//-----------------------------------------------------------------------------
static BOOL HASU_GetCenterPos( u8 id, int *p_gx, int *p_gz )
{
  int x, z;

  // ハスの中心までの差分
  const int diff = 2;   // 2グリッド

  if( id > HASU_POS_NUM )
  {
    GF_ASSERT_MSG( 0, "ハスIDが上限を超えています\n");
    return FALSE;
  }

  x = HasuPosParam[ id ].gx;
  z = HasuPosParam[ id ].gz;

  switch( HasuPosParam[ id ].move_dir ){
  case DIR_UP:    z -= diff; break;
  case DIR_DOWN:  z += diff; break;
  case DIR_LEFT:  x -= diff; break;
  case DIR_RIGHT: x += diff; break;
  default:
    GF_ASSERT(0);
  }

  *p_gx = x;
  *p_gz = z;

  return TRUE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  自機の位置から四方向を調べてハスがあると拡張高さを設定する
 *
 *	@param	GYM_WATER_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void HASU_SetExHeight_DirMax4( GYM_WATER_TMP *p_wk )
{
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );

  const int merge = (3);
  VecFx32 pos;
  u16 obj_idx = 0;
  s16 start_gx;
  s16 start_gy;
  s16 start_gz;
  int i;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &start_gx, &start_gy, &start_gz );

  // 4方向チェック
  for( i=0; i<DIR_MAX4; i++ )
  {
    fx32 x;
    fx32 z;
    int gx = start_gx;
    int gz = start_gz;

    switch( i ){
    case DIR_UP:    gz -= merge; break;
    case DIR_DOWN:  gz += merge; break;
    case DIR_LEFT:  gx -= merge; break;
    case DIR_RIGHT: gx += merge; break;
    default: GF_ASSERT(0);
    }

    x = GRID_TO_FX32(gx);
    z = GRID_TO_FX32(gz);

    // 半グリッドずれを調整
    x += GRID_HALF_SIZE;
    z += GRID_HALF_SIZE;

    if( HASU_CheckPos( p_wk->exp_obj, x, z, &obj_idx ) )
    {
      ICHI_Printf("周りにハス %d が見つかった\n", obj_idx );
      // 拡張高さセット
      EX_HEIGHT_Set2( p_wk, obj_idx );
      // セーブしておく
      SAVE_SetObjIdx( p_wk->p_sv_wk, HASU_EX_OBJ_IDX_OTHER, obj_idx );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス拡張高さの初期化
 *
 *	@param	p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void HASU_InitExHeight( GYM_WATER_TMP *p_wk )
{
  int i;

  for( i=0; i<2; i++ )
  {
    u8 ex_obj_idx = SAVE_GetObjIdx( p_wk->p_sv_wk, i );
    if( ex_obj_idx != HASU_EX_OBJ_NULL )
    {
      // 拡張高さセット
      EX_HEIGHT_Set2( p_wk, ex_obj_idx );
    }
  }

  // ハスの上情報
  {
    const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
    s16 gx, gy, gz;
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( cp_player );
    // 自機グリッド位置
    gx = MMDL_GetGridPosX( mmdl);
    gz = MMDL_GetGridPosZ( mmdl);

    p_wk->now_hasu_ride = EX_HEIGHT_CheckHasuRide( p_wk, gx, gz );
    ICHI_Printf("ハス乗タイプ %d\n", p_wk->now_hasu_ride );
  }
  
  // 接地させる
  {
    const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( cp_player );
    MMDL_UpdateCurrentHeight( mmdl );   // 接地
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス拡張高さの破棄
 *
 *	@param	p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void HASU_ExitExHeight( GYM_WATER_TMP *p_wk )
{
  // ハスに乗っていない場合は、NULLを入れておく
  if( p_wk->now_hasu_ride == EX_HEIGHT_RIDE_NO )
  {
    SAVE_SetObjIdx( p_wk->p_sv_wk, HASU_EX_OBJ_IDX_MINE, HASU_EX_OBJ_NULL );
    SAVE_SetObjIdx( p_wk->p_sv_wk, HASU_EX_OBJ_IDX_OTHER, HASU_EX_OBJ_NULL );
  }
  // ハスに乗っている場合は、
  // フラグを立てている場所で、すでにセーブしている
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス起動IDからハスの移動距離を求める
 *	        途中にハスがある場合はそのハスまでの距離を求める
 *
 *	@param	p_wk  イベントワーク
 *	@param	id    ハスID
 *
 *	@return 移動距離(グリッド)
 */
//-----------------------------------------------------------------------------
static int  HASU_EVENT_GetMoveDistance( HASU_EVENT_WORK *p_wk, u8 id )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );

  int i;
  u16 obj_idx = 0;
  int start_gx;
  int start_gz;

  HASU_GetCenterPos( id, &start_gx, &start_gz );

  for( i=1; i< HasuPosParam[ id ].move_distance; i++ )
  {
    fx32 x;
    fx32 z;
    int gx = start_gx;
    int gz = start_gz;

    switch( HasuPosParam[ id ].move_dir ){
    case DIR_UP:    gz -= i; break;
    case DIR_DOWN:  gz += i; break;
    case DIR_LEFT:  gx -= i; break;
    case DIR_RIGHT: gx += i; break;
    }

    x = GRID_TO_FX32(gx);
    z = GRID_TO_FX32(gz);

    // 半グリッドずれを調整
    x += GRID_HALF_SIZE;
    z += GRID_HALF_SIZE;

    if( HASU_CheckPos( exp_obj, x, z, &obj_idx ) )
    {
      ICHI_Printf("途中にハス %d が見つかった\n", obj_idx );
      
      // 衝突ハス情報
      {
        p_wk->is_collision        = TRUE;
        p_wk->other_obj_idx       = obj_idx;
        p_wk->other_move_distance = HasuPosParam[ id ].move_distance - i;
      }

      return i;
    }
  }

  // 見つからない場合は設定された距離
  return HasuPosParam[ id ].move_distance;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス移動初期化
 *
 *	@param	p_wk    イベントワーク
 *	@param	frame   動作フレーム数
 *	@param	pos     移動先座標
 */
//-----------------------------------------------------------------------------
static void HASU_EVENT_MoveInit( HASU_EVENT_WORK *p_wk, int frame, VecFx32* pos )
{
  HASU_MOVE_WORK* p_move_wk = &p_wk->move_wk;

  // 初期化
  p_move_wk->frame = 0;
  p_move_wk->endFrame = frame;
  VEC_Set( &p_move_wk->endPos, pos->x, pos->y, pos->z );
  FIELD_PLAYER_GetPos( p_wk->player, &p_move_wk->startPos );

#ifdef GYM_WATER_DEBUG_MENU
  p_move_wk->Height = debug_height;
  p_move_wk->Sync   = debug_sync;
#else
  p_move_wk->Height = HASU_SHAKE_HEIGHT;
  p_move_wk->Sync   = HASU_SHAKE_SYNC;
#endif
  p_move_wk->NowSync= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス移動メイン
 *
 *	@param	p_wk    イベントワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL HASU_EVENT_MoveMain( HASU_EVENT_WORK *p_wk )
{
  HASU_MOVE_WORK* p_move_wk = &p_wk->move_wk;
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );

  VecFx32 out_pos;      // 移動後位置
  BOOL    end = FALSE;

  p_move_wk->frame++;
 
  // 終了チェック
  if( p_move_wk->endFrame <= p_move_wk->frame )
  {
    end = TRUE;
  }

  //=====================================
  // 移動
  {
    VecFx32 v0, v1;
    fx32 w1 = FX_F32_TO_FX32( (float)p_move_wk->frame / (float)p_move_wk->endFrame );
    fx32 w0 = FX32_ONE - w1;

    GFL_CALC3D_VEC_MulScalar( &p_move_wk->startPos, w0, &v0 );
    GFL_CALC3D_VEC_MulScalar( &p_move_wk->endPos,   w1, &v1 );
    VEC_Add( &v0, &v1, &out_pos );
  }

  //=====================================
  // ゆれ
  {
    int   tmp;
    u16   rad;
    fx32  h;            // ハスゆれ値
    p_move_wk->NowSync++;
    tmp = (p_move_wk->NowSync * 0x10000) / p_move_wk->Sync;
    rad = tmp + (65536/2);    // マイナスからにする
    h = p_move_wk->Height * FX_SinIdx(rad);

    // ゆれセット
    if( end )
    {
      // オフセット初期化
      h = 0;
    }

    // ゆれ加算
    out_pos.y += h;
  }


  //=====================================
  // 自機更新
  FIELD_PLAYER_SetPos( p_wk->player, &out_pos );   // 移動

  // ハス更新
  {
    int obj_idx = p_wk->obj_idx;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_WATER_UNIT_IDX, obj_idx);
    status->trans = out_pos;
  }
  

  // エフェクト波紋
#ifdef GYM_WATER_DEBUG_MENU
  if( ( p_move_wk->frame % debug_ripple) == 0 )
#else
  if( ( p_move_wk->frame % WATER_MOVE_RIPPLE_INTERVAL) == 0 )
#endif
  {
    ANIME_WATER_MoveRipple( p_wk, p_wk->obj_idx );
  }

  // 終了チェック
  return end;
}

//----------------------------------------------------------------------------
/**
 *	@brief  衝突ハス移動初期化
 *
 *	@param	p_wk    イベントワーク
 *
 *	@return TRUEで移動する FALSE移動しない
 */
//-----------------------------------------------------------------------------
static BOOL HASU_EVENT_OtherMoveInit( HASU_EVENT_WORK *p_wk )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  HASU_MOVE_WORK* p_move_wk = &p_wk->move_wk;

  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_WATER_UNIT_IDX, p_wk->other_obj_idx);

  int move_distance = p_wk->other_move_distance;
  VecFx32 move_pos;
  // 実際の移動するのは3グリッド少ない
  move_distance -= 3;
  ICHI_Printf("移動距離 %d\n", move_distance);

  if( move_distance == 0 )
  {
    // 移動しない
    return FALSE;
  }

  // 移動後位置計算
  {
    move_pos = status->trans;
    switch( HasuPosParam[ p_wk->id ].move_dir ){
    case DIR_UP:
      move_pos.z -= GRID_TO_FX32(move_distance);
      break;
    case DIR_DOWN:
      move_pos.z += GRID_TO_FX32(move_distance);
      break;
    case DIR_LEFT:
      move_pos.x -= GRID_TO_FX32(move_distance);
      break;
    case DIR_RIGHT:
      move_pos.x += GRID_TO_FX32(move_distance);
      break;
    default:
      GF_ASSERT(0);
    }
  }

  // 初期化
  p_move_wk->frame = 0;
#ifdef GYM_WATER_DEBUG_MENU
  p_move_wk->endFrame = move_distance*debug_frame;
#else
  p_move_wk->endFrame = move_distance*HASU_MOVE_SPEED;
#endif
  VEC_Set( &p_move_wk->endPos, move_pos.x, move_pos.y, move_pos.z );
  p_move_wk->startPos = status->trans;

#ifdef GYM_WATER_DEBUG_MENU
  p_move_wk->Height = debug_height;
  p_move_wk->Sync   = debug_sync;
#else
  p_move_wk->Height = HASU_SHAKE_HEIGHT;
  p_move_wk->Sync   = HASU_SHAKE_SYNC;
#endif
  p_move_wk->NowSync= 0;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  衝突ハス移動メイン
 *
 *	@param	p_wk    イベントワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL HASU_EVENT_OtherMoveMain( HASU_EVENT_WORK *p_wk )
{
  HASU_MOVE_WORK* p_move_wk = &p_wk->move_wk;
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );

  VecFx32 out_pos;      // 移動後位置
  fx32    h;            // ハスゆれ値
  BOOL    end = FALSE;

  p_move_wk->frame++;
 
  // 終了チェック
  if( p_move_wk->endFrame <= p_move_wk->frame )
  {
    end = TRUE;
  }

  //=====================================
  // 移動
  {
    VecFx32 v0, v1;
    fx32 w1 = FX_F32_TO_FX32( (float)p_move_wk->frame / (float)p_move_wk->endFrame );
    fx32 w0 = FX32_ONE - w1;

    GFL_CALC3D_VEC_MulScalar( &p_move_wk->startPos, w0, &v0 );
    GFL_CALC3D_VEC_MulScalar( &p_move_wk->endPos,   w1, &v1 );
    VEC_Add( &v0, &v1, &out_pos );
  }

  //=====================================
  // ゆれ
  {
    int tmp;
    u16 rad;
    p_move_wk->NowSync++;
    tmp = (p_move_wk->NowSync * 0x10000) / p_move_wk->Sync;
    rad = tmp;
    h = p_move_wk->Height * FX_SinIdx(rad);

    if( end )
    {
      // オフセット初期化
      h = 0;
    }
  }

  //=====================================
  // ハス更新
  {
    int obj_idx = p_wk->other_obj_idx;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_WATER_UNIT_IDX, obj_idx);
    out_pos.y += h;
    status->trans = out_pos;
  }

  // エフェクト波紋
#ifdef GYM_WATER_DEBUG_MENU
  if( ( p_move_wk->frame % debug_ripple) == 0 )
#else
  if( ( p_move_wk->frame % WATER_MOVE_RIPPLE_INTERVAL) == 0 )
#endif
  {
    ANIME_WATER_MoveRipple( p_wk, p_wk->other_obj_idx );
  }

  // 終了チェック
  return end;
}

//=============================================================================
/**
 *        ANIME
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハスから出るアニメ再生
 *
 *	@param	p_wk  ギミックワーク
 *	@param  ride  EX_HEIGHT_RIDE_MINE or EX_HEIGHT_RIDE_OTHER
 */
//-----------------------------------------------------------------------------
static void ANIME_HASU_LeaveStart( GYM_WATER_TMP *p_wk, EX_HEIGHT_RIDE_TYPE ride )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  u16 obj_idx;    // OBJインデックス

  GF_ASSERT_MSG( ride!=EX_HEIGHT_RIDE_NO, "ハスの上情報が間違ってる");

  // セーブデータから ハスのOBJインデックスを取得
  obj_idx = SAVE_GetObjIdx( p_wk->p_sv_wk, ride );

  // 自機の向きによって回転
  {
    const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
    u16 rtbl[DIR_MAX4][3] = {{0,0xc000,0},{0,0xc000,0},{0,0,0},{0,0x8000,0}};
    u16 dir = FIELD_PLAYER_GetDir( cp_player );
    u16 *rot = rtbl[ dir ];

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    GFL_CALC3D_MTX_CreateRot( rot[0], rot[1], rot[2], &status->rotate );
  }

  // ハス常時アニメは停止する
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ停止

    // 常時
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM, FALSE );
  }

  // ハス降りるときアニメを再生する
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE );

    // 降りるとき
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE, TRUE );
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE, 0 );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
  }
  

  // フラグ立てる
  p_wk->is_anm_hasu = TRUE;
  p_wk->anm_hasu_obj= obj_idx;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハスから出るアニメ終了チェック
 *
 *	@param	GYM_WATER_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANIME_HASU_LeaveCheck( GYM_WATER_TMP *p_wk )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, p_wk->anm_hasu_obj, HASU_ANM_MOVE );

  // アニメ終了していないなら戻る
  if( !FLD_EXP_OBJ_ChkAnmEnd(exp_anm) )
  {
    return FALSE;
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハスから出るアニメ終了
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANIME_HASU_LeaveEnd( GYM_WATER_TMP *p_wk )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  u16 obj_idx = p_wk->anm_hasu_obj;    // OBJインデックス

  // 降りるアニメ停止
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM_MOVE, FALSE );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定

  }

  // 常時アニメ再生
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, HASU_ANM, TRUE );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
  }

  // フラグOFF
  p_wk->is_anm_hasu = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  水しぶきアニメ再生
 *
 *	@param	HASU_EVENT_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANIME_WATER_Splash( HASU_EVENT_WORK *p_wk )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  u16 obj_idx = EFFECT_SPLASH;    // OBJインデックス

  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

  // ハスの位置に更新
  {
    GFL_G3D_OBJSTATUS *hasu = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, p_wk->obj_idx );

    status->trans = hasu->trans;
  }

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, FALSE );

  // ANIME再生
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0 );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, TRUE );
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, 0 );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
  }

  // タスク登録
  {
    GYM_WATER_TMP *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );
    ANM_TASK_Add( p_gym_wk, obj_idx, 1 );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動波紋アニメ再生
 *
 *	@param	p_wk          イベントワーク
 *	@param  hasu_obj_idx  表示するハスOBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANIME_WATER_MoveRipple( HASU_EVENT_WORK *p_wk, const u16 hasu_obj_idx )
{

  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  u16 obj_idx;    // OBJインデックス

  // 使用できるOBJを調べる
  {
    int i;
    BOOL is_not = TRUE;
    for( i=EFFECT_MOVE_RIPPLE1; i<EFFECT_MOVE_RIPPLE_MAX; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_WATER_UNIT_IDX, i, 0 );
      if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) )
      {
        obj_idx = i;
        is_not = FALSE;
        break;
      }
    }

    if( is_not )
    {
      ICHI_Printf("余りOBJがない!\n");
      return;
    }
  }


  // ハスの位置に更新
  {
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, obj_idx );
    GFL_G3D_OBJSTATUS *hasu = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, hasu_obj_idx );

    status->trans = hasu->trans;
    status->trans.y = 0;

    // 方向により回転させる
    {
      u16 rtbl[DIR_MAX4][3] = {{0,0x4000,0},{0,0xc000,0},{0,0x8000,0},{0,0,0}};
      u16 *rot;
      u16 dir = FIELD_PLAYER_GetDir( p_wk->player );

      rot = rtbl[ dir ];

      GFL_CALC3D_MTX_CreateRot( rot[0], rot[1], rot[2], &status->rotate );
    }
  }

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, FALSE );

  // ANIME再生
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0 );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, TRUE );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 1, TRUE );
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, 0 );
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 1, 0 );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
  }

  // タスク登録
  {
    GYM_WATER_TMP *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );
    ANM_TASK_Add( p_gym_wk, obj_idx, 2 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  衝突波紋アニメ再生
 *
 *	@param	HASU_EVENT_WORK *p_wk イベントワーク
 *	@param  hasu_obj_idx  表示するハスOBJインデックス
 *	@param	anm_type      アニメ種類 ANM_HIT_RIPPLE_MINE or ANM_HIT_RIPPLE_OTHER
 */
//-----------------------------------------------------------------------------
static void ANIME_WATER_HitRipple( HASU_EVENT_WORK *p_wk, const u16 hasu_obj_idx, const u8 anm_type )
{

  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  u16 obj_idx;    // OBJインデックス

  // アニメ種類
  if( anm_type == ANM_HIT_RIPPLE_MINE ){ obj_idx = EFFECT_HIT_RIPPLE1; }
  else                                 { obj_idx = EFFECT_HIT_RIPPLE2; }

  // ハスの位置に更新
  {
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, obj_idx );
    GFL_G3D_OBJSTATUS *hasu = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, hasu_obj_idx );

    status->trans = hasu->trans;
    status->trans.y = 0;
  }

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, FALSE );

  // ANIME再生
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0 );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, TRUE );
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 1, TRUE );
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, 0 );
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 1, 0 );
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
  }

  // タスク登録
  {
    GYM_WATER_TMP *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );
    ANM_TASK_Add( p_gym_wk, obj_idx, 2 );
  }
}

//=============================================================================
/**
 *        EX_HEIGHT 拡張高さ関連
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハス(拡張高さ)の上に乗っているか
 *
 *	@param	p_wk  ギミックワーク 
 *	@param  gx    チェックするグリッドX座標
 *	@param  gz    チェックするグリッドZ座標
 *
 *	@retval EX_HEIGHT_RIDE_MINE   自分のハスに乗っている
 *	@retval EX_HEIGHT_RIDE_OTHER  周りのハスに乗っている
 *	@retval EX_HEIGHT_RIDE_NO     乗っていない
 */
//-----------------------------------------------------------------------------
static EX_HEIGHT_RIDE_TYPE EX_HEIGHT_CheckHasuRide( GYM_WATER_TMP *p_wk, const int gx, const int gz )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  BOOL ret;
  BOOL mine = FALSE;
  BOOL other = FALSE;
  int  i;

  // 拡張高さヒットチェック
  for( i=0; i<p_wk->ex_height_idx; i++ )
  {
    ret = EXH_HitCheckHeight(	gx, gz, ex_height_list, i );

    if( ret )
    {
      // どこのハスの上にいるのかチェックしてフラグを立てる
      if( (i/2) == EX_HEIGHT_RIDE_MINE )
      {
        mine = TRUE;
      }
      else if( (i/2) == EX_HEIGHT_RIDE_OTHER )
      {
        other = TRUE;
      }
    }
  }

  if( mine )
  {
    return EX_HEIGHT_RIDE_MINE;
  }
  else if( other )
  {
    return EX_HEIGHT_RIDE_OTHER;
  }
  else
  {
    return EX_HEIGHT_RIDE_NO;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  設定されている拡張高さをリセット
 *
 *	@param	p_wk  ギミックワーク 
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Reset( GYM_WATER_TMP *p_wk )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  int num = EXH_GetDatNum( ex_height_list );
  int i;

  for( i=0; i<num; i++ )
  {
    EXH_SetHeightValid( i, FALSE, ex_height_list );
  }

  p_wk->ex_height_idx = 0;

  // セーブデータも破棄
  HASU_ExitExHeight( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さを設定
 *
 *	@param	p_wk  ギミックワーク
 *	@param	id    ハスID
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Set( GYM_WATER_TMP *p_wk, u8 id )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  // ハスの中央グリッド
  int center_gx;
  int center_gz;
  HASU_GetCenterPos( id, &center_gx, &center_gz );

  // 1 縦3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      center_gx,   // 領域開始グリッドX座標
      center_gz-1,   // 領域開始グリッドZ座標
      1,      // 領域グリッド横幅
      3,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);


  // 1 横3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      center_gx-1,   // 領域開始グリッドX座標
      center_gz,   // 領域開始グリッドZ座標
      3,      // 領域グリッド横幅
      1,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);

}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さを設定(OBJインデックス指定版)
 *
 *	@param	p_wk    ギミックワーク
 *	@param	obj_idx OBJインデックス
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Set2( GYM_WATER_TMP *p_wk, u16 obj_idx )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

  int gx;
  int gz;

  gx = FX32_TO_GRID( status->trans.x - GRID_HALF_SIZE );
  gz = FX32_TO_GRID( status->trans.z - GRID_HALF_SIZE );

  // 1 縦3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      gx,     // 領域開始グリッドX座標
      gz-1,   // 領域開始グリッドZ座標
      1,      // 領域グリッド横幅
      3,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);


  // 1 横3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      gx-1,   // 領域開始グリッドX座標
      gz,     // 領域開始グリッドZ座標
      3,      // 領域グリッド横幅
      1,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);

}

//=============================================================================
/**
 *        アニメタスク
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  エフェクトアニメタスク追加
 *
 *	@param	GYM_WATER_TMP *p_wk   ジムワーク
 *	@param	obj_idx               エフェクトのOBJインデックス
 *	@param	anm_num               アニメの数
 */
//-----------------------------------------------------------------------------
static void ANM_TASK_Add( GYM_WATER_TMP *p_wk, u16 obj_idx, u8 anm_num )
{
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( p_wk->p_fieldmap );
  int i;

  // 空きタスクを探して登録
  for( i=0; i<TASK_MAX; i++ )
  {
    ANM_TASK_WORK *task_wk = &p_wk->anm_task_wk[i];
    if( task_wk->is_enable == FALSE )
    {
      task_wk->is_enable = TRUE;
      task_wk->exp_obj = p_wk->exp_obj;
      task_wk->obj_idx = obj_idx;
      task_wk->anm_num = anm_num;

      GFL_TCB_AddTask( tcbsys, tcb_CheckAnime, (void*)task_wk, 0 );
      break;
    }
  }
}

//=======================================================
///	アニメ終了監視タスク
//=======================================================
static void tcb_CheckAnime( GFL_TCB *tcb, void* work )
{
  ANM_TASK_WORK *p_wk = (ANM_TASK_WORK*)work;

  EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, p_wk->obj_idx, 0 );

  // アニメ終了していないなら戻る
  if( !FLD_EXP_OBJ_ChkAnmEnd(exp_anm) )
  {
    return;
  }

  // 非表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_WATER_UNIT_IDX, p_wk->obj_idx, TRUE );

  // ANIME終了
  {
    int i;
    for( i=0; i<p_wk->anm_num; i++ )
    {
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, p_wk->obj_idx, i, FALSE );
    }
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
  }
  ICHI_Printf("EffectIdx %d 破棄\n", p_wk->obj_idx);

  // フラグを元に戻す
  p_wk->is_enable = FALSE;

  GFL_TCB_DeleteTask( tcb );    // タスクを破棄(自殺)
}

//=============================================================================
/**
 *        セーブ情報
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハスの位置を設定
 *
 *	@param	GYM_WATER_SV_WORK *p_wk 水ジムセーブワーク
 *	@param	fx32 x    X座標
 *	@param	fx32 z    Z座標
 *	@param	int idx   ハスの番号
 */
//-----------------------------------------------------------------------------
static void SAVE_SetHasuPos( GYM_WATER_SV_WORK *p_wk, const fx32 x, const fx32 z, const int idx )
{
  p_wk->x[ idx ] = x;
  p_wk->z[ idx ] = z;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハスの位置を取得
 *
 *	@param	GYM_WATER_SV_WORK *p_wk 水ジムセーブワーク
 *	@param	*p_x      取得するX座標へのポインタ
 *	@param	*p_z      取得するZ座標へのポインタ
 *	@param	int idx   ハスの番号
 */
//-----------------------------------------------------------------------------
static void SAVE_GetHasuPos( GYM_WATER_SV_WORK *p_wk, fx32 *p_x, fx32 *p_z, const int idx )
{
  *p_x = p_wk->x[ idx ];
  *p_z = p_wk->z[ idx ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  初回セーブフラグを設定
 *
 *	@param	p_wk  水ジムセーブワーク
 *	@param	flag  フラグ
 */
//-----------------------------------------------------------------------------
static void SAVE_SetSaveFlag( GYM_WATER_SV_WORK *p_wk )
{
  p_wk->save_flag = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  初回セーブフラグを取得
 *
 *	@param	p_wk  水ジムセーブワーク
 *
 *	@return flag  フラグ
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetSaveFlag( GYM_WATER_SV_WORK *p_wk )
{
  return p_wk->save_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さが設定されているOBJインデックスを設定 
 *
 *	@param	GYM_WATER_SV_WORK *p_wk
 *	@param	idx       セーブインデックス
 *	@param	obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void SAVE_SetObjIdx( GYM_WATER_SV_WORK *p_wk, int idx, const u8 obj_idx )
{
  p_wk->ex_obj_idx[ idx ] = obj_idx;
}
//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さが設定されているOBJインデックスを取得 
 *
 *	@param	GYM_WATER_SV_WORK *p_wk
 *	@param	idx   セーブインデックス
 *
 *	@return OBJインデックス
 */
//-----------------------------------------------------------------------------
static u8   SAVE_GetObjIdx( GYM_WATER_SV_WORK *p_wk, int idx )
{
  return p_wk->ex_obj_idx[ idx ];
}


//----------------------------------------------------------------------------
/**
 *	@brief  クリア(ギミック解いた)フラグ設定
 *
 *	@param	GIMMICKWORK* p_gmkwk  ギミックワーク
 *	@param	BOOL flag             クリアフラグ
 */
//-----------------------------------------------------------------------------
static void SAVE_SetClearFlag( GIMMICKWORK* p_gmkwk, const BOOL flag )
{
  u8 *ex_wk;

  // セーブアドレス取得
  ex_wk = GIMMICKWORK_GetExWork2( p_gmkwk, FLD_GIMMICK_GYM_WATER );

  if( flag == TRUE ){ *ex_wk = 1; }
  else              { *ex_wk = 0; }

  ICHI_Printf("クリアフラグセーブ設定 %d \n", *ex_wk);
}

//----------------------------------------------------------------------------
/**
 *	@brief  クリア(ギミック解いた)フラグ取得
 *
 *	@param	GIMMICKWORK* p_gmkwk  ギミックワーク
 *
 *	@return BOOL  クリアフラグ
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetClearFlag( GIMMICKWORK* p_gmkwk )
{
  u8 *ex_wk;

  // セーブアドレス取得
  ex_wk = GIMMICKWORK_GetExWork2( p_gmkwk, FLD_GIMMICK_GYM_WATER );

  ICHI_Printf("クリアフラグセーブ取得 %d \n", *ex_wk);

  if( *ex_wk == 1 ){ return TRUE; }
  else             { return FALSE;}
}

#ifdef GYM_WATER_DEBUG_MENU
//=============================================================================
/**
 *    デバッグメニュー
 */
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#define DEBUGMENU_GYM_WATER (2)

//-------------------------------------
///	static
//=====================================
// キー入力でデータ変更
static BOOL DebugWin_ChangeData( int *p_param, int min, int max );
// 音量の変更
static void DebugWin_Update_ChangeData( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeData( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeSync( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeSync( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeRipple( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeRipple( void *userWork, DEBUGWIN_ITEM *item );

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
  DEBUGWIN_AddGroupToTop( DEBUGMENU_GYM_WATER, "みずジム", heapID );

  // 
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeData,
                              DebugWin_Draw_ChangeData,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeHeight,
                              DebugWin_Draw_ChangeHeight,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeSync,
                              DebugWin_Draw_ChangeSync,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeRipple,
                              DebugWin_Draw_ChangeRipple,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_GYM_WATER );
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
 *	@brief  ハス速度変更 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeData( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_frame;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_frame = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハス速度変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeData( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ハスそくど[%d]", debug_frame );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゆれ高さ 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_height;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_height = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゆれ高さ 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ゆれたかさ[%d]", debug_height );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゆれシンク 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeSync( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_sync;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_sync = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゆれシンク 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeSync( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ゆれシンク[%d]", debug_sync );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動波紋インターバル 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeRipple( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_ripple;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_ripple = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動波紋インターバル 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeRipple( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "はもんひんど[%d]", debug_ripple );
}


#endif // GYM_WATER_DEBUG_MENU

