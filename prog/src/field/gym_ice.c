//======================================================================
/**
 * @file  gym_ice.c
 * @brief  氷ジム
 * @author  Saito
 * @date  09.11.26
 */
//======================================================================

#include "fieldmap.h"
#include "gym_ice_sv.h"
#include "gym_ice.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_ice.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP

#include "../../../resource/fldmapdata/gimmick/gym_ice/gym_ice_local_def.h"

#include "sound/pm_sndsys.h"
#include "gym_ice_se_def.h"

#include "field/fieldmap_proc.h"    //for FLDMAP_CTRLTYPE
#include "fieldmap_ctrl_hybrid.h" //for FIELDMAP_CTRL_HYBRID

#define GYM_ICE_UNIT_IDX (0)
#define GYM_ICE_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define WALL_ANM_NUM  (2)

#define RAIL_CHECK_MAX  (8)

#define WALL1_X (WALL1_GX*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define WALL2_X (WALL2_GX*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define WALL3_X (WALL3_GX*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define WALL1_Y (WALL1_GY*FIELD_CONST_GRID_FX32_SIZE)
#define WALL2_Y (WALL2_GY*FIELD_CONST_GRID_FX32_SIZE)
#define WALL3_Y (WALL3_GY*FIELD_CONST_GRID_FX32_SIZE)
#define WALL1_Z (WALL1_GZ*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define WALL2_Z (WALL2_GZ*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define WALL3_Z (WALL3_GZ*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define SW1_X (SW1_GX*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define SW2_X (SW2_GX*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define SW3_X (SW3_GX*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define SW1_Y (SW1_GY*FIELD_CONST_GRID_FX32_SIZE)
#define SW2_Y (SW2_GY*FIELD_CONST_GRID_FX32_SIZE)
#define SW3_Y (SW3_GY*FIELD_CONST_GRID_FX32_SIZE)
#define SW1_Z (SW1_GZ*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define SW2_Z (SW2_GZ*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define SW3_Z (SW3_GZ*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)


static const VecFx32 WallPos[WALL_NUM_MAX] = {
  { WALL1_X, WALL1_Y, WALL1_Z },
  { WALL2_X, WALL2_Y, WALL2_Z },
  { WALL3_X, WALL3_Y, WALL3_Z },
};

static const VecFx32 SwPos[WALL_NUM_MAX] = {
  { SW1_X, SW1_Y, SW1_Z },
  { SW2_X, SW2_Y, SW2_Z },
  { SW3_X, SW3_Y, SW3_Z },
};

//リソースの並び順番
enum {
  RES_ID_WALL1_MDL = 0,
  RES_ID_WALL2_MDL,
  RES_ID_SW_MDL,
  RES_ID_WALL1_ANM_A,
  RES_ID_WALL1_ANM_B,
  RES_ID_WALL2_ANM_A,
  RES_ID_WALL2_ANM_B,
  RES_ID_SW_ANM_A,
  RES_ID_SW_ANM_B,
};

//ＯＢＪインデックス
enum {
  OBJ_WALL_1 = 0,
  OBJ_WALL_2,
  OBJ_WALL_3,
  OBJ_SW_1,
  OBJ_SW_2,
  OBJ_SW_3,
};

typedef enum{
  OBJ_KIND_SW,
  OBJ_KIND_WALL,
}OBJ_KIND;

//ジム内部中の一時ワーク
typedef struct GYM_ICE_TMP_tag
{
  int TargetIdx;
}GYM_ICE_TMP;

//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_ICE, NARC_gym_ice_ice_karakuri_01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_ICE, NARC_gym_ice_ice_karakuri_02_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD
  { ARCID_GYM_ICE, NARC_gym_ice_swich_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD

  { ARCID_GYM_ICE, NARC_gym_ice_ice_karakuri_01_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_ICE, NARC_gym_ice_ice_karakuri_01_2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_ICE, NARC_gym_ice_ice_karakuri_02_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_ICE, NARC_gym_ice_ice_karakuri_02_2_nsbca, GFL_G3D_UTIL_RESARC }, //ICA
  { ARCID_GYM_ICE, NARC_gym_ice_swich_rb_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_ICE, NARC_gym_ice_swich_br_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
};

//3Dアニメ　リフト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_wall1[] = {
	{ RES_ID_WALL1_ANM_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL1_ANM_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_wall2[] = {
	{ RES_ID_WALL2_ANM_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_WALL2_ANM_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw[] = {
	{ RES_ID_SW_ANM_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW_ANM_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};


//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //壁1
	{
		RES_ID_WALL2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall2),	//アニメリソース数
	},
  //壁2
	{
		RES_ID_WALL1_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL1_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall1),	//アニメリソース数
	},
  //壁3
	{
		RES_ID_WALL2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_wall2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_wall2),	//アニメリソース数
	},
  //スイッチ1
	{
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ2
	{
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
  //スイッチ3
	{
		RES_ID_SW_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

typedef struct RAIL_CHECK_tag
{
  u16 SwIdx;      //0～2
  u16 SwState;    //0or1  0：デフォルト　1：変更後
}RAIL_CHECK;

//0：スイッチがデフォルトのとき　1：スイッチを押したとき レールが有効になる
const RAIL_CHECK RailCheck[RAIL_CHECK_MAX] = {
  {0,0},    //チェックインデックス0
  {0,1},
  {1,0},
  {1,1},
  {2,0},
  {2,0},
  {2,1},
  {2,1},    //チェックインデックス7
};

static BOOL CheckChgRail(GYM_ICE_SV_WORK *gmk_sv_work, const int inCheckIdx);
static void SetupWallSwAnm(FLD_EXP_OBJ_CNT_PTR ptr, const BOOL inMoved, const int inTargetIdx, const OBJ_KIND inKind);
static int GetWatchAnmIdx(GYM_ICE_SV_WORK *gmk_sv_work, const int inTargetIdx);

static GMEVENT_RESULT SwitchEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WallEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_ICE_SV_WORK *gmk_sv_work;
  GYM_ICE_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }

  //汎用ワーク確保
  tmp = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ICE_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ICE_TMP));

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_ICE_UNIT_IDX );

  //座標セット
  for (i=0;i<WALL_NUM_MAX;i++)
  {
    int j;
    int idx;
    GFL_G3D_OBJSTATUS *status;
    //壁
    idx = OBJ_WALL_1 + i;
    status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ICE_UNIT_IDX, idx);
    status->trans = WallPos[i];
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_ICE_UNIT_IDX, idx, TRUE);
    for (j=0;j<WALL_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }

    //スイッチ
    idx = OBJ_SW_1 + i;
    status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ICE_UNIT_IDX, idx);
    status->trans = SwPos[i];
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_ICE_UNIT_IDX, idx, TRUE);
    for (j=0;j<WALL_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }
    
    SetupWallSwAnm(ptr, gmk_sv_work->WallMoved[i], i, OBJ_KIND_WALL);
    SetupWallSwAnm(ptr, gmk_sv_work->WallMoved[i], i, OBJ_KIND_SW);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_ICE_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_ICE_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }
#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      GMEVENT *event;
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      event = GYM_ICE_CreateSwEvt(gsys, 0);
      GAMESYSTEM_SetEvent(gsys, event);
    }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
      GMEVENT *event;
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      event = GYM_ICE_CreateWallEvt(gsys, 0);
      GAMESYSTEM_SetEvent(gsys, event);
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_START){
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GYM_ICE_ChangeRailMap(gsys, 0);
    }
  }
#endif
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * スイッチイベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inTargetIdx   ギミック対象インデックス
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_ICE_CreateSwEvt(GAMESYS_WORK *gsys, const int inTargetIdx)
{
  GMEVENT * event;
  GYM_ICE_TMP *tmp;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
  }

  if ( inTargetIdx >= WALL_NUM_MAX ){
    GF_ASSERT_MSG(0, "TARGET_INDEX_OVER %d",inTargetIdx);
    return NULL;
  }

  tmp->TargetIdx = inTargetIdx;

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, SwitchEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 壁イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inTargetIdx   ギミック対象インデックス
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_ICE_CreateWallEvt(GAMESYS_WORK *gsys, const int inTargetIdx)
{
  GMEVENT * event;
  GYM_ICE_TMP *tmp;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
  }

  if ( inTargetIdx >= WALL_NUM_MAX ){
    GF_ASSERT_MSG(0, "TARGET_INDEX_OVER %d",inTargetIdx);
    return NULL;
  }

  tmp->TargetIdx = inTargetIdx;

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, WallEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * レールマップへの切り替え
 * @param	      gsys        ゲームシステムポインタ
 * @param       inCheckIdx  レールマップ切り替え判定インデックス（出発位置インデックス）0～7
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_ChangeRailMap(GAMESYS_WORK *gsys, const int inCheckIdx)
{
  GYM_ICE_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLDMAP_CTRLTYPE type = FIELDMAP_GetMapControlType( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }

  if (inCheckIdx >= RAIL_CHECK_MAX)
  {
    GF_ASSERT(0);
    return;
  }

  if ( type == FLDMAP_CTRLTYPE_HYBRID )
  {
    //レールに切り替えるかどうかを現在のスイッチ状況で判断する
    if ( CheckChgRail(gmk_sv_work, inCheckIdx) )
    {
      FIELDMAP_CTRL_HYBRID *ctrl_work;
      ctrl_work = FIELDMAP_GetMapCtrlWork( fieldWork );
      FIELDMAP_CTRL_HYBRID_ChangeBaseSystem( ctrl_work, fieldWork );
    }
  }
  else
  {
    GF_ASSERT(0);
  }
}

//--------------------------------------------------------------
/**
 * レールマップ切り替えチェック
 *   
 * @param   gmk_sv_work ギミックセーブワークポインタ
 * @param   inCheckIdx    チェック座標インデックス  0～7
 * @return  BOOL      TRUEでレールマップに変更する
 */
//--------------------------------------------------------------
static BOOL CheckChgRail(GYM_ICE_SV_WORK *gmk_sv_work, const int inCheckIdx)
{
  u16 state;
  u16 target_sw = RailCheck[inCheckIdx].SwIdx;
  BOOL moved = gmk_sv_work->WallMoved[target_sw];
  if (moved) state = 1;
  else state = 0;

  if ( RailCheck[inCheckIdx].SwState == state ) return TRUE;

  return FALSE;
}

//--------------------------------------------------------------
/**
 * 壁、スイッチアニメ準備
 *   
 * @param   ptr       拡張ＯＢＪコントロールポインタ
 * @param   inMoved   基準位置から動いているか？
 * @param	  inTargetIdx 対象インデックス
 * @param   inKind      ＯＢＪ種類
 * @return  none
 */
//--------------------------------------------------------------
static void SetupWallSwAnm(FLD_EXP_OBJ_CNT_PTR ptr, const BOOL inMoved, const int inTargetIdx, const OBJ_KIND inKind)
{
  EXP_OBJ_ANM_CNT_PTR anm;
  int obj_idx;
  int valid_anm_idx, invalid_anm_idx;

  if ( inMoved )
  {
    valid_anm_idx = 1;
    invalid_anm_idx = 0;
  }
  else {
    valid_anm_idx = 0;
    invalid_anm_idx = 1;
  }

  if ( inKind == OBJ_KIND_WALL ) obj_idx = OBJ_WALL_1+inTargetIdx;
  else  obj_idx = OBJ_SW_1+inTargetIdx;

  
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ICE_UNIT_IDX, obj_idx, valid_anm_idx, TRUE);
  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ICE_UNIT_IDX, obj_idx, invalid_anm_idx, FALSE);
  //頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ICE_UNIT_IDX, obj_idx, valid_anm_idx, 0 );
  //停止にしておく
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, obj_idx, valid_anm_idx);
  FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);

}

//--------------------------------------------------------------
/**
 * 終了監視するアニメインデックスを取得
 * @param   gmk_sv_work   ギミックセーブワーク
 * @param	  inTargetIdx ギミック対象インデックス 0～2
 * @return  int     終了監視するアニメインデックス　0～1
 */
//--------------------------------------------------------------
static int GetWatchAnmIdx(GYM_ICE_SV_WORK *gmk_sv_work, const int inTargetIdx)
{
  int valid_anm_idx;
  if ( gmk_sv_work->WallMoved[inTargetIdx] ) valid_anm_idx = 1;
  else valid_anm_idx = 0;

  return valid_anm_idx;
}


//--------------------------------------------------------------
/**
 * スイッチイベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SwitchEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_ICE_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }

  switch(*seq){
  case 0:  //スイッチアニメ開始
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 sw_obj_idx;
      u8 anm_idx;
      sw_obj_idx = OBJ_SW_1 + tmp->TargetIdx;
      anm_idx = GetWatchAnmIdx(gmk_sv_work, tmp->TargetIdx);
      //アニメを開始
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, sw_obj_idx, anm_idx );
      //アニメ停止解除
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
    }
    (*seq)++;
    break;
  case 1:
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 obj_idx;
      u8 anm_idx;
      obj_idx = OBJ_SW_1 + tmp->TargetIdx;
      anm_idx = GetWatchAnmIdx(gmk_sv_work, tmp->TargetIdx);
      //↓アニメ終了判定
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, obj_idx, anm_idx);
      //スイッチアニメ終了待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        //次回に備え、アニメ切り替え
        SetupWallSwAnm(ptr, !gmk_sv_work->WallMoved[tmp->TargetIdx], tmp->TargetIdx, OBJ_KIND_SW);
        return GMEVENT_RES_FINISH; //イベント終了
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 壁回転イベント
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WallEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_ICE_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }

  switch(*seq){
  case 0:  //アニメ開始
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 obj_idx;
      u8 anm_idx;
      obj_idx = OBJ_WALL_1 + tmp->TargetIdx;
      anm_idx = GetWatchAnmIdx(gmk_sv_work, tmp->TargetIdx);
      //アニメを開始
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, obj_idx, anm_idx );
      //アニメ停止解除
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      //SE再生
      PMSND_PlaySE( GYM_ICE_SE_GMK_MOVE );

    }
    (*seq)++;
    break;
  case 1:
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 obj_idx;
      u8 anm_idx;

      obj_idx = OBJ_WALL_1 + tmp->TargetIdx;
      anm_idx = GetWatchAnmIdx(gmk_sv_work, tmp->TargetIdx);
      //↓アニメ終了判定
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, obj_idx, anm_idx);
      //アニメ終了待ち
      if ( FLD_EXP_OBJ_ChkAnmEnd(anm) )
      {
        //SEストップ
        PMSND_StopSE();
        //SE再生
        PMSND_PlaySE( GYM_ICE_SE_GMK_STOP);
        //次回に備え、アニメ切り替え
        SetupWallSwAnm(ptr, !gmk_sv_work->WallMoved[tmp->TargetIdx], tmp->TargetIdx, OBJ_KIND_WALL);
        //セーブワーク書き換え
        gmk_sv_work->WallMoved[tmp->TargetIdx] = (!gmk_sv_work->WallMoved[tmp->TargetIdx]);
        return GMEVENT_RES_FINISH; //イベント終了
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

