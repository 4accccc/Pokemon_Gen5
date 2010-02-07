//=============================================================================
/**
 * @file	ircbattlefriend.c
 * @bfief	フレンドコードを交換して通信を終了する
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	09/03/23
 */
//=============================================================================

#include <gflib.h>
#include "arc_def.h"
#include "net/network_define.h"

#include "ircbattlefriend.h"
#include "system/main.h"

#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"

#include "msg/msg_ircbattle.h"
#include "ircbattle.naix"
#include "net_app/connect_anm.h"
#include "../../field/event_ircbattle.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"
#include "savedata/system_data.h"
#include "savedata/wifi_negotiation.h"


#define _TIMING_ENDNO (12)

typedef enum {
  _NETCMD_MYSTATUS = GFL_NET_CMD_IRCFRIEND,
  _NETCMD_FRIENDCODE,
} _BATTLEIRC_SENDCMD;

typedef void (StateFunc)(IRC_BATTLE_FRIEND* pState);

struct _IRC_BATTLE_FRIEND {
  EVENT_IRCBATTLE_WORK* pParentWork;
  StateFunc* state;      ///< ハンドルのプログラム状態
  int selectType;   // 接続タイプ
  HEAPID heapID;
  int windowNum;
  BOOL IsIrc;
  u32 connect_bit;
  BOOL connect_ok;
  BOOL receive_ok;
  u32 receive_result_param;
  u32 receive_first_param;
  DWCFriendData friendData;
  BOOL bParent;
};



static void _changeState(IRC_BATTLE_FRIEND* pWork,StateFunc* state);
static void _changeStateDebug(IRC_BATTLE_FRIEND* pWork,StateFunc* state, int line);
static void _recvFriendCode(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvMystatus(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_recvMystatus,          NULL},  ///_NETCMD_MYSTATUS
  {_recvFriendCode,         NULL},    ///_NETCMD_FRIENDCODE

};



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(IRC_BATTLE_FRIEND* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(IRC_BATTLE_FRIEND* pWork,StateFunc state, int line)
{
  NET_PRINT("ircmatch: %d\n",line);
  _changeState(pWork, state);
}
#endif



static void _recvFriendCode(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  IRC_BATTLE_FRIEND *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは無視
  }

  GFL_STD_MemCopy(pData,&pWork->friendData, sizeof(DWCFriendData));
}

static void _recvMystatus(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  IRC_BATTLE_FRIEND *pWork = pWk;
  GAMEDATA* pGameData = GAMESYSTEM_GetGameData(IrcBattle_GetGAMESYS_WORK(pWork->pParentWork));

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは無視
  }

  GFL_NET_DWC_FriendDataAdd(pGameData,
                            (MYSTATUS*)pData, &pWork->friendData, HEAPID_IRCBATTLE);

  WIFI_NEGOTIATION_SV_SetFriend(GAMEDATA_GetWifiNegotiation(pGameData),(const MYSTATUS*)pData);

}


//----------------------------------------------------------------------------
/**
 *	@brief	終了同期を取ったのでプロセス終了
 *	@param	IRC_BATTLE_FRIEND		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTimingCheck(IRC_BATTLE_FRIEND* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO,WB_NET_IRCBATTLE)){
    _CHANGE_STATE(pWork,NULL);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了同期を取る
 *	@param	IRC_BATTLE_FRIEND		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTiming(IRC_BATTLE_FRIEND* pWork)
{
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO,WB_NET_IRCBATTLE);
  _CHANGE_STATE(pWork,_sendTimingCheck);
}

//----------------------------------------------------------------------------
/**
 *	@brief	MyStatusを送る
 *	@param	IRC_BATTLE_FRIEND		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendMystatus(IRC_BATTLE_FRIEND* pWork)
{
  DWCFriendData friendData;

  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_MYSTATUS,
                   MyStatus_GetWorkSize(),
                      GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(IrcBattle_GetGAMESYS_WORK(pWork->pParentWork))))){
    _CHANGE_STATE(pWork,_sendTiming);
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	FriendDataを送る
 *	@param	IRC_BATTLE_FRIEND		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendFriendCode(IRC_BATTLE_FRIEND* pWork)
{
  DWCFriendData friendData;

  GFL_NET_DWC_GetMySendedFriendCode(
    GAMEDATA_GetWiFiList(GAMESYSTEM_GetGameData(IrcBattle_GetGAMESYS_WORK(pWork->pParentWork))) ,&friendData);

  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_FRIENDCODE,
                      sizeof(DWCFriendData),&friendData)){
      _CHANGE_STATE(pWork,_sendMystatus);

  }

}

//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleFriendProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x10000 );

  {
    IRC_BATTLE_FRIEND *pWork = GFL_PROC_AllocWork( proc, sizeof( IRC_BATTLE_FRIEND ), HEAPID_IRCBATTLE );
    GFL_STD_MemClear(pWork, sizeof(IRC_BATTLE_FRIEND));
    pWork->heapID = HEAPID_IRCBATTLE;
    pWork->selectType =  EVENT_IrcBattleGetType((EVENT_IRCBATTLE_WORK*) pwk);
    pWork->pParentWork = pwk;
    // 通信テーブル追加
    GFL_NET_AddCommandTable(GFL_NET_CMD_IRCFRIEND,_PacketTbl,NELEMS(_PacketTbl), pWork);
    _CHANGE_STATE( pWork, _sendFriendCode);
  }
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleFriendProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  IRC_BATTLE_FRIEND* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;

  StateFunc* state = pWork->state;
  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }
  //	ConnectBGPalAnm_Main(&pWork->cbp);

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleFriendProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  IRC_BATTLE_FRIEND* pWork = mywk;

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA IrcBattleFriendProcData = {
  IrcBattleFriendProcInit,
  IrcBattleFriendProcMain,
  IrcBattleFriendProcEnd,
};


