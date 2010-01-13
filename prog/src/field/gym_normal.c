//======================================================================
/**
 * @file  gym_normal.c
 * @brief  ノーマルジム
 * @author  Saito
 */
//======================================================================

#include "fieldmap.h"
#include "gym_normal_sv.h"
#include "gym_normal.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_normal.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript

#include "../../../resource/fldmapdata/gimmick/gym_normal/gym_normal_local_def.h"

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define GYM_NORMAL_UNIT_IDX (0)
#define GYM_NORMAL_TMP_ASSIGN_ID  (1)

//本棚座標
#define WALL_X  (WALL_GX*FIELD_CONST_GRID_FX32_SIZE)
#define WALL_Z  (WALL_GZ*FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)

//OBJのＹ座標
#define OBJ3D_Y  (0*FIELD_CONST_GRID_FX32_SIZE)

#define WALL_NUM  (1)


typedef struct POS_XZ_tag
{
  fx32 x;
  fx32 z;
}POS_XZ;

//ジム内部中の一時ワーク
typedef struct GYM_NORMAL_TMP_tag
{
  u16 WallIdx;
  u16 dummy;
}GYM_NORMAL_TMP;

//ＯＢＪインデックス
enum {
  OBJ_WALL_1 = 0,
};

//アニメの順番
enum{
  ANM_WALL_MOV = 0,
};

//リソースの並び順番
enum {
  RES_ID_WALL_MDL = 0,
  RES_ID_WALL_MOVE,
};

static const POS_XZ WallPos[WALL_NUM] = {
  {WALL_X, WALL_Z},
};

//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
  { ARCID_GYM_NORMAL, NARC_gym_normal_book_anim01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD　本棚
  { ARCID_GYM_NORMAL, NARC_gym_normal_book_anim01_nsbca, GFL_G3D_UTIL_RESARC }, //IMD　本棚アニメ
};

//3Dアニメ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl[] = {
	{ RES_ID_WALL_MOVE,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[WALL_NUM] = {
  //本棚1
	{
		RES_ID_WALL_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_WALL_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

static GMEVENT_RESULT WallAnmEvt( GMEVENT* event, int* seq, void* work );

#ifdef PM_DEBUG
BOOL test_GYM_NORMAL_WallAnm(GAMESYS_WORK *gsys, const u8 inWallIdx);
#endif  //PM_DEBUG
//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_NORMAL_Setup(FIELDMAP_WORK *fieldWork)
{
  u8 wall_num;
  const POS_XZ *pos;
  int gmk_id;
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  GYM_NORMAL_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_NORMAL_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_NORMAL_TMP));

  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);

  {
    //必要なリソースの用意
    FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_NORMAL_UNIT_IDX );
    wall_num = WALL_NUM;
    pos = WallPos;
    gmk_id = FLD_GIMMICK_GYM_NORM;
  }

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, gmk_id );
  }

  //座標セット
  {
    u8 i;
    for (i=0;i<wall_num;i++){
      VecFx32 pos_vec;
      GFL_G3D_OBJSTATUS *status;
      pos_vec.x = pos[i].x;
      pos_vec.y = OBJ3D_Y;
      pos_vec.z = pos[i].z;
      status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_NORMAL_UNIT_IDX, OBJ_WALL_1+i);
      status->trans = pos_vec;
    }
  }

  //本棚初期アニメ決定
  {
    u8 i;
    for (i=0;i<wall_num;i++){
      u8 obj_idx;
      EXP_OBJ_ANM_CNT_PTR anm;

      obj_idx = OBJ_WALL_1+i;
      //1ループアニメ設定
      {
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_NORMAL_UNIT_IDX, obj_idx, ANM_WALL_MOV);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
        //1回再生設定
        FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      }
      //セーブデータを見てアニメ終了前かどうか判定
      if (gmk_sv_work->Wall[i]){  //終了
        fx32 last_frm;
        //最終フレームでストップ
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_NORMAL_UNIT_IDX, obj_idx, ANM_WALL_MOV, TRUE);
        //最終フレームセット
        last_frm = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
        FLD_EXP_OBJ_SetObjAnmFrm(ptr,GYM_NORMAL_UNIT_IDX, obj_idx, 0, last_frm);
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_NORMAL_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_NORMAL_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	        fieldWork     フィールドワークポインタ
 * @return        none
 */
//--------------------------------------------------------------
void GYM_NORMAL_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
#ifdef PM_DEBUG
  //テスト
  {
    GAMESYS_WORK *gsys  = FIELDMAP_GetGameSysWork( fieldWork );
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
      test_GYM_NORMAL_WallAnm(gsys, 0);
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
    }
  }
#endif  //PM_DEBUG
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

}

//--------------------------------------------------------------
/**
 * スイッチ起動
 * @param	        fieldWork     フィールドワークポインタ
 * @return        none
 */
//--------------------------------------------------------------
void GYM_NORMAL_Unrock(FIELDMAP_WORK *fieldWork)
{
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  GYM_NORMAL_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_NORM );

  gmk_sv_work->GmkUnrock = TRUE;
}

//--------------------------------------------------------------
/**
 * スイッチチェック
 * @param	        fieldWork     フィールドワークポインタ
 * @return        BOOL          TRUEで開錠
 */
//--------------------------------------------------------------
BOOL GYM_NORMAL_CheckRock(FIELDMAP_WORK *fieldWork)
{
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  GYM_NORMAL_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_NORM );

  return gmk_sv_work->GmkUnrock;
}

//--------------------------------------------------------------
/**
 * 棚アニメ起動
 * @param	        gsys       ゲームシステムポインタ
 * @param         inWallIdx  現在は0固定
 * @return        event       イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT * GYM_NORMAL_MoveWall(GAMESYS_WORK *gsys, const u8 inWallIdx)
{
  GMEVENT * event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_NORMAL_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);

  if (inWallIdx >= NRM_WALL_NUM_MAX){
    return NULL;
  }

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, WallAnmEvt, 0 );
    
  //操作予定レバーインデックスをセット
  tmp->WallIdx = inWallIdx;

  return event;

}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * レバーの切り替えを行うイベント起動
 * @param	
 * @return
 */
//--------------------------------------------------------------
BOOL test_GYM_NORMAL_WallAnm(GAMESYS_WORK *gsys, const u8 inWallIdx)
{
  int gmk_id;
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_NORMAL_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);

  gmk_id = FLD_GIMMICK_GYM_NORM;

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, gmk_id );
  tmp->WallIdx = inWallIdx;

  {
    //イベントセット
    GMEVENT * event = GMEVENT_Create( gsys, NULL, WallAnmEvt, 0/*sizeof(ENCEFF_WORK)*/ );
    GAMESYSTEM_SetEvent(gsys, event);
  }
  return TRUE;
  
}
#endif //PM_DEBUG
//--------------------------------------------------------------
/**
 * 本棚アニメイベント
 * @param	    event   イベントポインタ
 * @param     seq     シーケンサ
 * @param     work    ワークポインタ
 *
 * @return    GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT WallAnmEvt( GMEVENT* event, int* seq, void* work )
{
  int gmk_id;
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_NORMAL_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);

  gmk_id = FLD_GIMMICK_GYM_NORM;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, gmk_id );
  }
  
  switch(*seq){
  case 0:  //アニメ開始
    {      
      EXP_OBJ_ANM_CNT_PTR anm;
      u8 sw_obj_idx;
      sw_obj_idx = OBJ_WALL_1 + tmp->WallIdx;
      //アニメを開始
      FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_NORMAL_UNIT_IDX, sw_obj_idx, ANM_WALL_MOV, TRUE);
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_NORMAL_UNIT_IDX, sw_obj_idx, ANM_WALL_MOV);
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      //頭出し
      FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_NORMAL_UNIT_IDX, sw_obj_idx, ANM_WALL_MOV, 0 );
    }    
    (*seq)++;
    break;
  case 1: //アニメ待ち
    {
      u8 sw_obj_idx;
      EXP_OBJ_ANM_CNT_PTR sw_anm;
      sw_obj_idx = OBJ_WALL_1 + tmp->WallIdx;
      sw_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_NORMAL_UNIT_IDX, sw_obj_idx, ANM_WALL_MOV);
      if ( FLD_EXP_OBJ_ChkAnmEnd(sw_anm)&&FLD_EXP_OBJ_ChkAnmEnd(sw_anm) ){
        OS_Printf("アニメ終了\n");
        //ギミック起動終了フラグＯＮ
        gmk_sv_work->Wall[tmp->WallIdx] = 1;
        return GMEVENT_RES_FINISH;
      }
    }
  }
  return GMEVENT_RES_CONTINUE;
}



