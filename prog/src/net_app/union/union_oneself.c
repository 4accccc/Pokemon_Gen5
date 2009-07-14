//==============================================================================
/**
 * @file    union_oneself.c
 * @brief   ユニオン：自分動作
 * @author  matsuda
 * @date    2009.07.09(木)
 */
//==============================================================================
#include <gflib.h>
#include "union_types.h"
#include "union_local.h"
#include "net_app/union/union_main.h"
#include "union_oneself.h"
#include "field/field_player.h"
#include "field/fieldmap.h"
#include "net_app/union/union_msg.h"
#include "message.naix"
#include "msg/msg_union.h"


//==============================================================================
//  定数定義
//==============================================================================
///シーケンス番号
enum{
  ONESELF_SUBPROC_INIT,
  ONESELF_SUBPROC_UPDATE,
  ONESELF_SUBPROC_EXIT,
};

///話しかけてから反応があるまでのタイムアウト時間
#define ONESELF_SERVER_TIMEOUT      (30 * 10)


//==============================================================================
//  型定義
//==============================================================================
///動作型
typedef BOOL (*ONESELF_FUNC)(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);

///動作セット
typedef struct{
  ONESELF_FUNC func_init;
  ONESELF_FUNC func_update;
  ONESELF_FUNC func_exit;
}ONESELF_FUNC_DATA;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static BOOL OneselfSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);


//==============================================================================
//  データ
//==============================================================================
static const ONESELF_FUNC_DATA OneselfFuncTbl[] = {
  {//UNION_STATUS_NORMAL
    NULL,
    OneselfSeq_NormalUpdate,
    NULL,
  },
  {//UNION_STATUS_ENTER
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_LEAVE
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_CONNECT_REQ
    OneselfSeq_ConnectReqInit,
    OneselfSeq_ConnectReqUpdate,
    OneselfSeq_ConnectReqExit,
  },
  {//UNION_STATUS_CONNECT_ANSWER
    OneselfSeq_ConnectAnswerInit,
    OneselfSeq_ConnectAnswerUpdate,
    OneselfSeq_ConnectAnswerExit,
  },
  {//UNION_STATUS_TALK
    OneselfSeq_TalkInit,
    OneselfSeq_TalkUpdate,
    OneselfSeq_TalkExit,
  },
  {//UNION_STATUS_BATTLE
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_TRADE
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_RECORD
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_PICTURE
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_GURUGURU
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_CHAT
    NULL,
    NULL,
    NULL,
  },
};
SDK_COMPILER_ASSERT(NELEMS(OneselfFuncTbl) == UNION_STATUS_MAX);




int UnionOneself_Update(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  ONESELF_FUNC func;
  BOOL next_seq = TRUE;
  
  switch(situ->func_proc){
  case ONESELF_SUBPROC_INIT:
    func = OneselfFuncTbl[situ->union_status].func_init;
    break;
  case ONESELF_SUBPROC_UPDATE:
    func = OneselfFuncTbl[situ->union_status].func_update;
    break;
  case ONESELF_SUBPROC_EXIT:
    func = OneselfFuncTbl[situ->union_status].func_exit;
    break;
  }

  if(func != NULL){
    next_seq = func(unisys, situ, fieldWork, &situ->func_seq);
  }
  
  if(next_seq == TRUE){
    situ->func_seq = 0;
    situ->func_proc++;
    if(situ->func_proc > ONESELF_SUBPROC_EXIT){
      situ->func_proc = 0;
      situ->union_status = situ->next_union_status;
      situ->next_union_status = UNION_STATUS_NORMAL;
    }
  }
  
  return situ->union_status;
}

//==================================================================
/**
 * 次のUNION_STATUSをセットする
 *
 * @param   unisys		    
 * @param   req_status		UNION_STATUS_???
 *
 * @retval  BOOL		TRUE：成功
 */
//==================================================================
BOOL UnionOneself_ReqStatus(UNION_SYSTEM_PTR unisys, int req_status)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;

  situ->next_union_status = req_status;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 通常状態(何もしていない)
 *
 * @param   unisys		
 * @param   situ		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( fieldWork );
  u16 obj_id;
  s16 check_gx, check_gy, check_gz;
  MMDL *target_pc;
  
  //外部から動作リクエストを受けているなら切り替える
  if(situ->next_union_status != UNION_STATUS_NORMAL){
    OS_TPrintf("NormalUpdate:外部リクエスト受信。切り替えます next_status=%d\n", situ->next_union_status);
    return TRUE;
  }

  if(GFL_NET_GetConnectNum() > 1){
    OS_TPrintf("Normalなのに接続！！！！\n");
    UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK);
    return TRUE;
  }
  
  if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
    FIELD_PLAYER_GetFrontGridPos(player, &check_gx, &check_gy, &check_gz);
    target_pc = MMDLSYS_SearchGridPos(fldMdlSys, check_gx, check_gz, FALSE);
    if(target_pc == NULL){
      return FALSE;
    }
    
    obj_id = MMDL_GetOBJID(target_pc);
    if(obj_id > UNION_RECEIVE_BEACON_MAX){
      GF_ASSERT(0);
      return FALSE;
    }
    UnionMySituation_SetParam(unisys, 
      UNION_MYSITU_PARAM_IDX_CONNECT_MAC, unisys->receive_beacon[obj_id].mac_address);
    OS_TPrintf("ターゲット発見! obj_id = %d, gx=%d, gz=%d\n", obj_id, check_gx, check_gz);
    UnionOneself_ReqStatus(unisys, UNION_STATUS_CONNECT_REQ);
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続リクエスト：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectReqInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_WindowSetup(unisys, fieldWork);
  UnionMsg_TalkStream_Print(unisys, msg_union_select_01);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続リクエスト実行：更新
 *
 * @param   unisys		
 * @param   situ		  
 * @param   seq		    
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectReqUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_FIX_PARENT, TRUE, NULL);
    situ->wait = 0;
    (*seq)++;
    break;
  case 1:
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：親\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK);
      (*seq)++;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_MAC, NULL);
        OS_TPrintf("子が来なかった為キャンセルしました\n");
        (*seq)++;
      }
    }
    break;
  case 2:
    if(UnionMsg_TalkStrem_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続リクエスト：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectReqExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_MAC, NULL);
  UnionMsg_TalkStream_WindowDel(unisys);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続リクエストを受けたので接続を試みる：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectAnswerInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_WindowSetup(unisys, fieldWork);
  UnionMsg_TalkStream_Print(unisys, msg_union_select_02);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続リクエストを受けたので接続を試みる：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectAnswerUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    OS_TPrintf("親へ接続しにいきます\n");
    GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_FIX_CHILD, TRUE, situ->answer_mac_address);
    situ->wait = 0;
    (*seq)++;
    break;
  case 1:
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：子\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK);
      return TRUE;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        OS_TPrintf("親と接続出来なかった為キャンセルしました\n");
        return TRUE;
      }
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続リクエストを受けたので接続を試みる：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectAnswerExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_WindowDel(unisys);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_WindowSetup(unisys, fieldWork);
  UnionMsg_TalkStream_Print(unisys, msg_union_select_03);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_WindowDel(unisys);
  return TRUE;
}
