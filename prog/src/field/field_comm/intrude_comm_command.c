//==============================================================================
/**
 * @file    intrude_comm_command.c
 * @brief   侵入通信コマンド
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "net/network_define.h"
#include "intrude_types.h"
#include "intrude_comm_command.h"
#include "field/zonedata.h"
#include "intrude_main.h"
#include "intrude_mission.h"
#include "net/net_whpipe.h"


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvHugeBuffer_ChoiceList(int netID, void* pWork, int size);
static u8 * _RecvHugeBuffer_MissionData(int netID, void* pWork, int size);
static u8 * _RecvHugeBuffer_Profile(int netID, void* pWork, int size);
static u8 * _RecvHugeBuffer_MissionResult(int netID, void* pWork, int size);
static void _IntrudeRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MemberNum(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_ProfileReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Profile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_DeleteProfile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_PlayerStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TimeoutWarning(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Talk(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkRandDisagreement(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkCertificationParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MonolithStatusReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MonolithStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionListReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionList(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionEntryAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _MissionOrderConfirm(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionClientStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionClientStartAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionAchieve(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionAchieveAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionCheckAchieveUser(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionAnswerAchieveUserUse(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionAnswerAchieveUserNone(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_OccupyInfo(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_OccupyResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_OtherMonolithIn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_OtherMonolithOut(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionBattleTiming(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_PlayerSupport(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_SecretItem(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_GPowerEquip(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_WfbcReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Wfbc(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_WfbcNpcAns(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_WfbcNpcReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_SymbolDataReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_SymbolData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_SymbolDataChange(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※INTRUDE_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable Intrude_CommPacketTbl[] = {
  {_IntrudeRecv_Shutdown, NULL},               //INTRUDE_CMD_SHUTDOWN
  {_IntrudeRecv_MemberNum, NULL},              //INTRUDE_CMD_MEMBER_NUM
  {_IntrudeRecv_ProfileReq, NULL},             //INTRUDE_CMD_PROFILE_REQ
  {_IntrudeRecv_Profile, _RecvHugeBuffer_Profile},                //INTRUDE_CMD_PROFILE
  {_IntrudeRecv_DeleteProfile, NULL},          //INTRUDE_CMD_DELETE_PROFILE
  {_IntrudeRecv_PlayerStatus, NULL},           //INTRUDE_CMD_PLAYER_STATUS
  {_IntrudeRecv_TimeoutWarning, NULL},         //INTRUDE_CMD_TIMEOUT_WARNING
  {_IntrudeRecv_Talk, NULL},                   //INTRUDE_CMD_TALK
  {_IntrudeRecv_TalkAnswer, NULL},             //INTRUDE_CMD_TALK_ANSWER
  {_IntrudeRecv_TalkRandDisagreement, NULL},   //INTRUDE_CMD_TALK_RAND_DISAGREEMENT
  {_IntrudeRecv_TalkCancel, NULL},             //INTRUDE_CMD_TALK_CANCEL
  {_IntrudeRecv_TalkCertificationParam, NULL}, //INTRUDE_CMD_TALK_CERTIFICATION
  {_IntrudeRecv_MonolithStatusReq, NULL},      //INTRUDE_CMD_MONOLITH_STATUS_REQ
  {_IntrudeRecv_MonolithStatus, NULL},         //INTRUDE_CMD_MONOLITH_STATUS
  {_IntrudeRecv_MissionListReq, NULL},         //INTRUDE_CMD_MISSION_LIST_REQ
  {_IntrudeRecv_MissionList, _RecvHugeBuffer_ChoiceList}, //INTRUDE_CMD_MISSION_LIST
  {_MissionOrderConfirm, NULL},                //INTRUDE_CMD_MISSION_ORDER_CONFIRM
  {_IntrudeRecv_MissionEntryAnswer, NULL},     //INTRUDE_CMD_MISSION_ENTRY_ANSWER
  {_IntrudeRecv_MissionData, _RecvHugeBuffer_MissionData},            //INTRUDE_CMD_MISSION_DATA
  {_IntrudeRecv_MissionStart, NULL},           //INTRUDE_CMD_MISSION_START
  {_IntrudeRecv_MissionClientStart, NULL},     //INTRUDE_CMD_MISSION_CLIENT_START
  {_IntrudeRecv_MissionClientStartAnswer, NULL}, //INTRUDE_CMD_MISSION_CLIENT_START_ANSWER
  {_IntrudeRecv_MissionAchieve, NULL},         //INTRUDE_CMD_MISSION_ACHIEVE
  {_IntrudeRecv_MissionAchieveAnswer, NULL},   //INTRUDE_CMD_MISSION_ACHIEVE_ANSWER
  {_IntrudeRecv_MissionResult, _RecvHugeBuffer_MissionResult},  //INTRUDE_CMD_MISSION_RESULT
  {_IntrudeRecv_MissionCheckAchieveUser, NULL},//INTRUDE_CMD_MISSION_CHECK_ACHIEVE_USER
  {_IntrudeRecv_MissionAnswerAchieveUserUse, NULL},//INTRUDE_CMD_MISSION_ANSWER_ACHIEVE_USER_USE
  {_IntrudeRecv_MissionAnswerAchieveUserNone, NULL},//INTRUDE_CMD_MISSION_ANSWER_ACHIEVE_USER_NONE
  {_IntrudeRecv_OccupyInfo, NULL},             //INTRUDE_CMD_OCCUPY_INFO
  {_IntrudeRecv_OccupyResult, NULL},           //INTRUDE_CMD_OCCUPY_RESULT
  {_IntrudeRecv_OtherMonolithIn, NULL},        //INTRUDE_CMD_OTHER_MONOLITH_IN
  {_IntrudeRecv_OtherMonolithOut, NULL},       //INTRUDE_CMD_OTHER_MONOLITH_OUT
  {_IntrudeRecv_MissionBattleTiming, NULL},           //INTRUDE_CMD_TARGET_TIMING
  {_IntrudeRecv_PlayerSupport, NULL},          //INTRUDE_CMD_PLAYER_SUPPORT
  {_IntrudeRecv_SecretItem, NULL},             //INTRUDE_CMD_SECRET_ITEM
  {_IntrudeRecv_GPowerEquip, NULL},            //INTRUDE_CMD_GPOWER_EQUIP
  {_IntrudeRecv_WfbcReq, NULL},                //INTRUDE_CMD_WFBC_REQ
  {_IntrudeRecv_Wfbc, NULL},                   //INTRUDE_CMD_WFBC
  {_IntrudeRecv_WfbcNpcAns, NULL},             //INTRUDE_CMD_WFBC_NPC_ANS
  {_IntrudeRecv_WfbcNpcReq, NULL},             //INTRUDE_CMD_WFBC_NPC_REQ
  {_IntrudeRecv_SymbolDataReq, NULL},          //INTRUDE_CMD_SYMBOL_DATA_REQ
  {_IntrudeRecv_SymbolData, NULL},             //INTRUDE_CMD_SYMBOL_DATA
  {_IntrudeRecv_SymbolDataChange, NULL},       //INTRUDE_CMD_SYMBOL_DATA_CHANGE
};
SDK_COMPILER_ASSERT(NELEMS(Intrude_CommPacketTbl) == INTRUDE_CMD_NUM);



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * 巨大データ用受信バッファ取得：ミッションリスト
 *
 * @param   netID		
 * @param   pWork		
 * @param   size		
 *
 * @retval  u8 *		
 */
//--------------------------------------------------------------
static u8 * _RecvHugeBuffer_ChoiceList(int netID, void* pWork, int size)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  return (u8*)(&intcomm->huge_recv_choicelist);
}

//--------------------------------------------------------------
/**
 * 巨大データ用受信バッファ取得：ミッションデータ
 *
 * @param   netID		
 * @param   pWork		
 * @param   size		
 *
 * @retval  u8 *		
 */
//--------------------------------------------------------------
static u8 * _RecvHugeBuffer_MissionData(int netID, void* pWork, int size)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  return (u8*)(&intcomm->huge_recv_mission_data[netID]);
}

//--------------------------------------------------------------
/**
 * 巨大データ用受信バッファ取得：プロフィール
 *
 * @param   netID		
 * @param   pWork		
 * @param   size		
 *
 * @retval  u8 *		
 */
//--------------------------------------------------------------
static u8 * _RecvHugeBuffer_Profile(int netID, void* pWork, int size)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  return (u8*)(&intcomm->huge_recv_profile[netID]);
}

//--------------------------------------------------------------
/**
 * 巨大データ用受信バッファ取得：ミッション結果
 *
 * @param   netID		
 * @param   pWork		
 * @param   size		
 *
 * @retval  u8 *		
 */
//--------------------------------------------------------------
static u8 * _RecvHugeBuffer_MissionResult(int netID, void* pWork, int size)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  GF_ASSERT(netID == 0);  //親からしか送られてこないはず
  return (u8*)(&intcomm->huge_recv_mission_result);
}

//--------------------------------------------------------------
/**
 * 自分以外のプレイヤーが存在しているか調べる
 *
 * @param   none		
 *
 * @retval  BOOL		TRUE:存在している。　FALSE:誰もいない
 * 
 * 離脱などで親一人になった場合、キューに貯まったバッファが解消されず、キューが溢れてしまうので
 * 送信コマンドを実行しないようにチェックする関数
 */
//--------------------------------------------------------------
static BOOL _OtherPlayerExistence(void)
{
  return Intrude_OtherPlayerExistence();
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：切断
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  intcomm->exit_recv = TRUE;
  OS_TPrintf("切断コマンド受信 netID=%d\n", netID);
}

//==================================================================
/**
 * データ送信：切断
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_Shutdown(INTRUDE_COMM_SYS_PTR intcomm)
{
  OS_TPrintf("SEND:切断コマンド\n");
  
  if(_OtherPlayerExistence() == FALSE){
    //既に誰もいないので自分で受信命令を呼び出す
    OS_TPrintf("自分で切断コールバックを直接呼び出す\n");
    _IntrudeRecv_Shutdown(
      GFL_NET_SystemGetCurrentID(), 0, NULL, intcomm, GFL_NET_HANDLE_GetCurrentHandle());
    return TRUE;
  }
  
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_SHUTDOWN, 0, NULL);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：侵入参加人数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MemberNum(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  const u8 *member_num = pData;
  
	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;   //自分のデータは受け取らない
  }
  intcomm->member_num = *member_num;
  GAMEDATA_SetIntrudeNum(gamedata, *member_num);
  OS_TPrintf("member_num受信 %d\n", *member_num);
}

//==================================================================
/**
 * データ送信：侵入参加人数 (親機専用命令)
 *
 * @param   intcomm		
 * @param   member_num		参加人数
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MemberNum(INTRUDE_COMM_SYS_PTR intcomm, u8 member_num)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER, 
    INTRUDE_CMD_MEMBER_NUM, sizeof(member_num), &member_num, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：プロフィールを要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_ProfileReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  intcomm->profile_req = TRUE;
  OS_TPrintf("プロフィール要求コマンド受信 net_id=%d\n", netID);
}

//==================================================================
/**
 * データ送信：プロフィールを要求
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_ProfileReq(void)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_PROFILE_REQ, 0, NULL);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：プロフィール
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_Profile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_PROFILE *recv_profile = pData;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  
  if(netID >= intcomm->member_num){
    //管理している人数よりも大きいIDから来た物は受け取らない。
    //先に管理人数を受信してからでないとマップの繋がりが拡大できない。よって表示位置も不明になる
    OS_TPrintf("PROFILE RECV 管理人数よりも大きいIDの為、無視 net_id=%d, member_num=%d\n", netID, intcomm->member_num);
    return;
  }

  Intrude_SetProfile(intcomm, netID, recv_profile);

  //自分が親ではなくて、通信相手(親)がチュートリアル中の場合。
  //Intrude_SetProfileは非通信時に自分が呼び出しもしているので
  //必ず受信データのみに行うようここで判定
  if(netID != GAMEDATA_GetIntrudeMyID( gamedata ) && GFL_NET_IsParentMachine() == FALSE
      && netID == GFL_NET_NO_PARENTMACHINE){
    if(recv_profile->status.tutorial == TRUE){
      intcomm->member_is_tutorial = TRUE;
      OS_TPrintf("member_tutorial net_id=%d\n", netID);
    }
  }
}

//==================================================================
/**
 * データ送信：自分のプロフィール
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_Profile(INTRUDE_COMM_SYS_PTR intcomm)
{
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  if(GFL_NET_IsInSendCommandQueue(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_PROFILE) == TRUE){
    return FALSE;
  }
  
  GFL_STD_MemCopy(&intcomm->my_profile, &intcomm->huge_send_profile, sizeof(INTRUDE_PROFILE));

  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER,
    INTRUDE_CMD_PROFILE, sizeof(INTRUDE_PROFILE), &intcomm->huge_send_profile,
    FALSE, FALSE, TRUE);
  if(ret == TRUE){
    intcomm->profile_req = FALSE;
    OS_TPrintf("自分プロフィール送信\n");
    if(GFL_NET_IsParentMachine() == TRUE){
      //親の場合、プロフィールを要求されていたという事は途中参加者がいるので
      //現在の参加人数、ミッションも送信する
      intcomm->member_send_req = TRUE;
      MISSION_Set_DataSendReq(&intcomm->mission);
      MISSION_Set_ResultSendReq(&intcomm->mission);
    }
  }
  else{
    OS_TPrintf("自分プロフィール送信失敗\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：離脱者のプロフィールを削除
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_DeleteProfile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const int *leave_netid = pData;
  
  if(intcomm->recv_profile & (1 << (*leave_netid))){
    intcomm->recv_profile ^= 1 << (*leave_netid);
    CommPlayer_Del(intcomm->cps, *leave_netid);
    //GameCommInfo_MessageEntry_Leave(intcomm->game_comm, *leave_netid);
  }
  OS_TPrintf("Receive:離脱者のプロフィール削除 離脱者のNetID = %d\n", *leave_netid);
}

//==================================================================
/**
 * データ送信：離脱者のプロフィールを削除(親機専用命令)
 * @param   leave_netid   離脱者のNetID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_DeleteProfile(INTRUDE_COMM_SYS_PTR intcomm, int leave_netid)
{
  if(_OtherPlayerExistence() == FALSE){
    _IntrudeRecv_DeleteProfile(GFL_NET_SystemGetCurrentID(), 
      sizeof(leave_netid), &leave_netid, intcomm, GFL_NET_HANDLE_GetCurrentHandle());
    return TRUE;
  }

  GF_ASSERT(GFL_NET_IsParentMachine() == TRUE);
  OS_TPrintf("Send:離脱者のプロフィール削除 離脱者のNetID = %d\n", leave_netid);
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_DELETE_PROFILE, sizeof(leave_netid), &leave_netid);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：現在値情報
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_PlayerStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_STATUS *recv_data = pData;
  
	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){ //自分のデータは受け取らない
    //グレースケール化の為、これだけセット
    GameCommStatus_SetPlayerStatus(intcomm->game_comm, netID, recv_data->zone_id,
      recv_data->palace_area, PM_VERSION, FALSE);
    return;
  }
  if((intcomm->recv_profile & (1 << netID)) == 0){  //プロフィール未受信は受け取らない
    return;   
  }
  Intrude_SetPlayerStatus(intcomm, netID, recv_data, FALSE);
}

//==================================================================
/**
 * データ送信：現在値情報
 *
 * @param   intcomm		
 * @param   gamedata		
 * @param   send_status		送信データへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_PlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_STATUS *send_status)
{
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER,
    INTRUDE_CMD_PLAYER_STATUS, sizeof(INTRUDE_STATUS), send_status, FALSE, TRUE, FALSE);
  if(ret == TRUE){
    intcomm->send_status = FALSE;
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：「ミッションを開始しないからタイムアウトで切断しそう」
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_TimeoutWarning(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  intcomm->mission_start_timeout_warning = TRUE;
  OS_TPrintf("recv timeout warning netID=%d\n", netID);
}

//==================================================================
/**
 * データ送信：「ミッションを開始しないからタイムアウトで切断しそう」
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TimeoutWarning(void)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_TIMEOUT_WARNING, 0, NULL);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：話しかける
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_Talk(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_TALK_FIRST_ATTACK *first_attack = pData;
  
  OS_TPrintf("話しかけられた受信　net_id=%d, talk_type=%d\n", netID, first_attack->talk_type);
  if(Intrude_SetTalkReq(intcomm, netID, first_attack->talk_rand) == TRUE){
    intcomm->recv_talk_first_attack = *first_attack;
  }
}

//==================================================================
/**
 * データ送信：話しかける
 * @param   send_net_id		話しかけ先のNetID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_Talk(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id, const MISSION_DATA *mdata, INTRUDE_TALK_TYPE intrude_talk_type)
{
  INTRUDE_TALK_FIRST_ATTACK first_attack;
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  if(intcomm->now_talk_rand == TALK_RAND_NULL){
    intcomm->now_talk_rand++;
  }
  
  GFL_STD_MemClear(&first_attack, sizeof(INTRUDE_TALK_FIRST_ATTACK));
  first_attack.talk_type = intrude_talk_type;
  first_attack.mdata = *mdata;
  first_attack.talk_rand = intcomm->now_talk_rand;

  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK, sizeof(INTRUDE_TALK_FIRST_ATTACK), &first_attack, FALSE, FALSE, FALSE);
  if(ret == TRUE){
    intcomm->talk.talk_rand = intcomm->now_talk_rand;
    intcomm->talk.now_talk_rand = intcomm->now_talk_rand;
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：話しかけられたので自分の状況を返事として返す
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_TalkAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_TALK_ANSWER *talkans = pData;
  
  if(talkans->talk_rand == intcomm->talk.now_talk_rand){
    OS_TPrintf("話しかけの返事受信　net_id=%d, talk_status=%d\n", netID, talkans->talk_status);
    Intrude_SetTalkAnswer(intcomm, netID, talkans->talk_status, talkans->talk_rand);
  }
  else{
    OS_TPrintf("話しかけの返事受信　randが違う為無視 netid=%d, my_rand=%d, recv_rand=%d\n", netID, intcomm->talk.talk_rand, talkans->talk_rand);
    intcomm->send_talk_rand_disagreement[netID] = talkans->talk_rand;
  }
}

//==================================================================
/**
 * データ送信：話しかけられたので自分の状況を返事として返す
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id, INTRUDE_TALK_STATUS talk_status, u8 talk_rand)
{
  INTRUDE_TALK_ANSWER talkans;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  talkans.talk_status = talk_status;
  talkans.talk_rand = talk_rand;
  
  OS_TPrintf("話しかけの返事送信 answer=%d rand=%d\n", talk_status, talk_rand);
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_ANSWER, sizeof(INTRUDE_TALK_ANSWER), &talkans, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：会話コードの不一致が発生した
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_TalkRandDisagreement(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const u8 *talk_rand = pData;
  
  OS_TPrintf("RECV:talk_rand不一致：");

  if(intcomm->talk.talk_netid == netID && intcomm->talk.talk_rand == *talk_rand){
    OS_TPrintf("CANCELセット\n");
    Intrude_SetTalkAnswer(intcomm, netID, INTRUDE_TALK_STATUS_CANCEL, *talk_rand);
  }
  else{
    OS_TPrintf("無視\n");
  }
}

//==================================================================
/**
 * データ送信：会話コードの不一致が発生した
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TalkRandDisagreement(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id, u8 talk_rand)
{
  INTRUDE_TALK_ANSWER talkans;
  
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  GF_ASSERT(talk_rand != TALK_RAND_NULL);
  
  OS_TPrintf("SEND:会話コード不一致 送信 net_id=%d, rand=%d\n", send_net_id, talk_rand);
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_RAND_DISAGREEMENT, sizeof(talk_rand), &talk_rand, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：話しかけをキャンセル
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_TalkCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const u8 *talk_rand = pData;
  
  OS_TPrintf("話しかけキャンセル受信　net_id=%d\n", netID);
  Intrude_SetTalkCancel(intcomm, netID, *talk_rand);
}

//==================================================================
/**
 * データ送信：話しかけをキャンセル
 * @param   send_net_id		話しかけ先のNetID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TalkCancel(int send_net_id, u8 talk_rand)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_CANCEL, sizeof(talk_rand), &talk_rand, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：話しかけ成立時の最後の認証データ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_TalkCertificationParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_TALK_CERTIFICATION *recv_certifi = pData;
  
  OS_TPrintf("話しかけ最後の認証データ受信　net_id=%d\n", netID);
  intcomm->recv_certifi = *recv_certifi;
}

//==================================================================
/**
 * データ送信：話しかけ成立時の最後の認証データ
 *
 * @param   send_net_id		  話しかけ先のNetID
 * @param   achieve_user		既にミッション達成者がいるか自分で調べた結果
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TalkCertificationParam(int send_net_id, BOOL achieve_user)
{
  INTRUDE_TALK_CERTIFICATION send_certifi;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  GFL_STD_MemClear(&send_certifi, sizeof(INTRUDE_TALK_CERTIFICATION));
  send_certifi.occ = TRUE;
  send_certifi.mission_achieve_user = achieve_user;
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_CERTIFICATION, sizeof(INTRUDE_TALK_CERTIFICATION), 
    &send_certifi, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：モノリスステータス要求リクエスト
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MonolithStatusReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  intcomm->monost_req |= 1 << netID;
  OS_TPrintf("RECV:monost req netid=%d\n", netID);
}

//==================================================================
/**
 * データ送信：モノリスステータス要求リクエスト
 *
 * @param   intcomm		
 * @param   send_netid		要求相手のNetID
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MonolithStatusReq(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid)
{
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  OS_TPrintf("SEND:monost req netid=%d\n", send_netid);
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_netid,
    INTRUDE_CMD_MONOLITH_STATUS_REQ, 0, NULL,
    FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：モノリスステータス
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MonolithStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MONOLITH_STATUS *monost = pData;
  
  intcomm->monolith_status = *monost;
  OS_TPrintf("RECV:monost netid=%d\n", netID);
}

//==================================================================
/**
 * データ送信：モノリスステータス
 *
 * @param   intcomm		
 * @param   send_netid_bit		送信先(bit指定)
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MonolithStatus(INTRUDE_COMM_SYS_PTR intcomm, u32 send_netid_bit)
{
  BOOL ret;
  MONOLITH_STATUS monost;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork(gamedata) );
  
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  Intrude_MyMonolithStatusSet(gamedata, &monost);
  
  OS_TPrintf("SEND:monost netid_bit=%d\n", send_netid_bit);
  return GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), send_netid_bit,
    INTRUDE_CMD_MONOLITH_STATUS, sizeof(MONOLITH_STATUS), &monost,
    FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション選択候補リスト要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionListReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const u32 *palace_area = pData;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  OS_TPrintf("RECIEVE：ミッションリスト要求 net_id=%d\n", netID);
  MISSION_MissionList_Create(intcomm, &intcomm->mission, netID, *palace_area);
  MISSION_Set_ListSendReq(&intcomm->mission, *palace_area);
}

//==================================================================
/**
 * データ送信：ミッション選択候補リスト要求
 *
 * @param   intcomm         
 * @param   zone_id		      ミニモノリスがあるゾーンID
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionListReq(INTRUDE_COMM_SYS_PTR intcomm, u32 palace_area)
{
  MISSION_REQ req;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  OS_TPrintf("SEND:ミッションリスト要求\n");
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_LIST_REQ, sizeof(palace_area), &palace_area);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション選択候補リスト
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionList(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_CHOICE_LIST *mlist = pData;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  NetID list_netid;   //ミッションリスト所有者のNetID
  
  list_netid = mlist->target_info.net_id;
  MISSION_SetMissionList(&intcomm->mission, mlist, list_netid);

  if(list_netid != GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){//自分のデータで無いなら占拠情報もセット
    OCCUPY_INFO *dest_occupy = GAMEDATA_GetOccupyInfo(gamedata, list_netid);
    GFL_STD_MemCopy(&mlist->occupy, dest_occupy, sizeof(OCCUPY_INFO));
  }
  
  OS_TPrintf("RECEIVE: ミッションリスト list_netid = %d\n", list_netid);
}

//==================================================================
/**
 * データ送信：ミッション選択候補リスト
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 * @param   palace_area     どのパレスエリアの候補リストなのか
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionList(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission, int palace_area)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER,
    INTRUDE_CMD_MISSION_LIST, sizeof(MISSION_CHOICE_LIST), &mission->list[palace_area],
    FALSE, FALSE, TRUE);
  if(ret == TRUE){
    OS_TPrintf("SEND：ミッションリスト palace_area=%d, size=%d\n", 
      palace_area, sizeof(MISSION_CHOICE_LIST));
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション受注します
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _MissionOrderConfirm(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_ENTRY_REQ *entry_req = pData;
  BOOL entry_ret;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  OS_TPrintf("受信：ミッション受注します net_id=%d\n", netID);
  entry_ret = MISSION_SetEntryNew(intcomm, &intcomm->mission, entry_req, netID);
  if(entry_ret == TRUE && (entry_req->cdata.type == MISSION_TYPE_VICTORY
      || entry_req->cdata.type == MISSION_TYPE_ATTRIBUTE)){
    GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(), FALSE);
    GFL_NET_WL_PauseBeacon(TRUE);
    intcomm->member_fix = TRUE;
    OS_TPrintf("二人専用の為、乱入禁止\n");
  }
}

//==================================================================
/**
 * データ送信：ミッション受注します
 *
 * @param   intcomm         
 * @param   mdata           受注しようとしているミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionOrderConfirm(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_ENTRY_REQ *entry_req)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  MISSION_ClearRecvEntryAnswer(&intcomm->mission);

  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_ORDER_CONFIRM, sizeof(MISSION_ENTRY_REQ), entry_req);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：「ミッション受注します」の返事
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionEntryAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_ENTRY_ANSWER *entry_answer = pData;
  
  OS_TPrintf("Recv：「ミッション受注します」の結果 net_id=%d\n", netID);
  MISSION_SetRecvEntryAnswer(&intcomm->mission, entry_answer);
}

//==================================================================
/**
 * データ送信：「ミッション受注します」の返事
 *
 * @param   intcomm		
 * @param   entry_answer		
 * @param   send_netid		
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionEntryAnswer(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_ENTRY_ANSWER *entry_answer, int send_netid)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;  //相手がいない場合は送信済みにしてしまう
  }
  if(GFL_NET_IsConnectMember(send_netid) == FALSE){
    return TRUE;  //相手がいない場合は送信済みにしてしまう
  }
  
  OS_TPrintf("SEND：「ミッション受注します」の返事\n");
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_netid, 
    INTRUDE_CMD_MISSION_ENTRY_ANSWER, sizeof(MISSION_ENTRY_ANSWER), entry_answer, 
    FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッションデータ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_DATA *mdata = pData;
  BOOL new_mission;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  ZONEID zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata));
  
#if 0 //エントリー式にしたため、表でも受け取るようにした　2010.02.02(火)
  if(ZONEDATA_IsPalaceField(zone_id) == FALSE && ZONEDATA_IsPalace(zone_id) == FALSE){
    OS_TPrintf("ミッション受信：表フィールドにいるため、受け取らない\n");
    return;
  }
#endif

  new_mission = MISSION_SetMissionData(intcomm, &intcomm->mission, mdata);
  if(new_mission == TRUE){  //連続受信で上書きされないように直接代入はしない
    intcomm->new_mission_recv = TRUE;
  }
  
  OS_TPrintf("ミッション受信：new_mission_recv = %d\n", new_mission);
}

//==================================================================
/**
 * データ送信：ミッションデータ
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionData(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  if(GFL_NET_IsInSendCommandQueue(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_MISSION_DATA) == TRUE){
    return FALSE;
  }
  
  GFL_STD_MemCopy(&mission->data, &intcomm->huge_send_mission_data, sizeof(MISSION_DATA));
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER,
    INTRUDE_CMD_MISSION_DATA, sizeof(MISSION_DATA), &intcomm->huge_send_mission_data,
    FALSE, FALSE, TRUE);
  if(ret == TRUE){
    OS_TPrintf("送信：ミッションデータ \n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション開始
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_DATA *mdata = pData;
  const MISSION_DATA *my_recv_data = MISSION_GetRecvData(&intcomm->mission);
  
  if(my_recv_data == NULL){
    OS_TPrintf("RECV:ミッション開始：ミッション未受信の為無視\n");
    return;
  }
  if(GFL_STD_MemComp(my_recv_data, mdata, sizeof(MISSION_DATA)) != 0){
    OS_TPrintf("RECV:ミッション開始：所持しているミッションデータと違うので無視\n");
    return;
  }
  
  MISSION_RecvMissionStart(&intcomm->mission);
  OS_TPrintf("RECV:ミッション開始\n");
}

//==================================================================
/**
 * データ送信：ミッション開始
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionStart(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_START, sizeof(MISSION_DATA), &mission->data);
  if(ret == TRUE){
    OS_TPrintf("送信：ミッション開始 \n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：クライアントの「ミッション開始します」宣言
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionClientStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_DATA *mdata = pData;
  const MISSION_DATA *my_recv_data = MISSION_GetRecvData(&intcomm->mission);
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }

  MISSION_RecvMissionStartClient(&intcomm->mission, netID);
  OS_TPrintf("RECV:クライアントミッション開始宣言 netID=%d\n", netID);
}

//==================================================================
/**
 * データ送信：クライアントの「ミッション開始します」宣言
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionClientStart(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_CLIENT_START, sizeof(MISSION_DATA), &mission->data);
  if(ret == TRUE){
    OS_TPrintf("送信：クライアントミッション開始宣言 \n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：クライアントの「ミッション開始します」宣言の返事
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionClientStartAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const s32 *p_now_mission_time = pData;
  
  MISSION_RecvMissionStartClientAnswer(&intcomm->mission, *p_now_mission_time);
  OS_TPrintf("RECV:クライアントミッション開始宣言の返事\n");
}

//==================================================================
/**
 * データ送信：クライアントの「ミッション開始します」宣言の返事
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 * @param   send_netid_bit	送信先(Bit指定)
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionClientStartAnswer(INTRUDE_COMM_SYS_PTR intcomm, s32 now_mission_time, u32 send_netid_bit)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  ret = GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), send_netid_bit,
    INTRUDE_CMD_MISSION_CLIENT_START_ANSWER, sizeof(now_mission_time), &now_mission_time,
    FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("送信：クライアントミッション開始宣言の返事 send_bit=%d\n", send_netid_bit);
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション達成を親に伝える
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionAchieve(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_DATA *mdata = pData;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  OS_TPrintf("受信：ミッション達成 netID=%d\n", netID);
  MISSION_EntryAchieve(&intcomm->mission, mdata, netID);
}

//==================================================================
/**
 * データ送信：ミッション達成を親に伝える
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionAchieve(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  MISSION_ClearAchieveAnswer(&intcomm->mission);
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_ACHIEVE, sizeof(MISSION_DATA), &mission->data);
  if(ret == TRUE){
    MISSION_SetMissionComplete(&intcomm->mission);
    OS_TPrintf("送信：ミッション達成 \n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション達成報告の返事
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionAchieveAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_ACHIEVE *achieve = pData;
  
  OS_TPrintf("受信：ミッション達成報告の返事 netID=%d\n", netID);
  MISSION_SetParentAchieve(&intcomm->mission, *achieve);
}

//==================================================================
/**
 * データ送信：ミッション達成報告の返事
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionAchieveAnswer(INTRUDE_COMM_SYS_PTR intcomm, MISSION_ACHIEVE achieve, int send_netid)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return TRUE;  //相手がいない場合は送信済みにしてしまう
  }
  if(GFL_NET_IsConnectMember(send_netid) == FALSE){
    return TRUE;  //相手がいない場合は送信済みにしてしまう
  }
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_netid, 
    INTRUDE_CMD_MISSION_ACHIEVE_ANSWER, sizeof(MISSION_ACHIEVE), &achieve, 
    FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("送信：ミッション達成報告の返事 achieve=%d, send_netid=%d\n", achieve, send_netid);
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション結果
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_RESULT *mresult = pData;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  OCCUPY_INFO *my_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  BOOL complete = FALSE;
  u8 white_num=0, black_num=0;
  int add_white = 0;
  int add_black = 0;
  
#if 0 //返事を返さないと求めている側が永遠の待ちになるので 2010.04.23(金)
  if((intcomm->recv_profile & (1 << netID)) == 0){
    OS_TPrintf("受信：ミッション結果：プロフィール未受信の為、受け取らない recv_profile=%d\n", 
      intcomm->recv_profile);
    return;
  }
#endif
  
  if(mresult->mission_data.accept_netid == INTRUDE_NETID_NULL){
    OS_TPrintf("受信：ミッション結果：無効データ\n");
    return;
  }
  if(MISSION_CheckRecvResult(&intcomm->mission) == TRUE){
    OS_TPrintf("受信：ミッション結果：受信済み\n");
    return;
  }
  
  //自分がミッションのターゲットだった
  if(mresult->mission_data.target_info.net_id 
      == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    if(mresult->mission_fail == TRUE){  //ミッション失敗の場合はリストを作り直す
      MISSION_LIST_Create_Type(my_occupy, mresult->mission_data.cdata.type);
      intcomm->send_occupy = TRUE;
    }
    else{ //達成者がいるので、ターゲットだった自分の占拠情報をクリア済みにする
      MISSION_SetMissionClear(gamedata, intcomm, mresult);
      intcomm->send_occupy = TRUE;
    }

    //全占拠達成ならばミッションリストを作成しなおす
    if(MISSION_LIST_Create_Complete(my_occupy, &white_num, &black_num) == TRUE){
      intcomm->send_occupy = TRUE;
      complete = TRUE;
    }

    //占拠結果を達成者に対して送信
    if(mresult->mission_data.monolith_type == MONOLITH_TYPE_BLACK){
      add_black = MISSION_ACHIEVE_ADD_LEVEL;
    }
    else{
      add_white = MISSION_ACHIEVE_ADD_LEVEL;
    }
    if(complete == TRUE){
      add_black += black_num;
      add_white += white_num;
    }
    intcomm->send_occupy_result.add_white = add_white;
    intcomm->send_occupy_result.add_black = add_black;
    intcomm->send_occupy_result.occ = TRUE;
    intcomm->send_occupy_result_send_req = TRUE;
  }

#if 0
  if(MISSION_RecvCheck(&intcomm->mission) == FALSE 
      || MISSION_GetMissionEntry(&intcomm->mission) == FALSE){
    OS_TPrintf("受信：ミッション結果：ミッションに参加していない為、結果を受け取らない\n");
    return;
  }
#endif
  
  OS_TPrintf("受信：ミッション結果\n");
  MISSION_SetResult(intcomm, &intcomm->mission, mresult);
}

//==================================================================
/**
 * データ送信：ミッション結果
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionResult(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_SYSTEM *mission)
{
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  if(GFL_NET_IsInSendCommandQueue(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_MISSION_RESULT) == TRUE){
    return FALSE;
  }
  
  GFL_STD_MemCopy(&mission->result, &intcomm->huge_send_mission_result, sizeof(MISSION_RESULT));
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER,
    INTRUDE_CMD_MISSION_RESULT, sizeof(MISSION_RESULT), &intcomm->huge_send_mission_result,
    FALSE, FALSE, TRUE);
  if(ret == TRUE){
    OS_TPrintf("送信：ミッション結果\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッション達成者がいるかを調べる
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionCheckAchieveUser(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  OS_TPrintf("受信：ミッション達成者確認 netID=%d\n", netID);
  MISSION_Recv_CheckAchieveUser(&intcomm->mission, netID);
}

//==================================================================
/**
 * データ送信：ミッション達成者がいるかを調べる
 *
 * @param   intcomm         
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionCheckAchieveUser(INTRUDE_COMM_SYS_PTR intcomm)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  OS_TPrintf("送信：ミッション達成者確認 \n");
  MISSION_SetRecvBufAnswerAchieveUserBuf(&intcomm->mission, MISSION_ACHIEVE_USER_NULL);
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    GFL_NET_NO_PARENTMACHINE, INTRUDE_CMD_MISSION_CHECK_ACHIEVE_USER, 
    0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：「ミッション達成者がいるか」の返事（いる）
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionAnswerAchieveUserUse(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  OS_TPrintf("受信：ミッション達成者確認の返事：いる\n");
  MISSION_SetRecvBufAnswerAchieveUserBuf(&intcomm->mission, MISSION_ACHIEVE_USER_USE);
}

//==================================================================
/**
 * データ送信：「ミッション達成者がいるか」の返事（いる）
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionAnswerAchieveUserUse(u8 send_netid_bit)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  ret = GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid_bit, INTRUDE_CMD_MISSION_ANSWER_ACHIEVE_USER_USE, 
    0, NULL, FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("送信：ミッション達成者確認の返事：いる\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：「ミッション達成者がいるか」の返事（いない）
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionAnswerAchieveUserNone(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  OS_TPrintf("受信：ミッション達成者確認の返事：いない\n");
  MISSION_SetRecvBufAnswerAchieveUserBuf(&intcomm->mission, MISSION_ACHIEVE_USER_NONE);
}

//==================================================================
/**
 * データ送信：「ミッション達成者がいるか」の返事（いない）
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionAnswerAchieveUserNone(u8 send_netid_bit)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  ret = GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid_bit, INTRUDE_CMD_MISSION_ANSWER_ACHIEVE_USER_NONE, 
    0, NULL, FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("送信：ミッション達成者確認の返事：いない\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：占拠情報
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_OccupyInfo(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const OCCUPY_INFO *cp_occupy = pData;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  OCCUPY_INFO *dest_occupy;
  
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){ //自分のデータは受け取らない
    return;
  }
  if((intcomm->recv_profile & (1 << netID)) == 0){
    OS_TPrintf("受信：占拠情報：プロフィール未受信の為、受け取らない recv_profile=%d\n", 
      intcomm->recv_profile);
    return;
  }
  
  OS_TPrintf("受信：占拠情報 net_id = %d\n", netID);
  dest_occupy = GAMEDATA_GetOccupyInfo(gamedata, netID);
  GFL_STD_MemCopy(cp_occupy, dest_occupy, sizeof(OCCUPY_INFO));
  
  if(netID == intcomm->intrude_status_mine.palace_area){
    intcomm->area_occupy_update = TRUE;
  }
}

//==================================================================
/**
 * データ送信：占拠情報
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_OccupyInfo(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  const OCCUPY_INFO *occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_OCCUPY_INFO, sizeof(OCCUPY_INFO), occupy);
  if(ret == TRUE){
    OS_TPrintf("送信：占拠情報\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：占拠結果
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_OccupyResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_OCCUPY_RESULT *pOccupyResult = pData;
  
  OS_TPrintf("受信：占拠結果 net_id = %d\n", netID);
  GFL_STD_MemCopy(pOccupyResult, &intcomm->recv_occupy_result, sizeof(INTRUDE_OCCUPY_RESULT));
}

//==================================================================
/**
 * データ送信：占拠結果
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_OccupyResult(INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_OCCUPY_RESULT, sizeof(INTRUDE_OCCUPY_RESULT), &intcomm->send_occupy_result);
  if(ret == TRUE){
    OS_TPrintf("送信：占拠結果\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：他人のモノリス画面入室
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_OtherMonolithIn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  intcomm->other_monolith_count++;
  if(intcomm->member_fix == FALSE){
    GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(), FALSE);
    GFL_NET_WL_PauseBeacon(TRUE);
  }
  OS_TPrintf("受信：他人モノリス入室 乱入禁止 num=%d\n", intcomm->other_monolith_count);
}

//==================================================================
/**
 * データ送信：他人のモノリス画面入室
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_OtherMonolithIn(void)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    GFL_NET_NO_PARENTMACHINE, INTRUDE_CMD_OTHER_MONOLITH_IN, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：他人のモノリス画面退室
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_OtherMonolithOut(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  OS_TPrintf("受信：他人モノリス退室 num=%d\n", intcomm->other_monolith_count);

  intcomm->other_monolith_count--;

  //全員がモノリス画面から抜けていて二人専用ミッションが発動していないかチェック
  if(intcomm->other_monolith_count == 0){
    if(intcomm->member_fix == FALSE){
      GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(), TRUE);  //乱入許可
      GFL_NET_WL_PauseBeacon(FALSE);
      OS_TPrintf("乱入許可\n");
    }
  }
}

//==================================================================
/**
 * データ送信：他人のモノリス画面退室
 *
 * @param   intcomm         
 * @param   mission         ミッションデータ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_OtherMonolithOut(void)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    GFL_NET_NO_PARENTMACHINE, INTRUDE_CMD_OTHER_MONOLITH_OUT, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ミッションバトル用のタイミングコマンド(相手指定)
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionBattleTiming(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const u8 *timing_no = pData;
  
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){ //自分のデータは受け取らない
    return;
  }
  
  OS_TPrintf("受信：ミッションバトル用タイミングコマンド netID=%d, timing_no=%d\n", netID, *timing_no);
  GF_ASSERT_MSG((intcomm->mission_battle_timing_bit & (*timing_no)) == 0, 
    "before %d, new %d\n", intcomm->mission_battle_timing_bit, *timing_no);
  
  intcomm->mission_battle_timing_bit |= *timing_no;
}

//==================================================================
/**
 * データ送信：ミッションバトル用のタイミングコマンド(相手指定)
 *
 * @param   intcomm		
 * @param   timing_no		タイミング番号(MISSION_BATTLE_TIMING_BIT)
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionBattleTiming(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid, u8 timing_no)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_TARGET_TIMING, sizeof(timing_no), &timing_no, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：通信相手をサポート
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_PlayerSupport(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const SUPPORT_TYPE *support_type = pData;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  COMM_PLAYER_SUPPORT *cps = GAMEDATA_GetCommPlayerSupportPtr(gamedata);
  
  if((intcomm->recv_profile & (1 << netID)) == 0){
    OS_TPrintf("受信：プレイヤーサポート：プロフィール未受信の為、受け取らない netID=%d\n", netID);
    return;
  }
  
  OS_TPrintf("受信：プレイヤーサポート netID=%d, type=%d\n", netID, *support_type);
  {
    FIELD_STATUS *fldstatus = GAMEDATA_GetFieldStatus(gamedata);
    if(FIELD_STATUS_GetProcAction( fldstatus ) == PROC_ACTION_BATTLE){
      SUPPORT_TYPE now_support = COMM_PLAYER_SUPPORT_GetSupportType(cps);
      if(now_support == SUPPORT_TYPE_NULL || now_support == SUPPORT_TYPE_USED){
        COMM_PLAYER_SUPPORT_SetParam(
          cps, *support_type, GAMEDATA_GetMyStatusPlayer(gamedata, netID));
      }
    }
  }
}

//==================================================================
/**
 * データ送信：通信相手をサポート
 *
 * @param   intcomm		
 * @param   send_netid    サポート相手
 * @param   support_type  サポートタイプ(SUPPORT_TYPE_???)
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_PlayerSupport(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid, SUPPORT_TYPE support_type)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_PLAYER_SUPPORT, sizeof(support_type), &support_type, 
    FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：隠しアイテム
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_SecretItem(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_SECRET_ITEM_SAVE *itemdata = pData;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  
  if((intcomm->recv_profile & (1 << netID)) == 0){
    OS_TPrintf("受信：隠しアイテム：プロフィール未受信の為、受け取らない netID=%d\n", netID);
    return;
  }
  
  OS_TPrintf("受信：隠しアイテム netID=%d\n", netID);
  if(intcomm->recv_secret_item_flag == FALSE){
    GFL_STD_MemCopy(itemdata, &intcomm->recv_secret_item, sizeof(INTRUDE_SECRET_ITEM_SAVE));
    intcomm->recv_secret_item_flag = TRUE;
  }
}

//==================================================================
/**
 * データ送信：隠しアイテム
 *
 * @param   send_netid    サポート相手
 * @param   support_type  サポートタイプ(SUPPORT_TYPE_???)
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_SecretItem(NetID send_netid, const INTRUDE_SECRET_ITEM_SAVE *itemdata)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_SECRET_ITEM, sizeof(INTRUDE_SECRET_ITEM_SAVE), itemdata, 
    FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：Gパワーを装備した
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_GPowerEquip(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const u8 *p_palace_area = pData;
  
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){ //自分のデータは受け取らない
    return;
  }
  if((intcomm->recv_profile & (1 << netID)) == 0){
    OS_TPrintf("受信：GPOWER_EQUIP プロフィール未受信の為、受け取らない netID=%d\n", netID);
    return;
  }
  
  GameCommInfo_MessageEntry_GetPower(intcomm->game_comm, netID, *p_palace_area);
}

//==================================================================
/**
 * データ送信：Gパワーを装備した
 *
 * @param   send_netid    サポート相手
 * @param   support_type  サポートタイプ(SUPPORT_TYPE_???)
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_GPowerEquip(u8 palace_area)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }

  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_GPOWER_EQUIP, sizeof(palace_area), &palace_area);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：WFBCパラメータ要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_WfbcReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  OS_TPrintf("RECIEVE：WFBC要求 net_id=%d\n", netID);
  intcomm->wfbc_req |= 1 << netID;
}

//==================================================================
/**
 * データ送信：WFBCパラメータ要求
 *
 * @param   intcomm		
 * @param   send_netid		要求相手のNetID
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_WfbcReq(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid)
{
  BOOL ret;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  OS_TPrintf("SEND:WFBC要求  send_netid=%d\n", send_netid);
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_WFBC_REQ, 0, NULL, 
    FALSE, FALSE, FALSE);
  if(ret == TRUE){
    intcomm->wfbc_recv = FALSE;
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：WFBCパラメータ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_Wfbc(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const FIELD_WFBC_CORE *wfbc_core = pData;
  FIELD_WFBC_CORE *dest_wfbc;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  
  dest_wfbc = GAMEDATA_GetWFBCCoreData( gamedata, GAMEDATA_WFBC_ID_COMM );
  FIELD_WFBC_CORE_Management( (FIELD_WFBC_CORE*)wfbc_core );  //不正データ書き換え
  GFL_STD_MemCopy(wfbc_core, dest_wfbc, sizeof(FIELD_WFBC_CORE));
  GAMEDATA_SetUpPalaceWFBCCoreData( gamedata, wfbc_core );
  
  GF_ASSERT(intcomm->wfbc_recv == FALSE);
  intcomm->wfbc_recv = TRUE;
  OS_TPrintf("RECEIVE: WFBCパラメータ netID = %d\n", netID);
}

//==================================================================
/**
 * データ送信：WFBCパラメータ
 *
 * @param   intcomm		
 * @param   send_netid_bit		送信先(Bit指定)
 * @param   wfbc_core		      WFBCパラメータへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_Wfbc(INTRUDE_COMM_SYS_PTR intcomm, u32 send_netid_bit, const FIELD_WFBC_CORE *wfbc_core)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  if(GFL_NET_IsInSendCommandQueue(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_WFBC) == TRUE){
    return FALSE;
  }

  GFL_STD_MemCopy(wfbc_core, &intcomm->huge_send_wfbc_core, sizeof(FIELD_WFBC_CORE));

  ret = GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid_bit, INTRUDE_CMD_WFBC, sizeof(FIELD_WFBC_CORE), wfbc_core,
    FALSE, FALSE, TRUE);
  if(ret == TRUE){
    OS_TPrintf("SEND：WFBCパラメータ send_netid_bit = %d\n", send_netid_bit);
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：WFBCパラメータ：FIELD_WFBC_COMM_NPC_ANS
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_WfbcNpcAns(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const FIELD_WFBC_COMM_NPC_ANS *npc_ans = pData;
  
  FIELD_WFBC_COMM_DATA_SetRecvCommAnsData(&intcomm->wfbc_comm_data, npc_ans);
  OS_TPrintf("RECEIVE: WFBCパラメータNPC Ans netID = %d\n", netID);
}

//==================================================================
/**
 * データ送信：WFBCパラメータ：FIELD_WFBC_COMM_NPC_ANS
 *
 * @param   intcomm		
 * @param   send_netid		    送信先
 * @param   wfbc_core		      WFBCパラメータへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_WfbcNpcAns(const FIELD_WFBC_COMM_NPC_ANS *npc_ans, NetID send_netid)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_WFBC_NPC_ANS, sizeof(FIELD_WFBC_COMM_NPC_ANS), npc_ans, 
    FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("SEND：WFBCパラメータ:NPC_ANS send_netid = %d\n", send_netid);
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：WFBCパラメータ：FIELD_WFBC_COMM_NPC_REQ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_WfbcNpcReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const FIELD_WFBC_COMM_NPC_REQ *npc_req = pData;
  
  FIELD_WFBC_COMM_DATA_SetRecvCommReqData(&intcomm->wfbc_comm_data, netID, npc_req);
  OS_TPrintf("RECEIVE: WFBCパラメータNPC Req netID = %d\n", netID);
}

//==================================================================
/**
 * データ送信：WFBCパラメータ：FIELD_WFBC_COMM_NPC_REQ
 *
 * @param   intcomm		
 * @param   send_netid    		送信先
 * @param   wfbc_core		      WFBCパラメータへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_WfbcNpcReq(const FIELD_WFBC_COMM_NPC_REQ *npc_req, NetID send_netid)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  ret = GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_WFBC_NPC_REQ, sizeof(FIELD_WFBC_COMM_NPC_REQ), npc_req, 
    FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("SEND：WFBCパラメータ:NPC_REQ\n");
  }
  return ret;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：シンボルエンカウントデータを要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_SymbolDataReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const SYMBOL_DATA_REQ *p_sdr = pData;
  
  intcomm->req_symbol_data[netID] = *p_sdr;
  MATSUDA_Printf("RECV: symbol_req netID = %d\n", netID);
}

//==================================================================
/**
 * データ送信：シンボルエンカウントデータを要求
 *
 * @param   intcomm		
 * @param   send_netid    		送信先
 * @param   wfbc_core		      WFBCパラメータへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_SymbolDataReq(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid, const SYMBOL_DATA_REQ *p_sdr)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  MATSUDA_Printf("SEND: symbol_req\n");
  
  intcomm->recv_symbol_flag = FALSE;
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_SYMBOL_DATA_REQ, sizeof(SYMBOL_DATA_REQ), p_sdr, 
    FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：シンボルエンカウントデータ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_SymbolData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const INTRUDE_SYMBOL_WORK *p_symboldata = pData;
  
  intcomm->intrude_symbol = *p_symboldata;
  intcomm->recv_symbol_flag = TRUE;
  intcomm->recv_symbol_change_flag = FALSE;
  MATSUDA_Printf("RECV: symbol_data netID = %d\n", netID);
}

//==================================================================
/**
 * データ送信：シンボルエンカウントデータ
 *
 * @param   intcomm		
 * @param   send_netid_bit		送信先
 * @param   wfbc_core		      WFBCパラメータへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_SymbolData(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid)
{
  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  MATSUDA_Printf("SEND: symbol_data net_id\n", send_netid);
  
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid, INTRUDE_CMD_SYMBOL_DATA, sizeof(INTRUDE_SYMBOL_WORK), 
    &intcomm->intrude_send_symbol, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：自分のシンボルエンカウントデータが変更した事を知らせる
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_SymbolDataChange(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const SYMBOL_DATA_CHANGE *p_sdc = pData;
  const INTRUDE_SYMBOL_WORK *now_symbol = &intcomm->intrude_symbol;
  GAMEDATA *gamedata = GameCommSys_GetGameData(intcomm->game_comm);
  
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){ //自分のデータは受け取らない
    return;
  }

  MATSUDA_Printf("RECV: symbol_change net_id=%d ", netID);
  if(intcomm->intrude_status_mine.palace_area != netID){
//  if(now_symbol->net_id != netID){// || now_symbol->symbol_map_id != p_sdc->symbol_map_id){
    MATSUDA_Printf("侵入先と違うNetIDの為無視\n");
    return;
  }
  if(ZONEDATA_IsBingo( PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata)) ) == FALSE){
    MATSUDA_Printf("ビンゴマップにいないので無視\n");
    return;
  }
  MATSUDA_Printf("\n");
  intcomm->recv_symbol_change_flag = TRUE;
}

//==================================================================
/**
 * データ送信：自分のシンボルエンカウントデータが変更した事を知らせる
 *
 * @param   intcomm		
 * @param   send_netid_bit		送信先(Bit指定)
 * @param   wfbc_core		      WFBCパラメータへのポインタ
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_SymbolDataChange(const SYMBOL_DATA_CHANGE *p_sdc)
{
  BOOL ret;

  if(_OtherPlayerExistence() == FALSE){
    return TRUE;
  }
  
  MATSUDA_Printf("SEND: symbol_change\n");
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_SYMBOL_DATA_CHANGE, sizeof(SYMBOL_DATA_CHANGE), p_sdc);
}
