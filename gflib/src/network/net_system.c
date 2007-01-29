//=============================================================================
/**
 * @file	net_system.c
 * @brief	通信システム
 * @author	GAME FREAK Inc.
 * @date    2006.12.05
 */
//=============================================================================

#include "gflib.h"

#include "device/wh_config.h"

#include "net_def.h"
#include "device/net_wireless.h"
#include "net_system.h"
#include "net_command.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "tool/net_tool.h"

//#include "wifi/dwc_rap.h"   //WIFI

#define _SENDRECV_LIMIT  (3)  // 送信と受信の数がずれた場合送信を抑制する

//==============================================================================
// extern宣言
//==============================================================================




//==============================================================================
// 定義
//==============================================================================

/// クライアント送信用キューのバッファサイズ
#define _SENDQUEUE_NUM_MAX  (100)
/// サーバー送信用キューのバッファサイズ
#define _SENDQUEUE_SERVER_NUM_MAX      (280)

//子機送信バッファのサイズ    8台つなぐ場合の任天堂の推奨バイト数
#define _SEND_BUFF_SIZE_CHILD  GFL_NET_CHILD_DATA_SIZE
//子機送信バッファのサイズ    ４台以下接続時の送信バイト数
#define _SEND_BUFF_SIZE_4CHILD  GFL_NET_CHILD_DATA_SIZE
// 子機RING送信バッファサイズ
#define _SEND_RINGBUFF_SIZE_CHILD  (_SEND_BUFF_SIZE_CHILD * 22)
//親機送信バッファのサイズ
#define _SEND_BUFF_SIZE_PARENT  GFL_NET_PARENT_DATA_SIZE
// 親機RING送信バッファサイズ
#define _SEND_RINGBUFF_SIZE_PARENT  (_SEND_BUFF_SIZE_PARENT * 2)

// 子機受信バッファのサイズ
#define _RECV_BUFF_SIZE_CHILD  (_SEND_BUFF_SIZE_PARENT-1)
// 親機受信バッファサイズ
#define _RECV_BUFF_SIZE_PARENT (_SEND_BUFF_SIZE_CHILD-1)



// 初期化されていないイテレーターの数
#define _NULL_ITERATE (-1)
// ありえないID
#define _INVALID_ID  (COMM_INVALID_ID)
// 送信データがまだもらえていなかった
//#define _NODATA_SEND  (0x0e)

//VBlank処理のタスクのプライオリティー
#define _PRIORITY_VBLANKFUNC (0)

/** @brief 通信先頭バイトのBITの意味 */
#define _SEND_NONE  (0x00)  ///< 一回で送れる場合
#define _SEND_NEXT  (0x01)  ///< 一回で送れない場合
#define _MP_DATA_HEADER (0x08)  ///< MPデータの場合BITON  DS時にはOFF
#define _INVALID_HEADER  (0xff)  ///< 初期化＆送信データがまだセットされていない場合


typedef enum{   // 送信状態
    _SEND_CB_NONE,           // なにもしていない
    _SEND_CB_FIRST_SEND,     // 流れの中での最初の送信
    _SEND_CB_FIRST_SENDEND,  // 最初の送信のコールバックが来た
} _sendCallbackCondition_e;

typedef enum{   // 送信形態
    _MP_MODE,    // 親子型
    _DS_MODE,    // 並列型
    _CHANGE_MODE_DSMP,  // DSからMPに切り替え中
    _CHANGE_MODE_MPDS,  // MPからDSに切り替え中
} _connectMode_e;


typedef enum {
    _DEVICE_WIRELESS,
    _DEVICE_WIFI,
} _deviceMode_e;

//==============================================================================
// ワーク
//==============================================================================


typedef struct{
    int dataPoint; // 受信バッファ予約があるコマンドのスタート位置
    u8* pRecvBuff; // 受信バッファ予約があるコマンドのスタート位置
    u16 valSize;
    u8 valCommand;
    u8 sendID;
    u8 recvID;
} _RECV_COMMAND_PACK;


typedef struct{
    /// ----------------------------子機用＆親機用BUFF
    u8 sSendBuf[_SEND_BUFF_SIZE_4CHILD];          ///<  子機の送信用バッファ
    u8 sSendBufRing[_SEND_RINGBUFF_SIZE_CHILD];  ///<  子機の送信リングバッファ
    u8 sSendServerBuf[_SEND_BUFF_SIZE_PARENT];          ///<  親機の送信用バッファ
    u8 sSendServerBufRing[_SEND_RINGBUFF_SIZE_PARENT];
    u8* pMidRecvBufRing;          ///< 受け取るバッファをバックアップする DS専用
    u8* pServerRecvBufRing;       ///< 親機側受信バッファ
    u8* pRecvBufRing;             ///< 子機が受け取るバッファ
    u8* pTmpBuff;                 ///< 受信受け渡しのための一時バッファポインタ
    //----ringbuff manager
    RingBuffWork sendRing;
    RingBuffWork recvRing;                      ///< 子機の受信リングバッファ
    RingBuffWork recvMidRing[GFL_NET_MACHINE_MAX];
    RingBuffWork sendServerRing;
    RingBuffWork recvServerRing[GFL_NET_MACHINE_MAX];
    ///---quemanager 関連
    SEND_QUEUE_MANAGER sendQueueMgr;
    SEND_QUEUE_MANAGER sendQueueMgrServer;
    ///---受信関連
    _RECV_COMMAND_PACK recvCommServer[GFL_NET_MACHINE_MAX];
    _RECV_COMMAND_PACK recvCommClient;
    
    //---------  同期関連
    BOOL bWifiSendRecv;   // WIFIの場合同期を取る時ととらないときが必要なので 切り分ける
    volatile int countSendRecv;   // 送ったら＋受け取ったら－ 回数
    volatile int countSendRecvServer[GFL_NET_MACHINE_MAX];   // 送ったら＋受け取ったら－ 回数

    //-------
    int packetSizeMax;
    u16 bitmap;   // 接続している機器をBIT管理
    
    //-------------------
    NET_TOOLSYS* pTool;  ///< netTOOLのワーク
    GFL_NETHANDLE* pNetHandle[GFL_NET_MACHINE_MAX];
    u8 device;   ///< デバイス切り替え

//    u8 DSCountRecv[GFL_NET_MACHINE_MAX];  // 順番確認用
    u8 DSCount; // 順番確認用
    u8 recvDSCatchFlg[GFL_NET_MACHINE_MAX];  // 通信をもらったことを記憶 DS同期用
    u8 bFirstCatch[GFL_NET_MACHINE_MAX];  // コマンドをはじめてもらった時用

    u8 transmissionNum;
    u8 transmissionSend;
    u8 transmissionType;  // 通信形態 DSかMPかの管理
  //  u8 changeService;    // 通信形態の変更
   // u8 sendSwitch;   // 送信バッファのスイッチフラグ
    u8 sendServerSwitch;   // 送信バッファのスイッチフラグ（サーバ用）
    u8 bFirstCatchP2C;
   // u8 bSendNoneSend;        // 無効コマンドを送る
    u8 bNextSendData;  ///
    u8 bNextSendDataServer;  ///
    u8 bAlone;    // 一人で通信できるようにするモードの時TRUE
    u8 bWifiConnect; //WIFI通信可能になったらTRUE
    u8 bResetState;
    u8 bError;  // 復旧不可能な時はTRUE
    u8 bShutDown;
    u8 bNotRecvCheck;
} _COMM_WORK_SYSTEM;

static _COMM_WORK_SYSTEM* _pComm = NULL;  ///<　ワーク構造体のポインタ
// 親機になる場合のTGID 構造体に入れていないのは
// 通信ライブラリーを切ったとしてもインクリメントしたいため
static u16 _sTgid = 0;

// 送信したことを確認するためのフラグ
static volatile u8 _sendCallBackServer = _SEND_CB_FIRST_SENDEND;
// 同上
static volatile u8 _sendCallBack = _SEND_CB_FIRST_SENDEND;


//==============================================================================
// static宣言
//==============================================================================

static void _commCommandInit(void);
static void _dataMpStep(void);
static void _updateMpDataServer(void);
static void _dataMpServerStep(void);
static void _sendCallbackFunc(BOOL result);
static void _sendServerCallback(BOOL result);
static void _commRecvCallback(u16 aid, u16 *data, u16 size);
static void _commRecvParentCallback(u16 aid, u16 *data, u16 size);

static void _updateMpData(void);

static void _recvDataFunc(void);
static void _recvDataServerFunc(void);
static BOOL _setSendData(u8* pSendBuff);
static void _setSendDataServer(u8* pSendBuff);

static BOOL _sendQueueSet(void);
static BOOL _sendServerQueueSet(void);
static void _queueSet(int restSize);
static void _queueSetServer(int restSize);
static void _spritDataSendFunc(void);

//static void _transmission(void);
static u16 _getUserMaxSendByte(void);


//==============================================================================
/**
 * @brief   最大接続人数を得る
 * @return  最大接続人数
 */
//==============================================================================

static int _getUserMaxNum(void)
{
    return 5;  // 最大接続人数 @@OO 初期化構造体から得る
}

//==============================================================================
/**
 * @brief   最小接続人数を得る
 * @return  最大接続人数
 */
//==============================================================================

static int _getUserMinNum(void)
{
    return 5;  // 最小接続人数 @@OO 初期化構造体から得る
}

//==============================================================================
/**
 * @brief   親子共通、通信の初期化をまとめた
 * @param   packetSizeMax   確保したいパケットサイズ
 * @param   heapID          確保してもいいheapID
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================

static BOOL _commInit(int packetSizeMax, int heapID)
{
    void* pWork;
    int i;

    if(_pComm==NULL){
        int machineMax = _getUserMaxNum();
        NET_PRINT("_COMM_WORK_SYSTEM size %d \n", sizeof(_COMM_WORK_SYSTEM));
        _pComm = (_COMM_WORK_SYSTEM*)GFL_HEAP_AllocMemory(heapID, sizeof(_COMM_WORK_SYSTEM));
        MI_CpuClear8(_pComm, sizeof(_COMM_WORK_SYSTEM));
//        _pComm->pTool = GFL_NET_Tool_sysInit(heapID, machineMax);
        
        _pComm->packetSizeMax = packetSizeMax + 64;
        _pComm->transmissionType = _DS_MODE;
        
        _pComm->pRecvBufRing = GFL_HEAP_AllocMemory(heapID, _pComm->packetSizeMax*2); ///< 子機が受け取るバッファ
        _pComm->pTmpBuff = GFL_HEAP_AllocMemory(heapID, _pComm->packetSizeMax);  ///< 受信受け渡しのための一時バッファ
        _pComm->pServerRecvBufRing = GFL_HEAP_AllocMemory(heapID, machineMax * _pComm->packetSizeMax);   ///< 受け取るバッファをバックアップする
        _pComm->pMidRecvBufRing = GFL_HEAP_AllocMemory(heapID, machineMax * _pComm->packetSizeMax);   ///< 受け取るバッファをバックアップする DS専用
        // キューの初期化

        GFL_NET_QueueManagerInitialize(&_pComm->sendQueueMgr, _SENDQUEUE_NUM_MAX, &_pComm->sendRing, heapID);
        GFL_NET_QueueManagerInitialize(&_pComm->sendQueueMgrServer, _SENDQUEUE_SERVER_NUM_MAX, &_pComm->sendServerRing, heapID);

        _commCommandInit();
    }
    
    _pComm->bitmap = 0;
    _pComm->bWifiConnect = FALSE;
    return TRUE;
}


//==============================================================================
/**
 * @brief   親子共通、通信のコマンド管理の初期化をまとめた
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _commCommandInit(void)
{
    void* pWork;
    int i;

    // ワークの初期化
//    NET_PRINT("コマンド再初期化\n");
    
//    _pComm->sendSwitch = 0;
   // _pComm->sendServerSwitch = 0;
    
    // 親機のみの送受信
    {
        int machineMax = _getUserMaxNum();

        NET_PRINT("packet %d %d\n",_pComm->packetSizeMax,machineMax);
        MI_CpuFill8(_pComm->pServerRecvBufRing, 0, _pComm->packetSizeMax * machineMax);
        for(i = 0; i< machineMax;i++){
            GFL_NET_RingInitialize(&_pComm->recvServerRing[i],
                               &_pComm->pServerRecvBufRing[i*_pComm->packetSizeMax],
                               _pComm->packetSizeMax);
//            GFL_NET_RingInitialize(&_pComm->recvServerRingUndo[i],
//                               &_pComm->pServerRecvBufRing[i*_pComm->packetSizeMax],
//                               _pComm->packetSizeMax);
        }

        MI_CpuFill8(_pComm->pMidRecvBufRing, 0, _pComm->packetSizeMax * machineMax );
        for(i = 0; i < machineMax; i++){
            GFL_NET_RingInitialize(&_pComm->recvMidRing[i],
                               &_pComm->pMidRecvBufRing[i * _pComm->packetSizeMax],
                               _pComm->packetSizeMax);
        }
    }
    MI_CpuFill8(_pComm->sSendServerBufRing, 0, _SEND_RINGBUFF_SIZE_PARENT);
    GFL_NET_RingInitialize(&_pComm->sendServerRing, _pComm->sSendServerBufRing,
                       _SEND_RINGBUFF_SIZE_PARENT);
    for(i = 0; i < _SEND_BUFF_SIZE_PARENT; i++){
        _pComm->sSendServerBuf[i] = GFL_NET_CMD_NONE;
    }
    // 子機の送受信
    MI_CpuFill8(_pComm->sSendBufRing, 0, _SEND_RINGBUFF_SIZE_CHILD);
    GFL_NET_RingInitialize(&_pComm->sendRing, _pComm->sSendBufRing, _SEND_RINGBUFF_SIZE_CHILD);

    _pComm->sSendBuf[0] = _INVALID_HEADER;
    for(i = 1;i < _SEND_BUFF_SIZE_4CHILD;i++){
        _pComm->sSendBuf[i] = GFL_NET_CMD_NONE;
    }
    MI_CpuFill8(_pComm->pRecvBufRing, 0, _pComm->packetSizeMax*2);
    GFL_NET_RingInitialize(&_pComm->recvRing, _pComm->pRecvBufRing, _pComm->packetSizeMax*2);

    _pComm->bNextSendData = FALSE;
    _pComm->bNextSendDataServer = FALSE;
    for(i = 0; i< GFL_NET_MACHINE_MAX;i++){
  //      _pComm->DSCountRecv[i] = 0xff;
        _pComm->recvDSCatchFlg[i] = 0;  // 通信をもらったことを記憶
        _pComm->bFirstCatch[i] = TRUE;
         _pComm->recvCommServer[i].valCommand = GFL_NET_CMD_NONE;
        _pComm->recvCommServer[i].valSize = 0xffff;
        _pComm->recvCommServer[i].pRecvBuff = NULL;
        _pComm->recvCommServer[i].dataPoint = 0;
        _pComm->countSendRecvServer[i] = 0;
    }
    _pComm->countSendRecv = 0;
    _pComm->bWifiSendRecv = TRUE;
    _pComm->recvCommClient.valCommand = GFL_NET_CMD_NONE;
    _pComm->recvCommClient.valSize = 0xffff;
    _pComm->recvCommClient.pRecvBuff = NULL;
    _pComm->recvCommClient.dataPoint = 0;

    _pComm->bFirstCatchP2C = TRUE;
    //_pComm->bSendNoneSend = TRUE;
    
    _sendCallBackServer = _SEND_CB_FIRST_SENDEND;
    _sendCallBack = _SEND_CB_FIRST_SENDEND;

        // キューのリセット
    GFL_NET_QueueManagerReset(&_pComm->sendQueueMgr);
    GFL_NET_QueueManagerReset(&_pComm->sendQueueMgrServer);
    _pComm->bResetState = FALSE;
}

//==============================================================================
/**
 * @brief   親子共通、DSMPを交換する場合に呼ばれる
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _commCommandInitChange2(void)
{
    void* pWork;
    int i;

//    _pComm->randPadType = 0;
//    _pComm->randPadStep = 0;
//    _pComm->sendSwitch = 0;
//    _pComm->sendServerSwitch = 0;
    
    // 親機のみの送受信
    {
        int machineMax = _getUserMaxNum();

        NET_PRINT("packet %d %d\n",_pComm->packetSizeMax,machineMax);
        MI_CpuFill8(_pComm->pServerRecvBufRing, 0, _pComm->packetSizeMax * machineMax);
        for(i = 0; i< machineMax;i++){
            GFL_NET_RingInitialize(&_pComm->recvServerRing[i],
                               &_pComm->pServerRecvBufRing[i*_pComm->packetSizeMax],
                               _pComm->packetSizeMax);
        }

        MI_CpuFill8(_pComm->pMidRecvBufRing, 0, _pComm->packetSizeMax * machineMax );
        for(i = 0; i < machineMax; i++){
            GFL_NET_RingInitialize(&_pComm->recvMidRing[i],
                               &_pComm->pMidRecvBufRing[i * _pComm->packetSizeMax],
                               _pComm->packetSizeMax);
        }
    }
    MI_CpuFill8(_pComm->sSendServerBufRing, 0, _SEND_RINGBUFF_SIZE_PARENT);
    GFL_NET_RingInitialize(&_pComm->sendServerRing, _pComm->sSendServerBufRing,
                       _SEND_RINGBUFF_SIZE_PARENT);
    for(i = 0; i < _SEND_BUFF_SIZE_PARENT; i++){
        _pComm->sSendServerBuf[i] = GFL_NET_CMD_NONE;
    }
    // 子機の送受信
    MI_CpuFill8(_pComm->sSendBufRing, 0, _SEND_RINGBUFF_SIZE_CHILD);
    GFL_NET_RingInitialize(&_pComm->sendRing, _pComm->sSendBufRing, _SEND_RINGBUFF_SIZE_CHILD);

    _pComm->sSendBuf[0] = _INVALID_HEADER;
    for(i = 1;i < _SEND_BUFF_SIZE_4CHILD;i++){
        _pComm->sSendBuf[i] = GFL_NET_CMD_NONE;
    }
    MI_CpuFill8(_pComm->pRecvBufRing, 0, _pComm->packetSizeMax*2);
    GFL_NET_RingInitialize(&_pComm->recvRing, _pComm->pRecvBufRing, _pComm->packetSizeMax*2);

    _pComm->bNextSendData = FALSE;
    _pComm->bNextSendDataServer = FALSE;
    for(i = 0; i< GFL_NET_MACHINE_MAX;i++){
        _pComm->recvDSCatchFlg[i] = 0;  // 通信をもらったことを記憶
        _pComm->bFirstCatch[i] = TRUE;
        _pComm->recvCommServer[i].valCommand = GFL_NET_CMD_NONE;
        _pComm->recvCommServer[i].valSize = 0xffff;
        _pComm->recvCommServer[i].pRecvBuff = NULL;
        _pComm->recvCommServer[i].dataPoint = 0;
    }
    _pComm->bWifiSendRecv = TRUE;
    _pComm->recvCommClient.valCommand = GFL_NET_CMD_NONE;
    _pComm->recvCommClient.valSize = 0xffff;
    _pComm->recvCommClient.pRecvBuff = NULL;
    _pComm->recvCommClient.dataPoint = 0;

    _pComm->bFirstCatchP2C = TRUE;
    //_pComm->bSendNoneSend = TRUE;
    
//    _sendCallBackServer = _SEND_CB_SECOND_SENDEND;
//    _sendCallBack = _SEND_CB_SECOND_SENDEND;

        // キューのリセット
    GFL_NET_QueueManagerReset(&_pComm->sendQueueMgr);
    GFL_NET_QueueManagerReset(&_pComm->sendQueueMgrServer);
    _pComm->bResetState = FALSE;
}

//==============================================================================
/**
 * @brief   指定された子機の領域をクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _clearChildBuffers(int netID)
{
    _pComm->recvDSCatchFlg[netID] = 0;  // 通信をもらったことを記憶 DS同期用
    _pComm->bFirstCatch[netID] = TRUE;  // コマンドをはじめてもらった時用
    _pComm->countSendRecvServer[netID]=0;  //SERVER受信

    GFL_NET_RingInitialize(&_pComm->recvMidRing[netID],
                       &_pComm->pMidRecvBufRing[netID * _pComm->packetSizeMax],
                       _pComm->packetSizeMax);

    GFL_NET_RingInitialize(&_pComm->recvServerRing[netID],
                       &_pComm->pServerRecvBufRing[netID * _pComm->packetSizeMax],
                       _pComm->packetSizeMax);

    _pComm->recvCommServer[netID].valCommand = GFL_NET_CMD_NONE;
    _pComm->recvCommServer[netID].valSize = 0xffff;
    _pComm->recvCommServer[netID].pRecvBuff = NULL;
    _pComm->recvCommServer[netID].dataPoint = 0;
}

//==============================================================================
/**
 * @brief   指定された子機の領域をクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _connectFunc(void)
{
    int i;

    for(i = 1 ; i < GFL_NET_MACHINE_MAX ; i++){
        if((!GFL_NET_SystemIsConnect(i)) && !_pComm->bFirstCatch[i]){
            if(!GFL_NET_SystemGetAloneMode()){
                NET_PRINT("非接続になった時に %d\n",i);
                _clearChildBuffers(i);  // 非接続になった時に
            }
        }
    }

}

//==============================================================================
/**
 * @brief   親機の初期化を行う
 * @param   work_area 　システムで使うメモリー領域
 *                      NULLの場合すでに初期化済みとして動作
 * @param   regulationNo  ゲームの種類
 * @param   bTGIDChange  新規のゲームの初期化の場合TRUE 古いビーコンでの誤動作を防ぐため用
 * @param   bEntry  子機を受け入れるかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemParentModeInit(BOOL bTGIDChange, int packetSizeMax, BOOL bEntry)
{
    BOOL ret = TRUE;

    ret = GFL_NET_WLParentInit(bTGIDChange, bEntry, _clearChildBuffers);
    GFL_NET_WLSetRecvCallback( _commRecvParentCallback );
    _commInit(packetSizeMax, GFL_HEAPID_SYSTEM); //@@OO後で外部に出す
    return ret;
}

//==============================================================================
/**
 * @brief   子機の初期化を行う
 * @param   work_area 　システムで使うメモリー領域
 *                      NULLの場合はすでに初期化済みとして扱う
 * @param   regulationNo  ゲームの種類
 * @param   bBconInit  ビーコンデータを初期化するのかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemChildModeInit(BOOL bBconInit, int packetSizeMax)
{
    BOOL ret = TRUE;

    ret = GFL_NET_WLChildInit(bBconInit);
    GFL_NET_WLSetRecvCallback( _commRecvCallback );

    _commInit(packetSizeMax, GFL_HEAPID_SYSTEM);
    _sendCallBack = _SEND_CB_FIRST_SENDEND;

    return ret;
}

//==============================================================================
/**
 * @brief   子機の初期化を行う+ 接続にすぐ行く
 * @param   work_area 　システムで使うメモリー領域
 *                      NULLの場合はすでに初期化済みとして扱う
 * @param   regulationNo  ゲームの種類
 * @param   bBconInit  ビーコンデータを初期化するのかどうか
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemChildModeInitAndConnect(int packetSizeMax,_PARENTFIND_CALLBACK pCallback,GFL_NETHANDLE* pHandle)
{
    BOOL ret = TRUE;
    u8 mac[6]={0xff,0xff,0xff,0xff,0xff,0xff};

//    ret = GFL_NET_WLChildInit(bBconInit);
//    ret = GFL_NET_WLChildMacAddressConnect(mac);

    GFL_NET_WLSetRecvCallback( _commRecvCallback );

    _commInit(packetSizeMax, GFL_HEAPID_SYSTEM);
    _sendCallBack = _SEND_CB_FIRST_SENDEND;

    ret = GFL_NET_WLChildMacAddressConnect(mac,pCallback,pHandle);
    return ret;
}


//==============================================================================
/**
 * @brief   通信モード切替
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _transmissonTypeChange(void)
{
    int i;
    BOOL bChange = FALSE;

    if(_pComm->transmissionType == _CHANGE_MODE_DSMP){
        _pComm->transmissionType = _MP_MODE;
        bChange=TRUE;
        NET_PRINT("MPモードになりました\n");
    }
    if(_pComm->transmissionType == _CHANGE_MODE_MPDS){
        _pComm->transmissionType = _DS_MODE;
        bChange=TRUE;
        NET_PRINT("DSモードになりました\n");
    }

//    _transmission();

}

//==============================================================================
/**
 * @brief   DSモードに切り替える
 * @param   DSモードで動く場合_DS_MODE MPで動く場合_MP_MODE
 * @retval  none
 */
//==============================================================================

static void _commSetTransmissonType(int type)
{
    if((_pComm->transmissionType == _MP_MODE) && (type == _DS_MODE)){
        _pComm->transmissionType = _CHANGE_MODE_MPDS;
        return;
    }
    if((_pComm->transmissionType == _DS_MODE) && (type == _MP_MODE)){
        _pComm->transmissionType = _CHANGE_MODE_DSMP;
        return;
    }
}

//==============================================================================
/**
 * @brief   DSモードに切り替える
 * @retval  none
 */
//==============================================================================
void GFL_NET_SystemSetTransmissonTypeDS(void)
{
    _commSetTransmissonType(_DS_MODE);
}

//==============================================================================
/**
 * @brief   MPモードに切り替える
 * @retval  none
 */
//==============================================================================
void GFL_NET_SystemSetTransmissonTypeMP(void)
{
    _commSetTransmissonType(_MP_MODE);
   // _pComm->transmissionType = _MP_MODE;
}

//==============================================================================
/**
 * @brief   指定モードに切り替える
 * @param   DSかMPかの指定モード
 * @retval  none
 */
//==============================================================================
void GFL_NET_SystemSetTransmissonType(int type)
{
    _commSetTransmissonType(type);
}

//==============================================================================
/**
 * @brief   現在のモードの取得
 * @param   none
 * @retval  _DS_MODEか_MP_MODE
 */
//==============================================================================

static int _transmissonType(void)
{
    if(_pComm->transmissionType == _CHANGE_MODE_DSMP){
        return _DS_MODE;
    }
    if(_pComm->transmissionType == _CHANGE_MODE_MPDS){
        return _MP_MODE;
    }
    return _pComm->transmissionType;
}

//==============================================================================
/**
 * @brief   現在 DSモードかどうか
 * @param   none
 * @retval  TRUEならDS
 */
//==============================================================================

BOOL GFL_NET_SystemIsTransmissonDSType(void)
{
    if(_DS_MODE == _transmissonType()){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   通信切断を行う
 * @param   none
 * @retval  none
 */
//==============================================================================
void GFL_NET_SystemFinalize(void)
{
    BOOL bEnd = FALSE;

    if(_pComm){
        if( _pComm->device == _DEVICE_WIFI){
            //mydwc_Logout();  // 切断
            bEnd = TRUE;
        }
        else{
            if(GFL_NET_WLFinalize()){
                bEnd = TRUE;
            }
        }
    }
    if(bEnd){
        NET_PRINT("切断----開放処理--\n");
        GFL_NET_Tool_sysEnd(_pComm->pTool);
        _pComm->pTool = NULL;
        GFL_HEAP_FreeMemory(_pComm->pRecvBufRing);
        GFL_HEAP_FreeMemory(_pComm->pTmpBuff);
        GFL_HEAP_FreeMemory(_pComm->pServerRecvBufRing);
        GFL_HEAP_FreeMemory(_pComm->pMidRecvBufRing);
        GFL_NET_QueueManagerFinalize(&_pComm->sendQueueMgrServer);
        GFL_NET_QueueManagerFinalize(&_pComm->sendQueueMgr);
        GFL_HEAP_FreeMemory(_pComm);
        _pComm = NULL;
    }
}

//==============================================================================
/**
 * @brief   子機 index接続
 * @param   index   親のリストのindex
 * @retval  子機接続を親機に送ったらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemChildIndexConnect(u16 index,_PARENTFIND_CALLBACK pCallback,GFL_NETHANDLE* pHandle)
{
    return GFL_NET_WLChildIndexConnect(index, pCallback, pHandle);
}


//==============================================================================
/**
 * @brief   終了コマンドを受信したらモードに応じて処理を行い自動切断する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _autoExitSystemFunc(void)
{
    if(!GFL_NET_WLIsAutoExit()){
        return;
    }
    if(GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID){   // 自分が親の場合みんなに逆返信する
        if(GFL_NET_WLIsChildsConnecting()){
            return;
        }
        GFL_NET_SystemFinalize();  // 終了処理に入る
    }
    else{   //子機の場合
        GFL_NET_SystemFinalize();  // 終了処理に入る
    }
}

//==============================================================================
/**
 * @brief   通信データの更新処理  データを収集
 *    main.c   から  vblank後にすぐに呼ばれる
 * @param   none
 * @retval  データシェアリング同期が取れなかった場合FALSE
 */
//==============================================================================


BOOL GFL_NET_SystemUpdateData(void)
{
    int j;

#if 0 //通信アイコン機能ができたら出す @@OO 2006.12.13
    if(CommStateIsWifiConnect()){
        WirelessIconEasy_SetLevel(WM_LINK_LEVEL_3 - DWC_GetLinkLevel());
    }
    else if(CommMPIsInitialize()){
        WirelessIconEasy_SetLevel(WM_LINK_LEVEL_3 - WM_GetLinkLevel());
    }
#endif

    if(_pComm != NULL){
        
        if(!_pComm->bShutDown){

            _transmissonTypeChange();  //通信切り替え
            _dataMpStep();
            if(_transmissonType() == _MP_MODE){
                _recvDataFunc();    // 子機としての受け取り処理
            }
            if((GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID) && (GFL_NET_SystemIsConnect(COMM_PARENT_ID)) || GFL_NET_SystemGetAloneMode() ){
                // サーバーとしての処理
                _dataMpServerStep();
            }
            if((GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID) || (_transmissonType() == _DS_MODE) || GFL_NET_SystemGetAloneMode() ){
                _recvDataServerFunc();  // サーバー側の受信処理
            }
        }
        CommMpProcess(_pComm->bitmap);
        if(GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID){
            _connectFunc();
        }
        _autoExitSystemFunc();  // 自動切断 _pComm=NULLになるので注意
    }
    else{
        CommMpProcess(0);
    }
    //エラーの表示の仕組みができたら入れる  @@OO  2006.12.13
//    CommErrorDispCheck(GFL_HEAPID_SYSTEM);
    return TRUE;
}


//==============================================================================
/**
 * @brief   通信バッファをクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemReset(void)
{

    NET_PRINT("CommSystemReset\n");
    if(_pComm){
        _commCommandInit();
    }
}

//==============================================================================
/**
 * @brief   DSモードで通信バッファをクリアーする
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemResetDS(void)
{

    NET_PRINT("CommSystemReset\n");
    if(_pComm){
        _pComm->transmissionType = _DS_MODE;
        _commCommandInit();
    }
}

//==============================================================================
/**
 * @brief   通信バッファをクリアーする+ビーコンの初期化を行う
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemResetBattleChild(void)
{

    NET_PRINT("CommSystemReset\n");
    if(_pComm){
        _commCommandInit();
        GFL_NET_WLChildBconDataInit();
    }
}

//==============================================================================
/**
 * @brief   データ送信処理
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _dataMpStep(void)
{
#if 0
    if(_pComm->bSendNoneSend){
        if( _pComm->device == _DEVICE_WIFI){
#if 0        // wifiは封印
            if( _pComm->bWifiConnect ){
                if( mydwc_sendToServer( _pComm->sSendBuf, _SEND_BUFF_SIZE_4CHILD )){
                    _pComm->bSendNoneSend = FALSE;
                }
            }
#endif
        }
        else if((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID()) &&
                 (GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID()))) || GFL_NET_SystemGetAloneMode()){
            _sendCallBack = _SEND_CB_NONE;
            _updateMpData();     // データ送受信
            if(_sendCallBack != _SEND_CB_NONE){
                _pComm->bSendNoneSend = FALSE;
            }
        }
        return;
    }
#endif

    if( _pComm->device == _DEVICE_WIFI){
#if 0  //wifi封印
        if( _pComm->bWifiConnect ){
            if( _pComm->bWifiSendRecv ){  // 同期を取っている場合
                if( _pComm->countSendRecv > _SENDRECV_LIMIT ){  //送りすぎ
                    return;
                }
                if(_sendCallBack == _SEND_CB_FIRST_SENDEND){
                    _setSendData(_pComm->sSendBuf);   // 送るデータをリングバッファから差し替える
                    _sendCallBack = _SEND_CB_NONE;
                }
            }
            else{
                if(_sendCallBack == _SEND_CB_FIRST_SENDEND){
                    if(!_setSendData(_pComm->sSendBuf)){  // 送るデータをリングバッファから差し替える
                        return;  // 本当に送るものが何も無い場合
                    }
                    _sendCallBack = _SEND_CB_NONE;
                }
            }
            if( mydwc_sendToServer( _pComm->sSendBuf, _SEND_BUFF_SIZE_4CHILD )){
//                if( !_pComm->bWifiSendRecv ){  // 同期を取ってない場合
//                    DEBUG_DUMP(&_pComm->sSendBuf, 16,"sp0");
//                }
                _sendCallBack = _SEND_CB_FIRST_SENDEND;
                _pComm->countSendRecv++;  //wifi client
            }
            else{
            }
        }
#endif //wifi封印
    }
    else if(((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) &&
        (GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID()))) || GFL_NET_SystemGetAloneMode()){
        if(_sendCallBack != _SEND_CB_FIRST_SENDEND){  // 1個送ったが送信完了していない
            NET_PRINT("うけとってない _sendCallBack\n");
            return;
        }
        if( _pComm->countSendRecv > _SENDRECV_LIMIT ){  //送りすぎ
//            NET_PRINT("親の同期待ち\n");
            return;
        }
        _setSendData(_pComm->sSendBuf);  // 送るデータをリングバッファから差し替える
        _sendCallBack = _SEND_CB_NONE;
        _updateMpData();     // データ送受信
    }
}

//==============================================================================
/**
 * @brief   中間RINGBUFFから子機全員に逆送信するためbuffにコピー
 * @param   none
 * @retval  送ったらTRUE
 */
//==============================================================================

static BOOL _copyDSData(void)
{
    int mcSize;
    int machineMax;
    int i,num,bSend = FALSE;
    u8* pBuff;

    mcSize = _getUserMaxSendByte();
    machineMax = _getUserMaxNum();

    /// 中間RINGBUFFから子機全員に逆送信するためbuffにコピー
    for(i = 0; i < machineMax; i++){
        GFL_NET_RingEndChange(&_pComm->recvMidRing[i]);
        pBuff = &_pComm->sSendServerBuf[GFL_NET_DATA_HEADER + (i * mcSize)];
        pBuff[0] = _INVALID_HEADER;
        num = GFL_NET_RingGets(&_pComm->recvMidRing[i] ,
                               pBuff,
                               mcSize);
        if(pBuff[0] != _INVALID_HEADER){
            bSend = TRUE;
        }
    }
    if(bSend == FALSE){
        _pComm->sSendServerBuf[0] = _INVALID_HEADER;
        return FALSE;
    }
    _pComm->sSendServerBuf[0] = _SEND_NONE;
    _pComm->sSendServerBuf[1] = GFL_NET_WL_GetBitmap();
    _pComm->sSendServerBuf[2] = mcSize;
    return TRUE;
}


//==============================================================================
/**
 * @brief   データ送信処理  サーバー側
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _updateMpDataServer(void)
{
    int i;
    int debug=0;
    int mcSize ,machineMax;

    if(!_pComm){
        return;
    }
    if( _pComm->device == _DEVICE_WIFI){
        return;
    }
    mcSize = _getUserMaxSendByte();
    machineMax = _getUserMaxNum();
    if(_sendCallBackServer == _SEND_CB_NONE){
        _sendCallBackServer++;
        
        if(_transmissonType() == _DS_MODE){
            _copyDSData();
        }
        if( (GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID()))  && !GFL_NET_SystemGetAloneMode()){
            if(!GFL_NET_WL_SendData(_pComm->sSendServerBuf,
                            _SEND_BUFF_SIZE_PARENT,
                            _sendServerCallback)){
                _sendCallBackServer--;
            }
        }
        // 送信完了
        if((_sendCallBackServer == _SEND_CB_FIRST_SEND) ){

            for(i = 0; i < machineMax; i++){
                if(GFL_NET_SystemIsConnect(i)){
                    _pComm->countSendRecvServer[i]++; // 親MP送信
                }
                else if(GFL_NET_SystemGetAloneMode() && (i == 0)){
                    _pComm->countSendRecvServer[i]++; // 親MP送信
                }
            }

            // 親機自身に子機の動きをさせるためここでコールバックを呼ぶ
            _commRecvCallback(COMM_PARENT_ID,
                              (u16*)_pComm->sSendServerBuf,
                              _SEND_BUFF_SIZE_PARENT);
        }

        if( !(GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID()))  || GFL_NET_SystemGetAloneMode() ){
            // 割り込みが無い状況でも動かす為ここでカウント
            _sendCallBackServer++;
        }
    }
}

//==============================================================================
/**
 * @brief   データ送信処理  サーバー側
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _dataMpServerStep(void)
{
    int i;
#if 0
    if(_pComm->bSendNoneSend){
        if( _pComm->device == _DEVICE_WIFI){
#if 0  //wifi lock
            if( GFL_NET_SystemIsConnect(COMM_PARENT_ID) ){
                if( mydwc_sendToClient( _pComm->sSendServerBuf, WH_MP_4CHILD_DATA_SIZE*2 )){
                    _pComm->bSendNoneSend = FALSE;
                    return;
                }
            }
#endif //wifi lock
        }
        else if((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) || (GFL_NET_SystemGetAloneMode()) ){
            _updateMpDataServer();
            if(_sendCallBackServer == _SEND_CB_FIRST_SENDEND){
                _pComm->bSendNoneSend = FALSE;
                return;
            }
        }
    }
#endif
    if( _pComm->device == _DEVICE_WIFI){
#if 0 //wifi封印
        if( GFL_NET_SystemIsConnect(COMM_PARENT_ID) ){
            if( _pComm->bWifiSendRecv ){  // 同期を取っている場合
                if(_pComm->countSendRecvServer[1] > _SENDRECV_LIMIT){ // 送信しすぎの場合
                    return;
                }
                if(_pComm->countSendRecvServer[0] > _SENDRECV_LIMIT){ // 送信しすぎの場合
                    return;
                }
                if(_sendCallBackServer == _SEND_CB_FIRST_SENDEND){
                    if(_transmissonType() == _DS_MODE){
                        _copyDSData();  //DS通信ならコピー
                    }
                    _sendCallBackServer = _SEND_CB_NONE;
                }
            }
            else{
                if(_sendCallBackServer == _SEND_CB_FIRST_SENDEND){
                    if(_transmissonType() == _DS_MODE){
                        if(_copyDSData()){  //DS通信ならコピー
                            return;
                        }
                    }
                }
                _sendCallBackServer = _SEND_CB_NONE;
            }

            if( mydwc_sendToClient( _pComm->sSendServerBuf, WH_MP_4CHILD_DATA_SIZE*2 )){
                OHNO_SP_PRINT("send %d \n",_pComm->sSendServerBuf[0]);
                _sendCallBackServer = _SEND_CB_FIRST_SENDEND;
                _pComm->countSendRecvServer[0]++; // wifi server
                _pComm->countSendRecvServer[1]++; // wifi server
            }
            else{
                NET_PRINT("mydwc_sendToClientに失敗\n");
            }
        }
#endif  //wifi封印
    }
    else if((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) || (GFL_NET_SystemGetAloneMode()) ){
        for(i = 1; i < GFL_NET_MACHINE_MAX; i++){
            if(GFL_NET_SystemIsConnect(i)){
                if(_pComm->countSendRecvServer[i] > _SENDRECV_LIMIT){ // 送信しすぎの場合
//                    NET_PRINT("送信しすぎ%d \n",i);
                    return;
                }
            }
            else if((i == 0) && GFL_NET_SystemGetAloneMode()){
                if(_pComm->countSendRecvServer[0] > _SENDRECV_LIMIT){ // 送信しすぎの場合
                    return;
                }
            }
        }
//        if(_sendCallBackServer != _SEND_CB_FIRST_SENDEND){
  //          NET_PRINT("うけとってない_sendCallBackServer %d\n",_sendCallBackServer);
    //        return;
      //  }
//        if( _pComm->countSendRecv > _SENDRECV_LIMIT ){  //送りすぎ
  //          return;
    //    }
        if(_sendCallBackServer == _SEND_CB_FIRST_SENDEND){
            if(_transmissonType() == _MP_MODE){  // DS時にはすでにsSendServerBufにデータがある
                _setSendDataServer(_pComm->sSendServerBuf);  // 送るデータをリングバッファから差し替える
            }
        }
        _sendCallBackServer = _SEND_CB_NONE;
        // 最初の送信処理
        _updateMpDataServer();
    }
}


//#define WIFI_DUMP_TEST


//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック こちらが本体 上のはrapper
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================
static void _commRecvCallback(u16 aid, u16 *data, u16 size)
{
    u8* adr = (u8*)data;
    int i;
    int recvSize = size;

    _pComm->countSendRecv--;  //受信
#ifdef PM_DEBUG
    _pComm->countRecvNum++;
#endif
    if(adr==NULL){
        return;
    }
    if(adr[0] == _INVALID_HEADER){
        return;
    }

#ifdef WIFI_DUMP_TEST
    DEBUG_DUMP(&adr[0], _SEND_BUFF_SIZE_CHILD,"cr0");
    DEBUG_DUMP(&adr[_SEND_BUFF_SIZE_CHILD], _SEND_BUFF_SIZE_CHILD,"cr1");
#endif

    if(adr[0] & _MP_DATA_HEADER){   ///MPデータの場合
        if(_transmissonType() == _DS_MODE){
            NET_PRINT("DSなのにMPデータが来た \n");
            return;
        }
    }
    else if(_transmissonType() == _MP_MODE){  //DSデータの場合
        NET_PRINT("MPなのにDEPデータが来た \n");
        return;
    }
    _pComm->bFirstCatchP2C = FALSE;

    _pComm->bitmap = adr[1];

    if(_transmissonType() == _DS_MODE){
        int mcSize = _getUserMaxSendByte();
        int machineMax = _getUserMaxNum();
        adr += GFL_NET_DATA_HEADER;      // ヘッダー１バイト読み飛ばす + Bitmapでーた + サイズデータ

        for(i = 0; i < machineMax; i++){
            if(adr[0] == _INVALID_HEADER){
                adr += mcSize;
                continue;
            }
            if((_pComm->bFirstCatch[i]) && (adr[0] & _SEND_NEXT)){ // まだ一回もデータをもらったことがない状態なのに連続データだった
                NET_PRINT("連続データ parent %d \n",aid);
                adr += mcSize;
                continue;
            }
            adr++;
            GFL_NET_RingPuts(&_pComm->recvServerRing[i], adr, mcSize-1);
            adr += (mcSize-1);
            _pComm->bFirstCatch[i]=FALSE;
        }
    }
    else{   //MPデータ

        recvSize = adr[2]*256;  // MP受信サイズ
        recvSize += adr[3];  // MP受信サイズ

        if((_pComm->bFirstCatchP2C) && (adr[0] & _SEND_NEXT)){
            // まだ一回もデータをもらったことがない状態なのに連続データだった
            NET_PRINT("連続データ child %d \n",aid);
            DEBUG_DUMP((u8*)data,24,"cr");
            return;
        }
        if(recvSize!=0){
            DEBUG_DUMP(&adr[0], _SEND_BUFF_SIZE_CHILD,"MP");
        }
        adr += GFL_NET_DATA_HEADER;      // ヘッダー１バイト読み飛ばす + Bitmapでーた + サイズデータ
        GFL_NET_RingPuts(&_pComm->recvRing , adr, recvSize);
    }
}


//==============================================================================
/**
 * @brief   親機側で子機の通信を受信した時に呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

static void _commRecvParentCallback(u16 aid, u16 *data, u16 size)
{
    u8* adr = (u8*)data;
    int i;

    _pComm->countSendRecvServer[aid]--;  //SERVER受信
    if(adr==NULL){
        return;
    }
    if(adr[0] == _INVALID_HEADER){
        return;
    }

#ifdef WIFI_DUMP_TEST
    if(aid == 1){
        DEBUG_DUMP(&adr[0], _SEND_BUFF_SIZE_CHILD,"pr1");
    }
    else{
        DEBUG_DUMP(&adr[0], _SEND_BUFF_SIZE_CHILD,"pr0");
    }
#endif
    
    if((_pComm->bFirstCatch[aid]) && (adr[0] & _SEND_NEXT)){
        // まだ一回もデータをもらったことがない状態なのに連続データだった
        NET_PRINT("連続データ parent %d \n",aid);
        i = 0;
        DEBUG_DUMP(adr,12,"連続データ");
        return;
    }
    _pComm->bFirstCatch[aid] = FALSE;  //初回受信フラグをOFFする

    if(_transmissonType() == _DS_MODE){  //DS
        int mcSize = _getUserMaxSendByte();
        int machineMax = _getUserMaxNum();
        GFL_NET_RingPuts(&_pComm->recvMidRing[aid] , adr, mcSize);
        _pComm->recvDSCatchFlg[aid]++;  // 通信をもらったことを記憶
    }else{   // MPモード
        adr++;
        if(_RECV_BUFF_SIZE_PARENT > GFL_NET_RingDataRestSize(&_pComm->recvServerRing[aid])){
            NET_PRINT("Error Throw:受信オーバー\n");
            return;
        }
        GFL_NET_RingPuts(&_pComm->recvServerRing[aid] , adr, _RECV_BUFF_SIZE_PARENT);
    }
}

//==============================================================================
/**
 * @brief   通信を送信したときに呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

static void _sendCallbackFunc(BOOL result)
{
    if(result){
        _sendCallBack++;
    }
    else{
        GF_ASSERT_MSG(0,"send failed");
    }
}

//==============================================================================
/**
 * @brief   通信を送信したときに呼ばれるコールバック 
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

static void _sendServerCallback(BOOL result)
{
    int i;
    int mcSize = _getUserMaxSendByte();
    int machineMax = _getUserMaxNum();
    
    if(result){
        _sendCallBackServer++;
//        NET_PRINT("送信callback server側\n");
    }
    else{
        GF_ASSERT_MSG(0,"send failed");
    }

    for(i = 0; i < machineMax; i++){
        if(_transmissonType() == _DS_MODE){             // 初期化
            _pComm->sSendServerBuf[GFL_NET_DATA_HEADER+(i*mcSize)] = _INVALID_HEADER;
        }
    }
}

//==============================================================================
/**
 * @brief   データの収集
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _updateMpData(void)
{
    u16 i;
    u8 *adr;
    int size;

    if(!_pComm){
        return;
    }
    if( _pComm->device == _DEVICE_WIFI){
        return;
    }
    {
        int mcSize = _getUserMaxSendByte();
        int machineMax = _getUserMaxNum();
        if(GFL_NET_SystemGetAloneMode()){   // aloneモードの場合
            if(_sendCallBack == _SEND_CB_NONE){
                _sendCallBack++;
                _sendCallbackFunc(TRUE);
                // 子機のふりをする部分          // 親機は自分でコールバックを呼ぶ
                _commRecvParentCallback(COMM_PARENT_ID, (u16*)_pComm->sSendBuf,
                                    mcSize);
                //_pComm->sendSwitch = 1 - _pComm->sendSwitch;
                _pComm->countSendRecv++; // MP送信親
                return;
            }
        }
        if(GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())){
            if(!GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID())){
                if(GFL_NET_SystemGetCurrentID()==1){
                    NET_PRINT("自分自身の接続がまだ\n");
                }
                return;
            }
            if(_sendCallBack == _SEND_CB_NONE){
                // 子機データ送信
                if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){
                    _sendCallBack++;
                    if(!GFL_NET_WL_SendData(_pComm->sSendBuf,
                                    mcSize, _sendCallbackFunc)){
                        _sendCallBack--;
                        NET_PRINT("failed WH_SendData\n");
                    }
                    else{
                        _pComm->countSendRecv++; // MP送信
                    }
                }
                else{
//                else if(GFL_NET_WLIsChildsConnecting()){         // サーバーとしての処理 自分以外の誰かにつながっている時
                    _sendCallBack++;
                    _sendCallbackFunc(TRUE);
                    // 子機のふりをする部分          // 親機は自分でコールバックを呼ぶ
                    _commRecvParentCallback(COMM_PARENT_ID, (u16*)_pComm->sSendBuf,
                                        mcSize);
                    _pComm->countSendRecv++; // MP送信
                }
            }
        }
    }
}

//==============================================================================
/**
 * @brief   子機側のアクションが行われたことを設定
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemActionCommandSet(void)
{
//    _pComm->actionCount = _ACTION_COUNT_MOVE;
}

//==============================================================================
/**
 * @brief   送信キューにあったものを送信バッファに入れる
 * @param   none
 * @retval  none
 */
//==============================================================================

static BOOL _setSendData(u8* pSendBuff)
{
    int i;
    int mcSize = _getUserMaxSendByte();
    int machineMax = _getUserMaxNum();
    if(_pComm->bNextSendData == FALSE){  // 一回で送れる場合
        pSendBuff[0] = _SEND_NONE;
    }
    else{
        pSendBuff[0] = _SEND_NEXT;  // 一回で送れない場合
    }
    _pComm->bNextSendData = FALSE;
#if 0
    if(GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgr) && (_transmissonType() == _MP_MODE)){
        pSendBuff[0] |= _SEND_NO_DATA;  // 空っぽなら何も送らない
    }
#endif
    
    if(GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgr)){
        pSendBuff[0] = _INVALID_HEADER;  // 空っぽなら何も送らない
        return FALSE;  // 送るものが何も無い
    }
    else{
        SEND_BUFF_DATA buffData;
        buffData.size = mcSize - 1;
        buffData.pData = &pSendBuff[1];
        if(!GFL_NET_QueueGetData(&_pComm->sendQueueMgr, &buffData, TRUE)){
            _pComm->bNextSendData = TRUE;
        }
        if(_transmissonType() == _DS_MODE){
            _pComm->DSCount++;
//            NET_PRINT("DSデータセット %d\n",_pComm->DSCount);
            pSendBuff[0] |= ((_pComm->DSCount << 4) & 0xf0);  //DS通信順番カウンタ
        }
    }
#if 0
    if(GFL_NET_SystemGetCurrentID()==0){
        DEBUG_DUMP(pSendBuff,mcSize,"_setSendData");
    }
#endif
    return TRUE;
}

//==============================================================================
/**
 * @brief   送信キューにあったものを送信バッファに入れる サーバーMP通信用
 * @param   pSendBuff 入れる送信バッファ
 * @retval  none
 */
//==============================================================================

static void _setSendDataServer(u8* pSendBuff)
{
    int i;

    pSendBuff[0] = _MP_DATA_HEADER;


    if(_pComm->bNextSendDataServer != FALSE){
        pSendBuff[0] |= _SEND_NEXT;  // 一回で送れない場合
    }

    {
        u16 bitmap = GFL_NET_WL_GetBitmap();
        int size;
        pSendBuff[1] = bitmap;

        {
            SEND_BUFF_DATA buffData;
            buffData.size = _SEND_BUFF_SIZE_PARENT - GFL_NET_DATA_HEADER;
            buffData.pData = &pSendBuff[GFL_NET_DATA_HEADER];
            if(GFL_NET_QueueGetData(&_pComm->sendQueueMgrServer, &buffData, FALSE)){
                _pComm->bNextSendDataServer = FALSE;
                size = (_SEND_BUFF_SIZE_PARENT - GFL_NET_DATA_HEADER) - buffData.size;
            }
            else{
                _pComm->bNextSendDataServer = TRUE;
                size = _SEND_BUFF_SIZE_PARENT - GFL_NET_DATA_HEADER;
            }
            pSendBuff[2] = (size >> 8) & 0xff;
            pSendBuff[3] = size & 0xff;
        }
    }
#if 0
    DEBUG_DUMP(pSendBuff, _SEND_BUFF_SIZE_PARENT, "MP SERVER SEND");
#endif
}


//==============================================================================
/**
 * @brief   送信メソッド
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   size       送信量    コマンドだけの場合0
 * @param   bFast       送信量    コマンドだけの場合0
 * @param   myID       送信する人のハンドルno
 * @param   sendID     送る相手のID   全員なら0xff
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendData(int command, const void* data, int size, BOOL bFast, int myID, int sendID)
{
    int bSave=TRUE;
    int cSize = GFL_NET_CommandGetPacketSize(command);
    SEND_QUEUE_MANAGER* pMgr;

    NET_PRINT("< 送信 %d %d\n", command,GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgr));

    if(GFL_NET_COMMAND_SIZE_VARIABLE == cSize){
        cSize = size;
    }
    
    if(cSize > 255){
        bSave = FALSE;
    }
    if((_transmissonType() == _MP_MODE) && (myID == 0)){
        pMgr = &_pComm->sendQueueMgrServer;
    }
    else{
        pMgr = &_pComm->sendQueueMgr;
    }
    if(GFL_NET_QueuePut(pMgr, command, (u8*)data, cSize, bFast, bSave, myID, sendID)){
        return TRUE;
    }
    NET_PRINT("-キュー無い-");
    GFL_NET_SystemSetError();
    return FALSE;
}


//==============================================================================
/**
 * @brief   送信バッファ残り容量
 * @retval  サイズ
 */
//==============================================================================

int GFL_NET_SystemGetSendRestSize(void)
{
    return GFL_NET_RingDataRestSize(&_pComm->sendRing);
}

//==============================================================================
/**
 * @brief   サーバ側の送信バッファ残り容量
 * @retval  サイズ
 */
//==============================================================================

int GFL_NET_SystemGetSendRestSize_ServerSide(void)
{
    return GFL_NET_RingDataRestSize(&_pComm->sendServerRing);
}


//==============================================================================
/**
 * @brief   通信コールバックの最後に呼ぶ コマンドを消す
 * @param   netID   ネットID
 * @retval  none
 */
//==============================================================================

static void _endCallBack(int netID,int command,int size,void* pTemp, _RECV_COMMAND_PACK* pRecvComm)
{
    GFL_NET_CommandCallBack(pRecvComm->sendID, pRecvComm->recvID, command, size, pTemp);
    pRecvComm->valCommand = GFL_NET_CMD_NONE;
    pRecvComm->valSize = 0xffff;
    pRecvComm->pRecvBuff = NULL;
    pRecvComm->dataPoint = 0;
}


//==============================================================================
/**
 * @brief   受信したデータをプロセス中に処理する
 * @param   pRing  リングバッファのポインタ
 * @param   netID     処理しているnetID
 * @param   pTemp    コマンドを結合するためのtempバッファ
 * @retval  none
 */
//==============================================================================

static void _recvDataFuncSingle(RingBuffWork* pRing, int netID, u8* pTemp, _RECV_COMMAND_PACK* pRecvComm)
{
    int size;
    u8 command;
    int bkPos;
    int realbyte;
    int sendID;
    int recvID;

    
    while( GFL_NET_RingDataSize(pRing) != 0 ){
        bkPos = pRing->startPos;
        if(pRecvComm->valCommand != GFL_NET_CMD_NONE){
            command = pRecvComm->valCommand;
        }
        else{
            command = GFL_NET_RingGetByte(pRing);
            if(command == GFL_NET_CMD_NONE){
                continue;
            }
        }
        bkPos = pRing->startPos;
        pRecvComm->valCommand = command;
//        NET_PRINT("c %d\n",command);
        NET_PRINT(">>>cR %d %d %d\n", pRecvComm->valSize, GFL_NET_RingDataSize(pRing), command);
        if(pRecvComm->valSize != 0xffff){
            size = pRecvComm->valSize;
        }
        else{
            size = GFL_NET_CommandGetPacketSize(command);
            if(_pComm->bError){
                return;
            }
            if(GFL_NET_COMMAND_SIZE_VARIABLE == size){
                if( GFL_NET_RingDataSize(pRing) < 1 ){  // 残りデータが1以下だった
                    pRing->startPos = bkPos;
                    break;
                }
                // サイズがない通信データはここにサイズが入っている
                size = GFL_NET_RingGetByte(pRing)*0x100;
                size += GFL_NET_RingGetByte(pRing);
                NET_PRINT("受信サイズ  %d\n",size);
                bkPos = pRing->startPos; // ２個進める
            }
            pRecvComm->valSize = size;
            pRecvComm->sendID = GFL_NET_RingGetByte(pRing);
            pRecvComm->recvID = GFL_NET_RingGetByte(pRing);
            bkPos = pRing->startPos; // ２個進める
        }


        if(GFL_NET_CommandCreateBuffCheck(command)){  // 受信バッファがある場合
            if(pRecvComm->pRecvBuff==NULL){
                pRecvComm->pRecvBuff = GFL_NET_CommandCreateBuffStart(command, netID, pRecvComm->valSize);
            }
            realbyte = GFL_NET_RingGets(pRing, pTemp, size - pRecvComm->dataPoint);
            NET_PRINT("id %d -- rest %d\n",netID, size - pRecvComm->dataPoint);
            if(pRecvComm->pRecvBuff){
                MI_CpuCopy8(pTemp, &pRecvComm->pRecvBuff[pRecvComm->dataPoint], realbyte);
            }
            pRecvComm->dataPoint += realbyte;
            if(pRecvComm->dataPoint >= size ){
                _endCallBack(netID, command, size, pRecvComm->pRecvBuff, pRecvComm);
            }
        }
        else{
            if( GFL_NET_RingDataSize(pRing) >= size ){
                NET_PRINT(">>>受信 comm=%d id=%d -- size%d \n",command, netID, size);
                GFL_NET_RingGets(pRing, pTemp, size);
                _endCallBack(netID, command, size, (void*)pTemp, pRecvComm);
            }
            else{   // まだ届いていない大きいデータの場合ぬける
                NET_PRINT("結合待ち command %d size %d\n",command,size);
                pRing->startPos = bkPos;
                break;
            }
        }
    }
}

//==============================================================================
/**
 * @brief   受信したデータをプロセス中に処理する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _recvDataFunc(void)
{
    int id = COMM_PARENT_ID;
    int size;
    u8 command;
    int bkPos;

    if(!_pComm){
        return;
    }
    if(_pComm->bNotRecvCheck){
        return;
    }

    GFL_NET_RingEndChange(&_pComm->recvRing);
    if(GFL_NET_RingDataSize(&_pComm->recvRing) > 0){
        // 一個前の位置を変数に保存しておく
//        MI_CpuCopy8( &_pComm->recvRing,&_pComm->recvRingUndo, sizeof(RingBuffWork));
//        GFL_NET_RingStartPush(&_pComm->recvRingUndo); //start位置を保存
#if 0
        NET_PRINT("-解析開始 %d %d-%d\n",id,
                   _pComm->recvRing.startPos,_pComm->recvRing.endPos);
#endif
//        NET_PRINT("子機解析 %d \n",id);
        _recvDataFuncSingle(&_pComm->recvRing, id, _pComm->pTmpBuff, &_pComm->recvCommClient);
#if 0
        NET_PRINT("解析 %d %d-%d\n",id,
                   _pComm->recvRing.startPos,_pComm->recvRing.endPos);
#endif
    }
}

//==============================================================================
/**
 * @brief   受信したデータをプロセス中に処理する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _recvDataServerFunc(void)
{
    int id;
    int size;
    u8 command;
    int machineMax;

    if(!_pComm){
        return;
    }
    if(_pComm->bNotRecvCheck){
        return;
    }

    machineMax = _getUserMaxNum();

    for(id = 0; id < machineMax; id++){
        GFL_NET_RingEndChange(&_pComm->recvServerRing[id]);
        
        if(GFL_NET_RingDataSize(&_pComm->recvServerRing[id]) > 0){
#if 0
            NET_PRINT("解析開始 %d %d-%d\n",id,
                       _pComm->recvServerRing[id].startPos,_pComm->recvServerRing[id].endPos);
#endif
#if 0
            NET_PRINT("親機が子機%dを解析\n",id);
#endif
#if 0
            NET_PRINT("DS解析 %d\n",id);
#endif
            _recvDataFuncSingle(&_pComm->recvServerRing[id], id, _pComm->pTmpBuff, &_pComm->recvCommServer[id]);
        }
    }
}

//==============================================================================
/**
 * @brief   通信可能状態なのかどうかを返す ただしコマンドによるネゴシエーションがまだの状態
 * @param   親子機のnetID
 * @retval  TRUE  通信可能    FALSE 通信切断
 */
//==============================================================================
BOOL GFL_NET_SystemIsConnect(u16 netID)
{
    if(!_pComm){
        return FALSE;
    }
    if( _pComm->device == _DEVICE_WIFI){
#if 0 //wifi
        if(_pComm->bWifiConnect){
            int id = mydwc_getaid();
            if(-1 != id){
                if(netID == 0){
                    return TRUE;
                }
                else if(netID == 1){
                    return TRUE;
                }
            }
        }
#endif
        return FALSE;
    }
    if(!GFL_NET_SystemIsInitialize()){
        return FALSE;
    }
    if (!GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) {
        return FALSE;
    }
    if(GFL_NET_SystemGetCurrentID()==netID){// 自分はONLINE
        return TRUE;
    }
    else if(GFL_NET_SystemGetCurrentID()==COMM_PARENT_ID){  // 親機のみ子機情報をLIBで得られる
        u16 bitmap = GFL_NET_WL_GetBitmap();
        if( bitmap & (1<<netID)){
            return TRUE;
        }
    }
    else if(_pComm->bitmap & (1<<netID)){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   通信可能状態の人数を返す
 * @param   none
 * @retval  接続人数
 */
//==============================================================================
int GFL_NET_SystemGetConnectNum(void)
{
    int num = 0,i;

    for(i = 0; i < GFL_NET_MACHINE_MAX; i++){
        if(GFL_NET_SystemIsConnect(i)){
            num++;
        }
    }
    return num;
}

//==============================================================================
/**
 * @brief   初期化しているかどうかを返す
 * @param   none
 * @retval  初期が終わっていたらTRUE
 */
//==============================================================================
BOOL GFL_NET_SystemIsInitialize(void)
{
    if(_pComm){
        if( _pComm->device == _DEVICE_WIFI){
            return TRUE;
        }
    }
    return GFL_NET_WLIsInitialize();
}

//==============================================================================
/**
 * @brief   サーバー側から子機に送る場合 送信キューへの追加
 * @param   command    comm_sharing.hに定義したラベル
 * @param   sendNetID
 * @param   data       送信したいデータ ない時はNULL
 *                     このデータは静的でなければならない  バッファに溜めないため
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

#if 0
BOOL GFL_NET_SystemSetSendQueue_ServerSide(int command, const void* data, int size)
{
    if(_transmissonType() == _DS_MODE){
        return GFL_NET_QueuePut(&_pComm->sendQueueMgr, command, (u8*)data, size, TRUE, FALSE,0xf,0xf);
    }
    else{
        return GFL_NET_QueuePut(&_pComm->sendQueueMgrServer, command, (u8*)data, size, TRUE, FALSE,0xf,0xf);
    }
}
#endif

//==============================================================================
/**
 * @brief   クライアント側から親機に送る場合 送信キューへの追加
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 *                     このデータは静的でなければならない  バッファに溜めないため
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

//BOOL GFL_NET_SystemSetSendQueue(int command, const void* data, int size)
//{
//    return GFL_NET_QueuePut(&_pComm->sendQueueMgr, command, (u8*)data, size, FALSE, FALSE);
//}
#if 0
enum{
    _TRANS_NONE,
    _TRANS_LOAD,
    _TRANS_LOAD_END,
    _TRANS_SEND,
    _TRANS_SEND_END,
};


static void _transmission(void)
{
    BOOL bCatch=FALSE;

    if(!_pComm){
        return;
    }
#if 0    /// @@OO モード切替部分　　現在封印中
    switch(_pComm->transmissionNum){
      case _TRANS_LOAD:
        if(_transmissonType() == _DS_MODE){
            bCatch = GFL_NET_SystemSendFixSizeData(GFL_NET_CMD_DSMP_CHANGE_REQ,&_pComm->transmissionSend);
        }
        else{
            bCatch = GFL_NET_SystemSendData_ServerSide(GFL_NET_CMD_DSMP_CHANGE_REQ, &_pComm->transmissionSend, 1);
        }
        if(bCatch){
            _pComm->transmissionNum = _TRANS_LOAD_END;
        }
        break;
      case _TRANS_SEND:
        if(GFL_NET_SystemSendFixSizeData(GFL_NET_CMD_DSMP_CHANGE_END,&_pComm->transmissionSend)){
            _commSetTransmissonType(_pComm->transmissionSend);  // 切り替える  親機はここで切り替えない
            _pComm->transmissionNum = _TRANS_NONE;
        }
        break;
    }
#endif
}

//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え  GFL_NET_CMD_DSMP_CHANGE
 * @param   none
 * @retval  残り数
 */
//==============================================================================

void GFL_NET_SystemRecvDSMPChange(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){
        return;
    }
    NET_PRINT("CommRecvDSMPChange 受信\n");
    _pComm->transmissionNum = _TRANS_LOAD;
    _pComm->transmissionSend = pBuff[0];
}

//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え
 * @param   none
 * @retval  残り数
 */
//==============================================================================

void GFL_NET_SystemRecvDSMPChangeReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID){
        return;
    }
    _pComm->transmissionSend = pBuff[0];
    _pComm->transmissionNum = _TRANS_SEND;
    NET_PRINT("CommRecvDSMPChangeReq 受信\n");
}

//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え 終了処理 GFL_NET_CMD_DSMP_CHANGE_END
 * @param   none
 * @retval  残り数
 */
//==============================================================================

void GFL_NET_SystemRecvDSMPChangeEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){
        return;
    }
    NET_PRINT("CommRecvDSMPChangeEND 受信\n");

    if(_pComm->transmissionNum == _TRANS_LOAD_END){
        _commSetTransmissonType(pBuff[0]);  // 切り替える
        _pComm->transmissionNum = _TRANS_NONE;
    }
}
#endif

//==============================================================================
/**
 * @brief   自分の機のIDを返す
 * @param   
 * @retval  自分の機のID  つながっていない場合COMM_PARENT_ID
 */
//==============================================================================

u16 GFL_NET_SystemGetCurrentID(void)
{
    if(_pComm){
        if( _pComm->device == _DEVICE_WIFI){
#if 0  //wifi
            int id = mydwc_getaid();
            if(id != -1){
                return id;
            }
#endif
        }
        else if(GFL_NET_SystemGetAloneMode()){
            return COMM_PARENT_ID;
        }
        else{
            return GFL_NET_WL_GetCurrentAid();
        }
    }
    return COMM_PARENT_ID;
}


//==============================================================================
/**
 * @brief   WHライブラリで　通信状態のBITを確認
 * @param   none
 * @retval  接続がわかるBIT配列
 */
//==============================================================================

BOOL GFL_NET_SystemIsChildsConnecting(void)
{
    return GFL_NET_WLIsChildsConnecting();
}

//==============================================================================
/**
 * @brief   エラー状態かどうか
 * @param   none
 * @retval  エラーの時TRUE
 */
//==============================================================================

BOOL GFL_NET_SystemIsError(void)
{
    if(GFL_NET_SystemGetAloneMode()){  // 一人モードの場合ここはエラーにしない
        return FALSE;
    }
    if(_pComm){
        if(_pComm->bError){
//            CommStateSetErrorCheck(TRUE,TRUE);
            return TRUE;
        }
    }
    return GFL_NET_WLIsError();
}

//==============================================================================
/**
 * @brief   サービス番号に対応した子機送信byte数を得ます
 * サービス番号は include/communication/comm_def.hにあります
 * @param   serviceNo サービス番号
 * @retval  子機台数
 */
//==============================================================================

static u16 _getUserMaxSendByte(void)
{
    if(_transmissonType() == _MP_MODE){
        return _SEND_BUFF_SIZE_CHILD;
    }
    return _SEND_BUFF_SIZE_4CHILD;
}

//==============================================================================
/**
 * @brief   最大接続人数を得る
 * @param   none
 * @retval  最大接続人数
 */
//==============================================================================

int GFL_NET_SystemGetMaxEntry(int service)
{
    return _getUserMaxNum();
}

//==============================================================================
/**
 * @brief   最小接続人数を得る
 * @param   none
 * @retval  最小接続人数
 */
//==============================================================================

int GFL_NET_SystemGetMinEntry(int service)
{
    return _getUserMinNum();
}

//==============================================================================
/**
 * @brief   一人通信モードを設定
 * @param   bAlone    一人通信モード
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemSetAloneMode(BOOL bAlone)
{
    if(_pComm){
        _pComm->bAlone = bAlone;
    }
}

//==============================================================================
/**
 * @brief   一人通信モードかどうかを取得
 * @param   none
 * @retval  一人通信モードの場合TRUE
 */
//==============================================================================

BOOL GFL_NET_SystemGetAloneMode(void)
{
    if(_pComm){
        return _pComm->bAlone;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   自動終了コマンド受信
 * @param   netID  通信ID
 * @param   size   サイズ
 * @param   pData  データ
 * @param   pWork  ワーク
 * @return  none
 */
//==============================================================================

void GFL_NET_SystemRecvAutoExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    u8 dummy;

    NET_PRINT("CommRecvAutoExit 受信 \n");
    if(!GFL_NET_WLIsAutoExit()){
        if(GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID){   // 自分が親の場合みんなに逆返信する
            GFL_NET_SendData(pNetHandle,GFL_NET_CMD_EXIT_REQ, &dummy);
        }
    }
    GFL_NET_WLSetAutoExit();
}

#ifdef GFL_NET_DEBUG

//==============================================================================
/**
 * @brief   デバッグ用にダンプを表示する
 * @param   adr           表示したいアドレス
 * @param   length        長さ
 * @param   pInfoStr      表示したいメッセージ
 * @retval  サービス番号
 */
//==============================================================================

void GFL_NET_SystemDump_Debug(u8* adr, int length, char* pInfoStr)
{
    int i,j = 0;

    NET_PRINT("%s \n",pInfoStr);
    while(length){
        NET_PRINT(">> ");
        for(i = 0 ; i < 8 ; i++){
            NET_PRINT("%2x ",adr[j]);
            j++;
            if(j == length){
                break;
            }
        }
        NET_PRINT("\n");
        if(j == length){
            break;
        }
    }
    NET_PRINT(" --end\n");
}

#endif

//==============================================================================
/**
 * @brief   特定のコマンドを送信し終えたかどうかを調べる
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//==============================================================================


BOOL GFL_NET_SystemIsSendCommand(int command,int myID)
{
    if((_transmissonType() == _MP_MODE) && (myID == 0)){
        return GFL_NET_QueueIsCommand(&_pComm->sendQueueMgrServer, command);
    }
    return GFL_NET_QueueIsCommand(&_pComm->sendQueueMgr, command);

}


//==============================================================================
/**
 * @brief   キューが空っぽかどうか サーバー側
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================

BOOL GFL_NET_SystemIsEmptyQueue_ServerSize(void)
{
    return GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgrServer);

}

//==============================================================================
/**
 * @brief   キューが空っぽかどうか
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================

BOOL GFL_NET_SystemIsEmptyQueue(void)
{
    return GFL_NET_QueueIsEmpty(&_pComm->sendQueueMgr);

}

//==============================================================================
/**
 * @brief   wifi接続したかどうかを設定する
 * @param   none
 * @retval  コマンドが在ったらFALSE
 */
//==============================================================================

void GFL_NET_SystemSetWifiConnect(BOOL bConnect)
{
    // 通信待機状態にあるかどうかを伝える
    _pComm->bWifiConnect = bConnect;
}

//==============================================================================
/**
 * @brief   VCHAT状態かどうか
 * @param   none
 * @retval  VCHAT状態ならTRUE
 */
//==============================================================================

BOOL GFL_NET_SystemIsVChat(void)
{
    if( _pComm->device == _DEVICE_WIFI){
#if 0  // wifi
        return mydwc_IsVChat();
#endif
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   WIFI通信を同期通信するか、非同期通信するかの切り替えを行う
 * @param   TRUE 同期 FALSE 非同期
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemSetWifiBothNet(BOOL bFlg)
{
    if( _pComm->device == _DEVICE_WIFI){
        _pComm->bWifiSendRecv = bFlg;
        if(bFlg){
            _pComm->countSendRecv = 0;
            _pComm->countSendRecvServer[0] = 0;
            _pComm->countSendRecvServer[1] = 0;
        }
        NET_PRINT("oo同期切り替え %d\n",bFlg);
    }
}

//==============================================================================
/**
 * @brief   エラーにする場合TRUE
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemSetError(void)
{
    _pComm->bError=TRUE;
}

//==============================================================================
/**
 * @brief   通信を止める
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemShutdown(void)
{
    if(_pComm){
        _pComm->bShutDown = TRUE;
    }
}

//==============================================================================
/**
 * @brief   サーバー側の通信キューをリセットする
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemResetQueue_Server(void)
{
    GFL_NET_QueueManagerReset(&_pComm->sendQueueMgrServer);
}

//==============================================================================
/**
 * @brief   通信の受信を止めるフラグをセット
 * @param   bFlg  TRUEで止める  FALSEで許可
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemRecvStop(BOOL bFlg)
{
    if(_pComm){
        _pComm->bNotRecvCheck = bFlg;
    }
}

//==============================================================================
/**
 * @brief   通信の受信を止めるフラグをセット
 * @param   bFlg  TRUEで止める  FALSEで許可
 * @retval  none
 */
//==============================================================================

GFL_NETHANDLE* GFL_NET_SystemGetHandle(int NetID)
{
    return _pComm->pNetHandle[NetID];
}

