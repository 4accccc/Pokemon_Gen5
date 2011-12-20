//======================================================================
/**
 * @file  sp_poke_gimmick.c
 * @brief  配布系ポケモンギミック
 * @author  Saito
 */
//======================================================================

#include "fieldmap.h"
#include "sp_poke_gimmick.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/sppoke.naix"
#include "arc/fieldmap/zoroa.naix"
#include "arc/fieldmap/kerudhio.naix"

#include "sound/pm_sndsys.h"
#include "sp_poke_gimmick_se_def.h"

#include "../../../resource/fldmapdata/gimmick/zoroa/zoroa_gmk_timing_def.h" //for ZOROA_CHG_FRAME
#include "field/eventdata_sxy.h"  //for  EVENTDATA_GetNpcData

#define SPPOKE_GMK_ASSIGN_ID    (1)
#define SPPOKE_UNIT_IDX    (0)                      //特殊ポケイベントユニットインデックス
#define SPPOKE_TRIO_UNIT_IDX    (SPPOKE_UNIT_IDX)   //三銃士イベントユニットインデックス
#define SPPOKE_MERO_UNIT_IDX    (SPPOKE_UNIT_IDX)   //メロディアイベントユニットインデックス
#define ZOROA_UNIT_IDX    (SPPOKE_UNIT_IDX)         //ゾロアイベントユニットインデックス
#define KERUDHIO_UNIT_IDX    (SPPOKE_UNIT_IDX)         //ゾロアイベントユニットインデックス

#define BALL_ANM_NUM  (3)
#define SMOKE_ANM_NUM  (3)

#define BALL_OUT_TIMMING  (5)
#define BALL_IN_TIMMING (34)

#define ZOROA_CHG_FRAME_FX32 (ZOROA_CHG_FRAME * FX32_ONE)

typedef struct SPPOKE_GMK_WK_tag
{
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ

  BALL_ANM_TYPE AnmType;    //ターゲットとなるアニメタイプを格納する
  VecFx32 BallStart;
  VecFx32 BallEnd;
  fx32 Height;
  u16 Sync;
  u16 NowSync;

  void *Work;
}SPPOKE_GMK_WK;

typedef struct SMOKE_GMK_WK_tag
{
  u16 BeforeObj;
  u16 AfterObj;
}SMOKE_GMK_WK;

//==============================================================================================
/**
 @note    ※三銃士とメロディアとゾロアでボールリソースは共有するので、リソース番号は一致するようにする
*/
//==============================================================================================

//リソースの並び順番
enum {
  RES_ID_BALL_OUT_MDL = 0,
  RES_ID_BALL_IN_MDL,

  RES_ID_BALL_OUT_ANM1,
  RES_ID_BALL_OUT_ANM2,
  RES_ID_BALL_OUT_ANM3,
  RES_ID_BALL_IN_ANM1,
  RES_ID_BALL_IN_ANM2,
  RES_ID_BALL_IN_ANM3,

  // ↑↑↑↑↑↑↑↑
  // ここまで共通部分

  // ゾロア専用
  RES_ID_SMOKE_MDL = 8,
  RES_ID_SMOKE_ANM1,
  RES_ID_SMOKE_ANM2,
  RES_ID_SMOKE_ANM3,


  // ケルディオ専用
  RESID_EFFECT_01_MDL = 8,      // 気合のエフェクト
  RESID_EFFECT_01_ANM_ITA,  // 気合のエフェクト アニメ
  RESID_BIRIZION_MDL,       // ビリジオン モデル
  RESID_KOBARUON_MDL,       // コバルオン モデル
  RESID_TERAKION_MDL,       // テラキオン モデル
  RESID_EFFECT_APPEAR_IMA,  // 出現アニメ
  RESID_EFFECT_APPEAR_IVA,  // 出現アニメ
  RESID_EFFECT_STAY_IMA,    // 待機アニメ
  RESID_EFFECT_STAY_IVA,    // 待機アニメ
  RESID_EFFECT_DISAPPEAR_ICA,// 消えるアニメ
  RESID_EFFECT_DISAPPEAR_IMA,// 消えるアニメ
  RESID_EFFECT_DISAPPEAR_IVA,// 消えるアニメ


};

//ＯＢＪインデックス
enum {
  OBJ_BALL_OUT = 0,
  OBJ_BALL_IN,

  // ↑↑↑↑↑↑↑↑
  // ここまで共通部分
  
  // ゾロア専用
  OBJ_SMOKE = 2,

  // ケルディオ専用
  OBJ_EFFECT_01 = 2,        // 気合のエフェクト
  OBJ_BIRIZION,             // ビリジオン
  OBJ_KOBARUON,             // コバルオン
  OBJ_TERAKION,             // テラキオン
  OBJ_NUM_KERUDHIO,         // ケルディオのOBJ総数
};

//==========================================================================
/**
 リソース共通部分
*/
//==========================================================================

//3Dアニメ　ボールアウト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_ball_out[] = {
  { RES_ID_BALL_OUT_ANM1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_BALL_OUT_ANM2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_BALL_OUT_ANM3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　ボールアウト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_ball_in[] = {
  { RES_ID_BALL_IN_ANM1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_BALL_IN_ANM2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_BALL_IN_ANM3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　ゾロア　煙
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_smoke[] = {
  { RES_ID_SMOKE_ANM1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SMOKE_ANM2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SMOKE_ANM3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//==========================================================================
/**
 三銃士リソース
*/
//==========================================================================

static const GFL_G3D_UTIL_RES g3Dutil_resTbl_trio[] = {
	{ ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールアウト
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールイン

  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールアウトアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールインアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl_trio[] = {
  //出現モンスターボール
  {
		RES_ID_BALL_OUT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_OUT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_out,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_out),	//アニメリソース数
	},
  //格納モンスターボール
  {
		RES_ID_BALL_IN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_IN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_in,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_in),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP SetupTrio = {
  g3Dutil_resTbl_trio,				//リソーステーブル
	NELEMS(g3Dutil_resTbl_trio),		//リソース数
	g3Dutil_objTbl_trio,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl_trio),		//オブジェクト数
};

//==========================================================================
/**
 メロディア関連リソース
*/
//==========================================================================
static const GFL_G3D_UTIL_RES g3Dutil_resTbl_mero[] = {
	{ ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールアウト
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールイン

  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールアウトアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールインアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl_mero[] = {
  //出現モンスターボール
  {
		RES_ID_BALL_OUT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_OUT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_out,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_out),	//アニメリソース数
	},
  //格納モンスターボール
  {
		RES_ID_BALL_IN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_IN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_in,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_in),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP SetupMero = {
  g3Dutil_resTbl_mero,				//リソーステーブル
	NELEMS(g3Dutil_resTbl_mero),		//リソース数
	g3Dutil_objTbl_mero,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl_mero),		//オブジェクト数
};

//==========================================================================
/**
 ゾロア関連リソース
*/
//==========================================================================
static const GFL_G3D_UTIL_RES g3Dutil_resTbl_zoroa[] = {
	{ ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールアウト
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールイン

  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールアウトアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールインアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbma, GFL_G3D_UTIL_RESARC }, //IMA

  { ARCID_ZOROA_GMK, NARC_zoroa_zoroa_kemu_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD 煙
  { ARCID_ZOROA_GMK, NARC_zoroa_zoroa_kemu_nsbca, GFL_G3D_UTIL_RESARC }, //ICA 煙アニメ
  { ARCID_ZOROA_GMK, NARC_zoroa_zoroa_kemu_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_ZOROA_GMK, NARC_zoroa_zoroa_kemu_nsbma, GFL_G3D_UTIL_RESARC }, //IMA

};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl_zoroa[] = {
  //出現モンスターボール
  {
		RES_ID_BALL_OUT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_OUT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_out,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_out),	//アニメリソース数
	},
  //格納モンスターボール
  {
		RES_ID_BALL_IN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_IN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_in,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_in),	//アニメリソース数
	},
  //煙
  {
		RES_ID_SMOKE_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SMOKE_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_smoke,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_smoke),	//アニメリソース数
	},

};

static const GFL_G3D_UTIL_SETUP SetupZoroa = {
  g3Dutil_resTbl_zoroa,				//リソーステーブル
	NELEMS(g3Dutil_resTbl_zoroa),		//リソース数
	g3Dutil_objTbl_zoroa,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl_zoroa),		//オブジェクト数
};

//==========================================================================
/**
 ケルディオ関連リソース
*/
//==========================================================================
static const GFL_G3D_UTIL_RES g3Dutil_resTbl_kerudhio[] = {
	{ ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールアウト
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールイン

  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールアウトアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_out_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールインアニメ
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_SPPOKE_GMK, NARC_sppoke_mb_in_nsbma, GFL_G3D_UTIL_RESARC }, //IMA

  // ここからケルディオ専用
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_effect_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_effect_01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_bili_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_koba_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_tera_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_appear_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_appear_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_stay_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_stay_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_disappear_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_disappear_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_disappear_nsbva, GFL_G3D_UTIL_RESARC },
};

// ANIME
// ケルディオ気合
static const GFL_G3D_UTIL_ANM anm_effect_01[] =
{
  { RESID_EFFECT_01_ANM_ITA, 0 },
};
#define ANM_EFFECT_01_NUM  ( NELEMS(anm_effect_01) )

// 三銃士アニメ
static const GFL_G3D_UTIL_ANM anm_effect_02[] =
{
  { RESID_EFFECT_APPEAR_IMA, 0 },
  { RESID_EFFECT_APPEAR_IVA, 0 },
  { RESID_EFFECT_STAY_IMA, 0 },
  { RESID_EFFECT_STAY_IVA, 0 },
  { RESID_EFFECT_DISAPPEAR_ICA, 0 },
  { RESID_EFFECT_DISAPPEAR_IMA, 0 },
  { RESID_EFFECT_DISAPPEAR_IVA, 0 },
};
#define ANM_EFFECT_02_NUM  ( NELEMS(anm_effect_02) )

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl_kerudhio[] = {
  //出現モンスターボール
  {
		RES_ID_BALL_OUT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_OUT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_out,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_out),	//アニメリソース数
	},
  //格納モンスターボール
  {
		RES_ID_BALL_IN_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_IN_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_in,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_in),	//アニメリソース数
	},

  // ここからケルディオ専用
  // OBJ_EFFECT_01
  {
    RESID_EFFECT_01_MDL,  // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_01_MDL,  // テクスチャリソースID
    anm_effect_01,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_01_NUM,    // アニメリソース数
  },
  // OBJ_BIRIZION
  {
    RESID_BIRIZION_MDL,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_BIRIZION_MDL,   // テクスチャリソースID
    anm_effect_02,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,    // アニメリソース数
  },
  // OBJ_KOBARUON
  {
    RESID_KOBARUON_MDL,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_KOBARUON_MDL,   // テクスチャリソースID
    anm_effect_02,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,    // アニメリソース数
  },
  // OBJ_TERAKION
  {
    RESID_TERAKION_MDL,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_TERAKION_MDL,   // テクスチャリソースID
    anm_effect_02,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,    // アニメリソース数
  },
};

static const GFL_G3D_UTIL_SETUP SetupKerushio = {
  g3Dutil_resTbl_kerudhio,				//リソーステーブル
	NELEMS(g3Dutil_resTbl_kerudhio),		//リソース数
	g3Dutil_objTbl_kerudhio,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl_kerudhio),		//オブジェクト数
};


//============================================================================================================
//関数前方宣言
//============================================================================================================
static GMEVENT_RESULT BallMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitPokeAppFrmEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitBallAnmEvt( GMEVENT* event, int* seq, void* work );

static GMEVENT_RESULT SmokeMoveEvt( GMEVENT* event, int* seq, void* work );

//=============================================================================
// 【配布】ケルディオ用

// 座標
// 気合のエフェクト
#define EFF_POS_X       ( 248 << FX32_SHIFT )
#define EFF_POS_Y       ( 0 )
#define EFF_POS_Z       ( 232 << FX32_SHIFT )
// ビリジオン
#define BIRIZION_POS_X  ( 280 << FX32_SHIFT )
#define BIRIZION_POS_Y  ( 0 )
#define BIRIZION_POS_Z  ( 232  << FX32_SHIFT )
// コバルオン
#define KOBARUON_POS_X  ( 248 << FX32_SHIFT )
#define KOBARUON_POS_Y  ( 48  << FX32_SHIFT )
#define KOBARUON_POS_Z  ( 232 << FX32_SHIFT )
// テラキオン
#define TERAKION_POS_X  ( 216 << FX32_SHIFT )
#define TERAKION_POS_Y  ( 0 )
#define TERAKION_POS_Z  ( 232 << FX32_SHIFT )

// ケルディオ演出イベント関係
#define KERUDHIO_EFFECT_WAIT    (120)     // 気合のエフェクト表示フレーム数
#define KERUDHIO_DELETE_WAIT    (40)      // 3匹同時に消えるまでの表示フレーム数

// アニメの並び順(0オリジン)
enum{
  ANM_IDX_APPEAR_IMA,   // 出現
  ANM_IDX_APPEAR_IVA,   // 出現
  ANM_IDX_STAY_IMA,     // 待機
  ANM_IDX_STAY_IVA,     // 待機
  ANM_IDX_DISAPPEAR_ICA,// 消える
  ANM_IDX_DISAPPEAR_IMA,// 消える
  ANM_IDX_DISAPPEAR_IVA,// 消える
};

// OBJとアニメ総数の対応テーブル
static const int anm_num_tbl[ OBJ_NUM_KERUDHIO ] = {
  NELEMS(g3Dutil_anmTbl_ball_out),
  NELEMS(g3Dutil_anmTbl_ball_in),
  ANM_EFFECT_01_NUM,
  ANM_EFFECT_02_NUM,
  ANM_EFFECT_02_NUM,
  ANM_EFFECT_02_NUM,
};

// 配置座標テーブル
static const VecFx32 pos_tbl[ OBJ_NUM_KERUDHIO ] = {
  { 0, 0, 0 },    // ボールのダミー
  { 0, 0, 0 },    // ボールのダミー
  { EFF_POS_X, EFF_POS_Y, EFF_POS_Z },
  { BIRIZION_POS_X, BIRIZION_POS_Y, BIRIZION_POS_Z },
  { KOBARUON_POS_X, KOBARUON_POS_Y, KOBARUON_POS_Z },
  { TERAKION_POS_X, TERAKION_POS_Y, TERAKION_POS_Z },
};

// EXPOBJ
static void EXPOBJ_ObjSet( SPPOKE_GMK_WK *p_wk );
static void EXPOBJ_ObjDell( SPPOKE_GMK_WK *p_wk );
static void EXPOBJ_ObjMain( SPPOKE_GMK_WK *p_wk );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// 三銃士で共通のアニメ
// 登場アニメ
static void ANM_AppearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
static BOOL ANM_AppearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
// 待機アニメ
static void ANM_Stay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
// 消えるアニメ
static void ANM_DisappearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
static BOOL ANM_DisappearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );

///	ケルディオ演出イベント
// イベントワーク
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  int                 wait;           // ウェイト値
} KERUDHIO_EVENT_WORK;

static GMEVENT * KERUDHIO_CreateEvent( GAMESYS_WORK* gsys );
static GMEVENT_RESULT KerudhioEvent( GMEVENT* event, int* seq, void* wk );

//============================================================================================================
//三銃士関連
//============================================================================================================

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_SetupTrio(FIELDMAP_WORK *fieldWork)
{
  SPPOKE_GMK_WK *p_wk;
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, SPPOKE_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(SPPOKE_GMK_WK));

  p_wk = GMK_TMP_WK_GetWork( fieldWork, SPPOKE_GMK_ASSIGN_ID );
  p_wk->p_fieldmap  = fieldWork;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //三銃士イベント用ワークのアロケーション
  ;
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle(ptr, &SetupTrio, SPPOKE_TRIO_UNIT_IDX );

  //ボール初期化
  //OBJを非表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_TRIO_UNIT_IDX, OBJ_BALL_IN, TRUE);
  FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_TRIO_UNIT_IDX, OBJ_BALL_OUT, TRUE);
  
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_TRIO_UNIT_IDX, OBJ_BALL_OUT, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, SPPOKE_TRIO_UNIT_IDX, OBJ_BALL_OUT, i, FALSE);

    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_TRIO_UNIT_IDX, OBJ_BALL_IN, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, SPPOKE_TRIO_UNIT_IDX, OBJ_BALL_IN, i, FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	    fieldWork   フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_EndTrio(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //三銃士イベント用ワーク解放
  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, SPPOKE_TRIO_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_MoveTrio(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//============================================================================================================
//メロディア関連
//============================================================================================================

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_SetupMerodhia(FIELDMAP_WORK *fieldWork)
{
  SPPOKE_GMK_WK *p_wk;
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, SPPOKE_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(SPPOKE_GMK_WK));

  //メロディアイベント用ワークのアロケーション
  p_wk = GMK_TMP_WK_GetWork( fieldWork, SPPOKE_GMK_ASSIGN_ID );
  p_wk->p_fieldmap  = fieldWork;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( fieldWork );
  
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle(ptr, &SetupMero, SPPOKE_MERO_UNIT_IDX );
  
  //ボール初期化
  //OBJを非表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_MERO_UNIT_IDX, OBJ_BALL_IN, TRUE);
  FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_MERO_UNIT_IDX, OBJ_BALL_OUT, TRUE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_MERO_UNIT_IDX, OBJ_BALL_OUT, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, SPPOKE_MERO_UNIT_IDX, OBJ_BALL_OUT, i, FALSE);

    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_MERO_UNIT_IDX, OBJ_BALL_IN, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, SPPOKE_MERO_UNIT_IDX, OBJ_BALL_IN, i, FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	    fieldWork   フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_EndMerodhia(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //メロディアイベント用ワーク解放
  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, SPPOKE_MERO_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_MoveMerodhia(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//============================================================================================================
//ゾロア関連
//============================================================================================================

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_SetupZoroa(FIELDMAP_WORK *fieldWork)
{
  int i;
  HEAPID heap_id;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  heap_id = FIELDMAP_GetHeapID(fieldWork);

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, SPPOKE_GMK_ASSIGN_ID, GFL_HEAP_LOWID(heap_id), sizeof(SPPOKE_GMK_WK));
  //ゾロアイベント用ワークのアロケーション
  {
    SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
    gmk_wk->Work = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heap_id), sizeof(SMOKE_GMK_WK) );
    gmk_wk->p_fieldmap = fieldWork;
    gmk_wk->exp_obj    = FIELDMAP_GetExpObjCntPtr( fieldWork );
  }

  //必要なリソースの用意(アーカイブＩＤが２つ以上あるのでハンドル関数を使用しません)
  FLD_EXP_OBJ_AddUnit(ptr, &SetupZoroa, ZOROA_UNIT_IDX );
  
  //ボール初期化
  //OBJを非表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, ZOROA_UNIT_IDX, OBJ_BALL_IN, TRUE);
  FLD_EXP_OBJ_SetVanish(ptr, ZOROA_UNIT_IDX, OBJ_BALL_OUT, TRUE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, ZOROA_UNIT_IDX, OBJ_BALL_OUT, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, ZOROA_UNIT_IDX, OBJ_BALL_OUT, i, FALSE);

    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, ZOROA_UNIT_IDX, OBJ_BALL_IN, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, ZOROA_UNIT_IDX, OBJ_BALL_IN, i, FALSE);
  }
  //煙
  for (i=0;i<SMOKE_ANM_NUM;i++){
    EXP_OBJ_ANM_CNT_PTR anm;
    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, ZOROA_UNIT_IDX, OBJ_SMOKE, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, ZOROA_UNIT_IDX, OBJ_SMOKE, i, FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	    fieldWork   フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_EndZoroa(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //ゾロアイベント用ワーク解放
  {
    SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
    GFL_HEAP_FreeMemory( gmk_wk->Work );
  }

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, ZOROA_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_MoveZoroa(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//=============================================================================
/**
 *          【配布】ケルディオ
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @bried       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_SetupKerudhio( FIELDMAP_WORK *p_fieldmap )
{
  SPPOKE_GMK_WK *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(SPPOKE_GMK_WK) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );
  OS_Printf("ケルディオギミック\n");
}
//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_EndKerudhio( FIELDMAP_WORK *p_fieldmap )
{
  SPPOKE_GMK_WK *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );
}
//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_MoveKerudhio( FIELDMAP_WORK *p_fieldmap )
{
  SPPOKE_GMK_WK *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = KERUDHIO_CreateEvent(gsys);
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("ケルディオ演出\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      KERUDHIO_GIMMICK_EffectDelete(gsys);
      OS_Printf("エフェクト削除\n");
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ演出イベント
 *
 *	@param	GAMESYS_WORK *gsys
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* KERUDHIO_GIMMICK_Event( GAMESYS_WORK *gsys )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SPPOKE_GMK_WK *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );

  // イベント作成
  event = KERUDHIO_CreateEvent( gsys );
  
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  気合のエフェクトを削除
 *
 *	@param	GAMESYS_WORK *gsys 
 */
//-----------------------------------------------------------------------------
void KERUDHIO_GIMMICK_EffectDelete( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SPPOKE_GMK_WK *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );
  int i;

  // 気合エフェクト 非表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, KERUDHIO_UNIT_IDX, OBJ_EFFECT_01, TRUE );
  // 全アニメ停止
  for( i=0; i<anm_num_tbl[ OBJ_EFFECT_01 ]; i++ )
  {
    ANM_Stop( p_wk->exp_obj, OBJ_EFFECT_01, i );
  }
}

//=============================================================================
/**
 *					ケルディオ　static関数
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
 *	@param	SPPOKE_GMK_WK *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( SPPOKE_GMK_WK *p_wk )
{
  u16 obj_idx;

  //必要なリソースの用意(アーカイブＩＤが２つ以上あるのでハンドル関数を使用しません)
  FLD_EXP_OBJ_AddUnit( p_wk->exp_obj, &SetupKerushio, KERUDHIO_UNIT_IDX);

  // OBJ
  for( obj_idx=0; obj_idx<OBJ_NUM_KERUDHIO; obj_idx++ )
  {
    int anm_idx;
    int anm_num = anm_num_tbl[ obj_idx ];

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, KERUDHIO_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, pos_tbl[obj_idx].x, pos_tbl[obj_idx].y, pos_tbl[obj_idx].z );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, KERUDHIO_UNIT_IDX, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, KERUDHIO_UNIT_IDX, obj_idx, TRUE );

    // アニメ設定
    for( anm_idx=0; anm_idx<anm_num; anm_idx++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx, FALSE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	SPPOKE_GMK_WK *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( SPPOKE_GMK_WK *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, KERUDHIO_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	SPPOKE_GMK_WK *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( SPPOKE_GMK_WK *p_wk )
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
 *	@param  anm_idx   アニメインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  int anm_num = anm_num_tbl[ obj_idx ];

  GF_ASSERT( anm_idx < anm_num );

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx );

  // 1回再生設定
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定

  // 有効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, anm_idx, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通 登場アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_AppearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, FALSE );

  // 念のため全てのアニメ停止
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Stop( exp_obj, obj_idx, i );
  }

  // 登場アニメ再生
  ANM_Play( exp_obj, obj_idx, ANM_IDX_APPEAR_IMA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_APPEAR_IVA );
}
//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  登場アニメ終了チェック
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_AppearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, ANM_IDX_APPEAR_IMA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, ANM_IDX_APPEAR_IVA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  待機アニメ
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 */
//-----------------------------------------------------------------------------
static void ANM_Stay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;

  // 念のため全てのアニメ停止
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Stop( exp_obj, obj_idx, i );
  }

  // 登場アニメ再生
  ANM_Play( exp_obj, obj_idx, ANM_IDX_STAY_IMA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_STAY_IVA );
}

//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  消えるアニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 */
//-----------------------------------------------------------------------------
static void ANM_DisappearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;

  // 念のため全てのアニメ停止
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Stop( exp_obj, obj_idx, i );
  }

  // 登場アニメ再生
  ANM_Play( exp_obj, obj_idx, ANM_IDX_DISAPPEAR_ICA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_DISAPPEAR_IMA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_DISAPPEAR_IVA );
}
//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  消えるアニメ終了チェック
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_DisappearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, ANM_IDX_DISAPPEAR_ICA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, ANM_IDX_DISAPPEAR_IMA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, KERUDHIO_UNIT_IDX, obj_idx, ANM_IDX_DISAPPEAR_IVA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}

//=============================================================================
/**
 *          イベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ演出のイベント作成
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * KERUDHIO_CreateEvent( GAMESYS_WORK* gsys )
{
  GMEVENT*  event;
  KERUDHIO_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, KerudhioEvent, sizeof(KERUDHIO_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  work->p_fieldmap  = p_fieldmap;
  work->wait        = 0;

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ演出のイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT KerudhioEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK  *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  KERUDHIO_EVENT_WORK *p_wk = (KERUDHIO_EVENT_WORK*)wk;
  SPPOKE_GMK_WK *p_gmk_wk = GMK_TMP_WK_GetWork( p_fieldmap, SPPOKE_GMK_ASSIGN_ID );
  int i;

  // 処理シーケンス
  enum{
    SEQ_START,          // イベント開始
    SEQ_EFFECT_01,      // 気合のアニメ
    SEQ_EFFECT_01_WAIT, // 気合のアニメ待ち
    SEQ_TERAKION,       // テラキオン表示
    SEQ_TERAKION_WAIT,  // テラキオン表示待ち
    SEQ_BIRIJION,       // ビリジオン表示
    SEQ_BIRIJION_WAIT,  // ビリジオン表示待ち
    SEQ_KOBARUON,       // コバルオン表示
    SEQ_KOBARUON_WAIT,  // コバルオン表示待ち
    SEQ_ALL_DELETE,     // 3匹同時に消える
    SEQ_ALL_DELETE_WAIT,// 3匹同時に消える待ち
    SEQ_END,            // イベント終了
  };

  switch( *seq ){
  case SEQ_START:
    *seq = SEQ_EFFECT_01;
    break;

  case SEQ_EFFECT_01:      // 気合のアニメ
    {
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, KERUDHIO_UNIT_IDX, OBJ_EFFECT_01, FALSE );
      // 全アニメ再生
      for( i=0; i<anm_num_tbl[ OBJ_EFFECT_01 ]; i++ )
      {
        ANM_Play( p_wk->exp_obj, OBJ_EFFECT_01, i );
      }

      // ウェイトを設定
      p_wk->wait = KERUDHIO_EFFECT_WAIT;
    }
    *seq = SEQ_EFFECT_01_WAIT;
    break;

  case SEQ_EFFECT_01_WAIT: // 気合のアニメ待ち
    // ウェイト待ち
    if( p_wk->wait-- <= 0 )
    {
      *seq = SEQ_TERAKION;
    }
    break;

  case SEQ_TERAKION:       // テラキオン表示
    // 登場アニメ再生
    ANM_AppearPlay( p_wk->exp_obj, OBJ_TERAKION );
    *seq = SEQ_TERAKION_WAIT;
    break;

  case SEQ_TERAKION_WAIT:  // テラキオン表示待ち
    // 登場アニメ終了チェック
    if( ANM_AppearCheckEnd( p_wk->exp_obj, OBJ_TERAKION ) )
    {
      // 待機アニメ再生
      ANM_Stay( p_wk->exp_obj, OBJ_TERAKION );
      *seq = SEQ_BIRIJION;
    }
    break;

  case SEQ_BIRIJION:       // ビリジオン表示
    // 登場アニメ再生
    ANM_AppearPlay( p_wk->exp_obj, OBJ_BIRIZION );
    *seq = SEQ_BIRIJION_WAIT;
    break;

  case SEQ_BIRIJION_WAIT:  // ビリジオン表示待ち
    // 登場アニメ終了チェック
    if( ANM_AppearCheckEnd( p_wk->exp_obj, OBJ_BIRIZION ) )
    {
      // 待機アニメ再生
      ANM_Stay( p_wk->exp_obj, OBJ_BIRIZION );
      *seq = SEQ_KOBARUON;
    }
    break;

  case SEQ_KOBARUON:       // コバルオン表示
    // 登場アニメ再生
    ANM_AppearPlay( p_wk->exp_obj, OBJ_KOBARUON );
    *seq = SEQ_KOBARUON_WAIT;
    break;

  case SEQ_KOBARUON_WAIT:  // コバルオン表示待ち
    // 登場アニメ終了チェック
    if( ANM_AppearCheckEnd( p_wk->exp_obj, OBJ_KOBARUON ) )
    {
      // 待機アニメ再生
      ANM_Stay( p_wk->exp_obj, OBJ_KOBARUON );

      // ウェイトを設定
      p_wk->wait = KERUDHIO_DELETE_WAIT;
      *seq = SEQ_ALL_DELETE;
    }
    break;

  case SEQ_ALL_DELETE:     // 3匹同時に消える
    // ウェイトをまってから
    if( p_wk->wait-- <= 0 )
    {
      // 3匹同時に消えるアニメ再生
      ANM_DisappearPlay( p_wk->exp_obj, OBJ_TERAKION );
      ANM_DisappearPlay( p_wk->exp_obj, OBJ_BIRIZION );
      ANM_DisappearPlay( p_wk->exp_obj, OBJ_KOBARUON );

      *seq = SEQ_ALL_DELETE_WAIT;
    }
    break;

  case SEQ_ALL_DELETE_WAIT:// 3匹同時に消える待ち
    // アニメの内容が同じなので1つだけ消えるアニメ終了チェック
    if( ANM_DisappearCheckEnd( p_wk->exp_obj, OBJ_TERAKION) )
    {
      // 非表示にする(一応)
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, KERUDHIO_UNIT_IDX, OBJ_TERAKION, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, KERUDHIO_UNIT_IDX, OBJ_BIRIZION, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, KERUDHIO_UNIT_IDX, OBJ_KOBARUON, TRUE );

      *seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//============================================================================================================
//ボール関連　　共通部分
//============================================================================================================

//--------------------------------------------------------------
/**
 * ボールアニメイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *SPPOKE_GMK_MoveBall( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType, const VecFx32 *inStart, const VecFx32 *inEnd,
                              const fx32 inHeight, const u32 inSync)
{
  int i;
  int obj;
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  if ( inType == BALL_ANM_TYPE_OUT )
  {
    obj = OBJ_BALL_OUT;
    //投げるSE再生
    PMSND_PlaySE(SPPOKE_GMK_BALL_THROW);
  }
  else obj = OBJ_BALL_IN;

  //スタート座標に表示状態でボール配置
  {
     GFL_G3D_OBJSTATUS *status;
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, SPPOKE_UNIT_IDX, obj);
     status->trans = *inStart;
  }

  //ボールを表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_UNIT_IDX, obj, FALSE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_UNIT_IDX, obj, i);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, SPPOKE_UNIT_IDX, obj, i, FALSE);
    //頭だし
    FLD_EXP_OBJ_SetObjAnmFrm(ptr, SPPOKE_UNIT_IDX, obj, i, 0 );
  }

  //移動に必要なパラメータのセット
  {
    gmk_wk->BallStart = *inStart;
    gmk_wk->BallEnd = *inEnd;
    gmk_wk->Sync = inSync;
    gmk_wk->NowSync = 0;
    gmk_wk->Height = inHeight;
  }
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, BallMoveEvt, 0 );
  //アニメタイプを格納
  gmk_wk->AnmType = inType;

  return event;
}

//--------------------------------------------------------------
/**
 * ボールアニメイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_StartBallAnm( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType, const VecFx32 *inPos )
{
  int i;
  int obj;
  int se;
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  if ( inType == BALL_ANM_TYPE_OUT )
  {
    obj = OBJ_BALL_OUT;
    se = SPPOKE_GMK_BALL_OUT;
  }
  else
  {
    obj = OBJ_BALL_IN;
    se = SPPOKE_GMK_BALL_IN;
  }
  //SE再生
  PMSND_PlaySE(se);

  //ボール座標時セット
  {
     GFL_G3D_OBJSTATUS   *status;
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, SPPOKE_UNIT_IDX, obj);
     status->trans = *inPos;
  }

  //ボールを表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_UNIT_IDX, obj, FALSE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_UNIT_IDX, obj, i);
    //アニメ停止解除
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, SPPOKE_UNIT_IDX, obj, i, TRUE);
    //頭だし
    FLD_EXP_OBJ_SetObjAnmFrm(ptr, SPPOKE_UNIT_IDX, obj, i, 0 );
  }
}

//--------------------------------------------------------------
/**
 * ポケモンを表示・非表示していいタイミングでTRUEを返すイベント
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *SPPOKE_GMK_WaitPokeAppear( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType )
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  //イベント作成
  GMEVENT * event = GMEVENT_Create( gsys, NULL, WaitPokeAppFrmEvt, 0 );
  //アニメタイプを格納
  gmk_wk->AnmType = inType;

  return event;
}

//--------------------------------------------------------------
/**
 * ボールアニメ終了まで待つイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *SPPOKE_GMK_WaitBallAnmEnd( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType )
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  //イベント作成
  GMEVENT * event = GMEVENT_Create( gsys, NULL, WaitBallAnmEvt, 0 );
  //アニメタイプを格納
  gmk_wk->AnmType = inType;
  return event;
}

//--------------------------------------------------------------
/**
 * ボールの投げ、戻りによる移動イベント
 * @param       event             イベントポインタ
 * @param       seq               シーケンサ
 * @param       work              ワークポインタ
 * @return      GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT BallMoveEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  int obj;
  if ( gmk_wk->AnmType == BALL_ANM_TYPE_OUT ) obj = OBJ_BALL_OUT;
  else obj = OBJ_BALL_IN;

  //目的地に向かって飛ぶ
  {
    VecFx32 vec;
    fx32 diff;
    gmk_wk->NowSync++;
    diff = gmk_wk->BallEnd.x - gmk_wk->BallStart.x;
    vec.x = gmk_wk->BallStart.x + ( (diff*gmk_wk->NowSync) / gmk_wk->Sync );
    diff = gmk_wk->BallEnd.y - gmk_wk->BallStart.y;
    vec.y = gmk_wk->BallStart.y + ( (diff*gmk_wk->NowSync) / gmk_wk->Sync );
    diff = gmk_wk->BallEnd.z - gmk_wk->BallStart.z;
    vec.z = gmk_wk->BallStart.z + ( (diff*gmk_wk->NowSync) / gmk_wk->Sync );

    {
      int h_sync = (gmk_wk->Sync+1)/2;
      int now_h_sync;
      fx32 h;
      if ( gmk_wk->NowSync < h_sync ){
        now_h_sync = gmk_wk->NowSync;
      }else{
        now_h_sync = (gmk_wk->Sync-gmk_wk->NowSync);
      }
      h = (gmk_wk->Height * now_h_sync) / gmk_wk->Sync;
      vec.y += h;
    }

    {
     GFL_G3D_OBJSTATUS *status;
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, SPPOKE_UNIT_IDX, obj);
     status->trans = vec;
    }
  }
  //到着したか？
  if (gmk_wk->NowSync >= gmk_wk->Sync)
  {
    //戻りの場合はボールを非表示にする
    if ( gmk_wk->AnmType == BALL_ANM_TYPE_IN )
    {
      FLD_EXP_OBJ_SetVanish(ptr, SPPOKE_UNIT_IDX, obj, TRUE);
    }
    //イベント終了
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ポケモンを表示していいタイミングでT終了するイベント
 * @param       event             イベントポインタ
 * @param       seq               シーケンサ
 * @param       work              ワークポインタ
 * @return      GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WaitPokeAppFrmEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  fx32 frm, dst_frm;
  int obj;
  if ( gmk_wk->AnmType == BALL_ANM_TYPE_OUT )
  {
    dst_frm = BALL_OUT_TIMMING * FX32_ONE;
    obj = OBJ_BALL_OUT;
  }
  else
  {
    dst_frm = BALL_IN_TIMMING * FX32_ONE;
    obj = OBJ_BALL_IN;
  }


  //ボールアニメの現在フレームを取得
  frm = FLD_EXP_OBJ_GetObjAnmFrm(ptr, SPPOKE_UNIT_IDX, obj, 0 );
  //ポケモン出していいフレームか？
  if (frm >= dst_frm)
  {
    //イベント終了
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ボールアニメ終了で終了するイベント
 * @param       event             イベントポインタ
 * @param       seq               シーケンサ
 * @param       work              ワークポインタ
 * @return      GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WaitBallAnmEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  EXP_OBJ_ANM_CNT_PTR anm;
  int obj;

  if (gmk_wk->AnmType == BALL_ANM_TYPE_OUT) obj = OBJ_BALL_OUT;
  else obj = OBJ_BALL_IN;

  //０番目にアニメで終了判定する
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, SPPOKE_UNIT_IDX, obj, 0);
  if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
  {
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 煙アニメイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *SPPOKE_GMK_SmokeAnm( GAMESYS_WORK *gsys, const VecFx32 *inPos, const int inBefore, const int inAfter)
{
  int i;
  int obj;
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  {
    SMOKE_GMK_WK *wk = (SMOKE_GMK_WK*)gmk_wk->Work;
    wk->BeforeObj = inBefore;
    wk->AfterObj = inAfter;
  }

  obj = OBJ_SMOKE;
  //スタート座標に表示状態でボール配置
  {
     GFL_G3D_OBJSTATUS *status;
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, ZOROA_UNIT_IDX, obj);
     status->trans = *inPos;
  }

  //煙を表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, ZOROA_UNIT_IDX, obj, FALSE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, ZOROA_UNIT_IDX, obj, i);
    //アニメ停止解除
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, ZOROA_UNIT_IDX, obj, i, TRUE);
    //頭だし
    FLD_EXP_OBJ_SetObjAnmFrm(ptr, ZOROA_UNIT_IDX, obj, i, 0 );
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, SmokeMoveEvt, 0 );
  return event;
}

//--------------------------------------------------------------
/**
 * 煙アニメイベント
 * @param       event             イベントポインタ
 * @param       seq               シーケンサ
 * @param       work              ワークポインタ
 * @return      GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SmokeMoveEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  int obj = OBJ_SMOKE;
  EXP_OBJ_ANM_CNT_PTR anm;
  fx32 frame;

  //フレーム取得
  frame = FLD_EXP_OBJ_GetObjAnmFrm( ptr, ZOROA_UNIT_IDX, OBJ_SMOKE, 0 );

  //フレーム待ち
  if (frame == ZOROA_CHG_FRAME_FX32)
  {
    //@note 注意：以下の処理はスクリプトコマンド EvCmdObjDel, EvCmdObjAddEventと同じ処理を行うものです。
    //@note EvCmdObjDel, EvCmdObjAddEventの実装が変化する場合は注意が必要です。
    u16 before, after;
    MMDL *mmdl;
    MMDLSYS *mmdlsys;
    {
      SMOKE_GMK_WK *wk = (SMOKE_GMK_WK*)gmk_wk->Work;
      before = wk->BeforeObj;
      after = wk->AfterObj;
    }
    mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
    //ＯＢＪ消す
    mmdl = MMDLSYS_SearchOBJID( mmdlsys, before );
    GF_ASSERT_MSG( mmdl != NULL,"OBJ DEL %d 対象のOBJが居ません\n", before );
    MMDL_Delete( mmdl );
    //ＯＢＪ出す
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
      EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
      u16 count = EVENTDATA_GetNpcCount( evdata );
      PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
      int zone_id = PLAYERWORK_getZoneID( player );
      if( count )
      {
        EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
        const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
        mmdl = MMDLSYS_AddMMdlHeaderID(
            mmdlsys, header, zone_id, count, evwork, after );
        if( mmdl != NULL ){ //追加されたOBJは即ポーズさせる
          MMDL_OnMoveBitMoveProcPause( mmdl );
        }
        GF_ASSERT( mmdl != NULL );
      }
      else GF_ASSERT( 0 );
    }
  }

  //０番目にアニメで終了判定する
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, ZOROA_UNIT_IDX, obj, 0);
  if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
  {
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

