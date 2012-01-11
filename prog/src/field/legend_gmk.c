//======================================================================
/**
 * @file  legend_gmk.h
 * @brief  伝説ポケモンギミック
 *
 * @note    配布系で使用しているモンスターボールアニメーションをここではリソースのみ共有してプログラムを作成しています
 * @note    本当は統合した方がいいのですが、こちらは開発終盤まで調整が入る可能性があるため、ＦＩＸしている配布系ボールアニメーションと切り離します
 * @note    結果、同じ動きをするコードが生まれる可能性がありますが、切り離すことで、配布系ボールアニメーションの動作を保証することを優先します
 * @author  Saito
 */
//======================================================================

#include "fieldmap.h"
#include "legend_gmk.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/legend_gmk.naix"
#include "sound/pm_sndsys.h"
#include "legend_gmk_se_def.h"
#include "sp_poke_gimmick_se_def.h"

#include "../../../resource/fldmapdata/gimmick/legend/leg_gmk_timing_def.h"

#define LEGEND_GMK_ASSIGN_ID    (1)
#define LEGEND_UNIT_IDX    (0)                      //ユニットインデックス
#define STONE_ANM_NUM  (4)

#define FLASH_FRAME_FX32  ( FLASH_FRAME*FX32_ONE )

#define FADE_SPEED    (0)

#define BALL_ANM_NUM  (3)
#define BALL_OUT_TIMMING  (5)

typedef struct LEG_GMK_WK_tag
{
  u32 Fade;
  int Count;
  BOOL Se0;
  BOOL Se1;
  BOOL Se2;
  BOOL Se3;
  VecFx32 BallStart;
  VecFx32 BallEnd;
  fx32 Height;
  u16 Sync;
  u16 NowSync;

}LEG_GMK_WK;


//==========================================================================
/**
 *  関数前方宣言
*/
//==========================================================================
static GMEVENT_RESULT StoneEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitBallAnmEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WaitPokeAppFrmEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT BallMoveEvt( GMEVENT* event, int* seq, void* work );

//リソースの並び順番
enum {
  RES_ID_STONE_MDL = 0,

  RES_ID_STONE_ANM1,
  RES_ID_STONE_ANM2,
  RES_ID_STONE_ANM3,
  RES_ID_STONE_ANM4,

  RES_ID_BALL_OUT_MDL,
  RES_ID_BALL_OUT_ANM1,
  RES_ID_BALL_OUT_ANM2,
  RES_ID_BALL_OUT_ANM3,
};

//ＯＢＪインデックス
enum {
  OBJ_STONE = 0,
  OBJ_BALL_OUT,
};

//==========================================================================
/**
 リソース部分
*/
//==========================================================================

//3Dアニメ　石
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl[] = {
  { RES_ID_STONE_ANM1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_STONE_ANM2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_STONE_ANM3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_STONE_ANM4,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　ボールアウト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_ball_out[] = {
  { RES_ID_BALL_OUT_ANM1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_BALL_OUT_ANM2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_BALL_OUT_ANM3,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};


//==========================================================================
/**
 リソース
*/
//==========================================================================

static const GFL_G3D_UTIL_RES g3Dutil_resTbl_black[] = {
	{ ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD

  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_p_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP

  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールアウト
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールアウトアニメ
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
};

static const GFL_G3D_UTIL_RES g3Dutil_resTbl_white[] = {
	{ ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD

  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbma, GFL_G3D_UTIL_RESARC }, //IMA
  { ARCID_LEGEND_GMK, NARC_legend_gmk_shin_mu_p_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mu_p_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP

  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD ボールアウト
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA ボールアウトアニメ
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbtp, GFL_G3D_UTIL_RESARC }, //ITP
  { ARCID_LEGEND_GMK, NARC_legend_gmk_mb_out2_nsbma, GFL_G3D_UTIL_RESARC }, //IMA

};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //石
  {
		RES_ID_STONE_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_STONE_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
  //出現モンスターボール
  {
		RES_ID_BALL_OUT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_BALL_OUT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ball_out,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ball_out),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP SetupWhite = {
  g3Dutil_resTbl_white,				//リソーステーブル
	NELEMS(g3Dutil_resTbl_white),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

static const GFL_G3D_UTIL_SETUP SetupBlack = {
  g3Dutil_resTbl_black,				//リソーステーブル
	NELEMS(g3Dutil_resTbl_black),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void LEGEND_GMK_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  LEG_GMK_WK *gmk_wk;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  gmk_wk = GMK_TMP_WK_AllocWork
      (fieldWork, LEGEND_GMK_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(LEG_GMK_WK));
  gmk_wk->Count = 0;
  
  //必要なリソースの用意(バージョン分岐)
  if ( GetVersion() == VERSION_WHITE )
  {
    FLD_EXP_OBJ_AddUnitByHandle(ptr, &SetupWhite, LEGEND_UNIT_IDX );
  }
  else
  {
    FLD_EXP_OBJ_AddUnitByHandle(ptr, &SetupBlack, LEGEND_UNIT_IDX );
  }
  gmk_wk->Fade = GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN;

  //OBJを非表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, LEGEND_UNIT_IDX, OBJ_STONE, TRUE);
  FLD_EXP_OBJ_SetVanish(ptr, LEGEND_UNIT_IDX, OBJ_BALL_OUT, TRUE);

  //3ＤＯＢＪ初期化
  //アニメの状態を初期化
  for (i=0;i<STONE_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, OBJ_STONE, i);
    //1回再生
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, LEGEND_UNIT_IDX, OBJ_STONE, i, FALSE);
  }

  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    //1回再生設定
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, OBJ_BALL_OUT, i);
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, LEGEND_UNIT_IDX, OBJ_BALL_OUT, i, FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	    fieldWork   フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void LEGEND_GMK_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, LEGEND_GMK_ASSIGN_ID);

  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, LEGEND_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void LEGEND_GMK_Move(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * ギミック開始イベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *LEGEND_GMK_Start( GAMESYS_WORK *gsys )
{
  int i;
  int obj;
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  
  obj = OBJ_STONE;
  //主人公の位置に表示状態でＯＢＪ配置
  {
    VecFx32 pos;
    GFL_G3D_OBJSTATUS *status;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, LEGEND_UNIT_IDX, obj);
    status->trans = pos;
  }

  //OBJを表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, LEGEND_UNIT_IDX, obj, FALSE);
  //アニメの状態を初期化
  for (i=0;i<STONE_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, obj, i);
    //アニメ再生
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, LEGEND_UNIT_IDX, obj, i, TRUE);
    //頭だし
    FLD_EXP_OBJ_SetObjAnmFrm(ptr, LEGEND_UNIT_IDX, obj, i, 0 );
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, StoneEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 石アニメイベント
 * @param       event             イベントポインタ
 * @param       seq               シーケンサ
 * @param       work              ワークポインタ
 * @return      GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT StoneEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  int obj;
  fx32 frm, last_frm;
  LEG_GMK_WK *gmk_wk;

  gmk_wk = GMK_TMP_WK_GetWork(fieldWork, LEGEND_GMK_ASSIGN_ID);
  obj = OBJ_STONE;

  //現在フレームを取得
  frm = FLD_EXP_OBJ_GetObjAnmFrm(ptr, LEGEND_UNIT_IDX, obj, 0 );
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, OBJ_STONE, 0);
    last_frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm );
  }
  NOZOMU_Printf( "frm = %x::%d  last= %x::%d\n",frm, frm/FX32_ONE, last_frm, last_frm/FX32_ONE );

  if ( (frm >= LEG_SE0_FRM*FX32_ONE)&&(!gmk_wk->Se0) ){
    //音停止
    PMSND_StopSE();
    //ＳＥ再生
    PMSND_PlaySE( LEG_GMK_BALL_APP );
    gmk_wk->Se0 = TRUE;
  }
  else if ( (frm >= LEG_SE1_FRM*FX32_ONE)&&(!gmk_wk->Se1) ){
    //音停止
    PMSND_StopSE();
    //ＳＥ再生（ループ音）
    PMSND_PlaySE( LEG_GMK_ENERGY );
    gmk_wk->Se1 = TRUE;
  }
  else if ( (frm >= LEG_SE2_FRM*FX32_ONE)&&(!gmk_wk->Se2) )
  {
    //音停止
    PMSND_StopSE();
    //ＳＥ再生
    PMSND_PlaySE( LEG_GMK_BALL );
    gmk_wk->Se2 = TRUE;
  }
  else if ( (frm >= LEG_SE3_FRM*FX32_ONE)&&(!gmk_wk->Se3) )
  {
    //音停止
    PMSND_StopSE();
    //ＳＥ再生
    PMSND_PlaySE( LEG_GMK_FINISH );
    gmk_wk->Se3 = TRUE;
  }


  switch(*seq){
  case 0:
    if ( frm == FLASH_FRAME_FX32 )
    {
      //フェードアウト開始
      GFL_FADE_SetMasterBrightReq( gmk_wk->Fade, 0, 16, FADE_SPEED );
      gmk_wk->Count = FADE_WAIT;
      (*seq)++;
    }
    break;
  case 1:
    //フェードアウト完了待ち
    if ( GFL_FADE_CheckFade() == FALSE )
    {
      if ( gmk_wk->Count <=0 )
      {
        //フェードイン開始
        GFL_FADE_SetMasterBrightReq( gmk_wk->Fade, 16, 0, FADE_SPEED );
        (*seq)++;
      }
      else gmk_wk->Count--;
    }
    break;
  case 2:
    //フェードイン完了待ち
    if ( GFL_FADE_CheckFade() == FALSE )
    {
      (*seq)++;
    }
    break;
  default:
    ;
  }

  //終了していいフレームか？
  if (frm >= last_frm)
  {
    NOZOMU_Printf("アニメ終了\n");
    //イベント終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//============================================================================================================
//ボール関連
//============================================================================================================

//--------------------------------------------------------------
/**
 * ボールアニメイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *LEGEND_GMK_MoveBall( GAMESYS_WORK *gsys, const VecFx32 *inStart, const VecFx32 *inEnd,
                              const fx32 inHeight, const u32 inSync)
{
  int i;
  int obj;
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  LEG_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, LEGEND_GMK_ASSIGN_ID);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  obj = OBJ_BALL_OUT;
  //投げるSE再生
  PMSND_PlaySE(SPPOKE_GMK_BALL_THROW);

  //スタート座標に表示状態でボール配置
  {
     GFL_G3D_OBJSTATUS *status;
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, LEGEND_UNIT_IDX, obj);
     status->trans = *inStart;
  }

  //ボールを表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, LEGEND_UNIT_IDX, obj, FALSE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, obj, i);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    //無効化
    FLD_EXP_OBJ_ValidCntAnm(ptr, LEGEND_UNIT_IDX, obj, i, FALSE);
    //頭だし
    FLD_EXP_OBJ_SetObjAnmFrm(ptr, LEGEND_UNIT_IDX, obj, i, 0 );
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

  return event;
}

//--------------------------------------------------------------
/**
 * ボールアニメイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void LEGEND_GMK_StartBallAnm( GAMESYS_WORK *gsys, const VecFx32 *inPos )
{
  int i;
  int obj;
  int se;
  GMEVENT *event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  obj = OBJ_BALL_OUT;
  se = SPPOKE_GMK_BALL_OUT;
  //SE再生
  PMSND_PlaySE(se);

  //ボール座標時セット
  {
     GFL_G3D_OBJSTATUS   *status;
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, LEGEND_UNIT_IDX, obj);
     status->trans = *inPos;
  }

  //ボールを表示状態にする
  FLD_EXP_OBJ_SetVanish(ptr, LEGEND_UNIT_IDX, obj, FALSE);
  //アニメの状態を初期化
  for (i=0;i<BALL_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, obj, i);
    //アニメ停止解除
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    //無効化解除
    FLD_EXP_OBJ_ValidCntAnm(ptr, LEGEND_UNIT_IDX, obj, i, TRUE);
    //頭だし
    FLD_EXP_OBJ_SetObjAnmFrm(ptr, LEGEND_UNIT_IDX, obj, i, 0 );
  }
}

//--------------------------------------------------------------
/**
 * ポケモンを表示・非表示していいタイミングでTRUEを返すイベント
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *LEGEND_GMK_WaitPokeAppear( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  //イベント作成
  GMEVENT * event = GMEVENT_Create( gsys, NULL, WaitPokeAppFrmEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * ボールアニメ終了まで待つイベント作成
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *LEGEND_GMK_WaitBallAnmEnd( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  //イベント作成
  GMEVENT * event = GMEVENT_Create( gsys, NULL, WaitBallAnmEvt, 0 );
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
  LEG_GMK_WK *gmk_wk = GMK_TMP_WK_GetWork(fieldWork, LEGEND_GMK_ASSIGN_ID);
  int obj;
  obj = OBJ_BALL_OUT;

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
     status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, LEGEND_UNIT_IDX, obj);
     status->trans = vec;
    }
  }
  //到着したか？
  if (gmk_wk->NowSync >= gmk_wk->Sync)
  {
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
  fx32 frm, dst_frm;
  int obj;
  {
    dst_frm = BALL_OUT_TIMMING * FX32_ONE;
    obj = OBJ_BALL_OUT;
  }
  //ボールアニメの現在フレームを取得
  frm = FLD_EXP_OBJ_GetObjAnmFrm(ptr, LEGEND_UNIT_IDX, obj, 0 );
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
  EXP_OBJ_ANM_CNT_PTR anm;
  int obj;

  obj = OBJ_BALL_OUT;

  //０番目にアニメで終了判定する
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, LEGEND_UNIT_IDX, obj, 0);
  if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
  {
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}
