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
#include "bingo_system.h"
#include "intrude_main.h"
#include "intrude_mission.h"


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);
static void _IntrudeRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MemberNum(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_ProfileReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Profile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_DeleteProfile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_PlayerStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Talk(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_BingoIntrusion(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_BingoIntrusionAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_ReqBingoIntrusionParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_BingoIntrusionParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionListReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionList(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionEntryAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _MissionOrderConfirm(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionAchieve(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionAchieveAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_MissionResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_OccupyInfo(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TargetTiming(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_PlayerSupport(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_WfbcReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Wfbc(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※INTRUDE_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable Intrude_CommPacketTbl[] = {
  {_IntrudeRecv_Shutdown, NULL},               //INTRUDE_CMD_SHUTDOWN
  {_IntrudeRecv_MemberNum, NULL},              //INTRUDE_CMD_MEMBER_NUM
  {_IntrudeRecv_ProfileReq, NULL},             //INTRUDE_CMD_PROFILE_REQ
  {_IntrudeRecv_Profile, NULL},                //INTRUDE_CMD_PROFILE
  {_IntrudeRecv_DeleteProfile, NULL},          //INTRUDE_CMD_DELETE_PROFILE
  {_IntrudeRecv_PlayerStatus, NULL},           //INTRUDE_CMD_PLAYER_STATUS
  {_IntrudeRecv_Talk, NULL},                   //INTRUDE_CMD_TALK
  {_IntrudeRecv_TalkAnswer, NULL},             //INTRUDE_CMD_TALK_ANSWER
  {_IntrudeRecv_TalkCancel, NULL},             //INTRUDE_CMD_TALK_CANCEL
  {_IntrudeRecv_BingoIntrusion, NULL},         //INTRUDE_CMD_BINGO_INTRUSION
  {_IntrudeRecv_BingoIntrusionAnswer, NULL},   //INTRUDE_CMD_BINGO_INTRUSION_ANSWER
  {_IntrudeRecv_ReqBingoIntrusionParam, NULL}, //INTRUDE_CMD_REQ_BINGO_INTRUSION_PARAM
  {_IntrudeRecv_BingoIntrusionParam, NULL},    //INTRUDE_CMD_BINGO_INTRUSION_PARAM
  {_IntrudeRecv_MissionListReq, NULL},         //INTRUDE_CMD_MISSION_LIST_REQ
  {_IntrudeRecv_MissionList, NULL},            //INTRUDE_CMD_MISSION_LIST
  {_MissionOrderConfirm, NULL},                //INTRUDE_CMD_MISSION_ORDER_CONFIRM
  {_IntrudeRecv_MissionEntryAnswer, NULL},     //INTRUDE_CMD_MISSION_ENTRY_ANSWER
  {_IntrudeRecv_MissionReq, NULL},             //INTRUDE_CMD_MISSION_REQ
  {_IntrudeRecv_MissionData, NULL},            //INTRUDE_CMD_MISSION_DATA
  {_IntrudeRecv_MissionAchieve, NULL},         //INTRUDE_CMD_MISSION_ACHIEVE
  {_IntrudeRecv_MissionAchieveAnswer, NULL},   //INTRUDE_CMD_MISSION_ACHIEVE_ANSWER
  {_IntrudeRecv_MissionResult, NULL},          //INTRUDE_CMD_MISSION_RESULT
  {_IntrudeRecv_OccupyInfo, NULL},             //INTRUDE_CMD_OCCUPY_INFO
  {_IntrudeRecv_TargetTiming, NULL},           //INTRUDE_CMD_TARGET_TIMING
  {_IntrudeRecv_PlayerSupport, NULL},          //INTRUDE_CMD_PLAYER_SUPPORT
  {_IntrudeRecv_WfbcReq, NULL},                //INTRUDE_CMD_WFBC_REQ
  {_IntrudeRecv_Wfbc, NULL},                   //INTRUDE_CMD_WFBC
};
SDK_COMPILER_ASSERT(NELEMS(Intrude_CommPacketTbl) == INTRUDE_CMD_NUM);



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * 巨大データ用受信バッファ取得
 *
 * @param   netID		
 * @param   pWork		
 * @param   size		
 *
 * @retval  u8 *		
 */
//--------------------------------------------------------------
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size)
{
#if 0 //※check　まだ巨大データ用意していない
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  GF_ASSERT_MSG(size <= UNION_HUGE_RECEIVE_BUF_SIZE, "size=%x, recv_size=%x\n", size, UNION_HUGE_RECEIVE_BUF_SIZE);
	return intcomm->huge_receive_buf[netID];
#else
  GF_ASSERT(0);
  return NULL;
#endif
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
  const u8 *member_num = pData;
  
	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;   //自分のデータは受け取らない
  }
  intcomm->member_num = *member_num;
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
  
  if(netID >= intcomm->member_num){
    //管理している人数よりも大きいIDから来た物は受け取らない。
    //先に管理人数を受信してからでないとマップの繋がりが拡大できない。よって表示位置も不明になる
    OS_TPrintf("PROFILE RECV 管理人数よりも大きいIDの為、無視 net_id=%d, member_num=%d\n", netID, intcomm->member_num);
    return;
  }
  Intrude_SetProfile(intcomm, netID, recv_profile);
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

  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_PROFILE, sizeof(INTRUDE_PROFILE), &intcomm->send_profile);
  if(ret == TRUE){
    intcomm->profile_req = FALSE;
    OS_TPrintf("自分プロフィール送信\n");
    if(GFL_NET_IsParentMachine() == TRUE){
      //親の場合、プロフィールを要求されていたという事は途中参加者がいるので
      //現在の参加人数、ミッションも送信する
      intcomm->member_send_req = TRUE;
      MISSION_Set_DataSendReq(&intcomm->mission);
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
      recv_data->palace_area);
    return;
  }
  if((intcomm->recv_profile & (1 << netID)) == 0){  //プロフィール未受信は受け取らない
    return;   
  }
  Intrude_SetPlayerStatus(intcomm, netID, recv_data);
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

  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_PLAYER_STATUS, sizeof(INTRUDE_STATUS), send_status);
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
  if(Intrude_SetTalkReq(intcomm, netID) == TRUE){
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
BOOL IntrudeSend_Talk(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id)
{
  INTRUDE_TALK_FIRST_ATTACK first_attack;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }
  
  GFL_STD_MemClear(&first_attack, sizeof(INTRUDE_TALK_FIRST_ATTACK));
  if(MISSION_CheckMissionTargetNetID(&intcomm->mission, send_net_id) == TRUE){
    first_attack.talk_type = INTRUDE_TALK_TYPE_MISSION;
    first_attack.mdata = *(MISSION_GetRecvData(&intcomm->mission));
  }
  else{
    first_attack.talk_type = INTRUDE_TALK_TYPE_NORMAL;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK, sizeof(INTRUDE_TALK_FIRST_ATTACK), &first_attack, FALSE, FALSE, FALSE);
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
  const INTRUDE_TALK_STATUS *answer = pData;
  
  OS_TPrintf("話しかけの返事受信　net_id=%d, answer=%d\n", netID, *answer);
  Intrude_SetTalkAnswer(intcomm, netID, *answer);
}

//==================================================================
/**
 * データ送信：話しかけられたので自分の状況を返事として返す
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int send_net_id, INTRUDE_TALK_STATUS answer)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_ANSWER, sizeof(INTRUDE_TALK_STATUS), &answer, FALSE, FALSE, FALSE);
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
  
  OS_TPrintf("話しかけキャンセル受信　net_id=%d\n", netID);
  Intrude_SetTalkCancel(intcomm, netID);
}

//==================================================================
/**
 * データ送信：話しかけをキャンセル
 * @param   send_net_id		話しかけ先のNetID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TalkCancel(int send_net_id)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_CANCEL, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ビンゴバトル乱入許可要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_BingoIntrusion(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  BINGO_SYSTEM *bingo = Bingo_GetBingoSystemWork(intcomm);
  
  OS_TPrintf("乱入許可要求コマンド受信 net_id=%d\n", netID);
  Bingo_Req_IntrusionPlayer(intcomm, bingo, netID);
}

//==================================================================
/**
 * データ送信：ビンゴバトル乱入許可要求
 * @param   send_net_id		送信先のネットID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_BingoIntrusion(int send_net_id)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_BINGO_INTRUSION, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ビンゴバトル乱入許可要求の返事
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_BingoIntrusionAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  BINGO_SYSTEM *bingo = Bingo_GetBingoSystemWork(intcomm);
  const BINGO_INTRUSION_ANSWER *answer = pData;
  
  bingo->intrusion_recv_answer = *answer;
  OS_TPrintf("乱入の返事受信 answer=%d\n", *answer);
}

//==================================================================
/**
 * データ送信：ビンゴバトル乱入許可要求の返事
 *
 * @param   send_net_id		送信先のネットID
 * @param   answer		    返事
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_BingoIntrusionAnswer(int send_net_id, BINGO_INTRUSION_ANSWER answer)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_BINGO_INTRUSION_ANSWER, sizeof(BINGO_INTRUSION_ANSWER), 
    &answer, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ビンゴバトル乱入用のパラメータ要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_ReqBingoIntrusionParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  BINGO_SYSTEM *bingo = Bingo_GetBingoSystemWork(intcomm);
  
  bingo->param_req_bit |= 1 << netID;
  OS_TPrintf("ビンゴバトル乱入パラメータ要求受信 net_id = %d\n", netID);
}

//==================================================================
/**
 * データ送信：ビンゴバトル乱入用のパラメータ要求
 * @param   send_net_id		送信先のネットID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_ReqBingoBattleIntrusionParam(int send_net_id)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_REQ_BINGO_INTRUSION_PARAM, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ビンゴバトル乱入用パラメータ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_BingoIntrusionParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  BINGO_SYSTEM *bingo = Bingo_GetBingoSystemWork(intcomm);
  const BINGO_INTRUSION_PARAM *inpara = pData;
  
  GFL_STD_MemCopy(inpara, &bingo->intrusion_param, sizeof(BINGO_INTRUSION_PARAM));
  OS_TPrintf("ビンゴバトル乱入パラメータ受信 net_id = %d\n", netID);
}

//==================================================================
/**
 * データ送信：ビンゴバトル乱入用のパラメータ送信
 * @param   send_net_id		送信先のネットID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_BingoBattleIntrusionParam(BINGO_SYSTEM *bingo, int send_net_id)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_BINGO_INTRUSION_PARAM, sizeof(BINGO_INTRUSION_PARAM), &bingo->intrusion_param, 
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
 * @param   monolith_type		MONOLITH_TYPE_???
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
  
  MISSION_SetMissionList(&intcomm->mission, mlist);
  OS_TPrintf("RECEIVE: ミッションリスト palace_area = %d\n", mlist->md[0].palace_area);
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
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_LIST, sizeof(MISSION_CHOICE_LIST), &mission->list[palace_area]);
  if(ret == TRUE){
    OS_TPrintf("SEND：ミッションリスト palace_area=%d\n", palace_area);
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
  const MISSION_DATA *mdata = pData;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  OS_TPrintf("受信：ミッション受注します net_id=%d\n", netID);
  MISSION_SetEntryNew(intcomm, &intcomm->mission, mdata, netID);
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
BOOL IntrudeSend_MissionOrderConfirm(INTRUDE_COMM_SYS_PTR intcomm, const MISSION_DATA *mdata)
{
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  MISSION_ClearRecvEntryAnswer(&intcomm->mission);

  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_ORDER_CONFIRM, sizeof(MISSION_DATA), mdata);
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
 * @brief   コマンド受信：ミッション要求
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_MissionReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const MISSION_REQ *req = pData;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return;
  }
  
  OS_TPrintf("受信：ミッション要求 net_id=%d\n", netID);
  MISSION_SetEntry(intcomm, &intcomm->mission, req, netID);
  //要求されたからには今のミッションを返してあげる。発動後の途中乱入かもしれないので。
  MISSION_Set_DataSendReq(&intcomm->mission);
}

//==================================================================
/**
 * データ送信：ミッション要求
 *
 * @param   intcomm         
 * @param   monolith_type		MONOLITH_TYPE_???
 * @param   zone_id		      ミニモノリスがあるゾーンID
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_MissionReq(INTRUDE_COMM_SYS_PTR intcomm, int monolith_type, u16 zone_id)
{
  MISSION_REQ req;
  
  if(_OtherPlayerExistence() == FALSE){
    return FALSE;
  }

  req.monolith_type = monolith_type;
  req.zone_id = zone_id;
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_REQ, sizeof(MISSION_REQ), &req);
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
  
  if(ZONEDATA_IsPalaceField(zone_id) == FALSE && ZONEDATA_IsPalace(zone_id) == FALSE){
    OS_TPrintf("ミッション受信：表フィールドにいるため、受け取らない\n");
    return;
  }
  
  new_mission = MISSION_SetMissionData(&intcomm->mission, mdata);
  if(new_mission == TRUE){  //連続受信で上書きされないように直接代入はしない
    intcomm->new_mission_recv = TRUE;
    GameCommInfo_MessageEntry_Mission(intcomm->game_comm, mdata->accept_netid);
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
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_DATA, sizeof(MISSION_DATA), &mission->data);
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
  
  if((intcomm->recv_profile & (1 << netID)) == 0){
    OS_TPrintf("受信：ミッション結果：プロフィール未受信の為、受け取らない recv_profile=%d\n", 
      intcomm->recv_profile);
    return;
  }
  
  if(MISSION_RecvCheck(&intcomm->mission) == FALSE){
    OS_TPrintf("受信：ミッション結果：ミッションを受信していない為、結果も受け取らない\n");
    return;
  }

  OS_TPrintf("受信：ミッション結果\n");
  MISSION_SetResult(&intcomm->mission, mresult);
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

  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_MISSION_RESULT, sizeof(MISSION_RESULT), &mission->result);
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
 * @brief   コマンド受信：相手指定のタイミングコマンド
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _IntrudeRecv_TargetTiming(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  const u8 *timing_no = pData;
  
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){ //自分のデータは受け取らない
    return;
  }
  
  OS_TPrintf("受信：相手指定タイミングコマンド netID=%d, timing_no=%d\n", netID, *timing_no);
  GF_ASSERT_MSG(intcomm->recv_target_timing_no == 0, 
    "before %d, new %d\n", intcomm->recv_target_timing_no, *timing_no);
  
  intcomm->recv_target_timing_no = *timing_no;
  intcomm->recv_target_timing_netid = netID;
}

//==================================================================
/**
 * データ送信：相手指定のタイミングコマンド
 *
 * @param   intcomm		
 * @param   timing_no		タイミング番号
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_TargetTiming(INTRUDE_COMM_SYS_PTR intcomm, NetID send_netid, u8 timing_no)
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
  COMM_PLAYER_SUPPORT_SetParam(cps, *support_type, GAMEDATA_GetMyStatusPlayer(gamedata, netID));
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
  
  ret = GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), 
    send_netid_bit, INTRUDE_CMD_WFBC, sizeof(FIELD_WFBC_CORE), wfbc_core,
    FALSE, FALSE, FALSE);
  if(ret == TRUE){
    OS_TPrintf("SEND：WFBCパラメータ send_netid_bit = %d\n", send_netid_bit);
  }
  return ret;
}
