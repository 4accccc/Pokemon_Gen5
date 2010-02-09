//==============================================================================
/**
 * @file    comm_entry_menu_comm_command.c
 * @brief   参加募集メニュー制御：通信テーブル
 * @author  matsuda
 * @date    2010.01.11(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "savedata/mystatus.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "net_app/comm_entry_menu.h"
#include "comm_entry_menu_comm_command.h"
#include "savedata/mystatus_local.h"



//==============================================================================
//  構造体定義
//==============================================================================
///親へエントリー通知するときの送信データ
typedef struct{
  MYSTATUS mystatus;
  u8 mac_address[6];
  u8 force_entry;
  u8 padding;
}CEM_SEND_ENTRY;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvMemberInfo(int netID, void* pWork, int size);
static void _CemRecv_Entry(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _CemRecv_EntryOK(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _CemRecv_EntryNG(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _CemRecv_GameStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _CemRecv_GameCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _CemRecv_MemberInfo(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※ENTRYMENU_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable CommEntryMenu_CommPacketTbl[] = {
  {_CemRecv_Entry, NULL},               //ENTRYMENU_CMD_ENTRY
  {_CemRecv_EntryOK, NULL},             //ENTRYMENU_CMD_ENTRY_OK
  {_CemRecv_EntryNG, NULL},             //ENTRYMENU_CMD_ENTRY_NG
  {_CemRecv_GameStart, NULL},           //ENTRYMENU_CMD_GAME_START
  {_CemRecv_GameCancel, NULL},          //ENTRYMENU_CMD_GAME_CANCEL
  {_CemRecv_MemberInfo, _RecvMemberInfo}, //ENTRYMENU_CMD_MEMBER_INFO
};
SDK_COMPILER_ASSERT(NELEMS(CommEntryMenu_CommPacketTbl) == ENTRYMENU_CMD_NUM);



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 参加募集メニュー用の通信コマンドテーブルを追加する
 *
 * @param   em		
 */
//==================================================================
void CommEntryMenu_AddCommandTable(COMM_ENTRY_MENU_PTR em)
{
  GFL_NET_AddCommandTable(GFL_NET_CMD_COMM_ENTRY_MENU, CommEntryMenu_CommPacketTbl, 
    NELEMS(CommEntryMenu_CommPacketTbl), em);
}

//==================================================================
/**
 * 参加募集メニュー用の通信コマンドテーブルを削除する
 */
//==================================================================
void CommEntryMenu_DelCommandTable(void)
{
  GFL_NET_DelCommandTable(GFL_NET_CMD_COMM_ENTRY_MENU);
}


//--------------------------------------------------------------
/**
 * メンバー情報受信バッファ取得関数
 *
 * @param   netID		
 * @param   pWork		
 * @param   size		
 *
 * @retval  u8 *		
 */
//--------------------------------------------------------------
static u8 * _RecvMemberInfo(int netID, void* pWork, int size)
{
  COMM_ENTRY_MENU_PTR em = pWork;
	
	return (u8*)(CommEntryMenu_GetMemberInfo(em));
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：親へエントリーを通知
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _CemRecv_Entry(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  COMM_ENTRY_MENU_PTR em = pWork;
  const CEM_SEND_ENTRY *recv_entry = pData;
  
  OS_TPrintf("エントリー受信：net_id=%d\n", netID);
  CommEntryMenu_Entry(
    em, netID, &recv_entry->mystatus, recv_entry->force_entry, recv_entry->mac_address);
}

//==================================================================
/**
 * データ送信：親へエントリーを通知
 * @param   mystatus      MYSTATUS
 * @param   force_entry   TRUE:強制参加   FALSE:親に参加していいか判断してもらう
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL CemSend_Entry(const MYSTATUS *myst, BOOL force_entry)
{
  CEM_SEND_ENTRY send_entry;
  
  GFL_STD_MemClear(&send_entry, sizeof(CEM_SEND_ENTRY));
  MyStatus_Copy(myst, &send_entry.mystatus);
  OS_GetMacAddress(send_entry.mac_address);
  send_entry.force_entry = force_entry;
  
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_NO_PARENTMACHINE, 
    ENTRYMENU_CMD_ENTRY, sizeof(CEM_SEND_ENTRY), &send_entry, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：エントリーOK
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _CemRecv_EntryOK(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  COMM_ENTRY_MENU_PTR em = pWork;

  CommEntryMenu_SetEntryAnswer(em, ENTRY_PARENT_ANSWER_OK);
}

//==================================================================
/**
 * データ送信：エントリーOK
 * @param   send_id   送信先
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL CemSend_EntryOK(NetID send_id)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_id, 
    ENTRYMENU_CMD_ENTRY_OK, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：エントリーNG
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _CemRecv_EntryNG(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  COMM_ENTRY_MENU_PTR em = pWork;

  CommEntryMenu_SetEntryAnswer(em, ENTRY_PARENT_ANSWER_NG);
}

//==================================================================
/**
 * データ送信：エントリーNG
 * @param   send_id   送信先
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL CemSend_EntryNG(NetID send_id)
{
  return GFL_NET_SendDataEx(GFL_NET_HANDLE_GetCurrentHandle(), send_id, 
    ENTRYMENU_CMD_ENTRY_NG, 0, NULL, FALSE, FALSE, FALSE);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ゲーム開始を通知
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _CemRecv_GameStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  COMM_ENTRY_MENU_PTR em = pWork;

  CommEntryMenu_SetGameStart(em);
}

//==================================================================
/**
 * データ送信：ゲーム開始を通知
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL CemSend_GameStart(void)
{
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    ENTRYMENU_CMD_GAME_START, 0, NULL);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：ゲーム中止を通知
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _CemRecv_GameCancel(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  COMM_ENTRY_MENU_PTR em = pWork;

  CommEntryMenu_SetGameCancel(em);
}

//==================================================================
/**
 * データ送信：ゲーム中止を通知
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL CemSend_GameCancel(void)
{
  return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
    ENTRYMENU_CMD_GAME_CANCEL, 0, NULL);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：参加者情報
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _CemRecv_MemberInfo(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  COMM_ENTRY_MENU_PTR em = pWork;
  const ENTRYMENU_MEMBER_INFO *recv_member_info = pData;
  
  if(GFL_NET_IsParentMachine() == TRUE){
    CommEntyrMenu_MemberInfoReserveUpdate(em);
    return; //親機は受け取らない
  }
  
  OS_TPrintf("Recv:参加者情報\n");
  //巨大データ受信バッファに_RecvMemberInfoで直接代入先のアドレスを渡しているのでコピーの必要は
  //無いが、受信した事を知らせる為、セット関数を呼び出す
  CommEntryMenu_RecvMemberInfo(em, recv_member_info);
}

//==================================================================
/**
 * データ送信：参加者情報
 * @param   member_info   
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL CemSend_MemberInfo(const ENTRYMENU_MEMBER_INFO *member_info, u8 send_bit)
{
  OS_TPrintf("SEND:参加者情報 send_bit = %d\n", send_bit);
  return GFL_NET_SendDataExBit(GFL_NET_HANDLE_GetCurrentHandle(), send_bit, 
    ENTRYMENU_CMD_MEMBER_INFO, CommEntryMenu_GetMemberInfoSize(), member_info, 
    FALSE, FALSE, TRUE);
}

