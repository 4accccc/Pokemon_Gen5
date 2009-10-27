//======================================================================
/**
 * @file  gym_anti.c
 * @brief  アンチジム
 * @author  Saito
 */
//======================================================================
#include "fieldmap.h"
#include "gym_anti_sv.h"
#include "gym_anti.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_anti.naix"

#include "../../../resource/fldmapdata/gimmick/gym_anti/gym_anti_local_def.cdat"


#define GYM_ANTI_UNIT_IDX (0)
#define GYM_ANTI_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define SW1_X  (SW1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW1_Z  (SW1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW2_X  (SW2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW2_Z  (SW2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW3_X  (SW3_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW3_Z  (SW3_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW4_X  (SW4_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW4_Z  (SW4_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW5_X  (SW5_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW5_Z  (SW5_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW6_X  (SW6_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW6_Z  (SW6_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW7_X  (SW7_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW7_Z  (SW7_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW8_X  (SW8_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW8_Z  (SW8_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW9_X  (SW9_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define SW9_Z  (SW9_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

#define CTN1_X  (CTN1_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CTN1_Z  (CTN1_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CTN2_X  (CTN2_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CTN2_Z  (CTN2_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CTN3_X  (CTN3_GX*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)
#define CTN3_Z  (CTN3_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

//OBJのＹ座標
#define OBJ3D_Y  (0*FIELD_CONST_GRID_FX32_SIZE)



//ジム内部中の一時ワーク
typedef struct GYM_ANTI_TMP_tag
{
  u8 SwIdx;
  u8 DoorIdx;
  u8 IsOn;
  u8 dummy;
}GYM_ANTI_TMP;

//リソースの並び順番
enum {
  RES_ID_SW1_MDL = 0,
  RES_ID_SW2_MDL,
  RES_ID_SW3_MDL,

  RES_ID_CTN1_MDL,
  RES_ID_CTN2_MDL,
  RES_ID_CTN3_MDL,

  RES_ID_SW1_MOV1,
  RES_ID_SW1_MOV2,
  RES_ID_SW2_MOV1,
  RES_ID_SW2_MOV2,
  RES_ID_SW3_MOV1,
  RES_ID_SW3_MOV2,

  RES_ID_CTN1_MOV,
  RES_ID_CTN2_MOV,
  RES_ID_CTN3_MOV,

};


//ＯＢＪインデックス
enum {
  OBJ_SW_1 = 0,
  OBJ_SW_2,
  OBJ_SW_3,
  OBJ_SW_4,
  OBJ_SW_5,
  OBJ_SW_6,
  OBJ_SW_7,
  OBJ_SW_8,
  OBJ_SW_9,
  OBJ_DOOR_1,
  OBJ_DOOR_2,
  OBJ_DOOR_3,
};

static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_f_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　スイッチ火
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_w_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　スイッチ水
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_l_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　スイッチ草

  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_c_f_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　カーテン火
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_c_w_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　カーテン水
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_c_l_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　カーテン草

  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_f_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　スイッチ火
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_f_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ火
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_w_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　スイッチ水
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_w_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ水
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_l_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　スイッチ草
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_b_l_nsbta, GFL_G3D_UTIL_RESARC }, //ITA　スイッチ草

  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_c_f_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カーテン火
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_c_w_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カーテン水
  { ARCID_GYM_ANTI, NARC_gym_anti_gym01_c_l_nsbca, GFL_G3D_UTIL_RESARC }, //ICA　カーテン草
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw1[] = {
  { RES_ID_SW1_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW1_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw2[] = {
  { RES_ID_SW2_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW2_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　スイッチ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_sw3[] = {
  { RES_ID_SW3_MOV1,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_SW3_MOV2,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　カーテン
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_ctn1[] = {
  { RES_ID_CTN1_MOV,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　カーテン
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_ctn2[] = {
  { RES_ID_CTN2_MOV,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dアニメ　カーテン
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_ctn3[] = {
  { RES_ID_CTN3_MOV,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //スイッチ1（当たり　火）
  {
		RES_ID_SW1_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW1_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw1),	//アニメリソース数
	},
  //スイッチ2（当たり　水）
  {
		RES_ID_SW2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw2),	//アニメリソース数
	},
  //スイッチ3（当たり　草）
  {
		RES_ID_SW3_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW3_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw3,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw3),	//アニメリソース数
	},

  //スイッチ4（ハズレ　火）
  {
		RES_ID_SW1_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW1_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw1),	//アニメリソース数
	},
  //スイッチ5（ハズレ　水）
  {
		RES_ID_SW2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw2),	//アニメリソース数
	},
  //スイッチ6（ハズレ　草）
  {
		RES_ID_SW3_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW3_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw3,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw3),	//アニメリソース数
	},

  //スイッチ7（ハズレ　火）
  {
		RES_ID_SW1_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW1_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw1),	//アニメリソース数
	},
  //スイッチ8（ハズレ　水）
  {
		RES_ID_SW2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw2),	//アニメリソース数
	},
  //スイッチ9（ハズレ　草）
  {
		RES_ID_SW3_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_SW3_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_sw3,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_sw3),	//アニメリソース数
	},
  //カーテン1（火）
	{
		RES_ID_CTN1_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CTN1_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ctn1,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ctn1),	//アニメリソース数
	},
  //カーテン2（水）
  {
		RES_ID_CTN2_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CTN2_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ctn2,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ctn2),	//アニメリソース数
	},
  //カーテン3（草）
  {
		RES_ID_CTN3_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_CTN3_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_ctn3,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_ctn3),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};




static GMEVENT_RESULT OpenDoorEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PushSwEvt( GMEVENT* event, int* seq, void* work );

#ifdef PM_DEBUG
static BOOL test_GYM_ANTI_SwOn(GAMESYS_WORK *gsys, const u8 inSwIdx);
static BOOL test_GYM_ANTI_OpenDoor(GAMESYS_WORK *gsys, const u8 inDoorIdx);
#endif

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ANTI_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_ANTI_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ANTI_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ANTI_TMP));
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_ANTI_UNIT_IDX );
  //スイッチ座標セット
  {
    u8 i;
    const VecFx32 pos[ANTI_SW_NUM_MAX] = {
      {SW1_X, OBJ3D_Y, SW1_Z},
      {SW2_X, OBJ3D_Y, SW2_Z},
      {SW3_X, OBJ3D_Y, SW3_Z},
      {SW4_X, OBJ3D_Y, SW4_Z},
      {SW5_X, OBJ3D_Y, SW5_Z},
      {SW6_X, OBJ3D_Y, SW6_Z},
      {SW7_X, OBJ3D_Y, SW7_Z},
      {SW8_X, OBJ3D_Y, SW8_Z},
      {SW9_X, OBJ3D_Y, SW9_Z},
    };

    for (i=0;i<ANTI_SW_NUM_MAX;i++){
      u8 obj_idx = OBJ_SW_1+i;
      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ANTI_UNIT_IDX, obj_idx);
      status->trans = pos[i];
    }
  }

  //カーテン座標セット
  {
    u8 i;
    const VecFx32 pos[ANTI_SW_NUM_MAX] = {
      {CTN1_X, OBJ3D_Y, CTN1_Z},
      {CTN2_X, OBJ3D_Y, CTN2_Z},
      {CTN3_X, OBJ3D_Y, CTN3_Z},
    };
    for (i=0;i<ANTI_DOOR_NUM_MAX;i++){
      u8 obj_idx = OBJ_DOOR_1+i;
      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ANTI_UNIT_IDX, obj_idx);
      status->trans = pos[i];
    }
  }

  //セーブ状況による初期アニメをセット
  {
    //ドア
    u8 i;
    for (i=0;i<ANTI_DOOR_NUM_MAX;i++){
      if ( gmk_sv_work->Door[i] ){
        //アニメセット
        EXP_OBJ_ANM_CNT_PTR anm;
        u8 door_obj_idx;
        door_obj_idx = OBJ_DOOR_1 + i;
        {
          fx32 last_frm;
          //アニメを有効
          FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ANTI_UNIT_IDX, door_obj_idx, 0, TRUE);
          anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, door_obj_idx, 0);
          FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
          //1回再生設定
          FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
          //最終フレームにする
          last_frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
          FLD_EXP_OBJ_SetObjAnmFrm(ptr,GYM_ANTI_UNIT_IDX, door_obj_idx, 0, last_frm);
        }
      }
    }
    //スイッチ
    if (gmk_sv_work->PushSwIdx != ANTI_SW_NUM_MAX){
      //アニメセット
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 sw_obj_idx;
      sw_obj_idx = OBJ_SW_1 + gmk_sv_work->PushSwIdx;
      {
        fx32 last_frm;
        //アニメを有効
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0);
///        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //1回再生設定
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
        //最終フレームにする
        last_frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
        FLD_EXP_OBJ_SetObjAnmFrm(ptr,GYM_ANTI_UNIT_IDX, sw_obj_idx, 0, last_frm);
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ANTI_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_ANTI_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ANTI_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

}

//--------------------------------------------------------------
/**
 * スイッチアニメイベント起動
 * @param	  gsys      ゲームシステムポインタ
 * @param   inSwIdx   スイッチインデックス0～8
 * @param   inSwState スイッチ状態 0:オフ　1:オン
 * @return  event     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_ANTI_SwOnOff(GAMESYS_WORK *gsys, const u8 inSwIdx, const u8 inSwState)
{
  GMEVENT * event;
  GYM_ANTI_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );

  if (inSwIdx >= ANTI_SW_NUM_MAX){
    return NULL;
  }

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, PushSwEvt, 0 );
  if (event != NULL){
    //スイッチインデックスをセット
    tmp->SwIdx = inSwIdx;
    //スイッチ状態セット
    tmp->IsOn = inSwState;
  }

  return event;
}

//--------------------------------------------------------------
/**
 * ドアオープンアニメイベント起動
 * @param	
 * @return
 */
//--------------------------------------------------------------
GMEVENT *GYM_ANTI_OpenDoor(GAMESYS_WORK *gsys, const u8 inDoorIdx)
{
  GMEVENT * event;
  GYM_ANTI_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );

  if (inDoorIdx > ANTI_DOOR_NUM_MAX){
    return NULL;
  }

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, OpenDoorEvt, 0 );
    
  //ドアインデックスをセット
  tmp->DoorIdx = inDoorIdx;

  return event;

}

//--------------------------------------------------------------
/**
 * スイッチイベント
 * @param	
 * @return
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PushSwEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_ANTI_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );
  }
  
  switch(*seq){
  case 0:  //アニメ切り替え
    {      
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 sw_obj_idx;
      sw_obj_idx = OBJ_SW_1 + tmp->SwIdx;

      {
        //アニメを開始
        fx32 frm;
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0);
///        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        if (tmp->IsOn){
          FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
          frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
          //押したスイッチインデックスを記憶
          gmk_sv_work->PushSwIdx = tmp->SwIdx;
        }else{
          FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
          frm = 0;
          //押したスイッチインデックスをクリア
          gmk_sv_work->PushSwIdx = ANTI_SW_NUM_MAX;
        }
        //フレームセット
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0, frm );
        //1回再生設定
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      }
    }    
    (*seq)++;
    break;
  case 1: //アニメ待ち 
    {
      //フレーム切り替えによるアニメなので、アニメ待ちしない
      OS_Printf("アニメ終了\n");
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ドアイベント
 * @param	
 * @return
 */
//--------------------------------------------------------------
static GMEVENT_RESULT OpenDoorEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_ANTI_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );
  }
  
  switch(*seq){
  case 0:  //アニメ切り替え
    {      
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 door_obj_idx;
      door_obj_idx = OBJ_DOOR_1 + tmp->DoorIdx;
      {
        //アニメを開始
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ANTI_UNIT_IDX, door_obj_idx, 0, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, door_obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //頭出し
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ANTI_UNIT_IDX, door_obj_idx, 0, 0 );
        //1回再生設定
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      }
    }    
    (*seq)++;
    break;
  case 1: //アニメ待ち 
    {
      
      u8 door_obj_idx;
      EXP_OBJ_ANM_CNT_PTR door_anm;
      door_obj_idx = OBJ_DOOR_1 + tmp->DoorIdx;
      door_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, door_obj_idx, 0);
      if ( FLD_EXP_OBJ_ChkAnmEnd(door_anm) ){
        OS_Printf("アニメ終了\n");
        //スイッチ押下フラグオン
        gmk_sv_work->Door[tmp->DoorIdx] = 1;
        return GMEVENT_RES_FINISH;
      }
    }
  }
  return GMEVENT_RES_CONTINUE;
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * レバーの切り替えを行うイベント起動
 * @param	
 * @return
 */
//--------------------------------------------------------------
static BOOL test_GYM_ANTI_SwOn(GAMESYS_WORK *gsys, const u8 inSwIdx)
{
  GYM_ANTI_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );

  {
    //イベントセット
    GMEVENT * event = GMEVENT_Create( gsys, NULL, PushSwEvt, 0 );
    GAMESYSTEM_SetEvent(gsys, event);
    //操作予定インデックスをセット
    tmp->SwIdx = inSwIdx;
  }
  return TRUE;
  
}

static BOOL test_GYM_ANTI_OpenDoor(GAMESYS_WORK *gsys, const u8 inDoorIdx)
{
  GYM_ANTI_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );

  {
    //イベントセット
    GMEVENT * event = GMEVENT_Create( gsys, NULL, OpenDoorEvt, 0 );
    GAMESYSTEM_SetEvent(gsys, event);
    //操作予定インデックスをセット
    tmp->DoorIdx = inDoorIdx;
  }
  return TRUE;
  
}

#endif  //PM_DEBUG

