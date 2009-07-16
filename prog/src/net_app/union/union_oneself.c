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
#include "union_comm_command.h"
#include "system/main.h"
#include "net_app/union/union_event_check.h"
#include "net_app/union/union_subproc.h"


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
#define ONESELF_SERVER_TIMEOUT      (30 * 20)


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
static BOOL OneselfSeq_NormalInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkInit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkExit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkInit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkUpdate_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkExit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TrainerCardUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ShutdownUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq);
static void OneselfSeq_ShutdownCallback( void* pWork );


//==============================================================================
//  データ
//==============================================================================
static const ONESELF_FUNC_DATA OneselfFuncTbl[] = {
  {//UNION_STATUS_NORMAL
    OneselfSeq_NormalInit,
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
  {//UNION_STATUS_TALK_PARENT
    OneselfSeq_TalkInit_Parent,
    OneselfSeq_TalkUpdate_Parent,
    OneselfSeq_TalkExit_Parent,
  },
  {//UNION_STATUS_TALK_CHILD
    OneselfSeq_TalkInit_Child,
    OneselfSeq_TalkUpdate_Child,
    OneselfSeq_TalkExit_Child,
  },
  {//UNION_STATUS_TRAINERCARD
    NULL,
    OneselfSeq_TrainerCardUpdate,
    NULL,
  },
  {//UNION_STATUS_PICTURE
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_TRADE
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_GURUGURU
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_RECORD
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_SHUTDOWN
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_CHAT
    NULL,
    NULL,
    NULL,
  },
};
SDK_COMPILER_ASSERT(NELEMS(OneselfFuncTbl) == UNION_STATUS_MAX);



//==============================================================================
//
//  
//
//==============================================================================
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
 * 通常状態(何もしていない)：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_NormalInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMyComm_Init(&situ->mycomm);

  UnionMsg_AllDel(unisys);

  return TRUE;
}

//--------------------------------------------------------------
/**
 * 通常状態(何もしていない)：更新
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
    GF_ASSERT_MSG(0, "Normalなのに接続！！！！\n");
    //※check　暫定で先頭のビーコンデータを接続相手として代入しておく
    //         本来であればNormalで接続は出来ないようにする
    UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
    UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, &unisys->receive_beacon[0]);
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
      UNION_MYSITU_PARAM_IDX_CALLING_PC, &unisys->receive_beacon[obj_id]);
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
  UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_000);
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
  enum{
    LOCALSEQ_INIT,
    LOCALSEQ_WAIT,
    LOCALSEQ_END,
  };
  
  switch(*seq){
  case LOCALSEQ_INIT:
    OS_TPrintf("ChangeOver モード切替：親固定\n");
    GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_FIX_PARENT, TRUE, NULL);
    situ->wait = 0;
    (*seq)++;
    break;
  case LOCALSEQ_WAIT:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：親\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, situ->calling_pc);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CALLING_PC, NULL);
      (*seq)++;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CALLING_PC, NULL);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_007);
        OS_TPrintf("子が来なかった為キャンセルしました\n");
        (*seq)++;
      }
    }
    break;
  case LOCALSEQ_END:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
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
  UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CALLING_PC, NULL);
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
  UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_001);
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
    GF_ASSERT(situ->answer_pc != NULL);
    OS_TPrintf("ChangeOver モード切替：子固定\n");
    GFL_NET_ChangeoverModeSet(
      GFL_NET_CHANGEOVER_MODE_FIX_CHILD, TRUE, situ->answer_pc->mac_address);
    situ->wait = 0;
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：子\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_CHILD);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, situ->answer_pc);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_ANSWER_PC, NULL);
      (*seq)++;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_ANSWER_PC, NULL);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_007);
        OS_TPrintf("親と接続出来なかった為キャンセルしました\n");
        (*seq)++;
      }
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
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
  UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_ANSWER_PC, NULL);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（親）：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkInit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMyComm_Init(&unisys->my_situation.mycomm);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（親）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    if(situ->connect_pc->beacon.sex == PM_MALE){
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_002);
    }
    else{
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_002);
    }
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      (*seq)++;
    }
    break;
  case 2:   //メインメニュー描画
    UnionMsg_Menu_MainMenuSetup(unisys, fieldWork);
    (*seq)++;
    break;
  case 3:
    {
      u32 select_list;
      
      select_list = UnionMsg_Menu_MainMenuSelectLoop(unisys);
      switch(select_list){
      case FLDMENUFUNC_NULL:
        break;
      case FLDMENUFUNC_CANCEL:
      case UNION_MSG_MENU_SELECT_CANCEL:
        OS_TPrintf("メニューをキャンセルしました\n");
        UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
        UnionMsg_Menu_MainMenuDel(unisys);
        return TRUE;
      default:
        situ->mycomm.mainmenu_select = select_list;
        UnionMsg_Menu_MainMenuDel(unisys);
        (*seq)++;
        break;
      }
    }
    break;
  case 4: //相手に選択したメニューを通知
		if(UnionSend_MainMenuListResult(situ->mycomm.mainmenu_select) == TRUE){
      OS_TPrintf("リスト結果送信成功 : %d\n", situ->mycomm.mainmenu_select);
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_004);
      (*seq)++;
    }
    break;
  case 5: //返事待ち
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    if(situ->mycomm.mainmenu_yesno_result == TRUE){
      OS_TPrintf("「はい」受信\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TRAINERCARD + situ->mycomm.mainmenu_select);
      (*seq)++;
    }
    else if(situ->mycomm.mainmenu_yesno_result == FALSE){
      OS_TPrintf("「いいえ」受信\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_006);
      (*seq)++;
    }
    break;
  case 6:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（親）：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkExit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMsg_Menu_MainMenuDel(unisys);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（子）：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkInit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  UnionMyComm_Init(&unisys->my_situation.mycomm);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（子）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkUpdate_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }

  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    (*seq)++;
    break;
  case 1:
    if(situ->mycomm.mainmenu_select == UNION_MSG_MENU_SELECT_NULL){
      break;
    }
    if(situ->mycomm.mainmenu_select < UNION_MSG_MENU_SELECT_MAX){
      OS_TPrintf("選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
      UnionMsg_TalkStream_PrintPack(
        unisys, fieldWork, msg_union_test_003 + situ->mycomm.mainmenu_select);
      (*seq)++;
    }
    else{
      if(UnionSend_MainMenuListResultAnswer(FALSE) == TRUE){
        OS_TPrintf("未知の選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_009);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
        return TRUE;
      }
    }
    break;
  case 2:   //「はい・いいえ」選択
    UnionMsg_YesNo_Setup(unisys, fieldWork);
    (*seq)++;
    break;
  case 3:
    {
      BOOL result;
      if(UnionMsg_YesNo_SelectLoop(unisys, &result) == TRUE){
        UnionMsg_YesNo_Del(unisys);
        situ->mycomm.mainmenu_yesno_result = result;
        (*seq)++;
      }
    }
    break;
  case 4: //「はい・いいえ」選択結果送信
    if(UnionSend_MainMenuListResultAnswer(situ->mycomm.mainmenu_yesno_result) == TRUE){
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TRAINERCARD + situ->mycomm.mainmenu_select);
      return TRUE;
    }
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（子）：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkExit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  return TRUE;
}

//--------------------------------------------------------------
/**
 * トレーナーカード呼び出し：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TrainerCardUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:   //トレーナーカードの情報を送りあう
    //同期取りの前に送信データと受信バッファを作成
    GF_ASSERT(situ->mycomm.trcard.my_card == NULL && situ->mycomm.trcard.target_card == NULL);
    situ->mycomm.trcard.my_card 
      = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(TR_CARD_DATA));
    situ->mycomm.trcard.target_card 
      = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(TR_CARD_DATA));
    TRAINERCARD_GetSelfData(situ->mycomm.trcard.my_card, unisys->uniparent->game_data, TRUE);
    GFL_STD_MemCopy(  //一応何かの事故で受け取れなかった時のケアの為、自分のをコピーしておく
      situ->mycomm.trcard.my_card, situ->mycomm.trcard.target_card, sizeof(TR_CARD_DATA));
    
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PARAM);
    OS_TPrintf("トレーナーカード前の同期取り開始\n");

    //「はい、どうぞ！」
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_005);

    (*seq)++;
    break;
  case 1:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PARAM) == TRUE){
      OS_TPrintf("トレーナーカード前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 2:
    if(UnionSend_TrainerCardParam(unisys) == TRUE){
      OS_TPrintf("トレーナーカード情報送信\n");
      (*seq)++;
    }
    break;
  case 3:
    if(situ->mycomm.trcard.target_card_receive == TRUE
        && UnionMsg_TalkStream_Check(unisys) == TRUE){
      OS_TPrintf("相手のカード受信\n");
      situ->mycomm.trcard.card_param = TRAINERCASR_CreateCallParam_CommData(
        unisys->uniparent->game_data, situ->mycomm.trcard.target_card, HEAPID_UNION);
      (*seq)++;
    }
    break;
  case 4:   //画面切り替え前の同期取り
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_BEFORE);
    OS_TPrintf("トレーナーカード画面切り替え前の同期取り開始\n");
    (*seq)++;
    break;
  case 5:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_BEFORE) == TRUE){
      OS_TPrintf("トレーナーカード画面切り替え前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 6:
    UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_TRAINERCARD, situ->mycomm.trcard.card_param);
    (*seq)++;
    break;
  case 7:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      OS_TPrintf("サブPROC終了\n");
      (*seq)++;
    }
    break;
  case 8:   //トレーナーカード画面終了後の同期取り
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_AFTER);
    OS_TPrintf("トレーナーカード終了後の同期取り開始\n");
    (*seq)++;
    break;
  case 9:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_AFTER) == TRUE){
      OS_TPrintf("トレーナーカード終了後の同期取り成功\n");
      
      GFL_HEAP_FreeMemory(situ->mycomm.trcard.card_param);
      GFL_HEAP_FreeMemory(situ->mycomm.trcard.my_card);
      GFL_HEAP_FreeMemory(situ->mycomm.trcard.target_card);
      situ->mycomm.trcard.card_param = NULL;
      situ->mycomm.trcard.my_card = NULL;
      situ->mycomm.trcard.target_card = NULL;
      
      if(GFL_NET_IsParentMachine() == TRUE){
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
      }
      else{
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_CHILD);
      }
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 切断処理：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ShutdownUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_SHUTDOWN);
    OS_TPrintf("切断前の同期取り：開始\n");
    (*seq)++;
    break;
  case 1:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_SHUTDOWN) == TRUE){
      OS_TPrintf("切断前の同期取り：成功\n");
      (*seq)++;
    }
    break;
  case 2:
    if(GFL_NET_IsParentMachine() == TRUE){
      if(GFL_NET_GetConnectNum() <= 1){ //親は自分一人になってから終了する
        (*seq)++;
      }
      else{
        OS_TPrintf("親：子の終了待ち 残り=%d\n", GFL_NET_GetConnectNum() - 1);
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 3:
    GFL_NET_Exit( OneselfSeq_ShutdownCallback );
    (*seq)++;
    break;
  case 4:
    if(situ->connect_pc == NULL){   //OneselfSeq_ShutdownCallbackでNULL化されるのを待つ
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 切断処理：切断完了コールバック
 *
 * @param   pWork		
 */
//--------------------------------------------------------------
static void OneselfSeq_ShutdownCallback( void* pWork )
{
  UNION_SYSTEM_PTR unisys = pWork;
  
  unisys->my_situation.connect_pc = NULL;
  OS_TPrintf("Shutdown:切断完了コールバック\n");
}

//--------------------------------------------------------------
/**
 * 対戦の会話（親）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_Talk_Battle_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }

  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    (*seq)++;
    break;
  case 1:
    if(situ->mycomm.mainmenu_select == UNION_MSG_MENU_SELECT_NULL){
      break;
    }
    if(situ->mycomm.mainmenu_select < UNION_MSG_MENU_SELECT_MAX){
      OS_TPrintf("選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
      UnionMsg_TalkStream_PrintPack(
        unisys, fieldWork, msg_union_test_003 + situ->mycomm.mainmenu_select);
      (*seq)++;
    }
    else{
      if(UnionSend_MainMenuListResultAnswer(FALSE) == TRUE){
        OS_TPrintf("未知の選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_009);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
        return TRUE;
      }
    }
    break;
  case 2:   //「はい・いいえ」選択
    UnionMsg_YesNo_Setup(unisys, fieldWork);
    (*seq)++;
    break;
  case 3:
    {
      BOOL result;
      if(UnionMsg_YesNo_SelectLoop(unisys, &result) == TRUE){
        UnionMsg_YesNo_Del(unisys);
        situ->mycomm.mainmenu_yesno_result = result;
        (*seq)++;
      }
    }
    break;
  case 4: //「はい・いいえ」選択結果送信
    if(UnionSend_MainMenuListResultAnswer(situ->mycomm.mainmenu_yesno_result) == TRUE){
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TRAINERCARD + situ->mycomm.mainmenu_select);
      return TRUE;
    }
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 対戦の会話（子）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_Talk_Battle_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELD_MAIN_WORK *fieldWork, u8 *seq)
{
  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }

  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    (*seq)++;
    break;
  case 1:
    if(situ->mycomm.mainmenu_select == UNION_MSG_MENU_SELECT_NULL){
      break;
    }
    if(situ->mycomm.mainmenu_select < UNION_MSG_MENU_SELECT_MAX){
      OS_TPrintf("選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
      UnionMsg_TalkStream_PrintPack(
        unisys, fieldWork, msg_union_test_003 + situ->mycomm.mainmenu_select);
      (*seq)++;
    }
    else{
      if(UnionSend_MainMenuListResultAnswer(FALSE) == TRUE){
        OS_TPrintf("未知の選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_009);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
        return TRUE;
      }
    }
    break;
  case 2:   //「はい・いいえ」選択
    UnionMsg_YesNo_Setup(unisys, fieldWork);
    (*seq)++;
    break;
  case 3:
    {
      BOOL result;
      if(UnionMsg_YesNo_SelectLoop(unisys, &result) == TRUE){
        UnionMsg_YesNo_Del(unisys);
        situ->mycomm.mainmenu_yesno_result = result;
        (*seq)++;
      }
    }
    break;
  case 4: //「はい・いいえ」選択結果送信
    if(UnionSend_MainMenuListResultAnswer(situ->mycomm.mainmenu_yesno_result) == TRUE){
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TRAINERCARD + situ->mycomm.mainmenu_select);
      return TRUE;
    }
  }
  
  return FALSE;
}
