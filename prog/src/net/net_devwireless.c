//=============================================================================
/**
 * @file	net_devwireless.c
 * @bfief	無線通信ドライバー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	08/12/19
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "net_whpipe.h"
#include "net/wih.h"

//--------------------------------------------
//内部関数一覧
//--------------------------------------------
extern GFLNetDevTable *NET_GetWirelessDeviceTable(void);
static BOOL _DevInitFunc(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork);
static BOOL _DevStartFunc(NetDevEndCallback callback);
static int _DevMainFunc(u16 bitmap);
static BOOL _DevEndFunc(BOOL bForce,NetDevEndCallback callback);
static BOOL _DevExitFunc(NetDevEndCallback callback);
static BOOL _DevChildWorkInitFunc(int NetID);
static BOOL _DevParentWorkInitFunc(void);
static BOOL _DevMyBeaconSetFunc(NetDevEndCallback callback);
static BOOL _DevBeaconSearchFunc(NetDevEndCallback callback);
static void* _DevBeaconGetFunc(int index);
static u8* _DevBeaconGetMacFunc(int index);
static BOOL _DevIsBeaconChangeFunc(NetDevEndCallback callback);
static BOOL _DevResetBeaconChangeFlgFunc(NetDevEndCallback callback);
static BOOL _DevIsChangeOverTurnFunc(NetDevEndCallback callback);

static BOOL _DevSetChildConnectCallbackFunc(PTRChildConnectCallbackFunc func);  ///< 子供がつないだ時のコールバックを指定する
static BOOL _DevParentModeConnectFunc(BOOL bChannelChange, NetDevEndCallback callback); ///<DevParentModeConnect       親開始
static BOOL _DevChildModeConnectFunc(BOOL bBconInit, NetDevEndCallback callback); ///<DevChildModeConnect        子供開始        ビーコンindex + macAddress  つながない場合-1
static BOOL _DevChildModeMacConnectFunc(BOOL bBconInit,void* pMacAddr,int index,NetDevEndCallback callback,PTRPARENTFIND_CALLBACK pCallback); ///<DevChildModeConnect        子供開始        ビーコンindex + macAddress  つながない場合-1
static BOOL _DevChangeOverModeConnectFunc(u8* keyStr,int numEntry, BOOL bParent, u32 timelimit,NetDevEndCallback callback); ///<DevChangeOverModeConnect   ランダム接続     つながない場合-1
static BOOL _DevModeDisconnectFunc(BOOL bForce,NetDevEndCallback callback); ///<DevModeDisconnect          接続を終わる
static BOOL _DevIsStepDSFunc(void);  ///< データシェアリング方式送信出来る状態か
static BOOL _DevStepDSFunc(void* pSendData);  ///< データシェアリング方式送信
static void* _DevGetSharedDataAdrFunc(int index);

static BOOL _DevSendDataFunc(void* data,int size,int no,NetDevEndCallback callback); ///<DevSendData                送信関数
static BOOL _DevSetRecvCallbackFunc(PTRCommRecvLocalFunc recvCallback); ///<DevRecvCallback            受信コールバック

static BOOL _DevIsStartFunc(void); ///<DevIsStart                 通信接続してるかどうか
static BOOL _DevIsConnectFunc(void); ///<DevIsConnect               相手と接続してるかどうか
static BOOL _DevIsEndFunc(void); ///<DevIsEnd                 通信終了してよいかどうか
static BOOL _DevIsIdleFunc(void); ///<アイドル状態かどうか
static u32 _DevGetBitmapFunc(void); ///<DevGetBitmap               接続状態をBITMAPで返す
static u32 _DevGetCurrentIDFunc(void); ///<DevGetCurrentID            自分の接続IDを返す
static int _DevGetIconLevelFunc(void);

static int _DevGetError(void);  ///< エラーを得る
static void _DevSetNoChildErrorSet(BOOL bOn);
static void _DevSetClinetConnect(BOOL bEnable);
static BOOL _DevCrossScan(void);

//--------------------------------------------
// 構造体
//--------------------------------------------

static GFLNetDevTable netDevTbl={
	NULL,
	_DevInitFunc,
	_DevStartFunc,
	_DevMainFunc,
	_DevEndFunc,
	_DevExitFunc,
	_DevChildWorkInitFunc,
	_DevParentWorkInitFunc,
	_DevMyBeaconSetFunc,
	_DevBeaconSearchFunc,
	_DevBeaconGetFunc,
	_DevBeaconGetMacFunc,
	_DevIsBeaconChangeFunc,
	_DevResetBeaconChangeFlgFunc,
	_DevIsChangeOverTurnFunc,
	_DevSetChildConnectCallbackFunc,
	_DevParentModeConnectFunc,
	_DevChildModeConnectFunc,
	_DevChildModeMacConnectFunc,
	_DevChangeOverModeConnectFunc,
	NULL,
	_DevModeDisconnectFunc,
	_DevIsStepDSFunc,
	_DevStepDSFunc,
	_DevGetSharedDataAdrFunc,
	_DevSendDataFunc,
	_DevSetRecvCallbackFunc,
	_DevIsStartFunc,
	_DevIsConnectFunc,
	_DevIsEndFunc,
	_DevIsIdleFunc,
	_DevGetBitmapFunc,
	_DevGetCurrentIDFunc,
	_DevGetIconLevelFunc,
	_DevGetError,
	_DevSetNoChildErrorSet,
	NULL, //_DevIsConnectable
	NULL, //_DevIsVChat
	NULL, //_DevIsNewPlayer
	NULL, //_DevIrcMoveFunc

	NULL, //DevIsSendDataFunc DevIsSendData; ///< データを送ってよいかどうか
	NULL, //DevGetSendTurnFunc DevGetSendTurn; ///< 送信可能ターンフラグを取得
	NULL, //DevIsConnectSystemFunc DevIsConnectSystem;  ///< 再接続中など関係なく、純粋に今、繋がっているか
	NULL, //DevGetSendLockFlagFunc DevGetSendLockFlag;  ///< 送信ロックフラグを取得
	NULL, //DevConnectWorkInitFunc DevConnectWorkInit;           ///<初めての接続後のワーク設定
	_DevSetClinetConnect, //DevSetClientConnectFunc DevSetClientConnect; ///< 子機がつながってよいかどうかハードレベルで調整
  _DevCrossScan, //すれ違い通信

	NULL, //DevLobbyLoginFunc DevLobbyLogin;		///<Wi-Fi広場にログイン	DWC_LOBBY_Login
	NULL, //DevDebugSetRoomFunc DevDebugSetRoom;	///<デバッグ用 部屋データ設定 DWC_LOBBY_DEBUG_SetRoomData
	NULL, //DevLobbyUpdateErrorCheckFunc DevLobbyUpdateErrorCheck;	///<DWCロビー更新 DWC_LOBBY_UpdateErrorCheck
	NULL, //DevLobbyLoginWaitFunc DevLobbyLoginWait;	///<DWC_LOBBY_LoginWait
	NULL, //DevLobbyLogoutFunc DevLobbyLogout;	///<DWC_LOBBY_Logout
	NULL, //DevLobbyLogoutWaitFunc DevLobbyLogoutWait;	///<DWC_LOBBY_LogoutWait
	NULL, //DevLobbyMgCheckRecruitFunc DevLobbyMgCheckRecruit;  ///<DWC_LOBBY_MG_CheckRecruit
	NULL, //DevLobbyMgStartRecruitFunc DevLobbyMgStartRecruit;  ///<DWC_LOBBY_MG_StartRecruit
	NULL, //DevLobbyMgEntryFunc DevLobbyMgEntry;  ///<DWC_LOBBY_MG_Entry
	NULL, //DevLobbyMgForceEndFunc DevLobbyMgForceEnd;  ///<DWC_LOBBY_MG_ForceEnd
	NULL, //DevLobbyMgEndConnectFunc DevLobbyMgEndConnect; ///<DWC_LOBBY_MG_EndConnect
	NULL, //DevLobbyMgMyParentFunc DevLobbyMgMyParent;  ///<DWC_LOBBY_MG_MyParent
	NULL, //DevLobbyMgEndRecruitFunc DevLobbyMgEndRecruit;  ///<DWC_LOBBY_MG_EndRecruit

};

//--------------------------------------------
// 内部ワーク
//--------------------------------------------

typedef struct{
	GFL_NETSYS* pNet;
	NetDevEndCallback callback;
} NetWork;


static NetWork* _pWork = NULL;


//------------------------------------------------------------------------------
/**
 * @brief   ワイヤレスデバイステーブルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
GFLNetDevTable *NET_GetWirelessDeviceTable(void)
{
	return &netDevTbl;
}

//------------------------------------------------------------------------------
/**
 * @brief   デバイス初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevInitFunc(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork)
{
	GFLNetInitializeStruct* initStruct;
	BOOL isScanOnly;
	if(_pWork){
		return FALSE;  //すでに初期化済み
	}
	_pWork = GFL_HEAP_AllocClearMemory(heapID, sizeof(NetWork));
	_pWork->pNet = pNet;
	initStruct = GFL_NET_GetNETInitStruct();
	isScanOnly = (initStruct->bNetType == GFL_NET_TYPE_WIRELESS_SCANONLY ? TRUE : FALSE );
	return GFL_NET_WLInitialize(heapID, callback, pUserWork,isScanOnly);
}


//------------------------------------------------------------------------------
/**
 * @brief   接続
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevStartFunc(NetDevEndCallback callback)
{
	//ワイヤレスには接続は存在しない IDLE状態ならOK
	if(callback)
		callback(GFL_NET_WLIsStateIdle());
	return GFL_NET_WLIsStateIdle();
}

//------------------------------------------------------------------------------
/**
 * @brief   メインループ
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevMainFunc(u16 bitmap)
{
	GFI_NET_MLProcess(bitmap);
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信を一旦終了する
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevEndFunc(BOOL bForce, NetDevEndCallback callback)
{
	return GFL_NET_WLFinalize();
}


//------------------------------------------------------------------------------
/**
 * @brief   メモリ開放オーバーレイ開放のコールバック
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _callbackExitFunc(BOOL bReset)
{
	if(_pWork->callback){
		_pWork->callback(bReset);
	}
	GFL_HEAP_FreeMemory(_pWork);
	_pWork = NULL;
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   メモリ開放オーバーレイ開放
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevExitFunc(NetDevEndCallback callback)
{
	if(_pWork){
		_pWork->callback = callback;
		return GFI_NET_WHPipeEnd(_callbackExitFunc);
	}
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   子機エリア初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevChildWorkInitFunc(int NetID)
{
	GFL_NET_WLChildBconDataInit();
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   親機エリア初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevParentWorkInitFunc(void)
{
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   自分ビーコンを設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevMyBeaconSetFunc(NetDevEndCallback callback)
{
	return TRUE;
}
//------------------------------------------------------------------------------
/**
 * @brief   ビーコンを探す
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevBeaconSearchFunc(NetDevEndCallback callback)
{
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンを得る
 * @retval  none
 */
//------------------------------------------------------------------------------
static void* _DevBeaconGetFunc(int index)
{
	if(index >= GFL_NET_MACHINE_MAX){
		return NULL;
	}
	return GFL_NET_WLGetUserBss(index);
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンを得る
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8* _DevBeaconGetMacFunc(int index)
{
	if(index >= GFL_NET_MACHINE_MAX){
		return NULL;
	}
	return GFL_NET_WLGetUserMacAddress(index);
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンが変わったかどうか
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsBeaconChangeFunc(NetDevEndCallback callback)
{
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンを得る
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevResetBeaconChangeFlgFunc(NetDevEndCallback callback)
{
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンを得る
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsChangeOverTurnFunc(NetDevEndCallback callback)
{
	return GFL_NET_WLIsParentBeaconSent();
}

//------------------------------------------------------------------------------
/**
 * @brief   子供がつないだ時のコールバックを指定する
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevSetChildConnectCallbackFunc(PTRChildConnectCallbackFunc func)
{
	WHSetConnectCallBack(func);
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   親開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevParentModeConnectFunc(BOOL bChannelChange, NetDevEndCallback callback)
{
	return GFL_NET_WL_ParentConnect(bChannelChange);

}
//------------------------------------------------------------------------------
/**
 * @brief   子供開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevChildModeConnectFunc(BOOL bBconInit, NetDevEndCallback callback)
{
	return GFL_NET_WLChildInit(bBconInit);
}

//------------------------------------------------------------------------------
/**
 * @brief   MACアドレス指定で子供開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevChildModeMacConnectFunc(BOOL bBconInit, void* macAddress,int index, NetDevEndCallback callback,PTRPARENTFIND_CALLBACK pCallback)
{
	return GFL_NET_WLChildMacAddressConnect(bBconInit, macAddress,index, pCallback);
}


//------------------------------------------------------------------------------
/**
 * @brief   ランダム接続開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevChangeOverModeConnectFunc(u8* keyStr,int numEntry, BOOL bParent, u32 timelimit,NetDevEndCallback callback)
{
	return FALSE;
}
//------------------------------------------------------------------------------
/**
 * @brief   接続を終わる
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevModeDisconnectFunc(BOOL bForce,NetDevEndCallback callback)
{
	return GFL_NET_WLFinalize();
}

//------------------------------------------------------------------------------
/**
 * @brief   データシェアリング方式送信出来る状態か
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsStepDSFunc(void)
{
	return (WH_GetSystemState() == WH_SYSSTATE_DATASHARING);
}


//------------------------------------------------------------------------------
/**
 * @brief   データシェアリング方式送信
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevStepDSFunc(void* pSendData)
{
	return WH_StepDS(pSendData);
}

//------------------------------------------------------------------------------
/**
 * @brief   データを送る
 * @retval  none
 */
//------------------------------------------------------------------------------

static void* _DevGetSharedDataAdrFunc(int index)
{
	return WH_GetSharedDataAdr(index);
}

//------------------------------------------------------------------------------
/**
 * @brief   データを送る
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevSendDataFunc(void* data,int size,int no, NetDevEndCallback callback)
{
	return GFL_NET_WL_SendData( data, size, callback);
}

//------------------------------------------------------------------------------
/**
 * @brief   受信コールバックの設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevSetRecvCallbackFunc(PTRCommRecvLocalFunc recvCallback)
{
	GFL_NET_WLSetRecvCallback(recvCallback);
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信接続してるかどうか
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsStartFunc(void)
{
	switch(WH_GetSystemState()){
		//  WH_SYSSTATE_STOP,                  // 初期状態
	case WH_SYSSTATE_IDLE:                  // 待機中
	case WH_SYSSTATE_SCANNING:              // スキャン中
	case WH_SYSSTATE_BUSY:                  // 接続作業中
	case WH_SYSSTATE_CONNECTED:             // 接続完了（この状態で通信可能）
	case WH_SYSSTATE_DATASHARING:           // data-sharing有効で接続完了
	case WH_SYSSTATE_KEYSHARING_NONE:       // key-sharing有効で接続完了  (使っていない)
	case WH_SYSSTATE_MEASURECHANNEL:        // チャンネルの電波使用率をチェック
	case WH_SYSSTATE_CONNECT_FAIL:          // 親機への接続失敗
	case WH_SYSSTATE_ERROR:                 // エラー発生
	case WH_SYSSTATE_FATAL:                 // FATALエラー発生
		return TRUE;
	}
	return FALSE;
}
//------------------------------------------------------------------------------
/**
 * @brief   相手と接続してるかどうか
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsConnectFunc(void)
{
	return GFL_NET_WL_IsConnectLowDevice();
}
//------------------------------------------------------------------------------
/**
 * @brief   通信終了してよいかどうか
 * @retval  TRUE  通信終了してよい
 */
//------------------------------------------------------------------------------
static BOOL _DevIsEndFunc(void)
{
	return !GFL_NET_WLIsInitialize();
}


//------------------------------------------------------------------------------
/**
 * @brief   何もしていない　次の状態にいける状態かどうか？
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsIdleFunc(void)
{
	switch(WH_GetSystemState()){
	case WH_SYSSTATE_IDLE:
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   接続状態をBITMAPで返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u32 _DevGetBitmapFunc(void)
{
	return GFL_NET_WL_GetBitmap();
}
//------------------------------------------------------------------------------
/**
 * @brief   自分のNetIDを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u32 _DevGetCurrentIDFunc(void)
{
	return GFL_NET_WL_GetCurrentAid();
}

//------------------------------------------------------------------------------
/**
 * @brief   アイコンレベルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevGetIconLevelFunc(void)
{
	return (WM_LINK_LEVEL_3 - WM_GetLinkLevel());
}

//------------------------------------------------------------------------------
/**
 * @brief   エラー番号を返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevGetError(void)  ///< エラーを得る
{
	int err = WH_GetLastError();

  if(err == WM_ERRCODE_NO_DATASET){
    err = WM_ERRCODE_SUCCESS;
  }
  if(GFL_NET_WLIsError() && (err == 0)){
		err = WH_ERRCODE_DISCONNECTED;
	}
	return err;
}

//------------------------------------------------------------------------------
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _DevSetNoChildErrorSet(BOOL bOn)
{
	GFL_NET_WLSetDisconnectOtherError(bOn);
}

//------------------------------------------------------------------------------
/**
 * @brief   子機がつながってよいかどうかハードレベルで調整する
 * @param   bEnable TRUE=許可
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _DevSetClinetConnect(BOOL bEnable)
{
	GFL_NET_WHPipeSetClientConnect(bEnable);
}

//------------------------------------------------------------------------------
/**
 * @brief   すれ違い通信開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevCrossScan(void)
{
  return GFL_NET_WLCrossoverInit();
}

