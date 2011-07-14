//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_hide_ship_engine.c
 *	@brief  アジト船(エンジン)ギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.08
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
#include "arc/fieldmap/hide_ship_engine.naix"

// mine
#include "field_gimmick_hide_ship_engine.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define HIDE_SHIP_UNIT_IDX        (0)
#define HIDE_SHIP_TMP_ASSIGN_ID   (1)

// 座標関係
#define GRID_HALF_SIZE  ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)
// ボタン
#define BUTTON_NUM      ( 4 )   // ボタン数
#define BUTTON_1_X      ( 8 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン1 X座標
#define BUTTON_1_Z      (44 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン1 Z座標
#define BUTTON_2_X      ( 8 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン2 X座標
#define BUTTON_2_Z      (51 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン2 Z座標
#define BUTTON_3_X      (22 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン3 X座標
#define BUTTON_3_Z      (44 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン3 Z座標
#define BUTTON_4_X      (22 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン4 X座標
#define BUTTON_4_Z      (51 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン4 Z座標
// ゲート
#define GATE_NUM        ( 4 )   // ゲート数
#define GATE_1_X        (15 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート1 X座標
#define GATE_1_Z        (14 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート1 Z座標
#define GATE_2_X        (15 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート2 X座標
#define GATE_2_Z        (15 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート2 Z座標
#define GATE_3_X        (15 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート3 X座標
#define GATE_3_Z        (16 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート3 Z座標
#define GATE_4_X        (15 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート4 X座標
#define GATE_4_Z        (17 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート4 Z座標

// パイプ
#define PIPE_NUM        ( 16 )  // パイプ数
#define PIPE_Y          (-3 * FIELD_CONST_GRID_FX32_SIZE ) // パイプ Y座標

#define EFFECT_01_NUM      (4)
#define EFFECT_02_NUM      (4)

//-------------------------------------
///	パイプ初期設定データ
//=====================================
typedef struct {
  VecFx32   pos;    // 座標
  u8        dir;    // 向き
} PIPE_INIT_PARAM;

// パイプの位置はキューブの中心なのでボタンの位置にしている
static const PIPE_INIT_PARAM pipe_init_tbl[ PIPE_NUM ] = {
  { { BUTTON_1_X, PIPE_Y, BUTTON_1_Z }, DIR_UP },      // パイプ1
  { { BUTTON_1_X, PIPE_Y, BUTTON_1_Z }, DIR_LEFT },    // パイプ2
  { { BUTTON_1_X, PIPE_Y, BUTTON_1_Z }, DIR_RIGHT },   // パイプ3
  { { BUTTON_1_X, PIPE_Y, BUTTON_1_Z }, DIR_DOWN },    // パイプ4
  { { BUTTON_2_X, PIPE_Y, BUTTON_2_Z }, DIR_UP },      // パイプ5
  { { BUTTON_2_X, PIPE_Y, BUTTON_2_Z }, DIR_LEFT },    // パイプ6
  { { BUTTON_2_X, PIPE_Y, BUTTON_2_Z }, DIR_RIGHT },   // パイプ7
  { { BUTTON_2_X, PIPE_Y, BUTTON_2_Z }, DIR_DOWN },    // パイプ8
  { { BUTTON_3_X, PIPE_Y, BUTTON_3_Z }, DIR_UP },      // パイプ9
  { { BUTTON_3_X, PIPE_Y, BUTTON_3_Z }, DIR_LEFT },    // パイプ10
  { { BUTTON_3_X, PIPE_Y, BUTTON_3_Z }, DIR_RIGHT },   // パイプ11
  { { BUTTON_3_X, PIPE_Y, BUTTON_3_Z }, DIR_DOWN },    // パイプ12
  { { BUTTON_4_X, PIPE_Y, BUTTON_4_Z }, DIR_UP },      // パイプ13
  { { BUTTON_4_X, PIPE_Y, BUTTON_4_Z }, DIR_LEFT },    // パイプ14
  { { BUTTON_4_X, PIPE_Y, BUTTON_4_Z }, DIR_RIGHT },   // パイプ15
  { { BUTTON_4_X, PIPE_Y, BUTTON_4_Z }, DIR_DOWN },    // パイプ16
};


// 拡張高さ
#define EX_HEIGHT_NUM             (14)    // 総数

//-------------------------------------
///	拡張高さ設定構造体
//=====================================
typedef struct {
  int   start_x;    // 領域開始グリッドX座標
  int   start_z;    // 領域開始グリッドZ座標
  int   size_x;     // 領域グリッド横幅
  int   size_z;     // 領域グリッド縦幅
} EX_HEIGHT_DATA;

// 拡張高さのデータは上からの連番です。
//
//   ①　｜　⑧
// ②■③■⑨■⑩
//　 ④　｜　⑪
// ⑤■⑥■⑫■⑬
// 　⑦　｜　⑭
//
static const EX_HEIGHT_DATA ex_height_tbl[ EX_HEIGHT_NUM ] =
{
  { 8, 41, 1, 2 },    // 1
  { 5, 44, 2, 1 },    // 2
  {10, 44, 2, 1 },    // 3
  { 8, 46, 1, 4 },    // 4
  { 5, 51, 2, 1 },    // 5
  {10, 51, 2, 1 },    // 6
  { 8, 53, 1, 2 },    // 7
  {22, 41, 1, 2 },    // 8
  {19, 44, 2, 1 },    // 9
  {24, 44, 2, 1 },    // 10
  {22, 46, 1, 4 },    // 11
  {19, 51, 2, 1 },    // 12
  {24, 51, 2, 1 },    // 13
  {22, 53, 1, 2 },    // 14
};


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	パイプ1つのデータ
//=====================================
typedef struct {
  BOOL      start_flag;      // 起動フラグ(突き出てるか)
} PIPE_WORK;

//-------------------------------------
///	ギミック内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ

  PIPE_WORK           pipe_wk[ PIPE_NUM ];    // パイプワーク

} HIDE_SHIP_TMP;

//-------------------------------------
///	ボタンイベントワーク
//=====================================
typedef struct {
  int       idx;      // ボタンIDX
} BUTTON_EVENT_WORK;

//-------------------------------------
///	パイプアニメタスク
//=====================================
typedef struct {
  u16                 seq;
  u16                 obj_idx;
  HIDE_SHIP_TMP       *p_gmk_wk;    // ギミックワーク
} PIPE_TASK_WORK;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum{
  RESID_BUTTON_MDL = 0,         // ボタンモデル
  RESID_BUTTON_ANM_OFF,         // ボタンOFFアニメ
  RESID_BUTTON_ANM_ON,          // ボタンONアニメ
  RESID_GATE_MDL,               // ゲートモデル
  RESID_GATE_ANM_ITA,           // ゲート常時アニメ
  RESID_GATE_ANM_OFF,           // ゲート解除アニメ
  RESID_PIPE_MDL,               // パイプモデル
  RESID_PIPE_ANM_IN,            // パイプアニメ(引っ込む)
  RESID_PIPE_ANM_ON,            // パイプアニメ(くっついたとき)
  RESID_PIPE_ANM_OUT,           // パイプアニメ(突き出る)
  RESID_EFFECT_01_MDL,          // パイプのスチームエフェクト
  RESID_EFFECT_01_ANM_ICA,      // パイプのスチームエフェクト
  RESID_EFFECT_01_ANM_IMA,      // パイプのスチームエフェクト
  RESID_EFFECT_01_ANM_ITA,      // パイプのスチームエフェクト
  RESID_EFFECT_02_MDL,          // パイプ同士がつながったエフェクト
  RESID_EFFECT_02_ANM_ICA,      // パイプ同士がつながったエフェクト
  RESID_EFFECT_02_ANM_IMA,      // パイプ同士がつながったエフェクト
  RESID_EFFECT_02_ANM_ITA,      // パイプ同士がつながったエフェクト
};

// OBJインデックス
enum{
  OBJ_BUTTON_1,                 // ボタン
  OBJ_BUTTON_2,                 // ボタン
  OBJ_BUTTON_3,                 // ボタン
  OBJ_BUTTON_4,                 // ボタン
  OBJ_GATE_1,                   // ゲート
  OBJ_GATE_2,                   // ゲート
  OBJ_GATE_3,                   // ゲート
  OBJ_GATE_4,                   // ゲート
  OBJ_PIPE_1,                   // パイプ
  OBJ_PIPE_2,                   // パイプ
  OBJ_PIPE_3,                   // パイプ
  OBJ_PIPE_4,                   // パイプ
  OBJ_PIPE_5,                   // パイプ
  OBJ_PIPE_6,                   // パイプ
  OBJ_PIPE_7,                   // パイプ
  OBJ_PIPE_8,                   // パイプ
  OBJ_PIPE_9,                   // パイプ
  OBJ_PIPE_10,                  // パイプ
  OBJ_PIPE_11,                  // パイプ
  OBJ_PIPE_12,                  // パイプ
  OBJ_PIPE_13,                  // パイプ
  OBJ_PIPE_14,                  // パイプ
  OBJ_PIPE_15,                  // パイプ
  OBJ_PIPE_16,                  // パイプ

  OBJ_EFFECT_01_1,              // エフェクト
  OBJ_EFFECT_01_2,              // エフェクト
  OBJ_EFFECT_01_3,              // エフェクト
  OBJ_EFFECT_01_4,              // エフェクト
  OBJ_EFFECT_02_1,              // エフェクト
  OBJ_EFFECT_02_2,              // エフェクト
  OBJ_EFFECT_02_3,              // エフェクト
  OBJ_EFFECT_02_4,              // エフェクト

  OBJ_NUM,
};


// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_button01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_button01_off_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_button01_on_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_gate01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_gate01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_gate01_off_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_pipe01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_pipe01_in_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_pipe01_on_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_pipe01_out_nsbca, GFL_G3D_UTIL_RESARC },
  // effect
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect01_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect02_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_p_effect02_nsbta, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_button[] =
{
  { RESID_BUTTON_ANM_OFF, 0 },
  { RESID_BUTTON_ANM_ON, 0 },
};
#define ANM_BUTTON_NUM  ( NELEMS(anm_button) )

static const GFL_G3D_UTIL_ANM anm_gate[] =
{
  { RESID_GATE_ANM_ITA, 0 },
  { RESID_GATE_ANM_OFF, 0 },
};
#define ANM_GATE_NUM  ( NELEMS(anm_gate) )

static const GFL_G3D_UTIL_ANM anm_pipe[] =
{
  { RESID_PIPE_ANM_IN, 0 },
  { RESID_PIPE_ANM_ON, 0 },
  { RESID_PIPE_ANM_OUT, 0 },
};
#define ANM_PIPE_NUM  ( NELEMS(anm_pipe) )

static const GFL_G3D_UTIL_ANM anm_effect_01[] =
{
  { RESID_EFFECT_01_ANM_ICA, 0 },
  { RESID_EFFECT_01_ANM_IMA, 0 },
  { RESID_EFFECT_01_ANM_ITA, 0 },
};
#define ANM_EFFECT_01_NUM  ( NELEMS(anm_effect_01) )

static const GFL_G3D_UTIL_ANM anm_effect_02[] =
{
  { RESID_EFFECT_02_ANM_ICA, 0 },
  { RESID_EFFECT_02_ANM_IMA, 0 },
  { RESID_EFFECT_02_ANM_ITA, 0 },
};
#define ANM_EFFECT_02_NUM  ( NELEMS(anm_effect_02) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_BUTTON_1
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_BUTTON_2
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_BUTTON_3
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_BUTTON_4
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_GATE_1
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_GATE_2
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_GATE_3
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_GATE_4
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_1
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_2
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_3
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_4
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_5
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_6
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_7
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_8
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_9
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_10
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_11
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_12
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_13
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_14
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_15
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_PIPE_16
  {
    RESID_PIPE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_PIPE_MDL,         // テクスチャリソースID
    anm_pipe,               // アニメテーブル(複数指定のため)
    ANM_PIPE_NUM,           // アニメリソース数
  },
  // OBJ_EFFECT_01_1
  {
    RESID_EFFECT_01_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_01_MDL,    // テクスチャリソースID
    anm_effect_01,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_01_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_01_2
  {
    RESID_EFFECT_01_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_01_MDL,    // テクスチャリソースID
    anm_effect_01,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_01_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_01_3
  {
    RESID_EFFECT_01_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_01_MDL,    // テクスチャリソースID
    anm_effect_01,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_01_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_01_4
  {
    RESID_EFFECT_01_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_01_MDL,    // テクスチャリソースID
    anm_effect_01,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_01_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_02_1
  {
    RESID_EFFECT_02_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_02_MDL,    // テクスチャリソースID
    anm_effect_02,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_02_2
  {
    RESID_EFFECT_02_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_02_MDL,    // テクスチャリソースID
    anm_effect_02,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_02_3
  {
    RESID_EFFECT_02_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_02_MDL,    // テクスチャリソースID
    anm_effect_02,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,      // アニメリソース数
  },
  // OBJ_EFFECT_02_4
  {
    RESID_EFFECT_02_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_02_MDL,    // テクスチャリソースID
    anm_effect_02,          // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,      // アニメリソース数
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
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, VecFx32 pos );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// PIPEアニメ
static void ANM_PIPE_IN_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static BOOL ANM_PIPE_IN_CheckEnd( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static void ANM_PIPE_OUT_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static BOOL ANM_PIPE_OUT_CheckEnd( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static void ANM_PIPE_ON_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx );

// BUTTONアニメ
static void ANM_BUTTON_OFF_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static void ANM_BUTTON_ON_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx );

// EFFECT
static void ANM_EFFECT_01_Play( HIDE_SHIP_TMP *p_wk, u16 pipe_obj_idx );
static void ANM_EFFECT_02_Play( HIDE_SHIP_TMP *p_wk, u16 pipe_obj_idx );

// ボタンイベント
static GMEVENT * BUTTON_CreateEvent( GAMESYS_WORK* gsys, int idx );
static GMEVENT_RESULT ButtonEvent( GMEVENT* event, int* seq, void* wk );
static void BUTTON_EVENT_PosCheck( HIDE_SHIP_TMP *p_wk, int btn_idx );
static void BUTTON_EVENT_PipeStartup( HIDE_SHIP_TMP *p_wk, int pipe_idx );

// パイプタスク
static void PIPE_IN_TASK_Add( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static FIELD_TASK_RETVAL tcb_PipeInTask( void* wk );
static void PIPE_OUT_TASK_Add( HIDE_SHIP_TMP *p_wk, u16 obj_idx );
static FIELD_TASK_RETVAL tcb_PipeOutTask( void* wk );

// 拡張高さ
static void EX_HEIGHT_Set( HIDE_SHIP_TMP *p_wk );

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
void HIDE_SHIP_Setup( FIELDMAP_WORK *p_fieldmap )
{

  // 汎用ワーク確保
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(HIDE_SHIP_TMP) );

  p_wk->exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap = p_fieldmap;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 拡張高さセット
  EX_HEIGHT_Set( p_wk );

}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void HIDE_SHIP_End( FIELDMAP_WORK *p_fieldmap )
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
void HIDE_SHIP_Move( FIELDMAP_WORK *p_fieldmap )
{
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = BUTTON_CreateEvent( gsys, 0 );
      GAMESYSTEM_SetEvent(gsys, event);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = BUTTON_CreateEvent( gsys, 1 );
      GAMESYSTEM_SetEvent(gsys, event);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = BUTTON_CreateEvent( gsys, 2 );
      GAMESYSTEM_SetEvent(gsys, event);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = BUTTON_CreateEvent( gsys, 3 );
      GAMESYSTEM_SetEvent(gsys, event);
    }
  }
#endif

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
 *	@brief  ボタン押下イベント
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	id 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* HIDE_SHIP_Button( GAMESYS_WORK *gsys, int id )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );
  
  event = BUTTON_CreateEvent( gsys, id );

  return event;
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
  // ボタン
  {
    int i;
    VecFx32 pos[ BUTTON_NUM ] = {
      { BUTTON_1_X, 0, BUTTON_1_Z },
      { BUTTON_2_X, 0, BUTTON_2_Z },
      { BUTTON_3_X, 0, BUTTON_3_Z },
      { BUTTON_4_X, 0, BUTTON_4_Z },
    };

    for( i=0; i<BUTTON_NUM; i++ )
    {
      int obj_idx = OBJ_BUTTON_1 + i;
      
      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_BUTTON_NUM, pos[i] );
    }
  }

  //=====================================
  // ゲート
  {
    int i;
    VecFx32 pos[ GATE_NUM ] = {
      { GATE_1_X, 0, GATE_1_Z },
      { GATE_2_X, 0, GATE_2_Z },
      { GATE_3_X, 0, GATE_3_Z },
      { GATE_4_X, 0, GATE_4_Z },
    };

    for( i=0; i<GATE_NUM; i++ )
    {
      int obj_idx = OBJ_GATE_1 + i;
      
      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_GATE_NUM, pos[i] );
      // ループアニメは再生しておく
      ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
    }
  }

  //=====================================
  // パイプ
  {
    int i;

    for( i=0; i<PIPE_NUM; i++ )
    {
      int obj_idx = OBJ_PIPE_1 + i;
      
      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_PIPE_NUM, pipe_init_tbl[i].pos );

      // 回転
      {
        u16 rtbl[DIR_MAX4][3] = {{0,0xc000,0},{0,0x4000,0},{0,0,0},{0,0x8000,0}};
        u16 *rot = rtbl[ pipe_init_tbl[i].dir ];
        GFL_G3D_OBJSTATUS *status;

        status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx );
        GFL_CALC3D_MTX_CreateRot( rot[0], rot[1], rot[2], &status->rotate );
      }

    }
  }

  //=====================================
  // エフェクト
  {
    int i;
    VecFx32 pos = { 0,0,0 };

    for( i=0; i<EFFECT_01_NUM; i++ )
    {
      int obj_idx = OBJ_EFFECT_01_1 + i;

      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_EFFECT_01_NUM, pos );
      // 非表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, TRUE );
    }
    for( i=0; i<EFFECT_02_NUM; i++ )
    {
      int obj_idx = OBJ_EFFECT_02_1 + i;

      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_EFFECT_02_NUM, pos );
      // 非表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, TRUE );
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
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, pos.x, pos.y, pos.z );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, TRUE );
  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, FALSE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, i, FALSE );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//=============================================================================
/**
 *          PIPEアニメ
 *
 *          アニメインデックス
 *          0:引っ込みアニメ
 *          1:くっついた時アニメ
 *          2:突き出しアニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  引っ込みアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_PIPE_IN_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  引っ込みアニメ終了チェック
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_PIPE_IN_CheckEnd( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, 0 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
}
//----------------------------------------------------------------------------
/**
 *	@brief  突き出しアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_PIPE_OUT_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  突き出しアニメ終了チェック
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_PIPE_OUT_CheckEnd( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, 2 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
}
//----------------------------------------------------------------------------
/**
 *	@brief  くっついた時のアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_PIPE_ON_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
}

//=============================================================================
/**
 *          BUTTONアニメ
 *
 *          アニメインデックス
 *          0:OFFアニメ(ボタン凸)
 *          1:ONアニメ(ボタン凹)
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  オフアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_BUTTON_OFF_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  オンアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_BUTTON_ON_Play( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
}

//=============================================================================
/**
 *          EFFECT
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  パイプが突き出す時のスチームエフェクト再生
 *
 *	        引数として、パイプのOBJインデックスを渡します。
 *	        その引数を4でまるめる
 *	        パイプは最大4方向で4つ分のエフェクトが出せればよい。
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	pipe_obj_idx  パイプのOBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_EFFECT_01_Play( HIDE_SHIP_TMP *p_wk, u16 pipe_obj_idx )
{
  int i;
  u16 obj_idx;

  obj_idx = OBJ_EFFECT_01_1 + ( pipe_obj_idx % 4 );    // 最大4つなので

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, FALSE );

  // 座標設定
  {
    GFL_G3D_OBJSTATUS *status;
    GFL_G3D_OBJSTATUS *tmp_status;

    status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx );
    tmp_status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, pipe_obj_idx );

    status->trans   = tmp_status->trans;
    status->rotate  = tmp_status->rotate;
    status->scale   = tmp_status->scale;
  }

  // アニメ再生
  for( i=0; i<ANM_EFFECT_01_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  パイプが突き出す時のスチームエフェクト再生
 *
 *	        引数として、パイプのOBJインデックスを渡します。
 *	        その引数を4でまるめる
 *	        パイプは最大4方向で4つ分のエフェクトが出せればよい。
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	pipe_obj_idx  パイプのOBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_EFFECT_02_Play( HIDE_SHIP_TMP *p_wk, u16 pipe_obj_idx )
{
  int i;
  u16 obj_idx;

  obj_idx = OBJ_EFFECT_02_1 + ( pipe_obj_idx % 4 );    // 最大4つなので

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, FALSE );

  // 座標設定
  {
    GFL_G3D_OBJSTATUS *status;
    GFL_G3D_OBJSTATUS *tmp_status;

    status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx );
    tmp_status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, pipe_obj_idx );

    status->trans   = tmp_status->trans;
    status->rotate  = tmp_status->rotate;
    status->scale   = tmp_status->scale;
  }

  // アニメ再生
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
}


//=============================================================================
/**
 *          ボタンイベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンイベント作成
 *
 *	@param	GAMESYS_WORK* gsys  ゲームシステム
 *	@param	idx     どのボタンか 0～3
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * BUTTON_CreateEvent( GAMESYS_WORK* gsys, int idx )
{
  GMEVENT*          event;
  BUTTON_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  GF_ASSERT( idx<BUTTON_NUM );

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, ButtonEvent, sizeof(BUTTON_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->idx = idx;

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタンイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT ButtonEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  BUTTON_EVENT_WORK *p_wk = (BUTTON_EVENT_WORK*)wk;
  HIDE_SHIP_TMP *p_gmk_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  // 処理シーケンス
  enum {
    SEQ_START,      // 
    SEQ_WAIT,       // 
    SEQ_END,        // 
  };


  switch( *seq ){
  case SEQ_START:
    
    // ボタンの押したチェック
    BUTTON_EVENT_PosCheck( p_gmk_wk, p_wk->idx );

    *seq = SEQ_WAIT;
    break;

  case SEQ_WAIT:
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( p_fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) == TRUE ) {
        *seq = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタンのチェック
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	btn_idx  ボタンのIDX
 */
//-----------------------------------------------------------------------------
static void BUTTON_EVENT_PosCheck( HIDE_SHIP_TMP *p_wk, int btn_idx  )
{
  int i;

  GF_ASSERT( btn_idx < BUTTON_NUM );

  // 4方向のパイプをチェック
  for( i=0; i<DIR_MAX4; i++ )
  {
    int pipe_idx = i + (btn_idx*4);
    BUTTON_EVENT_PipeStartup( p_wk, pipe_idx );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パイプの状態から起動する
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	pipe_idx   パイプのIDX
 */
//-----------------------------------------------------------------------------
static void BUTTON_EVENT_PipeStartup( HIDE_SHIP_TMP *p_wk, int pipe_idx )
{
  int obj_idx;

  GF_ASSERT( pipe_idx < PIPE_NUM );

  // パイプのOBJインデックス算出
  obj_idx = OBJ_PIPE_1 + pipe_idx;

  if( p_wk->pipe_wk[ pipe_idx ].start_flag == TRUE )
  {
    PIPE_IN_TASK_Add( p_wk, obj_idx );
  }
  else
  {
    PIPE_OUT_TASK_Add( p_wk, obj_idx );
  }

  // フラグ反転
  p_wk->pipe_wk[ pipe_idx ].start_flag = !p_wk->pipe_wk[ pipe_idx ].start_flag;
}

//=============================================================================
/**
 *          パイプアニメ管理タスク
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  パイプ引っ込むアニメタスク追加
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param	obj_idx               パイプのOBJインデックス
 */
//-----------------------------------------------------------------------------
static void PIPE_IN_TASK_Add( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  FIELD_TASK      *task;
  FIELD_TASK_MAN  *taskMan;
  PIPE_TASK_WORK  *work;
  HEAPID heap_id  = FIELDMAP_GetHeapID( p_wk->p_fieldmap );

  taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(PIPE_TASK_WORK), tcb_PipeInTask );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq = 0;
  work->obj_idx = obj_idx;
  work->p_gmk_wk = p_wk;

  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
}
// パイプ引っ込むアニメタスク
static FIELD_TASK_RETVAL tcb_PipeInTask( void* wk )
{
  PIPE_TASK_WORK *p_wk = (PIPE_TASK_WORK*)wk;

  switch( p_wk->seq ){
  // 引っ込むアニメ再生
  case 0:
    ANM_PIPE_IN_Play( p_wk->p_gmk_wk, p_wk->obj_idx );
    p_wk->seq++;
    break;
  // アニメ終了チェック
  case 1:
    if( ANM_PIPE_IN_CheckEnd( p_wk->p_gmk_wk, p_wk->obj_idx ) )
    {
      p_wk->seq++;
    }
    break;
  // 終了
  case 2:
    return FIELD_TASK_RETVAL_FINISH;
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  パイプ突き出るアニメタスク追加
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param	obj_idx               パイプのOBJインデックス
 */
//-----------------------------------------------------------------------------
static void PIPE_OUT_TASK_Add( HIDE_SHIP_TMP *p_wk, u16 obj_idx )
{
  FIELD_TASK      *task;
  FIELD_TASK_MAN  *taskMan;
  PIPE_TASK_WORK  *work;
  HEAPID heap_id  = FIELDMAP_GetHeapID( p_wk->p_fieldmap );

  taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(PIPE_TASK_WORK), tcb_PipeOutTask );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq = 0;
  work->obj_idx = obj_idx;
  work->p_gmk_wk = p_wk;

  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
}
// パイプ突き出るアニメタスク
static FIELD_TASK_RETVAL tcb_PipeOutTask( void* wk )
{
  PIPE_TASK_WORK *p_wk = (PIPE_TASK_WORK*)wk;

  switch( p_wk->seq ){
  // 突き出るアニメ再生
  case 0:
    ANM_PIPE_OUT_Play( p_wk->p_gmk_wk, p_wk->obj_idx );

    // エフェクトも表示
    {
      ANM_EFFECT_01_Play( p_wk->p_gmk_wk, p_wk->obj_idx );
    }

    p_wk->seq++;
    break;
  // アニメ終了チェック
  case 1:
    if( ANM_PIPE_OUT_CheckEnd( p_wk->p_gmk_wk, p_wk->obj_idx ) )
    {
      p_wk->seq++;
    }
    break;
  // くっつくアニメ
  case 2:
    ANM_PIPE_ON_Play( p_wk->p_gmk_wk, p_wk->obj_idx );

    // エフェクトも表示
    {
      ANM_EFFECT_02_Play( p_wk->p_gmk_wk, p_wk->obj_idx );
    }

    p_wk->seq++;
    break;
  // 終了
  case 3:
    return FIELD_TASK_RETVAL_FINISH;
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さを設定
 *
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Set( HIDE_SHIP_TMP *p_wk )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );
  int i;

  for( i=0; i<EX_HEIGHT_NUM; i++ )
  {
    EXH_SetUpHeightData( i,
        ex_height_tbl[i].start_x,
        ex_height_tbl[i].start_z,
        ex_height_tbl[i].size_x,
        ex_height_tbl[i].size_z,
        -GRID_HALF_SIZE,
        (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
        ex_height_list);
  }

}

