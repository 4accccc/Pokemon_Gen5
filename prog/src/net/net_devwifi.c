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
#include "dwc_rap.h"
#include "dwc_rapinitialize.h"
#include "dwc_rapcommon.h"
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
static BOOL _DevIsIdleFunc(void); ///<アイドル状態かどうか
static u32 _DevGetBitmapFunc(void);
static u32 _DevGetCurrentIDFunc(void);
static int _DevGetIconLevelFunc(void);
static int _DevGetError(void);  ///< エラーを得る
static void _DevSetNoChildErrorSet(BOOL bOn);
static BOOL _DevIsConnectable(int index);
static BOOL _DevIsVChat(void);
static BOOL _DevIsNewPlayer(void);


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
    GFL_NET_WifiStart( heapID, errorFunc );
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
    int ret = mydwc_startConnect( GFI_NET_GetMyDWCUserData() ,GFI_NET_GetMyDWCFriendData());
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
    return mydwc_stepmatch(bitmap);
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
    if(mydwc_disconnect( !bForce ) ){
        if(mydwc_returnLobby()){
            return TRUE;
        }
    }
    else{
        mydwc_stepmatch(bForce);
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
        mydwc_Logout();  // 切断
        if(callback){
            callback(TRUE);
        }
        GFL_HEAP_FreeMemory(_pWork);
        _pWork = NULL;
    }
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
    if(mydwc_disconnect( !bForce )){
        mydwc_returnLobby();
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
    mydwc_setReceiver(recvCallback,recvCallback);
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
 * @brief   何もしていない　次の状態にいける状態かどうか？
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _DevIsIdleFunc(void)
{
    return mydwc_IsLogin();
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
        return 1;
    }
    return errorCode;  //WIFIでは０番が帰ることがあるため
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
    if( mydwc_getFriendStatus(index) != DWC_STATUS_MATCH_SC_SV ){
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
    return mydwc_IsVChat();
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
 * @brief   ワイヤレスデバイステーブルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
GFLNetDevTable *NET_GetWifiDeviceTable(void)
{
    return &netDevTbl;
}

