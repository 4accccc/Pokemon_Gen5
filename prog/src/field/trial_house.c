//======================================================================
/**
 * @file  trial_house.c
 * @brief トライアルハウス
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"   //for GFUser_GetPublicRand
#include "trial_house.h"
#include "trial_house_def.h"
#include "system/main.h"  //for HEAPID_APP_CONTROL
#include "fld_btl_inst_tool.h"

//#include "../../../resource/fldmapdata/script/c03r0801_def.h"  //for SCRID_～
//#include "script.h"     //for SCRIPT_SetEventScript

//#include "pl_boat_def.h"

//#include "pl_boat_setup.h"


//--------------------------------------------------------------
/**
 * @brief	//トライアルハウスワーク確保関数
 * @retval	ptr      TRIAL_HOUSE_WORK_PTR
*/
//--------------------------------------------------------------
TRIAL_HOUSE_WORK_PTR TRIAL_HOUSE_Start(void)
{
  TRIAL_HOUSE_WORK_PTR ptr;
  int size = sizeof(TRIAL_HOUSE_WORK);
  NOZOMU_Printf("TrialHouseWorkSize = 0x%x\n",size);
  ptr = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, size);
  ptr->HeapID = HEAPID_APP_CONTROL;//HEAPID_PROC;

  return ptr;
}

//--------------------------------------------------------------
/**
 * @brief	トライアルハウスワーク解放関数
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_End( TRIAL_HOUSE_WORK_PTR *ptr )
{
  if ( *ptr != NULL )
  {
    GFL_HEAP_FreeMemory( *ptr );
    *ptr = NULL;
  }
  else GF_ASSERT(0);
}

//--------------------------------------------------------------
/**
 * @brief	トライアルハウス プレイモードのセット
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inPlayMode     
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_SetPlayMode( TRIAL_HOUSE_WORK_PTR ptr, const BSWAY_PLAYMODE inPlayMode )
{
  if ( ptr == NULL )
  {
    GF_ASSERT(0);
    return;
  }

  if ( (inPlayMode == BSWAY_PLAYMODE_SINGLE)||(inPlayMode == BSWAY_PLAYMODE_DOUBLE) )
  {
    //シングル3　それ以外（ダブル）4
    if ( inPlayMode == BSWAY_PLAYMODE_SINGLE ) ptr->MemberNum = 3;
    else ptr->MemberNum = 4;

    ptr->PlayMode = inPlayMode;
  }
  else
  {
    GF_ASSERT_MSG(0,"mode error %d",inPlayMode);
    ptr->PlayMode = BSWAY_PLAYMODE_SINGLE;
    ptr->MemberNum = 3;
  }
}


//--------------------------------------------------------------
/**
 * @brief	対戦相手の抽選
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval		none
*/
//--------------------------------------------------------------
int TRIAL_HOUSE_MakeTrainer( TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount )
{
  int obj_id;
  int tr_no;
  int base,band;

  switch(inBtlCount){
  case 0:
    base = 0;
    band = 100;
    break;
  case 1:
    base = 140;
    band = 20;
    break;
  case 2:
    base = 180;
    band = 20;
    break;
  case 3:
    base = 220;
    band = 80;
    break;
  case 4:
    base = 220;
    band = 80;
    break;
  default:
    GF_ASSERT_MSG(0,"count error %d",inBtlCount);
    base = 0;
    band = 100;
  }
  //トレーナー抽選
  tr_no = base + GFUser_GetPublicRand(band);

  //データをトライアルハウスワークにセット
  {
    BOOL ret;
    ret = FBI_TOOL_MakeRomTrainerData(
        &ptr->TrData,
        tr_no, ptr->MemberNum,
        NULL,NULL,NULL, GFL_HEAP_LOWID(ptr->HeapID));

    if (!ret)
    {
      GF_ASSERT(0);
    }
  }

  //見た目を取得して返す
  obj_id = TRIAL_HOUSE_GetTrainerOBJCode( ptr );
  
  return obj_id;
}

//--------------------------------------------------------------
/**
 * トレーナータイプからOBJコードを取得してくる
 * @param ptr   トライアルハウスポインタ
 * @param idx トレーナーデータインデックス
 * @retval u16 OBJコード
 */
//--------------------------------------------------------------
u16 TRIAL_HOUSE_GetTrainerOBJCode( TRIAL_HOUSE_WORK_PTR ptr )
{
  return FBI_TOOL_GetTrainerOBJCode( ptr->TrData.bt_trd.tr_type );
}


//--------------------------------------------------------------
/**
 * @brief	対戦相手の先頭前メッセージ表示
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_DispBtlBeforeMsg( TRIAL_HOUSE_WORK_PTR ptr )
{
  ;
}
