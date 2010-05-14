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

#include "sound/pm_sndsys.h"
#include "sp_poke_gimmick_se_def.h"

#include "../../../resource/fldmapdata/gimmick/zoroa/zoroa_gmk_timing_def.h" //for ZOROA_CHG_FRAME
#include "field/eventdata_sxy.h"  //for  EVENTDATA_GetNpcData

#define SPPOKE_GMK_ASSIGN_ID    (1)
#define SPPOKE_UNIT_IDX    (0)                      //特殊ポケイベントユニットインデックス
#define SPPOKE_TRIO_UNIT_IDX    (SPPOKE_UNIT_IDX)   //三銃士イベントユニットインデックス
#define SPPOKE_MERO_UNIT_IDX    (SPPOKE_UNIT_IDX)   //メロディアイベントユニットインデックス
#define ZOROA_UNIT_IDX    (SPPOKE_UNIT_IDX)         //ゾロアイベントユニットインデックス

#define BALL_ANM_NUM  (3)
#define SMOKE_ANM_NUM  (3)

#define BALL_OUT_TIMMING  (5)
#define BALL_IN_TIMMING (34)

#define ZOROA_CHG_FRAME_FX32 (ZOROA_CHG_FRAME * FX32_ONE)

typedef struct SPPOKE_GMK_WK_tag
{
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

  RES_ID_SMOKE_MDL = 8,
  RES_ID_SMOKE_ANM1,
  RES_ID_SMOKE_ANM2,
  RES_ID_SMOKE_ANM3,
};

//ＯＢＪインデックス
enum {
  OBJ_BALL_OUT = 0,
  OBJ_BALL_IN,
  OBJ_SMOKE = 2,
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

//============================================================================================================
//関数前方宣言
//============================================================================================================
static GMEVENT_RESULT BallMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitPokeAppFrmEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitBallAnmEvt( GMEVENT* event, int* seq, void* work );

static GMEVENT_RESULT SmokeMoveEvt( GMEVENT* event, int* seq, void* work );

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
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, SPPOKE_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(SPPOKE_GMK_WK));
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
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, SPPOKE_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(SPPOKE_GMK_WK));
  //メロディアイベント用ワークのアロケーション
  ;
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

