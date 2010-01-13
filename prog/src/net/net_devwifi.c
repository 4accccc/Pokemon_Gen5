//=============================================================================
/**
 * @file	net_devwifi.c
 * @bfief	WIFI通信ドライバー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	08/12/22
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "net/network_define.h"

#define _WIFILOBBY (0)

#include "net/dwc_rap.h"
#include "dwc_rapinitialize.h"
#include "net/dwc_rapcommon.h"
#include "dwc_rapfriend.h"


extern GFLNetDevTable *NET_GetWifiDeviceTable(void);

static void _DevBootFunc(HEAPID heapID, NetErrorFunc errorFunc);
static BOOL _DevInitFunc(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork);
static int _DevStartFunc(NetDevEndCallback callback);
static int _DevMainFunc(u16 bitmap);
static BOOL _DevEndFunc(BOOL bForce,NetDevEndCallback callback);
static BOOL _DevExitFunc(NetDevEndCallback callback);
static BOOL _DevChangeOverModeConnectFunc(u8* keyStr,int numEntry, BOOL bParent, u32 timelimit,NetDevEndCallback callback);
static BOOL _DevWifiConnectFunc(int index, int maxnum, BOOL bVCT);
static BOOL _DevModeDisconnectFunc(BOOL bForce, NetDevEndCallback callback);
static BOOL _DevSendDataFunc(void* data,int size, int no,NetDevEndCallback callback);
static BOOL _DevSetRecvCallbackFunc(PTRCommRecvLocalFunc recvCallback);
static BOOL _DevIsStartFunc(void);
static BOOL _DevIsConnectFunc(void);
static BOOL _DevIsEndFunc(void);
static BOOL _DevIsIdleFunc(void); ///<アイドル状態かどうか
static u32 _DevGetBitmapFunc(void);
static u32 _DevGetCurrentIDFunc(void);
static int _DevGetIconLevelFunc(void);
static int _DevGetError(void);  ///< エラーを得る
static void _DevSetNoChildErrorSet(BOOL bOn);
static BOOL _DevIsConnectable(int index);
static BOOL _DevIsVChat(void);
static BOOL _DevIsNewPlayer(void);
static void _DevSetClinetConnect(BOOL bEnable);
static BOOL _DevLobbyLogin(const void* cp_loginprofile);
static void _DevDebugSetRoom( u32 locktime, u32 random, u8 roomtype, u8 season );
static BOOL _DevLobbyUpdateErrorCheck(void);
static BOOL _DevLobbyLoginWait(void);
static void _DevLobbyLogout(void);
static BOOL _DevLobbyLogoutWait(void);
static BOOL _DevLobbyMgCheckRecruit( int type );
static BOOL _DevLobbyMgStartRecruit( int type, u32 maxnum );
static BOOL _DevLobbyMgEntry( int type );
static BOOL _DevLobbyMgForceEnd( void );
static void _DevLobbyMgEndConnect( void );
static BOOL _DevLobbyMgMyParent( void );
static void _DevLobbyMgEndRecruit( void );


//--------------------------------------------
// 構造体
//--------------------------------------------

static GFLNetDevTable netDevTbl={
  _DevBootFunc,
  _DevInitFunc,
  _DevStartFunc,
  _DevMainFunc,
  _DevEndFunc,
  _DevExitFunc,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  _DevChangeOverModeConnectFunc,
  _DevWifiConnectFunc,
  _DevModeDisconnectFunc,
  NULL,
  NULL,
  NULL,
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
  _DevIsConnectable,
  _DevIsVChat,
  _DevIsNewPlayer,
  NULL, //_DevIrcMoveFunc

  NULL,	//DevIsConnectSystemFunc
  NULL,	//DevGetSendLockFlagFunc
  NULL,	//DevConnectWorkInitFunc
  NULL,	//DevGetSendLockFlagFunc
  NULL,	//DevConnectWorkInitFunc
	_DevSetClinetConnect, //DevSetClientConnectFunc DevSetClientConnect; ///< 子機がつながってよいかどうかハードレベルで調整
  NULL, //DevCrossScanFunc

  //-- 以下、Wi-Fi広場専用
  _DevLobbyLogin,	//DevLobbyLoginFunc
  _DevDebugSetRoom,	//DevDebugSetRoomFunc
  _DevLobbyUpdateErrorCheck,	//DevLobbyUpdateErrorCheckFunc
  _DevLobbyLoginWait,	//DevLobbyLoginWaitFunc
  _DevLobbyLogout,	//DevLobbyLogoutFunc
  _DevLobbyLogoutWait,	//DevLobbyLogoutWaitFunc
  _DevLobbyMgCheckRecruit,    //DevLobbyMgCheckRecruitFunc
  _DevLobbyMgStartRecruit,    //DevLobbyMgStartRecruitFunc
  _DevLobbyMgEntry,   //DevLobbyMgEntryFunc
  _DevLobbyMgForceEnd,    //DevLobbyMgForceEndFunc
  _DevLobbyMgEndConnect,    //DevLobbyMgEndConnectFunc
  _DevLobbyMgMyParent,    //DevLobbyMgMyParentFunc
  _DevLobbyMgEndRecruit,    //DevLobbyMgEndRecruitFunc
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
 * @brief   DS起動時に行う処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _DevBootFunc(HEAPID heapID, NetErrorFunc errorFunc)
{
//DWC5.3では使わない
//  DWC_SetAuthServer(GF_DWC_CONNECTINET_AUTH_TYPE);
  GFL_NET_WifiStart( heapID, errorFunc );

  //@todo 置き換えが必要
  GF_ASSERT(DWC_ERROR_FRIENDS_SHORTAGE == __ERROR_FRIENDS_SHORTAGE_DWC);

}

//------------------------------------------------------------------------------
/**
 * @brief   デバイス初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevInitFunc(HEAPID heapID, GFL_NETSYS* pNet,NetDevEndCallback callback, void* pUserWork)
{
  if(_pWork){
    return FALSE;  //すでに初期化済み
  }
  {
    GFLNetInitializeStruct* pNetInit = GFL_NET_GetNETInitStruct();
    DWC_RAPCOMMON_SetHeapID(pNetInit->wifiHeapID, pNetInit->heapSize);
  }
  _pWork = GFL_HEAP_AllocClearMemory(heapID, sizeof(NetWork));
  _pWork->pNet = pNet;
  return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   接続  WIFI接続までやる
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevStartFunc(NetDevEndCallback callback)
{
  int ret = GFL_NET_DWC_startConnect( GFI_NET_GetMyDWCUserData() ,GFI_NET_GetMyDWCFriendData());
  if(callback)
    callback(TRUE);
  //    mydwc_setFetalErrorCallback(CommFatalErrorFunc);   //@@OO エラー処理追加必要 07/02/22
  return ret;
}

//------------------------------------------------------------------------------
/**
 * @brief   メインループ
 * @retval  none
 */
//------------------------------------------------------------------------------

static int _DevMainFunc(u16 bitmap)
{
  return GFL_NET_DWC_stepmatch(bitmap);
}

//------------------------------------------------------------------------------
/**
 * @brief   通信を一旦終了する
 * @param   bForce 強制終了したい場合TRUE
 * @retval  TRUE 成功 FALSE 失敗＝呼び続ける必要がある
 */
//------------------------------------------------------------------------------
static BOOL _DevEndFunc(BOOL bForce, NetDevEndCallback callback)
{
  if(!GFL_NET_DWC_IsInit()){
    return TRUE;
  }
  if(GFL_NET_DWC_disconnect( !bForce ) )
  {
    if(GFL_NET_DWC_returnLobby())
    {
      return TRUE;
    }
  }
  else
  {
    GFL_NET_DWC_stepmatch(bForce);
  }
  return FALSE;
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
    GFL_NET_DWC_Logout();  // 切断
    if(callback){
      callback(TRUE);
    }
    GFL_HEAP_FreeMemory(_pWork);
    _pWork = NULL;
  }
  DWC_RAPCOMMON_ResetHeapID();
  return TRUE;
}


//------------------------------------------------------------------------------
/**
 * @brief   ランダム接続開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevChangeOverModeConnectFunc(u8* keyStr,int numEntry, BOOL bParent, u32 timelimit, NetDevEndCallback callback)
{
  return GFL_NET_DWC_StartMatch( keyStr, numEntry,  bParent, timelimit );
}

//------------------------------------------------------------------------------
/**
 * @brief   WIFI友人指定で接続開始
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevWifiConnectFunc(int index, int maxnum, BOOL bVCT)
{
  return GFL_NET_DWC_StartGame(index ,maxnum, bVCT );
}

//------------------------------------------------------------------------------
/**
 * @brief   接続を終わる
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevModeDisconnectFunc(BOOL bForce, NetDevEndCallback callback)
{
  if(GFL_NET_DWC_disconnect( !bForce )){
    GFL_NET_DWC_returnLobby();
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   データを送る
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevSendDataFunc(void* data, int size, int no, NetDevEndCallback callback)
{
  if(GFL_NET_DWC_SendToOther(data, size)){
    if(callback){
      callback(TRUE);
    }
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   受信コールバックの設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevSetRecvCallbackFunc(PTRCommRecvLocalFunc recvCallback)
{
  GFL_NET_DWC_setReceiver(recvCallback,recvCallback);
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
  return GFL_NET_DWC_IsInit();
}

//------------------------------------------------------------------------------
/**
 * @brief   相手と接続してるかどうか
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsConnectFunc(void)
{
  return (MYDWC_NONE_AID != GFL_NET_DWC_GetAid());
}

//------------------------------------------------------------------------------
/**
 * @brief   通信終了してるかどうか
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsEndFunc(void)
{
  return !GFL_NET_DWC_IsInit();
}


//------------------------------------------------------------------------------
/**
 * @brief   何もしていない　次の状態にいける状態かどうか？
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsIdleFunc(void)
{
  return GFL_NET_DWC_IsLogin();
}

//------------------------------------------------------------------------------
/**
 * @brief   接続状態をBITMAPで返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u32 _DevGetBitmapFunc(void)
{
  return DWC_GetAIDBitmap();
}



//------------------------------------------------------------------------------
/**
 * @brief   自分のNetIDを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u32 _DevGetCurrentIDFunc(void)
{
  return GFL_NET_DWC_GetAid();
}

//------------------------------------------------------------------------------
/**
 * @brief   アイコンレベルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevGetIconLevelFunc(void)
{
  return (WM_LINK_LEVEL_3 - DWC_GetLinkLevel());
}

//------------------------------------------------------------------------------
/**
 * @brief   エラー番号を返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static int _DevGetError(void)  ///< エラーを得る
{
  int errorCode;
  DWCErrorType myErrorType;
  int ret;

  ret = DWC_GetLastErrorEx( &errorCode, &myErrorType );

  if(ret == DWC_ERROR_NONE){
    return 0;
  }
  else if(errorCode==0){  //WIFIでは０番が帰ることがあるため変換して返す
    return -10000;
  }
  return errorCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _DevSetNoChildErrorSet(BOOL bOn)
{
  //    GFL_NET_WLSetDisconnectOtherError(bOn);
}


//------------------------------------------------------------------------------
/**
 * @brief   相手がつないでよい状態かどうか (ほぼfriendWifi用)
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevIsConnectable(int index)
{
  if( GFL_NET_DWC_getFriendStatus(index) != DWC_STATUS_MATCH_SC_SV ){
    return FALSE;
  }
  //    mydwc_setReceiver( CommRecvParentCallback, CommRecvCallback );
  return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   ボイスチャット状態かどうか (ほぼfriendWifi用)
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _DevIsVChat(void)
{
  return GFL_NET_DWC_IsVChat();
}

//------------------------------------------------------------------------------
/**
 * @brief   接続してきたかどうかを返します
 * @retval  TRUE…接続開始なのでキー操作をブロック   FALSE…
 */
//------------------------------------------------------------------------------
static BOOL _DevIsNewPlayer(void)
{
  return GFL_NET_DWC_IsNewPlayer();
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
	GFL_NET_DWC_SetClinetConnect(bEnable);
}

//--------------------------------------------------------------
/**
 * @brief   Wi-Fi広場にログイン	DWC_LOBBY_Login
 *
 * @param   cp_loginprofile
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	失敗		失敗した場合エラータイプを取得してLogoutしてください
 */
//--------------------------------------------------------------
static BOOL _DevLobbyLogin(const void* cp_loginprofile)
{
#if _WIFILOBBY
  return DWC_LOBBY_Login(cp_loginprofile);
#else
  return FALSE;
#endif
}

//--------------------------------------------------------------
/**
 * @brief   デバッグ用 部屋データ設定DWC_LOBBY_DEBUG_SetRoomData
 *
 * @param   locktime
 * @param   random
 * @param   roomtype
 * @param   season
 */
//--------------------------------------------------------------
static void _DevDebugSetRoom( u32 locktime, u32 random, u8 roomtype, u8 season )
{
#if _WIFILOBBY
  DWC_LOBBY_DEBUG_SetRoomData(locktime, random, roomtype, season);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   DWCロビーを更新し、戻り値でエラー発生をTRUEorFALSEで返す DWC_LOBBY_UpdateErrorCheck
 *
 * @retval  TRUE：正常　FALSE：エラー
 */
//--------------------------------------------------------------
static BOOL _DevLobbyUpdateErrorCheck(void)
{
#if _WIFILOBBY
  return DWC_LOBBY_UpdateErrorCheck();
#else
  return FALSE;
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ログイン完了待ち	DWC_LOBBY_LoginWait
 *
 *	@retval	TRUE	ログイン完了
 *	@retval	FALSE	ログイン中orログイン以外の状態
 */
//--------------------------------------------------------------
static BOOL _DevLobbyLoginWait(void)
{
#if _WIFILOBBY
  return DWC_LOBBY_LoginWait();
#else
  return FALSE;
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ロビーサーバからログアウト		DWC_LOBBY_Logout
 */
//--------------------------------------------------------------
static void _DevLobbyLogout(void)
{
#if _WIFILOBBY
  DWC_LOBBY_Logout();
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ログアウト完了待ち		DWC_LOBBY_LogoutWait
 *
 *	@retval		TRUE	完了
 *	@retval		FALSE	ログアウト中orログアウト状態以外の状態
 */
//--------------------------------------------------------------
static BOOL _DevLobbyLogoutWait(void)
{
#if _WIFILOBBY
  return DWC_LOBBY_LogoutWait();
#else
  return FALSE;
#endif
}

static BOOL _DevLobbyMgCheckRecruit( int type )
{
#if _WIFILOBBY
  return DWC_LOBBY_MG_CheckRecruit(type);
#else
  return FALSE;
#endif
}

static BOOL _DevLobbyMgStartRecruit( int type, u32 maxnum )
{
#if _WIFILOBBY
  return DWC_LOBBY_MG_StartRecruit(type, maxnum);
#else
  return FALSE;
#endif
}

static BOOL _DevLobbyMgEntry( int type )
{
#if _WIFILOBBY
  return DWC_LOBBY_MG_Entry(type);
#else
  return FALSE;
#endif
}

static BOOL _DevLobbyMgForceEnd( void )
{
#if _WIFILOBBY
  return DWC_LOBBY_MG_ForceEnd();
#else
  return FALSE;
#endif
}

static void _DevLobbyMgEndConnect( void )
{
#if _WIFILOBBY
  DWC_LOBBY_MG_EndConnect();
#else
#endif
}

static BOOL _DevLobbyMgMyParent( void )
{
#if _WIFILOBBY
  return DWC_LOBBY_MG_MyParent();
#else
  return FALSE;
#endif
}

static void _DevLobbyMgEndRecruit( void )
{
#if _WIFILOBBY
  DWC_LOBBY_MG_EndRecruit();
#else
#endif
}


//==============================================================================
//
//==============================================================================
//------------------------------------------------------------------------------
/**
 * @brief   ワイヤレスデバイステーブルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
GFLNetDevTable *NET_GetWifiDeviceTable(void)
{
  return &netDevTbl;
}
