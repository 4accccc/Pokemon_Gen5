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

//#include "arc/fieldmap/gym_anti.naix"

#define GYM_ANTI_UNIT_IDX (0)
#define GYM_ANTI_TMP_ASSIGN_ID  (1)


//ジム内部中の一時ワーク
typedef struct GYM_ANTI_TMP_tag
{
  u8 SwIdx;
  u8 DoorIdx;
  u8 dummy[2];
}GYM_ANTI_TMP;

//ＯＢＪインデックス
enum {
  OBJ_SW_1 = 0,
  OBJ_SW_2,
  OBJ_SW_3,
  OBJ_DOOR_1,
  OBJ_DOOR_2,
  OBJ_DOOR_3,
};



static GMEVENT_RESULT OpenDoorEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT PushSwEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_ANTI_Setup(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ANTI_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ANTI_TMP));
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
///  FLD_EXP_OBJ_DelUnit( ptr, GYM_ANTI_UNIT_IDX );
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
 * @param	
 * @return
 */
//--------------------------------------------------------------
GMEVENT *GYM_ANTI_SwOn(GAMESYS_WORK *gsys, const u8 inSwIdx)
{
  GMEVENT * event;
  GYM_ANTI_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ANTI_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, PushSwEvt, 0 );
    
  //スイッチインデックスをセット
  tmp->SwIdx = inSwIdx;

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
        FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0, TRUE);
        anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
        //頭出し
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0, 0 );
      }
    }    
    (*seq)++;
    break;
  case 1: //アニメ待ち 
    {
      
      u8 sw_obj_idx;
      EXP_OBJ_ANM_CNT_PTR sw_anm;
      sw_obj_idx = OBJ_SW_1 + tmp->SwIdx;
      sw_anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ANTI_UNIT_IDX, sw_obj_idx, 0);
      if ( FLD_EXP_OBJ_ChkAnmEnd(sw_anm) ){
        OS_Printf("アニメ終了\n");
        //スイッチ押下フラグオン
        gmk_sv_work->Sw[tmp->SwIdx] = 1;
        return GMEVENT_RES_FINISH;
      }
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
        return GMEVENT_RES_FINISH;
      }
    }
  }
  return GMEVENT_RES_CONTINUE;
}


