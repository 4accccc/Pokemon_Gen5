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


#define SPPOKE_GMK_ASSIGN_ID    (1)
#define SPPOKE_UNIT_IDX    (0)                      //特殊ポケイベントユニットインデックス
#define SPPOKE_TRIO_UNIT_IDX    (SPPOKE_UNIT_IDX)   //三銃士イベントユニットインデックス
#define SPPOKE_MERO_UNIT_IDX    (SPPOKE_UNIT_IDX)   //メロディアイベントユニットインデックス
#define BALL_ANM_NUM  (3)

#define BALL_OUT_TIMMING  (5)
#define BALL_IN_TIMMING (34)

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

//==============================================================================================
/**
 @note    ※三銃士とメロディアでボールリソースは共有するので、リソース番号は一致するようにする
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
};


//ＯＢＪインデックス
enum {
  OBJ_BALL_OUT = 0,
  OBJ_BALL_IN,
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

//==========================================================================
/**
 三銃士関連リソース
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


static GMEVENT_RESULT BallMoveEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitPokeAppFrmEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitBallAnmEvt( GMEVENT* event, int* seq, void* work );

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
  FLD_EXP_OBJ_AddUnit(ptr, &SetupTrio, SPPOKE_TRIO_UNIT_IDX );
  
  //ボール初期化
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
  FLD_EXP_OBJ_AddUnit(ptr, &SetupTrio, SPPOKE_MERO_UNIT_IDX );
  
  //ボール初期化
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

  if ( inType == BALL_ANM_TYPE_OUT ) obj = OBJ_BALL_OUT;
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
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  SPPOKE_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, SPPOKE_GMK_ASSIGN_ID);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  if ( inType == BALL_ANM_TYPE_OUT ) obj = OBJ_BALL_OUT;
  else obj = OBJ_BALL_IN;

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


