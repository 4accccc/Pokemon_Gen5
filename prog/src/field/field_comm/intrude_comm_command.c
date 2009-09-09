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
#include "fieldmap/zone_id.h"


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);
static void _IntrudeRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_ProfileReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_Profile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _IntrudeRecv_PlayerStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※INTRUDE_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable Intrude_CommPacketTbl[] = {
  {_IntrudeRecv_Shutdown, NULL},               //INTRUDE_CMD_SHUTDOWN
  {_IntrudeRecv_ProfileReq, NULL},             //INTRUDE_CMD_PROFILE_REQ
  {_IntrudeRecv_Profile, NULL},                //INTRUDE_CMD_PROFILE
  {_IntrudeRecv_PlayerStatus, NULL},           //INTRUDE_CMD_PLAYER_STATUS
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
  MYSTATUS *myst;
  
  myst = GAMEDATA_GetMyStatusPlayer(GameCommSys_GetGameData(intcomm->game_comm), netID);
  MyStatus_Copy(pData, myst);

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
  MYSTATUS *myst;
  
  myst = GAMEDATA_GetMyStatus(GameCommSys_GetGameData(intcomm->game_comm));
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_PROFILE, MyStatus_GetWorkSize(), myst);
  if(ret == TRUE){
    intcomm->profile_req = FALSE;
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
  intcomm->recv_status |= 1 << netID;
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
    else if(target_status->zone_id != ZONE_ID_PALACETEST 
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
 * @param   palace_area		パレスエリア
 * @param   mission_no		ミッション番号
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL IntrudeSend_PlayerStatus(INTRUDE_COMM_SYS_PTR intcomm, GAMEDATA *gamedata, INTRUDE_STATUS *send_status, int palace_area, int mission_no)
{
  BOOL ret;
  PLAYER_WORK *plWork = NULL;
  ZONEID zone_id;
  
  plWork = GAMEDATA_GetMyPlayerWork( gamedata );
  zone_id = PLAYERWORK_getZoneID( plWork );
  
  send_status->zone_id = zone_id;
  send_status->palace_area = palace_area;
  send_status->mission_no = mission_no;
  
  ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    INTRUDE_CMD_PLAYER_STATUS, sizeof(INTRUDE_STATUS), send_status);
  if(ret == TRUE){
    intcomm->send_status = FALSE;
  }
  return ret;
}

