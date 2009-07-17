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


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);
static void _ColosseumRecv_Shutdown(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_MainMenuListResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_MainMenuListResultAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _ColosseumRecv_TrainerCardParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);


//==============================================================================
//  データ
//==============================================================================
///ユニオン受信コマンドテーブル   ※COLOSSEUM_CMD_???と並びを同じにしておくこと！！
const NetRecvFuncTable Colosseum_CommPacketTbl[] = {
  {_ColosseumRecv_Shutdown, NULL},                //COLOSSEUM_CMD_SHUTDOWN
  {_ColosseumRecv_MainMenuListResult, NULL},      //COLOSSEUM_CMD_MAINMENU_LIST_RESULT
  {_ColosseumRecv_MainMenuListResultAnswer, NULL},      //COLOSSEUM_CMD_MAINMENU_LIST_RESULT_ANSWER
  {_ColosseumRecv_TrainerCardParam, _RecvHugeBuffer},        //COLOSSEUM_CMD_TRAINERCARD_PARAM
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
  UNION_SYSTEM_PTR unisys = pWork;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：メインメニューの選択結果
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_MainMenuListResult(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
#if 0
  UNION_SYSTEM_PTR unisys = pWork;
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  const u32 *select = pData;

	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return; //自分のデータなので無視
  }
  
  situ->mycomm.mainmenu_select = *select;
#endif
}

//==================================================================
/**
 * データ送信：メインメニューの選択結果
 * @param   select_list		選択結果(UNION_MSG_MENU_SELECT_???)
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_MainMenuListResult(u32 select_list)
{
  return 0;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：メインメニューの選択結果の返事
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_MainMenuListResultAnswer(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
#if 0
  UNION_SYSTEM_PTR unisys = pWork;
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  const BOOL *yes_no = pData;

	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return; //自分のデータなので無視
  }
  
  situ->mycomm.mainmenu_yesno_result = *yes_no;
#endif
}

//==================================================================
/**
 * データ送信：メインメニューの選択結果の返事
 * @param   yes_no    TRUE:はい　FALSE:いいえ
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_MainMenuListResultAnswer(BOOL yes_no)
{
  return 0;
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド受信：トレーナーカード情報
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _ColosseumRecv_TrainerCardParam(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
#if 0
  UNION_SYSTEM_PTR unisys = pWork;
  UNION_MY_SITUATION *situ = &unisys->my_situation;

	if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return; //自分のデータなので無視
  }
  
  OS_TPrintf("COmmCOmmand カード受信 netID = %d\n", netID);
  situ->mycomm.trcard.target_card_receive = TRUE;
  GFL_STD_MemCopy(pData, situ->mycomm.trcard.target_card, size);
#endif
}

//==================================================================
/**
 * データ送信：トレーナーカード情報
 * @param   yes_no    TRUE:はい　FALSE:いいえ
 * @retval  BOOL		TRUE:送信成功。　FALSE:失敗
 */
//==================================================================
BOOL ColosseumSend_TrainerCardParam(UNION_SYSTEM_PTR unisys)
{
  return 0;
}
