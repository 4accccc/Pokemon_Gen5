//======================================================================
/**
 * @file  field_gimmick_t01.c
 * @brief  Ｔ01 ギミック
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_t01.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "system/gfl_use.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/arc_def.h"
#include "arc/fieldmap/t01_gmk.naix"
#include "gmk_tmp_wk.h"
#include "system/rtc_tool.h"  //for PM_RTC_GetTimeZone
#include "field_gimmick_t01_sv.h"
#include "field_gimmick_t01_se_def.h"

#define EXPOBJ_UNIT_IDX (0)
#define T01_TMP_ASSIGN_ID  (1)

#define BIRD_X  (FIELD_CONST_GRID_FX32_SIZE * 784)
#define BIRD_Y  (FX32_ONE * 0)
#define BIRD_Z  (FIELD_CONST_GRID_FX32_SIZE * 752)

#define BIRD_ANM_NUM  (2)

#define DAY_SE_PLAY_FRM (40 * FX32_ONE)   //１羽遅れて羽ばたくフレーム

//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソース
typedef enum {
  RES_BIRD_NSBMD,   //鳥のモデル
  RES_BIRD_NSBCA,   //鳥アニメ
  RES_BIRD_NSBTP,   //鳥アニメ
} RES_INDEX;

//朝用
static const GFL_G3D_UTIL_RES res_table_morning[] = 
{
  { ARCID_T01_GMK, NARC_t01_gmk_t01_bird01_nsbmd, GFL_G3D_UTIL_RESARC },  // 鳥のモデル
  { ARCID_T01_GMK, NARC_t01_gmk_t01_bird01_nsbca, GFL_G3D_UTIL_RESARC },  // 鳥のアニメ
  { ARCID_T01_GMK, NARC_t01_gmk_t01_bird01_nsbtp, GFL_G3D_UTIL_RESARC },  // 鳥のアニメ
};

//夜用
static const GFL_G3D_UTIL_RES res_table_night[] = 
{
  { ARCID_T01_GMK, NARC_t01_gmk_t01_bird02_nsbmd, GFL_G3D_UTIL_RESARC },  // 鳥のモデル
  { ARCID_T01_GMK, NARC_t01_gmk_t01_bird02_nsbca, GFL_G3D_UTIL_RESARC },  // 鳥のアニメ
  { ARCID_T01_GMK, NARC_t01_gmk_t01_bird02_nsbtp, GFL_G3D_UTIL_RESARC },  // 鳥のアニメ
};

// オブジェクト
typedef enum {
  OBJ_BIRD,  // 鳥
} OBJ_INDEX;

//3Dアニメ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTb[] = {
  { RES_BIRD_NSBCA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_BIRD_NSBTP,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};


static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  { RES_BIRD_NSBMD, 0, RES_BIRD_NSBMD, g3Dutil_anmTb, NELEMS(g3Dutil_anmTb) },  // 鳥
};

//朝用
static const GFL_G3D_UTIL_SETUP SetupMorning = {
  res_table_morning,				//リソーステーブル
	NELEMS(res_table_morning),		//リソース数
	obj_table,				//オブジェクト設定テーブル
	NELEMS(obj_table),		//オブジェクト数
};

//夜用
static const GFL_G3D_UTIL_SETUP SetupNight = {
  res_table_night,				//リソーステーブル
	NELEMS(res_table_night),		//リソース数
	obj_table,				//オブジェクト設定テーブル
	NELEMS(obj_table),		//オブジェクト数
};

typedef struct
{ 
  BOOL Move;
  BOOL IsNight;
} T01_GMK_WORK;

static GMEVENT_RESULT EndChkEvt( GMEVENT* event, int* seq, void* work );


//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void T01_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  int i;
  T01_SV_WORK *gmk_sv_work;
  T01_GMK_WORK* gmk_work;  // ギミック管理ワーク
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
  {
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_T01 );
  }

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldmap, T01_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldmap), sizeof(T01_GMK_WORK));
  gmk_work = GMK_TMP_WK_GetWork(fieldmap, T01_TMP_ASSIGN_ID);

  //セットアップ時はギミックを動作させない
  gmk_work->Move = FALSE;

  //時間帯で分岐
  {
    int season, time_zone;
    season = GAMEDATA_GetSeasonID(gamedata);
    time_zone = PM_RTC_GetTimeZone(season);
    // 拡張オブジェクトのユニットを追加
    if ( (time_zone == TIMEZONE_NIGHT) || (time_zone == TIMEZONE_MIDNIGHT) )
    {
      FLD_EXP_OBJ_AddUnitByHandle( exobj_cnt, &SetupNight, EXPOBJ_UNIT_IDX );
      gmk_work->IsNight = TRUE;
    }
    else
    {
      FLD_EXP_OBJ_AddUnitByHandle(exobj_cnt, &SetupMorning, EXPOBJ_UNIT_IDX);
      gmk_work->IsNight = FALSE;
    }
  }

  {
    VecFx32 pos = { BIRD_X, BIRD_Y, BIRD_Z };
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exobj_cnt, EXPOBJ_UNIT_IDX, 0);
    status->trans = pos;
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, 0, TRUE);
    {
      BOOL vanish;
      if ( gmk_sv_work->Disp ) vanish = FALSE;   //表示
      else vanish = TRUE;                      //非表示
      FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, 0, vanish);
    }
  }

  //アニメの状態を初期化
  for (i=0;i<BIRD_ANM_NUM;i++)
  {
    BOOL valid;
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_BIRD, i);
    //1回再生
    FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
    //アニメ停止
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    if ( gmk_sv_work->Disp )
    {
      //有効
      valid = TRUE;
    }
    else
    {
      valid = FALSE;
    }
    FLD_EXP_OBJ_ValidCntAnm(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_BIRD, i, valid);
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void T01_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  //ユニット破棄
  FLD_EXP_OBJ_DelUnit( exobj_cnt, EXPOBJ_UNIT_IDX );
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldmap, T01_TMP_ASSIGN_ID);
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void T01_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  T01_GMK_WORK* gmk_work = GMK_TMP_WK_GetWork(fieldmap, T01_TMP_ASSIGN_ID);

  if ( gmk_work->Move )
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    //アニメーション再生
    FLD_EXP_OBJ_PlayAnime( exobj_cnt );

    if (!gmk_work->IsNight)
    {
      fx32 frm;
      //アニメフレーム取得
      frm = FLD_EXP_OBJ_GetObjAnmFrm( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_BIRD, 0 );
      if ( frm == DAY_SE_PLAY_FRM ) PMSND_PlaySE( T01_GMK_DAY_SE02 );      //朝2
    }

    //インデックス0のアニメで終了判定する
    anm = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_BIRD, 0);
    //終了チェック
    if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
    {
      //ＯＢＪ非表示
      FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, 0, TRUE);
      gmk_work->Move = FALSE;    //ギミック停止
      {
        T01_SV_WORK *gmk_sv_work;
        GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
        GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
        gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_T01 );
        gmk_sv_work->Disp = FALSE;    //非表示
      }
    }    
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックアニメ開始
 *
 * @param fieldmap フィールドマップポインタ
 */
//------------------------------------------------------------------------------------------
void T01_GIMMICK_Start( FIELDMAP_WORK* fieldmap )
{
  int i;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  T01_GMK_WORK* gmk_work;
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  gmk_work = GMK_TMP_WK_GetWork(fieldmap, T01_TMP_ASSIGN_ID);
  gmk_work->Move = TRUE;    //動作開始
  //表示
  FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, 0, FALSE);
  //アニメの状態を初期化
  for (i=0;i<BIRD_ANM_NUM;i++)
  {
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_BIRD, i);
    //アニメ再生
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
  }
  //SE再生
  if ( gmk_work->IsNight ) PMSND_PlaySE( T01_GMK_NIGHT_SE );    //夜
  else PMSND_PlaySE( T01_GMK_DAY_SE01 );      //朝1
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックアニメ終了監視イベント作成
 *
 * @param gsys ゲームシステムポインタ
 */
//------------------------------------------------------------------------------------------
GMEVENT *T01_GIMMICK_CreateEndChkEvt( GAMESYS_WORK *gsys )
{
  GMEVENT * event;
  event = GMEVENT_Create( gsys, NULL, EndChkEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * アニメ終了監視イベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EndChkEvt( GMEVENT* event, int* seq, void* work )
{
  EXP_OBJ_ANM_CNT_PTR anm;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  T01_GMK_WORK* gmk_work = GMK_TMP_WK_GetWork(fieldWork, T01_TMP_ASSIGN_ID);

  //終了チェック
  if ( gmk_work->Move == FALSE ) return GMEVENT_RES_FINISH;    //アニメ終了
  
  return GMEVENT_RES_CONTINUE;
}


