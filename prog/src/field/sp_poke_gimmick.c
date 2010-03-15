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

//#include "../../../resource/fldmapdata/gimmick/gym_anti/gym_anti_local_def.h"


#define SCR_BALL_ANM_TYPE_OUT   (0)
#define SCR_BALL_ANM_TYPE_IN   (1)


#define SPPOKE_GMK_ASSIGN_ID    (1)
#define SPPOKE_TRIO_UNIT_IDX    (0)   //三銃士イベントユニットインデックス

typedef struct SPPOKE_GMK_WK_tag
{
  BALL_ANM_TYPE AnmType;    //ターゲットとなるアニメタイプを格納する
  VecFx32 BallStart;
  VecFx32 BallEnd;
  fx32 Hieght;
  u16 Sync;
  u16 NowSync;

  void *Work;
}SPPOKE_GMK_WK;

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
};


//ＯＢＪインデックス
enum {
  OBJ_BALL_OUT = 0,
  OBJ_BALL_IN,
};

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


static GMEVENT_RESULT BallMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitPokeAppFrmEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitBallOutAnmEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_Setup(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, SPPOKE_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(SPPOKE_GMK_WK));
  //三銃士イベント用ワークのアロケーション
  ;
  //必要なリソースの用意
//  FLD_EXP_OBJ_AddUnit(ptr, &SetupTrio, SPPOKE_TRIO_UNIT_IDX );
  
  //ボール初期化
  {
    //始めは非表示
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	    fieldWork   フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //三銃士イベント用ワーク解放
  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  //ＯＢＪ解放
//  FLD_EXP_OBJ_DelUnit( ptr, SPPOKE_TRIO_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void SPPOKE_GMK_Move(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //アニメーション再生
//  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * ボールアニメイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *SPPOKE_GMK_StartBallAnm( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType, const VecFx32 *inStart, const VecFx32 *inEnd , const u32 inSync)
{
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //スタート座標に表示状態でボール配置
  ;
  //アニメの状態を初期化
  ;
  //移動に必要なパラメータのセット
  ;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, BallMoveEvt, 0 );
  //アニメタイプを格納
  gmk_wk->AnmType = inType;

  return event;
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
  GMEVENT * event = GMEVENT_Create( gsys, NULL, WaitBallOutAnmEvt, 0 );
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
//  SPPOKE_GMK_WK *tmp = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);

  //目的地に向かって飛ぶ
  //
  //到着したか？
  if (1)
  {
    //タイプを見てアニメ開始
    if (gmk_wk->AnmType == BALL_ANM_TYPE_OUT);
    else ;
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
//  SPPOKE_GMK_WK *tmp = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);

  //ボールアニメの現在フレームを取得
  ;
  //ポケモン出していいフレームか？
  if (1)
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
static GMEVENT_RESULT WaitBallOutAnmEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
//  SPPOKE_GMK_WK *tmp = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);

  //ボールアニメの現在フレームを取得
  ;
  //アニメ終了か？
  if (1)
  {
    //イベント終了
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
