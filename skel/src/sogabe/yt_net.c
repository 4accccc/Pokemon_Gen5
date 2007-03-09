//============================================================================================
/**
 * @file	yt_net.c
 * @brief	DS版ヨッシーのたまご ネットワーク関連のプログラム
 * @author	ohno
 * @date	2007.03.05
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"
#include "yt_net.h"
#include "../ohno/fatal_error.h"


//通信用構造体
struct _NET_PARAM
{
    GFL_NETHANDLE* pNetHandle[2];  //通信用ハンドル 親と子のハンドルを管理するから２個必要
    u32 seq;
    BOOL bGameStart;
};


#define _BCON_GET_NUM  (1)

static void _netAutoParent(void* work)
{
//    NET_PARAM* pNet= (NET_PARAM*)work;
//    pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // クライアントハンドル作成
}

typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { 1 };


static void* _netBeaconGetFunc(void)    ///< ビーコンデータ取得関数
{
    return &_testBeacon;
}

static int _netBeaconGetSizeFunc(void)    ///< ビーコンデータサイズ取得関数
{
    return sizeof(_testBeacon);
}

#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}

static BOOL _netBeaconCompFunc(int myNo,int beaconNo)    ///< ビーコンデータ取得関数
{
    OS_TPrintf("比較してます%d\n",beaconNo);
    return TRUE;
}

static void _recvCLACTPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
}

#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (120)  // 送信最大サイズ

//普通のワイヤレス通信

// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    {_recvCLACTPos, GFL_NET_COMMAND_VARIABLE, NULL},
    {_recvCLACTPos, GFL_NET_COMMAND_VARIABLE, NULL},
    {_recvCLACTPos, GFL_NET_COMMAND_VARIABLE, NULL},
    {_recvCLACTPos, GFL_NET_COMMAND_VARIABLE, NULL},
};


// 通信初期化構造体  wifi用
static GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ワークポインタ
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    NULL, //_netAutoParent,    //自動接続の際に親になる場合に呼ばれる関数
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 通信を開始するかどうか
};


enum{
    _INIT_WAIT_PARENT,
    _INIT_WAIT_CHILD,
    _SEARCH_CHILD,
    _CONNECT_WAIT,
    _NEGO_START,
    _TIM_START,
    _TIM_OK,
    _LOOP,
};


#define _TEST_TIMING (14)

static void _seqChange(NET_PARAM* pNet, int no)
{
    NET_PRINT("ytst change %d\n",no);
    pNet->seq = no;
}


#define _SEQCHANGE(no)   _seqChange(pNet,no)


BOOL YT_NET_Main(void *work)
{
    NET_PARAM* pNet = work;

    switch(pNet->seq){
      case _INIT_WAIT_PARENT:
        if(GFL_NET_IsInit()){
            pNet->pNetHandle[0] = GFL_NET_CreateHandle();   // ハンドル作成
            GFL_NET_ServerConnect(pNet->pNetHandle[0]); // 自動接続
            pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // ハンドル作成
            _SEQCHANGE(_CONNECT_WAIT);
        }
        break;
      case _INIT_WAIT_CHILD:
        if(GFL_NET_IsInit()){
            pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // ハンドル作成
            GFL_NET_ClientConnect(pNet->pNetHandle[1]); // 自動接続
            _SEQCHANGE( _SEARCH_CHILD );
        }
        break;
      case _SEARCH_CHILD:
        {
            u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る
            if(pData){
                GFL_NET_ClientToAccess(pNet->pNetHandle[1], pData);
                _SEQCHANGE( _CONNECT_WAIT );
            }
        }
        break;
      case _CONNECT_WAIT:
        _SEQCHANGE( _NEGO_START );
        break;
      case _NEGO_START:
        if(pNet->pNetHandle[0]){
            if(GFL_NET_NegotiationRequest( pNet->pNetHandle[1] )){
                _SEQCHANGE( _TIM_START );
            }
        }
        else{
            if(GFL_NET_GetNegotiationConnectNum( pNet->pNetHandle[1]) != 0){
                if(GFL_NET_NegotiationRequest( pNet->pNetHandle[1] )){
                    _SEQCHANGE( _TIM_START );
                }
            }
        }
        break;
      case _TIM_START:
        if(GFL_NET_GetNegotiationConnectNum(pNet->pNetHandle[1])==2){
            GFL_NET_TimingSyncStart(pNet->pNetHandle[1] , _TEST_TIMING);
            _SEQCHANGE( _TIM_OK );
        }
        break;
      case _TIM_OK:
        if(GFL_NET_IsTimingSync(pNet->pNetHandle[1] , _TEST_TIMING)){
            pNet->bGameStart = TRUE;
            _SEQCHANGE( _LOOP );
        }
        break;

    }
    return pNet->bGameStart;
}


//----------------------------------------------------------------------------
/**
 *	@brief	
 */
//-----------------------------------------------------------------------------

void YT_NET_Init(GAME_PARAM* gp, BOOL bParent)
{
    TCBSYS* tcbsys;
    NET_PARAM* pNet = GFL_HEAP_AllocMemory(gp->heapID, sizeof(NET_PARAM));

    GFL_STD_MemClear(pNet, sizeof(NET_PARAM));
    aGFLNetInit.pWork = pNet;
    gp->pNetParam = pNet;

    GFL_NET_sysInit(&aGFLNetInit);

    if(bParent){
        pNet->seq = _INIT_WAIT_PARENT;
    }
    else{
        pNet->seq = _INIT_WAIT_CHILD;
    }

//	GFL_TCB_AddTask(gp->tcbsys,YT_NET_Main,pNet,1);
}



