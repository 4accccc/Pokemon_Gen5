//==============================================================================
/**
 * @file    union_comm_command.c
 * @brief   コロシアム：通信コマンド
 * @author  matsuda
 * @date    2009.07.14(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "union_local.h"
#include "savedata/mystatus.h"
#include "net_app/union/union_main.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "colosseum_comm_command.h"
#include "colosseum.h"


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);
static void _ColosseumRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_BasicStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_TrainerCard(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_PosPackage(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_StandingPositionConfirm(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_AnswerStandingPosition(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_Pokeparty(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_StandingPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_Leave(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_BattleReady(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_BattleReadyCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_BattleReadyCancelOK(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_AllBattleReady(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_PokeListSelected(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※COLOSSEUM_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable Colosseum_CommPacketTbl[] = {
  {_ColosseumRecv_Shutdown, NULL},                  //COLOSSEUM_CMD_SHUTDOWN
  {_ColosseumRecv_BasicStatus, _RecvHugeBuffer},    //COLOSSEUM_CMD_BASIC_STATUS
  {_ColosseumRecv_TrainerCard, _RecvHugeBuffer},    //COLOSSEUM_CMD_TRAINERCARD
  {_ColosseumRecv_PosPackage, _RecvHugeBuffer},     //COLOSSEUM_CMD_POS_PACKAGE
  {_ColosseumRecv_StandingPositionConfirm, NULL},   //COLOSSEUM_CMD_STANDPOS_CONFIRM
  {_ColosseumRecv_AnswerStandingPosition, NULL},    //COLOSSEUM_CMD_ANSWER_STANDPOS
  {_ColosseumRecv_Pokeparty, _RecvHugeBuffer},      //COLOSSEUM_CMD_POKEPARTY
  {_ColosseumRecv_StandingPos, NULL},               //COLOSSEUM_CMD_STANDING_POS
  {_ColosseumRecv_BattleReady, NULL},               //COLOSSEUM_CMD_BATTLE_READY
  {_ColosseumRecv_BattleReadyCancel, NULL},         //COLOSSEUM_CMD_BATTLE_READY_CANCEL
  {_ColosseumRecv_BattleReadyCancelOK, NULL},       //COLOSSEUM_CMD_BATTLE_READY_CANCEL_OK
  {_ColosseumRecv_AllBattleReady, NULL},            //COLOSSEUM_CMD_ALL_BATTLE_READY
  {_ColosseumRecv_PokeListSelected, NULL},          //COLOSSEUM_CMD_POKELIST_SELECTED
  {_ColosseumRecv_Leave, NULL},                     //COLOSSEUM_CMD_LEAVE
};
SDK_COMPILER_ASSERT(NELEMS(Colosseum_CommPacketTbl) == COLOSSEUM_CMD_NUM);


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * コロシアム用の通信コマンドテーブルを追加する
 *
 * @param   unisys		
 */
//==================================================================
void Colosseum_AddCommandTable(UNION_SYSTEM_PTR unisys)
{
  GFL_NET_AddCommandTable(
    GFL_NET_CMD_COLOSSEUM, Colosseum_CommPacketTbl, NELEMS(Colosseum_CommPacketTbl), unisys);
}

//==================================================================
/**
 * コロシアム用の通信コマンドテーブルを削除する
 */
//==================================================================
void Colosseum_DelCommandTable(void)
{
  GFL_NET_DelCommandTable(GFL_NET_CMD_COLOSSEUM);
}


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
  UNION_SYSTEM_PTR unisys = pWork;
  GF_ASSERT_MSG(size <= UNION_HUGE_RECEIVE_BUF_SIZE, "size=%x, recv_size=%x\n", size, UNION_HUGE_RECEIVE_BUF_SIZE);
	return unisys->huge_receive_buf[netID];
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
static void _ColosseumRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  ;
}

//==================================================================
/**
 * データ送信：切断
 * @param   select_list		選択結果(UNION_MSG_MENU_SELECT_???)
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_Shutdown(COMM_PLAYER_PACKAGE *pos_package)
{
  GF_ASSERT(0); //未作成
  return 0;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：基本情報
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_BasicStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return; //自分のデータなので無視
  }
  if(clsys == NULL || clsys->cps == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  OS_TPrintf("コロシアム：基本情報受信：net_id = %d\n", netID);
  GFL_STD_MemCopy(pData, &clsys->basic_status[netID], size);
}

//==================================================================
/**
 * データ送信：基本情報
 * @param   select_list		選択結果(UNION_MSG_MENU_SELECT_???)
 * @param   parent_only   TRUE:親のみに送信。　FALSE:全員に送信
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_BasicStatus(COLOSSEUM_BASIC_STATUS *basic_status, BOOL parent_only)
{
  NetID send_id;
  
  send_id = (parent_only == TRUE) ? GFL_NET_NO_PARENTMACHINE : GFL_NET_SENDID_ALLUSER;
  OS_TPrintf("コロシアム：基本情報送信：send_id = %d\n", send_id);
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_id, 
    COLOSSEUM_CMD_BASIC_STATUS, sizeof(COLOSSEUM_BASIC_STATUS), 
    basic_status, TRUE, FALSE, TRUE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：トレーナーカード
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_TrainerCard(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  COMM_PLAYER_SYS_PTR cps;

	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return; //自分のデータなので無視
  }
  if(clsys == NULL || clsys->cps == NULL || clsys->recvbuf.tr_card[netID] == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  OS_TPrintf("コロシアム：トレーナーカード受信 net_id = %d\n", netID);
  GFL_STD_MemCopy(pData, clsys->recvbuf.tr_card[netID], size);
  clsys->recvbuf.tr_card_occ[netID] = TRUE;
}

//==================================================================
/**
 * データ送信：座標パッケージ
 * @param   select_list		選択結果(UNION_MSG_MENU_SELECT_???)
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_TrainerCard(TR_CARD_DATA *send_card)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER, 
    COLOSSEUM_CMD_TRAINERCARD, sizeof(TR_CARD_DATA), send_card, TRUE, FALSE, TRUE);
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：座標パッケージ
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_PosPackage(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return; //自分のデータなので無視
  }
  if(clsys == NULL || clsys->cps == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  Colosseum_SetCommPlayerPos(clsys, netID, pData);
}

//==================================================================
/**
 * データ送信：座標パッケージ
 * @param   select_list		選択結果(UNION_MSG_MENU_SELECT_???)
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_PosPackage(COMM_PLAYER_PACKAGE *pos_package)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER, 
    COLOSSEUM_CMD_POS_PACKAGE, sizeof(COMM_PLAYER_PACKAGE), 
    pos_package, FALSE, TRUE, TRUE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：親に自分の今の立ち位置を使っても問題ないか確認する
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_StandingPositionConfirm(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  COMM_PLAYER_SYS_PTR cps;
  const u8 *stand_pos = pData;

  if(clsys == NULL || clsys->cps == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  cps = clsys->cps;
  
  if(GFL_NET_IsParentMachine() == FALSE){
    return; //親機以外には送信していないはずだけどここでも一応チェック
  }
  
  OS_TPrintf("受信：立ち位置確認 net_id=%d, stand_pos=%d\n", netID, *stand_pos);
  Colosseum_Parent_SetStandingPosition(clsys, netID, *stand_pos);
}

//==================================================================
/**
 * データ送信：親に自分の今の立ち位置を使っても問題ないか確認する
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_StandingPositionConfirm(COLOSSEUM_SYSTEM_PTR clsys)
{
  u8 stand_pos;
  
  stand_pos = Colosseum_Mine_GetStandingPostion(clsys);

  //親機にだけ送信
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_NO_PARENTMACHINE, 
    COLOSSEUM_CMD_STANDPOS_CONFIRM, sizeof(stand_pos), 
    &stand_pos, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：立ち位置使用結果の返事
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_AnswerStandingPosition(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  COMM_PLAYER_SYS_PTR cps;
  const BOOL *result = pData;

  if(clsys == NULL || clsys->cps == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  cps = clsys->cps;
  
  OS_TPrintf("受信：立ち位置の返事 result=%d\n", *result);
  Colosseum_Mine_SetAnswerStandingPosition(clsys, *result);
}

//==================================================================
/**
 * データ送信：立ち位置使用結果の返事を送信
 *
 * @param   clsys		
 * @param   send_net_id		送信先のnetID
 * @param   result		    結果
 *
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_AnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys, int send_net_id, BOOL result)
{
  u8 stand_pos;
  
  stand_pos = Colosseum_Mine_GetStandingPostion(clsys);

  //対象者にだけ送信
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_net_id, 
    COLOSSEUM_CMD_ANSWER_STANDPOS, sizeof(BOOL), &result, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：POKEPARTY
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_Pokeparty(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL || clsys->recvbuf.pokeparty[netID] == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  OS_TPrintf("コロシアム：POKEPARTY受信：net_id = %d\n", netID);
  GFL_STD_MemCopy(pData, clsys->recvbuf.pokeparty[netID], size);
  clsys->recvbuf.pokeparty_occ[netID] = TRUE;
}

//==================================================================
/**
 * データ送信：POKEPARTY
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_Pokeparty(POKEPARTY *pokeparty)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER, 
    COLOSSEUM_CMD_POKEPARTY, sizeof(PokeParty_GetWorkSize()), 
    pokeparty, TRUE, FALSE, TRUE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：全員分の立ち位置
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_StandingPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  OS_TPrintf("コロシアム：立ち位置受信：net_id = %d\n", netID);
  GFL_STD_MemCopy(pData, clsys->recvbuf.stand_position, size);
  clsys->recvbuf.stand_position_occ = TRUE;
}

//==================================================================
/**
 * データ送信：全員分の立ち位置
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_StandingPos(u8 *standing_pos)
{
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    COLOSSEUM_CMD_STANDING_POS, sizeof(u8) * COLOSSEUM_MEMBER_MAX, standing_pos);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：戦闘準備完了
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_BattleReady(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  OS_TPrintf("コロシアム：戦闘準備完了：net_id = %d\n", netID);
  clsys->parentsys.battle_ready[netID] = TRUE;
}

//==================================================================
/**
 * データ送信：戦闘準備完了
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_BattleReady(void)
{
  //親機にだけ送信
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_NO_PARENTMACHINE, 
    COLOSSEUM_CMD_BATTLE_READY, 0, NULL, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：戦闘準備キャンセル
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_BattleReadyCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  clsys->parentsys.battle_ready_cancel[netID] = TRUE;
  clsys->parentsys.battle_ready[netID] = FALSE;
  OS_TPrintf("コロシアム：戦闘準備完了キャンセル受信：net_id = %d\n", netID);
}

//==================================================================
/**
 * データ送信：戦闘準備キャンセル
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_BattleReadyCancel(void)
{
  //親機にだけ送信
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_NO_PARENTMACHINE, 
    COLOSSEUM_CMD_BATTLE_READY_CANCEL, 0, NULL, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：戦闘準備キャンセルOK
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_BattleReadyCancelOK(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  clsys->mine.battle_ready = FALSE;
  OS_TPrintf("コロシアム：戦闘準備完了キャンセルOK受信：net_id = %d\n", netID);
}

//==================================================================
/**
 * データ送信：戦闘準備キャンセルOK
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_BattleReadyCancelOK(int send_netid)
{
  //対象の子にだけ送信
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_netid, 
    COLOSSEUM_CMD_BATTLE_READY_CANCEL_OK, 0, NULL, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：全員戦闘準備完了
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_AllBattleReady(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  clsys->all_battle_ready = TRUE;
  OS_TPrintf("コロシアム：全員戦闘準備完了受信：net_id = %d\n", netID);
}

//==================================================================
/**
 * データ送信：全員戦闘準備完了
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_AllBattleReady(void)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_SENDID_ALLUSER, 
    COLOSSEUM_CMD_ALL_BATTLE_READY, 0, NULL, TRUE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ポケモンリスト選択画面終了しました
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_PokeListSelected(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  clsys->recvbuf.pokelist_selected_num++;
  OS_TPrintf("ポケモンリスト選択完了：net_id = %d, total_num = %d\n", netID, clsys->recvbuf.pokelist_selected_num);
}

//==================================================================
/**
 * データ送信：ポケモンリスト選択画面終了しました
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_PokeListSelected(void)
{
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    COLOSSEUM_CMD_POKELIST_SELECTED, 0, NULL);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：退出
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_Leave(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  UNION_SYSTEM_PTR unisys = pWork;
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  if(clsys == NULL){
    GF_ASSERT(0);
    return; //準備が出来ていないので受け取らない
  }
  
  OS_TPrintf("コロシアム：退出受信：net_id = %d\n", netID);
  clsys->recvbuf.leave[netID] = TRUE;
}

//==================================================================
/**
 * データ送信：退出
 * @param   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_Leave(void)
{
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), COLOSSEUM_CMD_LEAVE, 0, NULL);
}

