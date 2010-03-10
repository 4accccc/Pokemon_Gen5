//=============================================================================
/**
 * @file	  delivery_irc.c
 * @bfief	  赤外線配信
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  10/02/27
 */
//=============================================================================

#include <gflib.h>

#include "net/network_define.h"

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"

#include "net/delivery_irc.h"


//ビーコン単体の中身
typedef struct
{
  u8 data[DELIVERY_IRC_MAX_NUM];   // 全体のデータでRC4がかかっている
} DELIVERY_IRC;


typedef void (StateFunc)(DELIVERY_IRC_WORK* pState);

//ローカルワーク
struct _DELIVERY_IRC_LOCALWORK{
  DELIVERY_IRC_INIT aInit;   //初期化構造体のコピー
//   DELIVERY_IRC aSend;  //配信する、受け取る構造体
  DELIVERY_IRC aRecv;  //配信する、受け取る構造体
  u8 end;
  u8 bSend;
  u16 bNego;
  u16 crc;
  StateFunc* state;      ///< ハンドルのプログラム状態
};

#define _BCON_GET_NUM (16)

#define _TIMING_START (23)
#define _TIMING_START2 (24)
#define _TIMING_END (35)

static void _RecvDeliveryData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static u8* _getDeliveryData(int netID, void* pWk, int size);
static void _Recvcrc16Data(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

#define _NET_CMD(x) (x<<8)  //ネットワークサービスIDからコマンドへの変換マクロ

enum{
  _DELIVERY_DATA, //この値に上記マクロを足す
  _CRCCCTI_DATA,
};

static const NetRecvFuncTable _CommPacketTbl[] = {
  {_RecvDeliveryData,         _getDeliveryData},    ///受信データ
  {_Recvcrc16Data,         NULL},    ///受信データ
};



static void _netNegotiationFunc(void* pWk,int NetID)
{
  DELIVERY_IRC_WORK *pWork = pWk;
  pWork->bNego = TRUE;


}


/**
 * @brief   ステートの変更
 * @param   state  変えるステートの関数
 * @retval  none
 */

static void _changeState(DELIVERY_IRC_WORK* pWork, StateFunc state)
{
  pWork->state = state;
}

/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   line 行
 * @retval  none
 */
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(DELIVERY_IRC_WORK* pWork,StateFunc state, int line)
{
  NET_PRINT("deli %d\n",line);
  _changeState(pWork, state);
}
#endif

#ifdef GFL_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state,  __LINE__)
#else  //GFL_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state )
#endif //GFL_NET_DEBUG


// 受信
static void _RecvDeliveryData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  //すでに受信済み
/*
  DELIVERY_IRC_WORK *pWork = pWk;
  int i,j;
  const u8* pU8 = pData;
  
    for(j=0;j<size;){
      for(i=0;i<16;i++){
        OS_TPrintf("%x ",pU8[j]);
        j++;
      }
      OS_TPrintf("\n");
    }
  
  GFL_STD_MemCopy( pData, pWork->aInit.pData,pWork->aInit.datasize);

  OS_TPrintf("_RecvDeliveryData %d %d\n",netID,size);
*/
}


static void _Recvcrc16Data(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const u16* pU16 = pData;
  DELIVERY_IRC_WORK *pWork = pWk;
  
  pWork->crc = pU16[0];
  OS_TPrintf("crc16\n");
}


//------------------------------------------------------------------------------
/**
 * @brief   受信バッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8* _getDeliveryData(int netID, void* pWk, int size)
{
  DELIVERY_IRC_WORK *pWork = pWk;
  return (u8*)pWork->aRecv.data;
}

//なにもしない
static void _nop(DELIVERY_IRC_WORK* pWork)
{
}

//完了
static void _sendInit7(DELIVERY_IRC_WORK* pWork)
{
  if( GFL_NET_IsInit() == FALSE ){
    if(!pWork->bSend){
      pWork->end = DELIVERY_IRC_SUCCESS;
      
      GFL_STD_MemCopy( pWork->aRecv.data, pWork->aInit.pData,pWork->aInit.datasize);
      if(pWork->crc != GFL_STD_CrcCalc( pWork->aInit.pData, pWork->aInit.datasize) ){
        pWork->end = DELIVERY_IRC_FAILED;
      }
    }
    else{
      pWork->end = DELIVERY_IRC_SUCCESS;
    }
    _CHANGE_STATE(_nop);
  }
}

//NET終了
static void _sendInit6(DELIVERY_IRC_WORK* pWork)
{
  GFL_NET_Exit(NULL);
  _CHANGE_STATE(_sendInit7);
}

//終了タイミング
static void _sendInit5(DELIVERY_IRC_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_END, pWork->aInit.NetDevID) ){
    _CHANGE_STATE(_sendInit6);
  }
}

//終了タイミング
static void _sendInit4(DELIVERY_IRC_WORK* pWork)
{
  u16 crc = GFL_STD_CrcCalc( pWork->aInit.pData, pWork->aInit.datasize);

  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),  _TIMING_END, pWork->aInit.NetDevID);
  _CHANGE_STATE(_sendInit5);
}


//送る
static void _sendInit3(DELIVERY_IRC_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_START2, pWork->aInit.NetDevID) ){
    if(pWork->bSend){
      if( GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle() , GFL_NET_SENDID_ALLUSER ,
                              _DELIVERY_DATA + _NET_CMD( pWork->aInit.NetDevID ), pWork->aInit.datasize ,
                              pWork->aInit.pData , FALSE , FALSE , TRUE )){
        _CHANGE_STATE(_sendInit4);
      }
    }
    else{
      _CHANGE_STATE(_sendInit4);
    }
  }
}

static void _sendInit25(DELIVERY_IRC_WORK* pWork)

{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_START, pWork->aInit.NetDevID) ){
    if( pWork->bSend ){
      u16 crc=0;
      crc = GFL_STD_CrcCalc( pWork->aInit.pData, pWork->aInit.datasize);
      if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle() , _CRCCCTI_DATA + _NET_CMD( pWork->aInit.NetDevID ), sizeof(u16) ,  &crc )){
        GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),  _TIMING_START2, pWork->aInit.NetDevID);
        pWork->bNego=FALSE;
        _CHANGE_STATE(_sendInit3);
      }
    }
    else{
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),  _TIMING_START2, pWork->aInit.NetDevID);
      _CHANGE_STATE(_sendInit3);
    }
  }
}


//タイミング
static void _sendInit2(DELIVERY_IRC_WORK* pWork)
{
  if( GFL_NET_GetConnectNum() > 1 ){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),  _TIMING_START, pWork->aInit.NetDevID);
    _CHANGE_STATE(_sendInit25);
  }
}

//接続
static void _sendInit1(DELIVERY_IRC_WORK* pWork)
{
  GFL_NET_ChangeoverConnect( NULL );
  _CHANGE_STATE(_sendInit2);
}

//初期化完了
static void _sendInit(DELIVERY_IRC_WORK* pWork)
{
  if( GFL_NET_IsInit() == TRUE ){
    _CHANGE_STATE(_sendInit1);
  }
}

//--------------------------------------------------------------
/**
 * @brief   赤外線配信初期化
 * @param   DELIVERY_IRC_INIT 初期化構造体 ローカル変数でも大丈夫
 * @retval  DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------

DELIVERY_IRC_WORK* DELIVERY_IRC_Init(DELIVERY_IRC_INIT* pInit)
{
  int i;
  DELIVERY_IRC_WORK*  pWork = GFL_HEAP_AllocClearMemory(pInit->heapID,sizeof(DELIVERY_IRC_WORK));

  GFL_STD_MemCopy(pInit, &pWork->aInit, sizeof(DELIVERY_IRC_INIT));
  return pWork;
}


static BOOL _initNet(DELIVERY_IRC_WORK* pWork)
{
  static GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), //受信テーブル要素数
    NULL,   ///< ハードで接続した時に呼ばれる
    _netNegotiationFunc,   ///< ネゴシエーション完了時にコール
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    NULL,//IrcBattleBeaconGetFunc,  // ビーコンデータ取得関数
    NULL,//IrcBattleBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    NULL,//IrcBattleBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL,            // 普通のエラーが起こった場合 通信終了
    NULL,//FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL, //_endCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
    SYASHI_NETWORK_GGID,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
    GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
    2,     // 最大接続人数
    64,  //最大送信バイト数
    16,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_IRC_BATTLE,  //GameServiceID
    0xfffe, // 赤外線タイムアウト時間
    0,//MP通信親機送信バイト数
    0,//dummy
  };

  if( GFL_NET_IsInit() ){  // もう通信している場合終了
    return FALSE;
  }
  aGFLNetInit.gsid = pWork->aInit.NetDevID;

  GFL_NET_Init(&aGFLNetInit, NULL, pWork);

  return TRUE;
}




//--------------------------------------------------------------
/**
 * @brief   赤外線配信開始
 * @param   DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------
BOOL DELIVERY_IRC_SendStart(DELIVERY_IRC_WORK* pWork)
{
  BOOL bRet = _initNet(pWork);
  pWork->bSend=TRUE;
  _CHANGE_STATE(_sendInit);
  return bRet;

}


//--------------------------------------------------------------
/**
 * @brief   IRC受信開始
 * @param   DELIVERY_IRC_WORK 管理ワーク
 * @retval  開始できたらTRUE
 */
//--------------------------------------------------------------
BOOL DELIVERY_IRC_RecvStart(DELIVERY_IRC_WORK* pWork)
{

  BOOL bRet = _initNet(pWork);
  pWork->bSend = FALSE;
  _CHANGE_STATE(_sendInit);
  return bRet;
}

//--------------------------------------------------------------
/**
 * @brief   IRC受信完了かどうか
 * @param   DELIVERY_IRC_WORK 管理ワーク
 * @retval  受け取れた場合DELIVERY_IRC_SUCCESS 失敗DELIVERY_IRC_FAILED
 */
//--------------------------------------------------------------
int DELIVERY_IRC_RecvCheck(DELIVERY_IRC_WORK* pWork)
{
  return pWork->end;
}


//--------------------------------------------------------------
/**
 * @brief   IRC配送メイン
 * @param   DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------

void DELIVERY_IRC_Main(DELIVERY_IRC_WORK*  pWork)
{
  if( pWork->state )
  { 
    pWork->state(pWork);
  }
}



//--------------------------------------------------------------
/**
 * @brief   IRC配信終了
 * @param   DELIVERY_IRC_WORK 管理ワーク
 */
//--------------------------------------------------------------

void DELIVERY_IRC_End(DELIVERY_IRC_WORK*  pWork)
{
  GFL_HEAP_FreeMemory(pWork);
  GFL_NET_Exit(NULL);
}

