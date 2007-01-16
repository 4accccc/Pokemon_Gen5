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

#include "net.h"
#include "net_def.h"
#include "device/net_wireless.h"
#include "net_system.h"
#include "net_command.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "tool/net_tool.h"

//#include "wifi/dwc_rap.h"   //WIFI

#define FREEZE_SORCE (0)
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

//子機送信バッファのサイズ    １６台つなぐ場合の任天堂の推奨バイト数
#define _SEND_BUFF_SIZE_CHILD  WH_MP_CHILD_DATA_SIZE
//子機送信バッファのサイズ    ４台以下接続時の送信バイト数
#define _SEND_BUFF_SIZE_4CHILD  WH_MP_4CHILD_DATA_SIZE
// 子機RING送信バッファサイズ
#define _SEND_RINGBUFF_SIZE_CHILD  (_SEND_BUFF_SIZE_CHILD * 22)
//親機送信バッファのサイズ
#define _SEND_BUFF_SIZE_PARENT  WH_MP_PARENT_DATA_SIZE
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
// ありえないHEADER
#define _INVALID_HEADER  (0xff)
// 送信データがまだもらえていなかった
#define _NODATA_SEND  (0x0e)

//VBlank処理のタスクのプライオリティー
#define _PRIORITY_VBLANKFUNC (0)

// 通信先頭バイトのBITの意味
#define _SEND_NONE  (0x00)  // 一回で送れる場合
#define _SEND_NEXT  (0x01)  // 一回で送れない場合
#define _SEND_NO_DATA  (0x02)  // キーデーター以外は無い場合 MP子機
#define _MP_DATA_HEADER (0x0b)  // 親機MPデータの場合の番号  DSの場合は 0x00 or 0x01 or 0xfe or 0xff

#define _ACTION_COUNT_MOVE  (8)  // 移動キーを送らない期間


typedef enum{   // 送信状態
    _SEND_CB_NONE,           // なにもしていない
    _SEND_CB_FIRST_SEND,     // 流れの中での最初の送信
    _SEND_CB_FIRST_SENDEND,  // 最初の送信のコールバックが来た
//    _SEND_CB_SECOND_SEND,    // 割り込みでの送信
//    _SEND_CB_SECOND_SENDEND  // 割り込みでの送信コールバックが来た

} _sendCallbackCondition_e;

typedef enum{   // 送信形態
    _MP_MODE,    // 親子型
    _DS_MODE,    // 並列型
    _CHANGE_MODE_DSMP,  // DSからMPに切り替え中
    _CHANGE_MODE_MPDS,  // MPからDSに切り替え中
} _connectMode_e;

typedef enum {
    _NONE_KEY,
    _RANDOM_KEY,
    _REVERSE_KEY,
} _trapKeyMode_e;


typedef enum {
    _DEVICE_WIRELESS,
    _DEVICE_WIFI,
} _deviceMode_e;

//==============================================================================
// ワーク
//==============================================================================

typedef struct{
    u8* pData;     ///< データポインタ
    u16 size;       ///< サイズ
    u8 command;    ///< コマンド
    u8 priority;   ///< 優先順位  もしくは送信中かどうか
} _SEND_QUEUE;

typedef struct{
    int dataPoint; // 受信バッファ予約があるコマンドのスタート位置
    u8* pRecvBuff; // 受信バッファ予約があるコマンドのスタート位置
    u16 valSize;
    u8 valCommand;
} _RECV_COMMAND_PACK;


typedef struct{
    /// ----------------------------子機用＆親機用BUFF
    u8 sSendBuf[2][_SEND_BUFF_SIZE_4CHILD];          ///<  子機の送信用バッファ
    u8 sSendBufRing[_SEND_RINGBUFF_SIZE_CHILD];  ///<  子機の送信リングバッファ
    u8 sSendServerBuf[2][_SEND_BUFF_SIZE_PARENT];          ///<  親機の送信用バッファ
    u8 sSendServerBufRing[_SEND_RINGBUFF_SIZE_PARENT];
    u8* pMidRecvBufRing;          ///< 受け取るバッファをバックアップする DS専用
    u8* pServerRecvBufRing;       ///< 親機側受信バッファ
    u8* pRecvBufRing;             ///< 子機が受け取るバッファ
    u8* pTmpBuff;                 ///< 受信受け渡しのための一時バッファポインタ
    //----ring
    RingBuffWork sendRing;
    RingBuffWork recvRing;                      ///< 子機の受信リングバッファ
//    RingBuffWork recvRingUndo;                      ///< 子機の受信リングバッファ
    RingBuffWork recvMidRing[GFL_NET_MACHINE_MAX];
    RingBuffWork sendServerRing;
    RingBuffWork recvServerRing[GFL_NET_MACHINE_MAX];
//    RingBuffWork recvServerRingUndo[GFL_NET_MACHINE_MAX];
//    TCB_PTR pVBlankTCB;
    ///---que関連
    SEND_QUEUE_MANAGER sendQueueMgr;
    SEND_QUEUE_MANAGER sendQueueMgrServer;
    ///---受信関連
    _RECV_COMMAND_PACK recvCommServer[GFL_NET_MACHINE_MAX];
    _RECV_COMMAND_PACK recvCommClient;
    
    //---------  同期関連
    BOOL bWifiSendRecv;   // WIFIの場合同期を取る時ととらないときが必要なので 切り分ける
    volatile int countSendRecv;   // 送ったら＋受け取ったら－ 回数
    volatile int countSendRecvServer[GFL_NET_MACHINE_MAX];   // 送ったら＋受け取ったら－ 回数

#ifdef PM_DEBUG

    volatile int countSendNum;   // 送ったら＋
    volatile int countRecvNum;   // 受け取ったら＋ 回数
    volatile int countSendNumServer[GFL_NET_MACHINE_MAX];   // 送ったら＋ 回数
    volatile int countRecvNumServer[GFL_NET_MACHINE_MAX];   // 受け取ったら＋ 回数
#endif
    
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

    u8 bPSendNoneRecv[GFL_NET_MACHINE_MAX];        // 最初のコールバックを無条件無視

#ifdef PM_DEBUG
    u8 DebugAutoMove;
#endif
    u8 standNo[4];     // 戦闘のときの立った位置
   // u8 actionCount;
    u8 transmissionNum;
    u8 transmissionSend;
    u8 transmissionType;  // 通信形態 DSかMPかの管理
  //  u8 changeService;    // 通信形態の変更
   // u8 sendSwitch;   // 送信バッファのスイッチフラグ
    u8 sendServerSwitch;   // 送信バッファのスイッチフラグ（サーバ用）
    u8 bFirstCatchP2C;
    u8 bSendNoneSend;        // 無効コマンドを送る
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
// 割り込みでデータを処理するかどうかのフラグ
static volatile u8 _bVSAccess = FALSE;

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

static BOOL _setSendQueue(_SEND_QUEUE* pSendQueue,
                          int command, const void* data, int size);
static int _getNowQueueDataSize(_SEND_QUEUE* pSendQueue);
static BOOL _sendQueueSet(void);
static BOOL _sendServerQueueSet(void);
static void _queueSet(int restSize);
static void _queueSetServer(int restSize);
static void _spritDataSendFunc(void);

static void _transmission(void);
static u16 _getUserMaxSendByte(void);



#ifdef PM_DEBUG

extern int DebugCommGetNum(int id);
int DebugCommGetNum(int id)
{
    if(_pComm==NULL){
        return 0;
    }
    switch(id){
      case 0:
        return _pComm->countSendRecv;
      case 1:
        return _pComm->countSendRecvServer[0];
      case 2:
        return _pComm->countSendRecvServer[1];
      case 3:
        return _pComm->countSendRecvServer[2];
      case 4:
        return _pComm->countSendRecvServer[3];
      case 5:
        return _pComm->countSendNum;   // 送ったら＋
      case 6:
        return _pComm->countRecvNum;   // 受け取ったら＋ 回数
      case 7:
        return _pComm->countSendNumServer[0];   // 送ったら＋ 回数
      case 8:
        return _pComm->countSendNumServer[1];   // 送ったら＋ 回数
      case 9:
        return _pComm->countRecvNumServer[0];   // 送ったら＋ 回数
      case 10:
        return _pComm->countRecvNumServer[1];   // 送ったら＋ 回数
    }
    return 0;
}

#endif


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
 * @param   bAlloc          メモリー確保するかどうか
 * @param   packetSizeMax   確保したいパケットサイズ
 * @param   heapID          確保してもいいheapID
 * @retval  初期化に成功したらTRUE
 */
//==============================================================================

static BOOL _commInit(BOOL bAlloc, int packetSizeMax, int heapID)
{
    void* pWork;
    int i;
    // イクニューモン等ネットワークライブラリの再初期化が必要な場合TRUE
    BOOL bReInit = FALSE;

    _bVSAccess = FALSE;  // 割り込み内での処理を禁止

    if(bAlloc){
        int machineMax = _getUserMaxNum();
        if(_pComm!=NULL){  // すでに初期化している場合はreturn
            return TRUE;
        }
        NET_PRINT("_COMM_WORK_SYSTEM size %d \n", sizeof(_COMM_WORK_SYSTEM));
        _pComm = (_COMM_WORK_SYSTEM*)GFL_HEAP_AllocMemory(heapID, sizeof(_COMM_WORK_SYSTEM));
        MI_CpuClear8(_pComm, sizeof(_COMM_WORK_SYSTEM));
//        _pComm->pTool = GFL_NET_Tool_sysInit(heapID, machineMax);
        
        _pComm->packetSizeMax = packetSizeMax + 64;
        _pComm->transmissionType = _DS_MODE;
//        _pComm->changeService = COMM_MODE_NONE;
//        _pComm->bAlone = FALSE;
        
        _pComm->pRecvBufRing = GFL_HEAP_AllocMemory(heapID, _pComm->packetSizeMax*2); ///< 子機が受け取るバッファ
        _pComm->pTmpBuff = GFL_HEAP_AllocMemory(heapID, _pComm->packetSizeMax);  ///< 受信受け渡しのための一時バッファ
        _pComm->pServerRecvBufRing = GFL_HEAP_AllocMemory(heapID, machineMax * _pComm->packetSizeMax);   ///< 受け取るバッファをバックアップする
        _pComm->pMidRecvBufRing = GFL_HEAP_AllocMemory(heapID, machineMax * _pComm->packetSizeMax);   ///< 受け取るバッファをバックアップする DS専用
        // キューの初期化

        GFL_NET_QueueManagerInitialize(&_pComm->sendQueueMgr, _SENDQUEUE_NUM_MAX, &_pComm->sendRing, heapID);
        GFL_NET_QueueManagerInitialize(&_pComm->sendQueueMgrServer, _SENDQUEUE_SERVER_NUM_MAX, &_pComm->sendServerRing, heapID);

#ifdef PM_DEBUG
//        GFL_NET_QueueDebugTest();  // キューのプログラムテスト
#endif
    }
    else{
        bReInit = TRUE;
        GF_ASSERT((_pComm) && "切り替えの時はすでに初期化済み\n");
    }
    
#ifdef PM_DEBUG
    _pComm->DebugAutoMove = 0;
#endif
    _pComm->bitmap = 0;
    for(i =0; i < 4;i++){
        _pComm->standNo[i] = 0xff;
    }

    if(!bReInit){   // コマンドの初期化
        _commCommandInit();
    }

    //************************************

//    if(!bReInit){   // プロセスタスクの作成
        // VBLANK
  //      _pComm->pVBlankTCB = VIntrTCB_Add(_commMpVBlankIntr, NULL, _PRIORITY_VBLANKFUNC);

    //}
    _pComm->bWifiConnect = FALSE;
    return TRUE;
}


#ifdef PM_DEBUG
static u8 _sendDataNext=FALSE;
#endif


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
        _pComm->sSendServerBuf[0][i] = GFL_NET_CMD_NONE;
        _pComm->sSendServerBuf[1][i] = GFL_NET_CMD_NONE;
    }
    // 子機の送受信
    MI_CpuFill8(_pComm->sSendBufRing, 0, _SEND_RINGBUFF_SIZE_CHILD);
    GFL_NET_RingInitialize(&_pComm->sendRing, _pComm->sSendBufRing, _SEND_RINGBUFF_SIZE_CHILD);

    _pComm->sSendBuf[0][0] = _INVALID_HEADER;
    _pComm->sSendBuf[1][0] = _INVALID_HEADER;
    for(i = 1;i < _SEND_BUFF_SIZE_4CHILD;i++){
        _pComm->sSendBuf[0][i] = GFL_NET_CMD_NONE;
        _pComm->sSendBuf[1][i] = GFL_NET_CMD_NONE;
    }
    MI_CpuFill8(_pComm->pRecvBufRing, 0, _pComm->packetSizeMax*2);
    GFL_NET_RingInitialize(&_pComm->recvRing, _pComm->pRecvBufRing, _pComm->packetSizeMax*2);

    _pComm->bNextSendData = FALSE;
    _pComm->bNextSendDataServer = FALSE;
#ifdef PM_DEBUG
    _sendDataNext=FALSE;
#endif
    for(i = 0; i< GFL_NET_MACHINE_MAX;i++){
  //      _pComm->DSCountRecv[i] = 0xff;
        _pComm->recvDSCatchFlg[i] = 0;  // 通信をもらったことを記憶
        _pComm->bFirstCatch[i] = TRUE;
        _pComm->bPSendNoneRecv[i] = TRUE;
        _pComm->recvCommServer[i].valCommand = GFL_NET_CMD_NONE;
        _pComm->recvCommServer[i].valSize = 0xffff;
        _pComm->recvCommServer[i].pRecvBuff = NULL;
        _pComm->recvCommServer[i].dataPoint = 0;
        _pComm->countSendRecvServer[i] = 0;
#ifdef PM_DEBUG
        _pComm->countSendNumServer[i] = 0;
        _pComm->countRecvNumServer[i] = 0;
#endif
    }
    _pComm->countSendRecv = 0;
#ifdef PM_DEBUG
    _pComm->countSendNum = 0;
    _pComm->countRecvNum = 0;
#endif
    _pComm->bWifiSendRecv = TRUE;
    _pComm->recvCommClient.valCommand = GFL_NET_CMD_NONE;
    _pComm->recvCommClient.valSize = 0xffff;
    _pComm->recvCommClient.pRecvBuff = NULL;
    _pComm->recvCommClient.dataPoint = 0;

    _pComm->bFirstCatchP2C = TRUE;
    _pComm->bSendNoneSend = TRUE;
    
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
        _pComm->sSendServerBuf[0][i] = GFL_NET_CMD_NONE;
        _pComm->sSendServerBuf[1][i] = GFL_NET_CMD_NONE;
    }
    // 子機の送受信
    MI_CpuFill8(_pComm->sSendBufRing, 0, _SEND_RINGBUFF_SIZE_CHILD);
    GFL_NET_RingInitialize(&_pComm->sendRing, _pComm->sSendBufRing, _SEND_RINGBUFF_SIZE_CHILD);

    _pComm->sSendBuf[0][0] = _INVALID_HEADER;
    _pComm->sSendBuf[1][0] = _INVALID_HEADER;
    for(i = 1;i < _SEND_BUFF_SIZE_4CHILD;i++){
        _pComm->sSendBuf[0][i] = GFL_NET_CMD_NONE;
        _pComm->sSendBuf[1][i] = GFL_NET_CMD_NONE;
    }
    MI_CpuFill8(_pComm->pRecvBufRing, 0, _pComm->packetSizeMax*2);
    GFL_NET_RingInitialize(&_pComm->recvRing, _pComm->pRecvBufRing, _pComm->packetSizeMax*2);

    _pComm->bNextSendData = FALSE;
    _pComm->bNextSendDataServer = FALSE;
#ifdef PM_DEBUG
    _sendDataNext = FALSE;
#endif
    for(i = 0; i< GFL_NET_MACHINE_MAX;i++){
        _pComm->recvDSCatchFlg[i] = 0;  // 通信をもらったことを記憶
        _pComm->bFirstCatch[i] = TRUE;
        _pComm->bPSendNoneRecv[i] = TRUE;
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
    _pComm->bSendNoneSend = TRUE;
    
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
//    _pComm->DSCountRecv[netID] = 0xff;
    _pComm->recvDSCatchFlg[netID] = 0;  // 通信をもらったことを記憶 DS同期用
    _pComm->bFirstCatch[netID] = TRUE;  // コマンドをはじめてもらった時用
    _pComm->countSendRecvServer[netID]=0;  //SERVER受信
    _pComm->bPSendNoneRecv[netID] = TRUE;  // 一個無視

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
BOOL GFL_NET_SystemParentModeInit(BOOL bAlloc, BOOL bTGIDChange, int packetSizeMax, BOOL bEntry)
{
    BOOL ret = TRUE;

    ret = GFL_NET_WLParentInit(bAlloc, bTGIDChange, bEntry, _clearChildBuffers);
    GFL_NET_WLSetRecvCallback( GFL_NET_SystemRecvParentCallback );
    _commInit(bAlloc, packetSizeMax, GFL_HEAPID_SYSTEM); //@@OO後で外部に出す
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
BOOL GFL_NET_SystemChildModeInit(BOOL bAlloc, BOOL bBconInit, int packetSizeMax)
{
    BOOL ret = TRUE;

    ret = GFL_NET_WLChildInit(bAlloc, bBconInit);
    GFL_NET_WLSetRecvCallback( GFL_NET_SystemRecvCallback );

    _commInit(bAlloc, packetSizeMax, GFL_HEAPID_SYSTEM);
    _sendCallBack = _SEND_CB_FIRST_SENDEND;

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

    // 遅延してる時に変更しないようにした
    if(GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID){
        if(_sendCallBackServer != _SEND_CB_FIRST_SENDEND){
            return;
        }
    }
    else{
        if(_sendCallBack != _SEND_CB_FIRST_SENDEND){
            return;
        }
    }
    
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
    if(bChange){
        _commCommandInitChange2(); // コマンド全部消し
        NET_PRINT("send Recv %d %d %d \n",_pComm->countSendRecv,_pComm->countSendRecvServer[0],_pComm->countSendRecvServer[1]);
    }

    _transmission();

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
//        CommInfoFinalize();
        // VBLANKタスクを切る
        _bVSAccess = FALSE;  // 割り込み内での処理を禁止
//        NET_PRINT("VBLANKタスクを切る\n");
    //    TCB_Delete(_pComm->pVBlankTCB);
  //      _pComm->pVBlankTCB = NULL;
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
BOOL GFL_NET_SystemChildIndexConnect(u16 index)
{
    return GFL_NET_WLChildIndexConnect(index);
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

//    CommStateCheckFunc(); //commstateをタスク処理しないことにしたのでここに

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

            _bVSAccess = FALSE;   // 安全のためVBlank割り込みでのアクセス禁止宣言
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
            _bVSAccess = TRUE;  // 次の割り込み時での処理を許可
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
    BOOL bAcc = _bVSAccess;

    NET_PRINT("CommSystemReset\n");
    _bVSAccess = FALSE;  // 割り込み内での処理を禁止
    if(_pComm){
        _commCommandInit();
    }
    _bVSAccess = bAcc;
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
    BOOL bAcc = _bVSAccess;

    NET_PRINT("CommSystemReset\n");
    _bVSAccess = FALSE;  // 割り込み内での処理を禁止
    if(_pComm){
        _pComm->transmissionType = _DS_MODE;
        _commCommandInit();
    }
    _bVSAccess = bAcc;
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
    BOOL bAcc = _bVSAccess;

    NET_PRINT("CommSystemReset\n");
    _bVSAccess = FALSE;  // 割り込み内での処理を禁止
    if(_pComm){
        _commCommandInit();
        GFL_NET_WLChildBconDataInit();
    }
    _bVSAccess = bAcc;
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
    if(_pComm->bSendNoneSend){
        if( _pComm->device == _DEVICE_WIFI){
#if 0        // wifiは封印
            if( _pComm->bWifiConnect ){
                if( mydwc_sendToServer( _pComm->sSendBuf[0], _SEND_BUFF_SIZE_4CHILD )){
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


    if( _pComm->device == _DEVICE_WIFI){
#if 0  //wifi封印
        if( _pComm->bWifiConnect ){
            if( _pComm->bWifiSendRecv ){  // 同期を取っている場合
                if( _pComm->countSendRecv > _SENDRECV_LIMIT ){  //送りすぎ
                    return;
                }
                if(_sendCallBack == _SEND_CB_FIRST_SENDEND){
                    _setSendData(_pComm->sSendBuf[0]);   // 送るデータをリングバッファから差し替える
                    _sendCallBack = _SEND_CB_NONE;
                }
            }
            else{
                if(_sendCallBack == _SEND_CB_FIRST_SENDEND){
                    if(!_setSendData(_pComm->sSendBuf[0])){  // 送るデータをリングバッファから差し替える
                        return;  // 本当に送るものが何も無い場合
                    }
                    _sendCallBack = _SEND_CB_NONE;
                }
            }
            if( mydwc_sendToServer( _pComm->sSendBuf[0], _SEND_BUFF_SIZE_4CHILD )){
//                if( !_pComm->bWifiSendRecv ){  // 同期を取ってない場合
//                    DEBUG_DUMP(&_pComm->sSendBuf[0][0], 16,"sp0");
//                }
                _sendCallBack = _SEND_CB_FIRST_SENDEND;
                _pComm->countSendRecv++;  //wifi client
#ifdef PM_DEBUG
                _pComm->countSendNum++;
#endif
            }
            else{
            }
        }
#endif //wifi封印
    }
    else if(((GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID())) &&
        (GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID()))) || GFL_NET_SystemGetAloneMode()){
        if(_sendCallBack != _SEND_CB_FIRST_SENDEND){  // 1個送ったが送信完了していない
//            NET_PRINT("うけとってない _sendCallBack\n");
            return;
        }
        if( _pComm->countSendRecv > _SENDRECV_LIMIT ){  //送りすぎ
            NET_PRINT("親の同期待ち\n");
            return;
        }
        _setSendData(_pComm->sSendBuf[0]);  // 送るデータをリングバッファから差し替える
  //      _setSendData(_pComm->sSendBuf[ 1 - _pComm->sendSwitch]);  // 送るデータをリングバッファから差し替える
        _sendCallBack = _SEND_CB_NONE;
        _updateMpData();     // データ送受信
    }
}

//==============================================================================
/**
 * @brief   中間RINGBUFFから子機全員に逆送信するためbuffにコピー
 * @param   none
 * @retval  none
 */
//==============================================================================

static BOOL _copyDSData(int switchNo)
{
    int mcSize;
    int machineMax;
    int i,num,nosend = 0;

    mcSize = _getUserMaxSendByte();//CommGetServiceMaxChildSendByte(CommStateGetServiceNo());
    machineMax = _getUserMaxNum();
/*
    // 初期化
    for(i = 0; i < machineMax; i++){
        if(GFL_NET_SystemIsConnect(i) && (_pComm->recvDSCatchFlg[i] > 1) ){
            // NET_PRINT("--------------追い越しが発生 %d\n",i);
            //                        _pComm->sSendServerBuf[_pComm->sendServerSwitch][i*mcSize] = _NODATA_SEND;
        }
        if(GFL_NET_SystemIsConnect(i) && (_pComm->recvDSCatchFlg[i] == 0) ){
            NET_PRINT("--------------追いぬきが発生 %d\n",i);
            _pComm->sSendServerBuf[switchNo][i * mcSize] = _NODATA_SEND;
            if(CommStateGetServiceNo() < COMM_MODE_BATTLE_SINGLE_WIFI){
                _sendCallBackServer--;
                return FALSE;
            }
        }
        _pComm->recvDSCatchFlg[i] = 0;
    }
   */
    /// 中間RINGBUFFから子機全員に逆送信するためbuffにコピー
    for(i = 0; i < machineMax; i++){
        GFL_NET_RingEndChange(&_pComm->recvMidRing[i]);
        if(GFL_NET_SystemIsConnect(i)){
            _pComm->sSendServerBuf[switchNo][i * mcSize] = _NODATA_SEND;
        }
        num = GFL_NET_RingGets(&_pComm->recvMidRing[i] ,
                     &_pComm->sSendServerBuf[switchNo][i*mcSize],
                     mcSize);
        if(_pComm->sSendServerBuf[switchNo][i * mcSize] == _NODATA_SEND){
            nosend++;
        }
    }
    if(nosend == machineMax){
        return FALSE;
    }
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
            _copyDSData(0);
        }
        if( (GFL_NET_WL_IsConnectLowDevice(GFL_NET_SystemGetCurrentID()))  && !GFL_NET_SystemGetAloneMode()){
            if(!GFL_NET_WL_SendData(_pComm->sSendServerBuf[0],
                            _SEND_BUFF_SIZE_PARENT,
                            _sendServerCallback)){
                _sendCallBackServer--;
                NET_PRINT("しっぱい\n");
            }
            else{
                NET_PRINT("おくった\n");
            }
        }
        // 送信完了
        if((_sendCallBackServer == _SEND_CB_FIRST_SEND) ){

            for(i = 0; i < machineMax; i++){
                if(GFL_NET_SystemIsConnect(i)){
                    _pComm->countSendRecvServer[i]++; // 親MP送信
#ifdef PM_DEBUG
                    _pComm->countSendNumServer[i]++;
#endif
                }
                else if(GFL_NET_SystemGetAloneMode() && (i == 0)){
                    _pComm->countSendRecvServer[i]++; // 親MP送信
#ifdef PM_DEBUG
                    _pComm->countSendNumServer[i]++;
#endif
                }
            }

            // 親機自身に子機の動きをさせるためここでコールバックを呼ぶ
            _commRecvCallback(COMM_PARENT_ID,
                              (u16*)_pComm->sSendServerBuf[ 0 ],
                              _SEND_BUFF_SIZE_PARENT);
        }
        for(i = 0; i < machineMax; i++){
            if(!GFL_NET_SystemIsConnect(i)){
                if(_transmissonType() == _DS_MODE){             // 初期化
                    _pComm->sSendServerBuf[0][i*mcSize] = _INVALID_HEADER;
                }
                else{
  //                        _clearChildBuffers(i);
                }
            }
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

    if(_pComm->bSendNoneSend){
        if( _pComm->device == _DEVICE_WIFI){
#if 0  //wifi lock
            if( GFL_NET_SystemIsConnect(COMM_PARENT_ID) ){
                if( mydwc_sendToClient( _pComm->sSendServerBuf[0], WH_MP_4CHILD_DATA_SIZE*2 )){
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
                        _copyDSData(0);  //DS通信ならコピー
                    }
                    _sendCallBackServer = _SEND_CB_NONE;
                }
            }
            else{
                if(_sendCallBackServer == _SEND_CB_FIRST_SENDEND){
                    if(_transmissonType() == _DS_MODE){
                        if(!_copyDSData(0)){  //DS通信ならコピー
                            return;
                        }
                    }
                }
                _sendCallBackServer = _SEND_CB_NONE;
            }

            if( mydwc_sendToClient( _pComm->sSendServerBuf[0], WH_MP_4CHILD_DATA_SIZE*2 )){
                OHNO_SP_PRINT("send %d \n",_pComm->sSendServerBuf[0][0]);
                _sendCallBackServer = _SEND_CB_FIRST_SENDEND;
                _pComm->countSendRecvServer[0]++; // wifi server
                _pComm->countSendRecvServer[1]++; // wifi server
#ifdef PM_DEBUG
                _pComm->countSendNumServer[0]++;
                _pComm->countSendNumServer[1]++;
#endif
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
                    NET_PRINT("送信しすぎ%d \n",i);
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
                _setSendDataServer(_pComm->sSendServerBuf[ 0 ]);  // 送るデータをリングバッファから差し替える
//            _setSendDataServer(_pComm->sSendServerBuf[ 1 - _pComm->sendServerSwitch ]);  // 送るデータをリングバッファから差し替える
            }
        }
        _sendCallBackServer = _SEND_CB_NONE;
        // 最初の送信処理
        _updateMpDataServer();
    }
}

#ifdef PM_DEBUG
static u8 debugHeadData[GFL_NET_MACHINE_MAX][100];
static int debugCnt[GFL_NET_MACHINE_MAX]={0,0,0,0,0,0,0,0};
static u8 debugHeadDataC[5][100];
static int debugCntC = 0;
#endif

#define WIFI_DUMP_TEST

//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemRecvCallback(u16 aid, u16 *data, u16 size)
{
    u8* adr = (u8*)data;

    if(!data){
        return;
    }
    
#ifdef WIFI_DUMP_TEST
    if((adr[0] & 0xf) != 0xe){
    DEBUG_DUMP(&adr[0], 38,"cr0");
    }
    if((adr[38] & 0xf) != 0xe){
  DEBUG_DUMP(&adr[38], 38,"cr1");
    }
#endif

    if(_pComm->bPSendNoneRecv[COMM_PARENT_ID]){
        _pComm->countSendRecv--;  //受信
#ifdef PM_DEBUG
        _pComm->countRecvNum++;
#endif
        _pComm->bPSendNoneRecv[COMM_PARENT_ID] = FALSE;
        return;
    }
    _commRecvCallback(aid, data, size);
}


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

#ifdef PM_DEBUG
    debugHeadDataC[0][debugCntC] = adr[0];
    debugHeadDataC[1][debugCntC] = adr[1];
    debugHeadDataC[2][debugCntC] = adr[2];
    debugHeadDataC[3][debugCntC] = adr[3];
    debugHeadDataC[4][debugCntC] = adr[4];
    debugCntC++;
    if(debugCntC == 100){
        debugCntC = 0;
    }
#endif

    
//    if( !_pComm->bWifiSendRecv ){  // 同期を取ってない
    //DEBUG_DUMP(&adr[0], 16,"cr0");
    //DEBUG_DUMP(&adr[38], 16,"cr1");
//    }
    if(adr[0] == _MP_DATA_HEADER){   ///MPデータの場合
        if(_transmissonType() == _DS_MODE){
            NET_PRINT("DSなのにMPデータが来た \n");
            return;
        }
        adr++;
        recvSize--;
    }
    else if(_transmissonType() == _MP_MODE){  //DSデータの場合
        NET_PRINT("MPなのにDEPデータが来た \n");
        return;
    }
    if((_pComm->bFirstCatchP2C) && (adr[0] & _SEND_NEXT)){
        // まだ一回もデータをもらったことがない状態なのに連続データだった
        NET_PRINT("連続データ child %d \n",aid);
        DEBUG_DUMP((u8*)data,24,"cr");
        return;
    }
#if 0
    DEBUG_DUMP(adr,24,"cr");
#endif
    _pComm->bFirstCatchP2C = FALSE;

    if(_transmissonType() == _DS_MODE){
        int mcSize = _getUserMaxSendByte();
        int machineMax = _getUserMaxNum();

        for(i = 0; i < machineMax; i++){
            if(adr[0] == _INVALID_HEADER){
                _pComm->bitmap = _pComm->bitmap & ~(1<<i);
//                _pComm->DSCountRecv[i] = 0xff;
            }
            else{
                _pComm->bitmap = _pComm->bitmap | (1<<i);
            }
            if(adr[0] == _INVALID_HEADER){
                adr += mcSize;
            }
            else if(adr[0] == _NODATA_SEND){
                adr += mcSize;
//                NET_PRINT("--------------今回データなし %d\n",i);
            }
            else if((_pComm->bFirstCatch[i]) && (adr[0] & _SEND_NEXT)){ // まだ一回もデータをもらったことがない状態なのに連続データだった
                NET_PRINT("連続データ parent %d \n",aid);
                adr += mcSize;
            }
            else{
#if 0
                u8 cnt = ((adr[0] >> 4)  & 0xf);
                if(_pComm->DSCountRecv[i] != 0xff){
                    if(cnt > _pComm->DSCountRecv[i]){
                        if((cnt-1)!=_pComm->DSCountRecv[i]){
                            NET_PRINT("コマンドずれ  %d %d %d\n",i,cnt-1,_pComm->DSCountRecv[i]);
                            DEBUG_DUMP((u8*)data,size,"コマンドずれ");
                            GF_ASSERT((cnt-1)==_pComm->DSCountRecv[i]);
                        }
                    }
                    else{
                        if((cnt!=0) || (_pComm->DSCountRecv[i]!=0xf)){
                            NET_PRINT("コマンドずれ  %d %d %d\n",i,cnt-1,_pComm->DSCountRecv[i]);
                            DEBUG_DUMP((u8*)data,size,"コマンドずれ");
                        }
                        GF_ASSERT((cnt==0) && (_pComm->DSCountRecv[i]==0xf));
                    }
                }
                _pComm->DSCountRecv[i] = cnt;
#endif
                adr++;
                GFL_NET_RingPuts(&_pComm->recvServerRing[i], adr, mcSize-1);
                adr += (mcSize-1);
                _pComm->bFirstCatch[i]=FALSE;
            }
        }
    }
    else{   //MPデータ
#if 0
        DEBUG_DUMP(adr,recvSize,"子機データ受信");
#endif
//        DEBUG_DUMP(adr,recvSize,"子機データ受信");
        
        adr++;      // ヘッダー１バイト読み飛ばす
        _pComm->bitmap = adr[0];
        _pComm->bitmap *= 256;
        adr++;   // Bitmapでーた
        _pComm->bitmap += adr[0];
        adr++;   // Bitmapでーた
        recvSize -= 3;
        //    NET_PRINT("bitmap %x\n",_pComm->bitmap);
        recvSize = adr[0]; 
        adr++;
        //if(recvSize > GFL_NET_RingDataRestSize(&_pComm->recvRing)){
          //  GF_ASSERT_RETURN("Error:受信オーバー\n",);
        //}
        GFL_NET_RingPuts(&_pComm->recvRing , adr, recvSize);
    }
}


//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック
 * @param   result  成功か失敗
 * @retval  none
 */
//==============================================================================

void GFL_NET_SystemRecvParentCallback(u16 aid, u16 *data, u16 size)
{
    u8* adr = (u8*)data;

    if(!data){  //データが存在しない
        return;
    }
    
#ifdef WIFI_DUMP_TEST
//   if((adr[0] & 0xf) != 0xe){
    if(aid == 1){
        DEBUG_DUMP(&adr[0], 38,"pr1");
        NET_PRINT("adr %x size %d\n",(u32)data, size);
    }
    else{
        DEBUG_DUMP(&adr[0], 38,"pr0");
    }
//   }
#endif

    if(_pComm->bPSendNoneRecv[aid]){
        _pComm->countSendRecvServer[aid]--;  //SERVER受信
#ifdef PM_DEBUG
        _pComm->countRecvNumServer[aid]++;
#endif
        _pComm->bPSendNoneRecv[aid] = FALSE;
        return;
    }


    _commRecvParentCallback(aid, data, size);
}


//==============================================================================
/**
 * @brief   通信を受信した時に呼ばれるコールバック
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

    if((_pComm->bFirstCatch[aid]) && (adr[0] & _SEND_NEXT)){
        // まだ一回もデータをもらったことがない状態なのに連続データだった
        NET_PRINT("連続データ parent %d \n",aid);
        i = 0;
        DEBUG_DUMP(adr,12,"連続データ");
        return;
    }
    _pComm->bFirstCatch[aid] = FALSE;

    if(_transmissonType() == _DS_MODE){
        int mcSize = _getUserMaxSendByte();
        int machineMax = _getUserMaxNum();
#if 0
        if((aid == 1) && ((adr[0] & 0x0f)== _SEND_NEXT)){
           DEBUG_DUMP(&adr[1],mcSize-1,"poke");
        }
   //     if(aid == 0){
   //         DEBUG_DUMP(adr,mcSize,"親機のDS");
  //      }
#endif
#if 0
        for(i = 1;i < mcSize; i++){
            if(adr[i] != GFL_NET_CMD_NONE){  //何かが送られてきてたら
                GFL_NET_RingPuts(&_pComm->recvMidRing[aid] , adr, mcSize);
                //OHNO_SP_PRINT("middle %d %d\n",GFL_NET_RingDataSize(&_pComm->recvMidRing[aid]),aid);
//                NET_PRINT(" まるまま中間リングに入れる %d %d \n", aid, GFL_NET_RingDataSize(&_pComm->recvMidRing[aid]));
                break;
            }
        }
#endif
        if(!(adr[0] & _SEND_NO_DATA)){
            GFL_NET_RingPuts(&_pComm->recvMidRing[aid] , adr, mcSize);
//            NET_PRINT("MidRingのこり%d\n",GFL_NET_RingDataRestSize(&_pComm->recvMidRing[aid]));
        }
        _pComm->recvDSCatchFlg[aid]++;  // 通信をもらったことを記憶
    }else{   // MPモード

        if(adr[0] & _SEND_NO_DATA){   // データが空っぽの場合受け取らない
            return;
        }
        adr++;
        if(_RECV_BUFF_SIZE_PARENT > GFL_NET_RingDataRestSize(&_pComm->recvServerRing[aid])){
            NET_PRINT("Error Throw:受信オーバー\n");
            return;
        }
//        if(aid==0)
//            DEBUG_DUMP(adr,_RECV_BUFF_SIZE_PARENT,"MP 受け取った");
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
    if(result){
        _sendCallBackServer++;
        NET_PRINT("送信callback server側\n");
    }
    else{
        GF_ASSERT_MSG(0,"send failed");
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
                _commRecvParentCallback(COMM_PARENT_ID, (u16*)_pComm->sSendBuf[0],
                                    mcSize);
                //_pComm->sendSwitch = 1 - _pComm->sendSwitch;
                _pComm->countSendRecv++; // MP送信親
#ifdef PM_DEBUG
                _pComm->countSendNum++;
#endif
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
                    if(!GFL_NET_WL_SendData(_pComm->sSendBuf[0],
                                    mcSize, _sendCallbackFunc)){
                        _sendCallBack--;
                        NET_PRINT("failed WH_SendData\n");
                    }
                    else{
//                        _pComm->sendSwitch = 1 - _pComm->sendSwitch;
                        _pComm->countSendRecv++; // MP送信
#ifdef PM_DEBUG
                        _pComm->countSendNum++;
#endif
                    }
                }
                else if(GFL_NET_WLIsChildsConnecting()){         // サーバーとしての処理 自分以外の誰かにつながっている時
                    _sendCallBack++;
                    _sendCallbackFunc(TRUE);
                    // 子機のふりをする部分          // 親機は自分でコールバックを呼ぶ
                    _commRecvParentCallback(COMM_PARENT_ID, (u16*)_pComm->sSendBuf[0],
                                        mcSize);
//                    _pComm->sendSwitch = 1 - _pComm->sendSwitch;
                    _pComm->countSendRecv++; // MP送信
#ifdef PM_DEBUG
                    _pComm->countSendNum++;
#endif
                }
            }
            else{
//                NET_PRINT("sendCallBack != _SEND_CB_FIRST_SENDE\n");
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
        pSendBuff[0] |= _SEND_NO_DATA;  // 空っぽなら何も送らない
        if(pSendBuff[0] == _SEND_NO_DATA){
            return FALSE;  // 送るものが何も無い
        }
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

#ifdef PM_DEBUG
    if(CommStateGetServiceNo() == COMM_MODE_UNDERGROUND){
        if(_sendDataNext != _pComm->bNextSendDataServer){
            GF_ASSERT(0);
        }
    }
#endif

    if(_pComm->bNextSendDataServer == FALSE){  // 一回で送れる場合
        pSendBuff[1] = _SEND_NONE;
    }
    else{
        pSendBuff[1] = _SEND_NEXT;  // 一回で送れない場合
    }

    {
        u16 bitmap = GFL_NET_WL_GetBitmap();
        pSendBuff[2] = bitmap >> 8;
        pSendBuff[3] = bitmap & 0xff;

        {
            SEND_BUFF_DATA buffData;
            buffData.size = _SEND_BUFF_SIZE_PARENT - 5;
            buffData.pData = &pSendBuff[5];
            if(GFL_NET_QueueGetData(&_pComm->sendQueueMgrServer, &buffData, FALSE)){
                _pComm->bNextSendDataServer = FALSE;
                pSendBuff[4] = (_SEND_BUFF_SIZE_PARENT - 5) - buffData.size;
            }
            else{
                _pComm->bNextSendDataServer = TRUE;
                pSendBuff[4] = _SEND_BUFF_SIZE_PARENT - 5;
            }
#ifdef PM_DEBUG
            _sendDataNext = _pComm->bNextSendDataServer;
#endif

        }
    }
#if 0
    DEBUG_DUMP(pSendBuff, _SEND_BUFF_SIZE_PARENT, "MP SERVER SEND");
#endif
}


//==============================================================================
/**
 * @brief   子機送信メソッド  大きいサイズのデータを送信する
 *     バックアップしないので dataの中身を書き換えると、
 *     書き換えたものを送ってしまう可能性がある
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendHugeData(int command, const void* data, int size)
{
    if(!GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID()) && !GFL_NET_SystemGetAloneMode()){
        //        NET_PRINT("接続してなくて送れなかった\n");
        return FALSE;   // 通信状態が悪い場合送らない
    }
    if(GFL_NET_QueuePut(&_pComm->sendQueueMgr, command, (u8*)data, size, TRUE, FALSE)){
//        if(25 == command){
            //OHNO_SP_PRINT("%d ",GFL_NET_SystemGetCurrentID());
            //DEBUG_DUMP((u8*)data,size,"poke");
  //      }
#if 0
        NET_PRINT("<<<送信 NetId=%d -- size%d ",GFL_NET_SystemGetCurrentID(), size);
        GFL_NET_CommandDebugPrint(command);
#endif
        return TRUE;
    }
#ifdef DEBUG_ONLY_FOR_ohno
    NET_PRINT("-キュ- %d %d\n",GFL_NET_SystemGetCurrentID(),
               GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgr));
    GF_ASSERT(0);
#endif
//    if(CommStateGetServiceNo() == COMM_MODE_UNDERGROUND){
        GFL_NET_SystemSetError();
//    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   子機送信メソッド
 * 親機がデータを子機全員に送信するのは別関数
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendData(int command, const void* data, int size)
{
    NET_PRINT("< 送信 %d %d\n", command,GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgr));
//    GF_ASSERT_RETURN(size < 256 && "CommSendHugeDataを使ってください",FALSE);
    if(!GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID()) && !GFL_NET_SystemGetAloneMode()){
        NET_PRINT("接続してなくて送れなかった\n");
        return FALSE;   // 通信状態が悪い場合送らない
    }
    if(GFL_NET_QueuePut(&_pComm->sendQueueMgr, command, (u8*)data, size, TRUE, TRUE)){
#if 0
        NET_PRINT("<<<送信 NetId=%d -- size%d ",GFL_NET_SystemGetCurrentID(), size);
        GFL_NET_CommandDebugPrint(command);
#endif
        return TRUE;
    }
#ifdef DEBUG_ONLY_FOR_ohno
    NET_PRINT("-キュー無い- %d %d\n",GFL_NET_SystemGetCurrentID(),
               GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgr));
    GF_ASSERT(0);
#endif
//    if(CommStateGetServiceNo() == COMM_MODE_UNDERGROUND){
        GFL_NET_SystemSetError();
//    }
    return FALSE;
}


//==============================================================================
/**
 * @brief   親機専用サーバー送信メソッド
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

static BOOL _data_ServerSide(int command, const void* data, int size, BOOL bCopy)
{
    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){  // 親機以外は使えない
        GF_ASSERT(0 && "親以外は使用不可");
        return FALSE;
    }
    if(!GFL_NET_SystemIsConnect(COMM_PARENT_ID)  && !GFL_NET_SystemGetAloneMode()){
//        NET_PRINT("接続してなくて送れなかった\n");
        return FALSE;   // 通信状態が悪い場合送らない
    }
    if(_transmissonType() == _DS_MODE){
        NET_PRINT("WARRNING: DS通信状態なのにサーバー送信が使われた\n");
        return GFL_NET_SystemSendData(command, data, size);
    }

    if(GFL_NET_QueuePut(&_pComm->sendQueueMgrServer, command, (u8*)data, size, TRUE, bCopy)){
#if 0
        NET_PRINT("<<S送信 id=%d size=%d ",GFL_NET_SystemGetCurrentID(), size);
        GFL_NET_CommandDebugPrint(command);
//        DEBUG_DUMP(pSend, size, "S送信");
#endif
        return TRUE;
    }
#ifdef DEBUG_ONLY_FOR_ohno
    GF_ASSERT(0);
#endif
    //if(CommStateGetServiceNo() == COMM_MODE_UNDERGROUND){
        GFL_NET_SystemSetError();
    //}
    return FALSE;
}

//==============================================================================
/**
 * @brief   親機送信メソッド  大きいサイズのデータを送信する  サイズ固定
 *     バックアップしないので dataの中身を書き換えると、
 *     書き換えたものを送ってしまう可能性がある
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendFixHugeSizeData_ServerSide(int command, const void* data)
{
    return GFL_NET_SystemSendHugeData_ServerSide(command, data, 0);
}

//==============================================================================
/**
 * @brief   親機送信メソッド  大きいサイズのデータを送信する
 *     バックアップしないので dataの中身を書き換えると、
 *     書き換えたものを送ってしまう可能性がある
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendHugeData_ServerSide(int command, const void* data, int size)
{
    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){  // 親機以外は使えない
        GF_ASSERT(0 && "親以外は使用不可");
        return FALSE;
    }
    if(!GFL_NET_SystemIsConnect(COMM_PARENT_ID)  && !GFL_NET_SystemGetAloneMode()){
//        NET_PRINT("接続してなくて送れなかった\n");
        return FALSE;   // 通信状態が悪い場合送らない
    }
    if(_transmissonType() == _DS_MODE){
        NET_PRINT("WARRNING: DS通信状態なのにサーバー送信が使われた\n");
        return GFL_NET_SystemSendHugeData(command, data, size);
    }

    if(GFL_NET_QueuePut(&_pComm->sendQueueMgrServer, command, (u8*)data, size, TRUE, FALSE)){
#if 0
        NET_PRINT("<<S送信 id=%d size=%d ",GFL_NET_SystemGetCurrentID(), size);
        GFL_NET_CommandDebugPrint(command);
//        DEBUG_DUMP(pSend, size, "S送信");
#endif
        return TRUE;
    }
#ifdef DEBUG_ONLY_FOR_ohno
    NET_PRINT("-キュ無い- %d %d\n",GFL_NET_SystemGetCurrentID(),
               GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgrServer));
    GF_ASSERT(0);
#endif
//    if(CommStateGetServiceNo() == COMM_MODE_UNDERGROUND){
        GFL_NET_SystemSetError();
//    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   親機専用サーバー送信メソッド
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendData_ServerSide(int command, const void* data, int size)
{
    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){  // 親機以外は使えない
        GFL_NET_SystemSetError();
//        GF_ASSERT(0 && "親以外は使用不可");
        return FALSE;
    }
    if(!GFL_NET_SystemIsConnect(COMM_PARENT_ID)  && !GFL_NET_SystemGetAloneMode()){
        NET_PRINT("接続してなくて送れなかった\n");
        return FALSE;   // 通信状態が悪い場合送らない
    }
    if(_transmissonType() == _DS_MODE){
        NET_PRINT("WARRNING: DS通信状態なのにサーバー送信が使われた\n");
        return GFL_NET_SystemSendData(command, data, size);
    }

    if(GFL_NET_QueuePut(&_pComm->sendQueueMgrServer, command, (u8*)data, size, TRUE, TRUE)){

//        NET_PRINT("qnum %d %d\n",command,GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgrServer));

#if 0
        NET_PRINT("<<S送信 id=%d size=%d ",GFL_NET_SystemGetCurrentID(), size);
        GFL_NET_CommandDebugPrint(command);
//        DEBUG_DUMP(pSend, size, "S送信");
#endif
        return TRUE;
    }
#ifdef DEBUG_ONLY_FOR_ohno
    NET_PRINT("キュー無い- %d %d\n",GFL_NET_SystemGetCurrentID(),
               GFL_NET_QueueGetNowNum(&_pComm->sendQueueMgrServer));
    GF_ASSERT(0);
#endif
    //if(CommStateGetServiceNo() == COMM_MODE_UNDERGROUND){
        GFL_NET_SystemSetError();
    //}
    return FALSE;
}

//==============================================================================
/**
 * @brief   親機専用サーバー送信メソッド サイズ固定版
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @param   byte       送信量    コマンドだけの場合0
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendFixSizeData_ServerSide(int command, const void* data)
{
    return GFL_NET_SystemSendData_ServerSide(command, data, 0);
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
    GFL_NET_CommandCallBack(netID, command, size, pTemp);
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

static void _recvDataFuncSingle(RingBuffWork* pRing, int netID, u8* pTemp, BOOL bDebug, _RECV_COMMAND_PACK* pRecvComm)
{
    int size;
    u8 command;
    int bkPos;
    int realbyte;

    
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
        if(bDebug){
            NET_PRINT(">>>cR %d %d %d\n", bkPos, GFL_NET_RingDataSize(pRing), command);
        }
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
        }
        if(GFL_NET_CommandCreateBuffCheck(command)){  // 受信バッファがある場合
            if(pRecvComm->pRecvBuff==NULL){
                pRecvComm->pRecvBuff = GFL_NET_CommandCreateBuffStart(command, netID, pRecvComm->valSize);
            }
            realbyte = GFL_NET_RingGets(pRing, pTemp, size - pRecvComm->dataPoint);
//            OHNO_SP_PRINT("id %d -- rest %d\n",netID, size - pRecvComm->dataPoint);
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
                if(bDebug){
                    NET_PRINT(">>>受信 comm=%d id=%d -- size%d \n",command, netID, size);
                }
                GFL_NET_RingGets(pRing, pTemp, size);
                _endCallBack(netID, command, size, (void*)pTemp, pRecvComm);
            }
            else{   // まだ届いていない大きいデータの場合ぬける
                //            NET_PRINT("結合待ち command %d size %d\n",command,size);
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
        _recvDataFuncSingle(&_pComm->recvRing, id, _pComm->pTmpBuff, TRUE, &_pComm->recvCommClient);
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
    //        NET_PRINT("DS解析 %d\n",id);
#endif
            // 一個前の位置を変数に保存しておく
//            MI_CpuCopy8(&_pComm->recvServerRing[id],
  //                      &_pComm->recvServerRingUndo[id],
    //                    sizeof(RingBuffWork));
      //      GFL_NET_RingStartPush(&_pComm->recvServerRingUndo[id]); //start位置を保存
//            NET_PRINT("親機が子機%dを解析\n",id);
            _recvDataFuncSingle(&_pComm->recvServerRing[id], id, _pComm->pTmpBuff, FALSE, &_pComm->recvCommServer[id]);
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

BOOL GFL_NET_SystemSetSendQueue_ServerSide(int command, const void* data, int size)
{
    if(_transmissonType() == _DS_MODE){
        return GFL_NET_QueuePut(&_pComm->sendQueueMgr, command, (u8*)data, size, TRUE, FALSE);
    }
    else{
        return GFL_NET_QueuePut(&_pComm->sendQueueMgrServer, command, (u8*)data, size, TRUE, FALSE);
    }
}

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

BOOL GFL_NET_SystemSetSendQueue(int command, const void* data, int size)
{
    return GFL_NET_QueuePut(&_pComm->sendQueueMgr, command, (u8*)data, size, FALSE, FALSE);
}

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
 * @brief   汎用送信メソッド  送信サイズ固定でしかも大きい場合
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendFixHugeSizeData(int command, const void* data)
{
    return GFL_NET_SystemSendHugeData(command, data, 0);
}

//==============================================================================
/**
 * 汎用送信メソッド  送信サイズ固定の場合
 * @param   command    comm_sharing.hに定義したラベル
 * @param   data       送信したいデータ ない時はNULL
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendFixSizeData(int command, const void* data)
{
    return GFL_NET_SystemSendData(command, data, 0);
}

//==============================================================================
/**
 * @brief   汎用送信メソッド  コマンド以外存在しない場合
 * @param   command    comm_sharing.hに定義したラベル
 * @retval  送信キューに入ったかどうか
 */
//==============================================================================

BOOL GFL_NET_SystemSendFixData(int command)
{
    return GFL_NET_SystemSendData(command, NULL, 0);
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
            GFL_NET_SystemSendFixSizeData_ServerSide(GFL_NET_CMD_AUTO_EXIT, &dummy);
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
 * @brief   特定のコマンドを送信し終えたかどうかを調べる サーバ側
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//==============================================================================


BOOL GFL_NET_SystemIsSendCommand_ServerSize(int command)
{
    return GFL_NET_QueueIsCommand(&_pComm->sendQueueMgrServer, command);

}


//==============================================================================
/**
 * @brief   特定のコマンドを送信し終えたかどうかを調べる クライアント側
 * @param   command 調べるコマンド
 * @retval  コマンドが在ったらTRUE
 */
//==============================================================================


BOOL GFL_NET_SystemIsSendCommand(int command)
{
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
 * @brief   戦闘に入る前の敵味方の立ち位置を設定
 * @param   no   立っていた位置の番号に直したもの
 * @param   netID   通信のID
 * @retval  none
 */
//==============================================================================
void GFL_NET_SystemSetStandNo(int no,int netID)
{
    if(_pComm){
        _pComm->standNo[netID] = no;
        NET_PRINT("id = %d  がたってるところは %d\n",netID,no);
    }
}

//==============================================================================
/**
 * @brief   戦闘に入る前の敵味方の立ち位置を得る
 * @param   netID 通信のID
 * @retval  立っていた位置の番号に直したもの  0-3  0,2 vs 1,3
 */
//==============================================================================
int GFL_NET_SystemGetStandNo(int netID)
{
    if(_pComm){
        if(_pComm->standNo[netID] != 0xff){
            NET_PRINT("立ち位置 %d ばん id%d\n",_pComm->standNo[netID], netID);
            return _pComm->standNo[netID];
        }
    }
    return netID;
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

