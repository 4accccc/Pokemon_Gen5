//=============================================================================
/**
 * @file	  delivery_beacon.c
 * @bfief	  ビーコン配信
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  10/02/15
 */
//=============================================================================

#include <gflib.h>


#include "net/network_define.h"

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/net_err.h"

#include "net/delivery_beacon.h"
#include "net/wih.h"
#include "net_whpipe.h"


#define _BEACON_CHANGE_COUNT (0)

//ビーコン単体の中身
typedef struct 
{
  u16 crc16;      // CRC16-CCITT 生成多項式 0x1021 初期値0xffff出力XORなし 左送り  このCRCはビーコン単体用
  u16 ConfusionID;         // 何の配信なのか  同時配信時にかぶらせないように
  u8 data[DELIVERY_BEACON_ONCE_NUM];   // 全体のデータでRC4がかかっている
  u8 count;       //何回目のビーコンか
  u8 countMax;       //いくつ連結するのか
} DELIVERY_BEACON;


typedef void (StateFunc)(DELIVERY_BEACON_WORK* pState);

//ローカルワーク
struct _DELIVERY_BEACON_LOCALWORK{
  DELIVERY_BEACON_INIT aInit;   //初期化構造体のコピー
  DELIVERY_BEACON aSendRecv[DELIVERY_BEACON_MAX_NUM];  //配信する、受け取る構造体
  int nowCount;
  StateFunc* state;      ///< ハンドルのプログラム状態
};

#define _BCON_GET_NUM (16)



static void _netConnectFunc(void* pWork,int hardID);
static void _netNegotiationFunc(void* pWork,int NetID);
static void* _netBeaconGetFunc(void* pWork);
static int _netBeaconGetSizeFunc(void* pWork);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
static void _endCallBack(void* pWork);
static void _RecvDummyData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);



static const NetRecvFuncTable _CommPacketTbl[] = {
  {_RecvDummyData,         NULL},    ///DUMMY
};


static GFLNetInitializeStruct aGFLNetInit = {
  _CommPacketTbl,  // 受信関数テーブル
  NELEMS(_CommPacketTbl), // 受信テーブル要素数
  _netConnectFunc,    ///< ハードで接続した時に呼ばれる
  _netNegotiationFunc,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  _netBeaconGetFunc,  // ビーコンデータ取得関数
  _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
  _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  NULL,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  _endCallBack,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
#if GFL_NET_WIFI
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  NULL,   ///< DWC形式の友達リスト
  NULL,  ///< DWCのユーザデータ（自分のデータ）
  0,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  SYASHI_NETWORK_GGID,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,  //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
  1,                            // 最大接続人数
  88,                  //最大送信バイト数
  _BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,  //通信種別
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_DEBUG_OHNO_SERVICEID,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
  500,
  0,
};


static void _netConnectFunc(void* pWork,int hardID)
{}
static void _RecvDummyData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{}
static void _netNegotiationFunc(void* pWork,int NetID)
{}
static void _endCallBack(void* pWork)
{}



//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------

static void* _netBeaconGetFunc(void* pWk)
{
  DELIVERY_BEACON_WORK* pWork=pWk;
  int no = pWork->nowCount%DELIVERY_BEACON_MAX_NUM;
  return &pWork->aSendRecv[no];
}

///< ビーコンデータサイズ取得関数
static int _netBeaconGetSizeFunc(void* pWork)
{
  return sizeof(DELIVERY_BEACON);
}

///< ビーコンデータ取得関数
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
{
  if(myNo != beaconNo){
    return FALSE;
  }
  return TRUE;
}



/**
 * @brief   ステートの変更
 * @param   state  変えるステートの関数
 * @retval  none
 */

static void _changeState(DELIVERY_BEACON_WORK* pWork, StateFunc state)
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
static void _changeStateDebug(DELIVERY_BEACON_WORK* pWork,StateFunc state, int line)
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


//データをビーコンの形に分解
static void _beaconDataDiv(DELIVERY_BEACON_WORK* pWork)
{
  int i;
  int max = (pWork->aInit.datasize/DELIVERY_BEACON_ONCE_NUM)+1;

  OS_TPrintf("%d %d\n",pWork->aInit.datasize , (DELIVERY_BEACON_MAX_NUM*DELIVERY_BEACON_ONCE_NUM));
  GF_ASSERT(pWork->aInit.datasize < (DELIVERY_BEACON_MAX_NUM*DELIVERY_BEACON_ONCE_NUM));

  for(i = 0; i < DELIVERY_BEACON_MAX_NUM ; i++){
    DELIVERY_BEACON* pBeacon;
    DELIVERY_BEACON_INIT* pInit;
    u8* pData;
    u16 crc;

    pInit = &pWork->aInit;
    pBeacon = &pWork->aSendRecv[i];
    GFL_STD_MemCopy( &pWork->aInit.pData[DELIVERY_BEACON_ONCE_NUM*i] , pBeacon->data,  DELIVERY_BEACON_ONCE_NUM);
    pBeacon->count = i+1;
    pBeacon->countMax = max;
    pBeacon->ConfusionID = pInit->ConfusionID;

    pData = (u8*)pBeacon;
    
    crc = GFL_STD_CrcCalc( &pData[2], DELIVERY_BEACON_ONCE_NUM-2);
    pBeacon->crc16 = crc;
  }

}

//一定間隔でビーコンいれかえ送信
static void _sendLoop(DELIVERY_BEACON_WORK* pWork)
{
  if( WHGetBeaconSendNum() > _BEACON_CHANGE_COUNT ){
    pWork->nowCount++;
    NET_WHPIPE_BeaconSetInfo();
    OS_TPrintf("いれかえ%d\n",pWork->nowCount);
  }
}
//サーバ起動
static void _sendInit1(DELIVERY_BEACON_WORK* pWork)
{
  GFL_NET_InitServer();
  _CHANGE_STATE( _sendLoop );
}
//初期化完了 
static void _sendInit(void* pWK)
{
  DELIVERY_BEACON_WORK* pWork = pWK;
  _CHANGE_STATE(_sendInit1);
}
//何もしない
static void _not(DELIVERY_BEACON_WORK* pWork)
{
}

//--------------------------------------------------------------
/**
 * @brief   ビーコン配信初期化
 * @param   DELIVERY_BEACON_INIT 初期化構造体 ローカル変数でも大丈夫
 * @retval  DELIVERY_BEACON_WORK 管理ワーク
 */
//--------------------------------------------------------------

DELIVERY_BEACON_WORK* DELIVERY_BEACON_Init(DELIVERY_BEACON_INIT* pInit)
{
  int i;
  DELIVERY_BEACON_WORK*  pWork = GFL_HEAP_AllocClearMemory(pInit->heapID,sizeof(DELIVERY_BEACON_WORK));

  GFL_STD_MemCopy(pInit, &pWork->aInit, sizeof(DELIVERY_BEACON_INIT));
  return pWork;
}

//--------------------------------------------------------------
/**
 * @brief   ビーコン配信開始
 * @param   DELIVERY_BEACON_WORK 管理ワーク
 */
//--------------------------------------------------------------
BOOL DELIVERY_BEACON_SendStart(DELIVERY_BEACON_WORK* pWork)
{

  if( GFL_NET_IsInit() ){  // もう通信している場合終了
    return FALSE;
  }
  aGFLNetInit.gsid = pWork->aInit.NetDevID;
  _beaconDataDiv( pWork );

  GFL_NET_Init(&aGFLNetInit, _sendInit, pWork);

  _CHANGE_STATE( _not );
  return TRUE;
}




//格納場所作成
static void _beaconAlloc(DELIVERY_BEACON_WORK* pWork)
{
  if(pWork->aInit.datasize){
    int max = (pWork->aInit.datasize/DELIVERY_BEACON_ONCE_NUM)+1;
    OS_TPrintf("%d %d\n",pWork->aInit.datasize , (DELIVERY_BEACON_MAX_NUM*DELIVERY_BEACON_ONCE_NUM));
    GF_ASSERT(pWork->aInit.datasize < (DELIVERY_BEACON_MAX_NUM*DELIVERY_BEACON_ONCE_NUM));
  }
}


//データ取得検査
static BOOL  _recvCheck(DELIVERY_BEACON_WORK* pWork)
{
  int i;
  int max = pWork->aSendRecv[0].countMax;
  int ConfusionID = pWork->aSendRecv[0].ConfusionID;  //0のビーコン情報と同じ物をそろえる

  if(max == 0){
    return FALSE;
  }
  for(i = 0;i < max;i++){
    DELIVERY_BEACON* pBeacon = &pWork->aSendRecv[i];
    if(max != pBeacon->countMax){
      return  FALSE;
    }
    if(ConfusionID != pBeacon->ConfusionID){
      // IDがちがったら消す
      GFL_STD_MemClear( pBeacon,  sizeof(DELIVERY_BEACON));
      return  FALSE;
    }
  }
  return TRUE;
}

//ビーコンスキャン
static void  _recvLoop(DELIVERY_BEACON_WORK* pWork)
{
  DELIVERY_BEACON* pBeacon;
//  DELIVERY_BEACON_INIT* pInit;
  int i,index;
  int max;
//  pInit = &pWork->aInit;

  for(i = 0;i < _BCON_GET_NUM;i++){
    DELIVERY_BEACON* pBeacon = GFL_NET_WLGetUserBss(i);
    u8* pData;
    u16 crc;

    if(pBeacon==NULL){
      continue;
    }
    index = pBeacon->count - 1;
    if(index >= DELIVERY_BEACON_MAX_NUM){
      OS_TPrintf("DELIVERY_BEACON_MAX_NUM\n");
      continue;
    }
    if(pWork->aSendRecv[index].count != 0){
      continue;  //もう拾っている
    }
    pData = (u8*)pBeacon;
    crc = GFL_STD_CrcCalc( &pData[2], DELIVERY_BEACON_ONCE_NUM-2);
    if(crc != pBeacon->crc16){
      OS_TPrintf("間違ったデータ\n");
      continue; //間違ったデータ
    }
    //取得
    OS_TPrintf("取得%d \n",index);

    GFL_NET_WLFIXScan( i ); //スキャンを限定する

    GFL_STD_MemCopy( pBeacon, &pWork->aSendRecv[index],  sizeof(DELIVERY_BEACON));
  }

}
//   連結したビーコンデータをつくる
static void _CreateRecvData(DELIVERY_BEACON_WORK* pWork)
{
  int i;
  
  for(i = 0; i < DELIVERY_BEACON_MAX_NUM ; i++){
    DELIVERY_BEACON* pBeacon;
    DELIVERY_BEACON_INIT* pInit;
    u8* pData;
    u16 block = i*DELIVERY_BEACON_ONCE_NUM;

    pInit = &pWork->aInit;
    pBeacon = &pWork->aSendRecv[i];

    if(pInit->datasize > (block + DELIVERY_BEACON_ONCE_NUM)){
      GFL_STD_MemCopy( pBeacon->data , &pInit->pData[i*DELIVERY_BEACON_ONCE_NUM],  DELIVERY_BEACON_ONCE_NUM);
    }
    else{
      GFL_STD_MemCopy( pBeacon->data , &pInit->pData[i*DELIVERY_BEACON_ONCE_NUM],  (pInit->datasize - block));
    }
  }
}


//ビーコンスキャン開始
static void _recvInit1(DELIVERY_BEACON_WORK* pWork)
{
  GFL_NET_StartBeaconScan();
  _CHANGE_STATE(_recvLoop);
}
//初期化完了 
static void _recvInit(void* pWK)
{
  DELIVERY_BEACON_WORK* pWork = pWK;
  _CHANGE_STATE(_recvInit1);
}


//--------------------------------------------------------------
/**
 * @brief   ビーコン受信開始
 * @param   DELIVERY_BEACON_WORK 管理ワーク
 * @retval  開始できたらTRUE
 */
//--------------------------------------------------------------
BOOL DELIVERY_BEACON_RecvStart(DELIVERY_BEACON_WORK* pWork)
{

  if( GFL_NET_IsInit() ){  // もう通信している場合終了
    return FALSE;
  }
  aGFLNetInit.gsid = pWork->aInit.NetDevID;
  _beaconAlloc( pWork );
  GFL_NET_Init(&aGFLNetInit, _recvInit, pWork);
  _CHANGE_STATE( _not );
  return TRUE;
}


//--------------------------------------------------------------
/**
 * @brief   ビーコンが１つでもあるかどうか
 * @param   DELIVERY_BEACON_WORK 管理ワーク
 * @retval  ある場合TRUE
 */
//--------------------------------------------------------------
BOOL DELIVERY_BEACON_RecvSingleCheck(DELIVERY_BEACON_WORK* pWork)
{
  int i;

  for(i = 0;i < DELIVERY_BEACON_MAX_NUM;i++){
    DELIVERY_BEACON* pBeacon = &pWork->aSendRecv[i];
    if(0 != pBeacon->countMax){
      return TRUE;
    }
  }
  return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief   ビーコン受信完了かどうか
 * @param   DELIVERY_BEACON_WORK 管理ワーク
 * @retval  受け取れた場合TRUE
 */
//--------------------------------------------------------------
BOOL DELIVERY_BEACON_RecvCheck(DELIVERY_BEACON_WORK* pWork)
{
  BOOL bRet = _recvCheck(pWork);

  if(bRet){
    _CreateRecvData(pWork);
  }
  return bRet;
}


//--------------------------------------------------------------
/**
 * @brief   ビーコン配送メイン
 * @param   DELIVERY_BEACON_WORK 管理ワーク
 */
//--------------------------------------------------------------

void DELIVERY_BEACON_Main(DELIVERY_BEACON_WORK*  pWork)
{
  pWork->state(pWork);
}



//--------------------------------------------------------------
/**
 * @brief   ビーコン配信終了
 * @param   DELIVERY_BEACON_WORK 管理ワーク
 */
//--------------------------------------------------------------

void DELIVERY_BEACON_End(DELIVERY_BEACON_WORK*  pWork)
{
  GFL_HEAP_FreeMemory(pWork);
  GFL_NET_Exit(NULL);
}

