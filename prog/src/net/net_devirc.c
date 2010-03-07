//=============================================================================
/**
 * @file	net_devirc.c
 * @bfief	IRC通信ドライバー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	08/12/22
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "net/net_irc.h"

//--------------------------------------------
//内部関数一覧
//--------------------------------------------
extern GFLNetDevTable *NET_GetIrcDeviceTable(void);
static BOOL _DevInitFunc(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork);
static BOOL _DevStartFunc(NetDevEndCallback callback);
static int _DevMainFunc(u16 bitmap);
static BOOL _DevEndFunc(BOOL bForce,NetDevEndCallback callback);
static BOOL _DevExitFunc(NetDevEndCallback callback);
static BOOL _DevChildWorkInitFunc(int NetID);
static BOOL _DevParentWorkInitFunc(void);
static BOOL _DevIsChangeOverTurnFunc(NetDevEndCallback callback);

static BOOL _DevSetChildConnectCallbackFunc(PTRChildConnectCallbackFunc func);  ///< 子供がつないだ時のコールバックを指定する
static BOOL _DevChangeOverModeConnectFunc(u8* keyStr,int numEntry, BOOL bParent, u32 timelimit,NetDevEndCallback callback); ///<DevChangeOverModeConnect   ランダム接続     つながない場合-1
static BOOL _DevModeDisconnectFunc(BOOL bForce,NetDevEndCallback callback); ///<DevModeDisconnect          接続を終わる
static BOOL _DevIsStepDSFunc(void);  ///< データシェアリング方式送信出来る状態か
static BOOL _DevStepDSFunc(void* pSendData);  ///< データシェアリング方式送信
static void* _DevGetSharedDataAdrFunc(int index);

static BOOL _DevSendDataFunc(void* data,int size,int no,NetDevEndCallback callback); ///<DevSendData                送信関数
static BOOL _DevSetRecvCallbackFunc(PTRCommRecvLocalFunc recvCallback); ///<DevRecvCallback            受信コールバック

static BOOL _DevIsStartFunc(void); ///<DevIsStart                 通信接続してるかどうか
static BOOL _DevIsConnectFunc(void); ///<DevIsConnect               相手と接続してるかどうか
static BOOL _DevIsEndFunc(void); ///<DevIsEnd                 通信終了してるかどうか
static BOOL _DevIsIdleFunc(void); ///<アイドル状態かどうか
static u32 _DevGetBitmapFunc(void); ///<DevGetBitmap               接続状態をBITMAPで返す
static u32 _DevGetCurrentIDFunc(void); ///<DevGetCurrentID            自分の接続IDを返す
static int _DevGetErrorFunc(void);


static void _DevSetNoChildErrorSet(BOOL bOn);
static void _DevIrcMoveFunc(void);
static BOOL _DevIsSendDataFunc(void);
static BOOL _DevGetSendTurnFunc(void);
static BOOL _DevIsConnectSystemFunc(void);
static BOOL _DevGetSendLockFlagFunc(void);
static void _DevConnectWorkInitFunc(void);



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
	NULL, //_DevChildWorkInitFunc,
	NULL, //_DevParentWorkInitFunc,
	NULL, //_DevMyBeaconSetFunc,
	NULL,
	NULL, //_DevBeaconGetFunc,
	NULL, //_DevBeaconGetMacFunc,
	NULL, //_DevIsBeaconChangeFunc,
	NULL, //_DevResetBeaconChangeFlgFunc,
	NULL, //_DevIsChangeOverTurnFunc,
	NULL, //_DevSetChildConnectCallbackFunc,
	NULL,
	NULL,
	NULL,
	NULL, //_DevChangeOverModeConnectFunc,
	NULL,
	NULL, //_DevModeDisconnectFunc,
	NULL, //_DevIsStepDSFunc,
	NULL, //_DevStepDSFunc,
	NULL, //_DevGetSharedDataAdrFunc,
	_DevSendDataFunc,
	_DevSetRecvCallbackFunc,
	_DevIsStartFunc,
	_DevIsConnectFunc,
	_DevIsEndFunc,
	_DevIsIdleFunc,
	_DevGetBitmapFunc, //_DevGetBitmapFunc,
	_DevGetCurrentIDFunc,
	NULL,
	_DevGetErrorFunc, //_DevGetError,
	NULL, //_DevSetNoChildErrorSet,
	NULL, //_DevIsConnectable
	NULL, //_DevIsVChat
	NULL, //_DevIsNewPlayer
	_DevIrcMoveFunc,
	_DevIsSendDataFunc,
	_DevGetSendTurnFunc,
	_DevIsConnectSystemFunc,
	_DevGetSendLockFlagFunc,
	_DevConnectWorkInitFunc,
	NULL, //DevSetClientConnectFunc DevSetClientConnect; ///< 子機がつながってよいかどうかハードレベルで調整
  NULL, //DevCrossScanFunc
  NULL, //DevCrossScanChangeSpeedFunc

#if 0
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
#endif
};

//------------------------------------------------------------------------------
/**
 * @brief   初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevInitFunc(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork)
{
	GFL_NET_IRC_Init((GFL_NET_GetNETInitStruct())->irc_timeout);
	GFL_NET_IRC_InitializeFlagSet();
	return TRUE;
}


static BOOL _DevStartFunc(NetDevEndCallback callback)
{
	OS_TPrintf("IRC_Connect実行\n");
	IRC_Connect();
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   メイン処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevMainFunc(u16 bitmap)
{
	GFL_NET_IRC_CommandContinue();
	return 0;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信終了処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevEndFunc(BOOL bForce,NetDevEndCallback callback)
{
	//赤外線には切断処理がない
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   終了処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevExitFunc(NetDevEndCallback callback)
{
	GFL_NET_IRC_Exit();
	return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   スタート状態かどうか
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevIsStartFunc(void)
{
	return GFL_NET_IRC_InitializeFlagGet();
}

//------------------------------------------------------------------------------
/**
 * @brief   エンド状態かどうか
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevIsEndFunc(void)
{
	return !GFL_NET_IRC_InitializeFlagGet();
}


static BOOL _DevSendDataFunc(void* data,int size,int no,NetDevEndCallback callback)
{
	GFL_NET_IRC_Send(data, size, 0);
	if(callback){
		callback( TRUE );
	}
	return TRUE;
}

static BOOL _DevSetRecvCallbackFunc(PTRCommRecvLocalFunc recvCallback)
{
	GFL_NET_IRC_RecieveFuncSet(recvCallback);
	return TRUE;
}


static u32 _DevGetCurrentIDFunc(void) ///<DevGetCurrentID            自分の接続IDを返す
{
	if(GFL_NET_IRC_IsConnect()){
		return GFL_NET_IRC_System_GetCurrentAid();
	}
	return GFL_NET_NO_PARENTMACHINE;
}

static int _DevGetErrorFunc(void)
{
	return GFL_NET_IRC_ErrorCheck();
}

static void _DevIrcMoveFunc(void)
{
	GFL_NET_IRC_Move();
}

static BOOL _DevIsSendDataFunc(void)
{
	return GFL_NET_IRC_SendCheck();
}


static BOOL _DevGetSendTurnFunc(void)
{
	return GFL_NET_IRC_SendTurnGet();
}

static BOOL _DevIsConnectSystemFunc(void)
{
	return GFL_NET_IRC_IsConnectSystem();
}

static BOOL _DevIsConnectFunc(void)
{
	return GFL_NET_IRC_IsConnect();
}

//------------------------------------------------------------------------------
/**
 * @brief   アイドル状態かどうか  IRCには待機状態がないので STARTと同じにしてある
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevIsIdleFunc(void)
{
	return GFL_NET_IRC_InitializeFlagGet();
}

//------------------------------------------------------------------------------
/**
 * @brief   BITMAPを返す
            赤外線は１vs１なので
 * @retval  none
 */
//------------------------------------------------------------------------------

static u32 _DevGetBitmapFunc(void)
{
	if(GFL_NET_IRC_IsConnect()){
		return 0x03;
	}
	return 0;
}

static BOOL _DevGetSendLockFlagFunc(void)
{
	return GFL_NET_IRC_SendLockFlagGet();
}

static void _DevConnectWorkInitFunc(void)
{
	GFL_NET_IRC_FirstConnect();
}


//------------------------------------------------------------------------------
/**
 * @brief   ワイヤレスデバイステーブルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
GFLNetDevTable *NET_GetIrcDeviceTable(void)
{
	return &netDevTbl;
}

