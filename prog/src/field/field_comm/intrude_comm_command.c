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


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);
static void _IntrudeRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_ProfileReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Profile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_PlayerStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Talk(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_TalkAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_BingoIntrusion(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_BingoIntrusionAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_ReqBingoIntrusionParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_BingoIntrusionParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※INTRUDE_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable Intrude_CommPacketTbl[] = {
  {_IntrudeRecv_Shutdown, NULL},               //INTRUDE_CMD_SHUTDOWN
  {_IntrudeRecv_ProfileReq, NULL},             //INTRUDE_CMD_PROFILE_REQ
  {_IntrudeRecv_Profile, NULL},                //INTRUDE_CMD_PROFILE
  {_IntrudeRecv_PlayerStatus, NULL},           //INTRUDE_CMD_PLAYER_STATUS
  {_IntrudeRecv_Talk, NULL},                   //INTRUDE_CMD_TALK
  {_IntrudeRecv_TalkAnswer, NULL},             //INTRUDE_CMD_TALK_ANSWER
  {_IntrudeRecv_BingoIntrusion, NULL},         //INTRUDE_CMD_BINGO_INTRUSION
  {_IntrudeRecv_BingoIntrusionAnswer, NULL},   //INTRUDE_CMD_BINGO_INTRUSION_ANSWER
  {_IntrudeRecv_ReqBingoIntrusionParam, NULL}, //INTRUDE_CMD_REQ_BINGO_INTRUSION_PARAM
  {_IntrudeRecv_BingoIntrusionParam, NULL},    //INTRUDE_CMD_BINGO_INTRUSION_PARAM
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
  OS_TPrintf("切断コマンド受信\n");
}

//==================================================================
/**
 * データ送信：切断
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_Shutdown(void)
{
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_SHUTDOWN, 0, NULL);
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
  MYSTATUS *myst;
  OCCUPY_INFO *occupy;
  
  myst = GAMEDATA_GetMyStatusPlayer(gamedata, netID);
  MyStatus_Copy(&recv_profile->mystatus, myst);

  occupy = GAMEDATA_GetOccupyInfo(gamedata, netID);
  GFL_STD_MemCopy(&recv_profile->occupy, occupy, sizeof(OCCUPY_INFO));
  
  _IntrudeRecv_PlayerStatus(netID, size, &recv_profile->status, pWork, pNetHandle);
  
  intcomm->recv_profile |= 1 << netID;
  OS_TPrintf("プロフィール受信　net_id=%d, recv_bit=%d\n", netID, intcomm->recv_profile);
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
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_PROFILE, sizeof(INTRUDE_PROFILE), &intcomm->send_profile);
  if(ret == TRUE){
    intcomm->profile_req = FALSE;
    OS_TPrintf("自分プロフィール送信\n");
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
  INTRUDE_STATUS *target_status;
  int mission_no;
  
	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;   //自分のデータは受け取らない
  }
  target_status = &intcomm->intrude_status[netID];
  GFL_STD_MemCopy(recv_data, target_status, sizeof(INTRUDE_STATUS));

  //プレイヤーステータスにデータセット　※game_commに持たせているのを変えるかも
  if(netID == 0){
    mission_no = target_status->mission_no;
  }
  else{
    mission_no = GameCommStatus_GetPlayerStatus_MissionNo(intcomm->game_comm, 0);
    //mission_no = GameCommStatus_GetPlayerStatus_MissionNo(game_comm, GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()));
  }
  GameCommStatus_SetPlayerStatus(intcomm->game_comm, netID, target_status->zone_id,
    target_status->palace_area, target_status->zone_id);

  {//ゾーン違いによるアクター表示・非表示設定
    PLAYER_WORK *my_player = GAMEDATA_GetMyPlayerWork(GameCommSys_GetGameData(intcomm->game_comm));
    ZONEID my_zone_id = PLAYERWORK_getZoneID( my_player );
    
    if(target_status->zone_id != my_zone_id){
      target_status->player_pack.vanish = TRUE;   //違うゾーンにいるので非表示
    }
    else if(ZONEDATA_IsPalace(target_status->zone_id) == FALSE
        && target_status->palace_area != intcomm->intrude_status_mine.palace_area){
      //通常フィールドの同じゾーンに居ても、侵入先ROMが違うなら非表示
      target_status->player_pack.vanish = TRUE;
    }
    else{
      target_status->player_pack.vanish = FALSE;
    }
  }
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
  
  OS_TPrintf("話しかけられた受信　net_id=%d\n", netID);
  Intrude_SetTalkReq(intcomm, netID);
}

//==================================================================
/**
 * データ送信：話しかける
 * @param   send_net_id		話しかけ先のNetID
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_Talk(int send_net_id)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK, 0, NULL, FALSE, FALSE, NULL);
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
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_TALK_ANSWER, sizeof(INTRUDE_TALK_STATUS), &answer, FALSE, FALSE, NULL);
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
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_BINGO_INTRUSION, 0, NULL, FALSE, FALSE, NULL);
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
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_BINGO_INTRUSION_ANSWER, sizeof(BINGO_INTRUSION_ANSWER), 
    &answer, FALSE, FALSE, NULL);
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
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_REQ_BINGO_INTRUSION_PARAM, 0, NULL, FALSE, FALSE, NULL);
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
  
  GFL_STD_MemCopy(inpara, &bingo->intrusion_param, size);
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
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    INTRUDE_CMD_BINGO_INTRUSION_PARAM, sizeof(BINGO_INTRUSION_PARAM), &bingo->intrusion_param, 
    FALSE, FALSE, NULL);
}

