//=============================================================================
/**
 * @file	net_wh.c
 * @brief	任天堂のwh.cを改造したもの
            ベースは Revision 1.40  2005/06/27 11:10:11  yosizaki
 * @author	Katsumi Ohno
 * @date    2006.12.15
 */
//=============================================================================

#include "gflib.h"
#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/cht.h>

#include "wh.h"
#include "gf_standard.h"
#include "net_wireless.h"

/*
  wh.c : Wireless Manager 関連 Wrapper

  ・ ユーザ向け内部解説

  WM API で実装されているのは、特殊化の程度が低い順に

  - 一般的な MP 通信
  - データシェアリング
  - キーシェアリング

  の3通りの通信様式です。
  これらは各々別のものというわけではなく、例えばキーシェアリングは
  データシェアリングを利用した手法の一つで、データシェアリングは
  単に MP 通信を使ったデータ共有の仕組みのことですが、都合上この
  3つに区別して解説しています。

  「一般的 MP 通信」というのは Multi Poll プロトコルそのままのレベルでの
  通信モードのことを指し、簡単にまとめると

  (1) 親機から全ての子機へデータのブロードキャストを行う
  (2) 各子機から反応を返す
  (3) 親機が通信サイクルの終りをブロードキャストで告知

  というステップを一つのサイクルとして通信を行う、というものです。

  データシェアリングというのは、親機が自分に接続されている子機全てから
  データを吸出し（上で言う(2)での処理）、自分の分も含め一つにまとめて
  共有データとして子機全てに配る（上で言う(1)での処理）、というモードの
  ことを言います。

  キーシェアリングというのは、データシェアリングで共有するデータが
  各機のキーデータの配列になっている、というものです。


  1) 通信処理の概要

  Wireless LAN で実現されている機能について、それらの具体的な作業を
  実際に行っているのは、 ARM7プロセッサです。
  従って ARM9 側からの送受信要求などは全て ARM7 を通す必要があり、その
  ために ARM9 側のほとんどの WM 関係 API は非同期になっていて、およそ

  1. ARM7 への要求を FIFO に書き出す関数を呼ぶ
  2. ARM7 が返した結果を FIFO から受け取ったシステムが（1.で指定した）
  コールバック関数を呼ぶ

  といった流れが基本的な処理となります。

  このサンプルの実装では、この処理に必要な２つの関数を１セットとして
  Wireless LAN 機能の「状態」を表現しているものと考え、その「状態」の
  連鎖を作ることで通信処理を行っています（以後、文脈上特に区別して
  ここで言う Wireless LAN の「状態」を指したい場合、「WH状態」という
  言葉を使うことにします）。

  このソースで該当する関数の名前は各WH状態に対応しており、

  WH_StateInXXXXXX   - WH状態 XXXXXX の開始関数
  WH_StateOutXXXXXX  - WH状態 XXXXXX の終了通知を受け取るコールバック関数

  のようになっています。


  2) WH状態間の遷移

  自動的に次のWH状態に遷移するべきWH状態のコールバック関数では、処理の
  最後で次に移行すべきWH状態の開始関数を呼び出すようにしています
  （基本的に状態遷移の進行はソース中で上から下へと進むようになっています）。

  全てのWH状態名とそれら相互のシーケンス内での遷移関係は、以下のように
  なります。

  - 初期化シーケンス
  （WH_Initialize 関数）
  ↓
  Initialize

  - 親機通信準備シーケンス
  （WH_Connect 関数）
  ↓
  SetParentParam
  ↓
  StartParent
  ↓
  StartParentMP
  ↓
  (**)
  ↓
  StartParentKeyShare

  注： WH_Connect で MP 接続モード（または DS 接続モード）を指定して
  いたら、(**) で終了する。

  - 親機終了シーケンス
  （WH_Finalize 関数）
  ↓
  （EndParentKeyShare）
  ↓
  EndParentMP
  ↓
  EndParent

  - 子機通信準備シーケンス
  （WH_Connect 関数）
  ↓
  StartScan
  ↓
  EndScan
  ↓
  StartChild
  ↓
  StartChildMP
  ↓
  (**)
  ↓
  StartChildKeyShare

  注： WH_Connect で MP 接続モード（または DS 接続モード）を指定して
  いたら、(**) で終了する。

  - 子機終了シーケンス
  （WH_Finalize 関数）
  ↓
  （EndChildKeyShare）
  ↓
  EndChildMP
  ↓
  EndChild

  ここで注意が必要なのは、各WH状態のコールバック関数は、そのWH状態の終了時に
  呼ばれるだけとは限らない、ということです。

  例えば、 WH状態 StartParentMP のコールバックである WH_StateOutStartParentMP
  という関数は、 StartMP の処理が終了した時だけでなく、 MP シーケンスが一通り
  完了した時などにも呼ばれます（関数本体のコメントも参照して下さい）。


  3) 実際の送受信作業

  このソースでは、一般的な MP 通信、データシェアリング、キーシェアリングの
  3通りの通信様式を実装していますが、このうちいずれが行われるかで、データの
  送信・受信の方法が全く違っています。

  一般的な MP 通信の場合、初期化時に以下の関数でコールバックを設定する必要が
  あります。

  WH_SetReceiver(WHReceiverFunc proc);

  データを受信すると、受信のタイミングで受け取った内容がセットした
  コールバック関数に渡されます。
  データの送信は WH_SendData で行います。
  WH_SendData で設定した callback が呼ばれるまでは
  送信データのメモリを上書きしないようにしてください。

  データシェアリングでは、 WH_StepDS 関数を呼んでデータを渡すと同時に
  内部の更新処理を行い、 WH_GetSharedDataAdr でデータを取得します。

  キーシェアリング通信では、 WH_GetKeySet 関数でキーデータを取得します
  （WH_StepDS に相当する処理も内部で行われます。送信すべきキーデータは
  ライブラリが勝手に送信してくれるので、特に何かする必要はありません）。


  4) エラーと復帰について

  この実装での方針は、

  - 上位の関数でのエラー判定は返値で行う。TRUE （成功）、 FALSE （失敗）で
    判断する。
  - より詳細なエラーコードは、 sErrCode に格納しておく。
  - 続行不可能な状態（操作を必要とする状態など）になった時は sSysState が
    WH_SYSSTATE_ERROR に変化するので、それを検知する。

  というものです。
  アプリケーションによっては、エラーの報告にコールバック関数や
  OS_SendMessage を使う必要があると思われますが、この実装では特に対応して
  いません（ソースを流用するなら、 WH_SetError 関数を書き換えるのが良い
  でしょう）。

  また、切断（復帰）処理に関しては「行儀の良い」切断のために、なるべく状態の
  終了処理を行ってから明示的に WM_Disconnect などを呼ぶことが推奨されて
  います。この実装では、その「行儀の良い終了」を行う為に WH_Finalize 関数が
  用意されています。この関数は現在の状態によって（少なくともそれが可能な時は）
  各終了処理への分岐をするようになっています。


  5) ガイドライン準拠について

  ガイドラインに準拠する上で必要なコードには、その旨コメントがついています
  （ガイドラインの規定に対応するための処理はこのファイル以外の場所にもある
  ので注意して下さい。該当個所には `GUIDELINE' の文字を付したコメントが
  添えられていますので、ご参考ください）。
 */

/* ======================================================================
   static contents
   ====================================================================== */


/*  下記の値はコード中で設定されています。
static WMParentParam sParentParam ATTRIBUTE_ALIGN(32) =
{
    // beacon にユーザ定義のデータを載せる場合はここに指定します。
    // 子機の親機選択画面で親機の Nickname を表示させたい場合などは
    // ここに情報をセットして親機から子機に伝えることになります。
    NULL,                         // userGameInfo
    0,                            // userGameInfoLength
    0,                            // padding
    // ggid は、ゲームグループの識別に使用します。
    // ここには、ゲームによって割り当てられた値を使用します。
    0,
    // tgid は、テンポラリグループの識別に使用します。
    // 親機としての活動を開始するたびに違う値である必要があります。
    // このソースでは、 WH_Initialize 関数の中で設定しています。
    0x0001,                       // tgid
    // 子機からの新規接続を受け入れるかどうかを指定します。
    1,                            // entryFlag
    // 接続可能な子機の最大数を指定します。
    WH_CHILD_MAX,                 // maxEntry
    // マルチブートしない場合は 0 にしておきます。
    0,                            // multiBootFlag
    // KS_Flag を 1 に設定すると、接続時に自動的にキーシェアリング用の補正が
    // parentMaxSize と childMaxSize に入るようになります。
    // キーシェアリングを使用する場合でも、きちんと計算して
    // parentMaxSize と childMaxSize を指定する場合には 0 で構いません。
    0,                            // KS_Flag
    // 過去との互換性のために用意されたフィールドで、現在は無視されます。
    0,                            // CS_Flag
    // 親機が発する beacon の間隔です。[ms]
    // 実際の起動時には WM_GetDispersionBeaconPeriod() によって
    // 適度にバラついた値を採用しますが、200[ms] 前後です。
    200,                          // beaconPeriod (millisec)
    // 予約領域です。
    {0},                           // rsv1
    {0},                           // rsv2
    // 親機が子機を待ち受ける無線チャンネルです。
    WH_DEFAULT_CHANNEL ,          // channel
    // 以下のメンバには最大値として適切な値を設定しています。
    // parentMaxSize - 親機が送るデータの最大サイズ
    WH_PARENT_MAX_SIZE,
    // childMaxSize - 子機が送るデータの最大サイズ
    WH_CHILD_MAX_SIZE,
};
*/

static u16 WH_GetConnectNum(void);


#define SSID  "DP"

// 子機最大数（親機を含まない数）
#define WH_CHILD_MAX              7

// シェア出来るデータの最大サイズ
#define WH_DS_DATA_SIZE           12


// エラー処理を厳密にしたプログラム
#define ERROR_CHECK (1)


// 1回の通信で送れるデータの最大サイズ
// データシェアリングに加えて通常の通信をする場合は、その分だけ
// ここの値を増やしてください。その際は、複数パケット送信による追加の
// ヘッダフッタ分を加算する必要があります。
// 詳しくは docs/TechnicalNotes/WirelessManager.doc を参照してください。
// GUIDELINE : ガイドライン準拠ポイント(6.3.2)
// リファレンスのワイヤレスマネージャ(WM)→図表・情報→無線通信時間計算シート
// で計算した MP 通信1回分の所要時間が 5600 μ秒以下となることを推奨しています。



struct _WM_INFO_STRUCT {
    WMParentParam sParentParam;
    /* WM 用システムワーク領域バッファ */
    u8 sWmBuffer[WM_SYSTEM_BUF_SIZE];
/* 親機検索用 */
    WMBssDesc sBssDesc;
    WMScanParam sScanParam;
    WHStartScanCallbackFunc sScanCallback;

// 内部使用する送受信バッファとそのサイズ
    u8* sSendBuffer;   //送信バッファ 32ALIGN
    u8* sRecvBuffer;   //受信バッファ 32ALIGN
    u8* pSendBufferBK;  //送信バッファalloc
    u8* pRecvBufferBK;  //受信バッファalloc
    s32 sSendBufferSize;
    s32 sRecvBufferSize;
    s32 sParentSendBufferSize;
    s32 sParentRecvBufferSize;
    s32 sChildSendBufferSize;
    s32 sChildRecvBufferSize;
    s32 parentMPSize;
    s32 childMPSize;

    u16 sChannelIndex;
    u16 sAutoConnectFlag;

// wh の状態（WH状態とは別モノです。外からは参照のみ）
    int sSysState;

// 接続モード（外から設定し、このソース内では触りません）
    int sConnectMode;

// MP 通信モードでのデータ受信ユーザ関数
    WHReceiverFunc sReceiverFunc;

// 接続許可判定用ユーザ関数
    WHJudgeAcceptFunc sJudgeAcceptFunc;

    
    WHdisconnectCallBack disconnectCallBack;

    /// 子機接続時のコールバック
    WHdisconnectCallBack connectCallBack;
    /// 接続時の検査コールバック
    WHConnectCheckCallBack connectCheckCallBack;
    /// エラー発生時のコールバック
    WHErrorCallBack errorCallBack;
    
    /// 自分の aid が入ります（子機は切断・再接続時に変化する可能性あり）
    u16 sMyAid;
    /// 接続状態を示す bitmap が格納されます
    u16 sConnectBitmap;
    /// エラーコード格納用
    int sErrCode;
    /// 接続人数
    u8 maxEntry;
    /// 子機として接続したらすぐきる
    u8 bDisconnectChild;
    /// 乱数用
    u32 sRand;
    /* 通信利用率測定用 (WH_StartMeasureChannel, WH_GetMeasureChannel で使用) */
    u16 sChannel;
    u16 sChannelBusyRatio;
    u16 sChannelBitmap;
    u8 bPauseConnect;
    u8 bPauseConnectSystem;
    u8 bSetEntry;
    u8 stateBeaconSentNum;
} ;


//static _WM_INFO_STRUCT* pNetWH;







/* デバッグ出力フック関数 */
static void (*wh_trace) (const char *, ...) =
#if !defined(SDK_FINALROM)
    OS_TPrintf;
#else
    NULL;
#endif


/* ======================================================================
   state functions
   ====================================================================== */

// 乱数マクロ
#define RAND()  ( pNetWH->sRand = pNetWH->sRand * 69069UL + 12345 )
#define RAND_INIT(x) ( pNetWH->sRand = (u32)(x) )

#define WH_MAX(a, b) ( ((a) > (b)) ? (a) : (b) )

/* 不定期な通知を受け取る関数 */
static void WH_IndicateHandler(void *arg);

/* (stateless) -> READY -> STOP -> IDLE */
static BOOL WH_StateInInitialize(void);

#ifndef WH_USE_DETAILED_INITIALIZE
static void WH_StateOutInitialize(void *arg);
#else
static BOOL WH_StateInEnable(void);
static void WH_StateOutEnable(void *arg);
static BOOL WH_StateInPowerOn(void);
static void WH_StateOutPowerOn(void *arg);
#endif

/* IDLE -> MEASURECHANNEL -> IDLE */
static u16 WH_StateInMeasureChannel(u16 channel);
static void WH_StateOutMeasureChannel(void *arg);
static WMErrCode WHi_MeasureChannel(WMCallbackFunc func, u16 channel);
static s16 SelectChannel(u16 bitmap);


/* IDLE -> PARENT -> MP-PARENT */
static BOOL WH_StateInSetParentParam(void);
static void WH_StateOutSetParentParam(void *arg);
static BOOL WH_StateInSetParentWEPKey(void);
static void WH_StateOutSetParentWEPKey(void *arg);
static BOOL WH_StateInStartParent(void);
static void WH_StateOutStartParent(void *arg);
static BOOL WH_StateInStartParentMP(void);
static void WH_StateOutStartParentMP(void *arg);

/* IDLE -> SCAN */
static BOOL WH_StateInStartScan(void);
static void WH_StateOutStartScan(void *arg);

/* SCAN -> IDLE */
static BOOL WH_StateInEndScan(void);
static void WH_StateOutEndScan(void *arg);

/* IDLE -> CHILD -> MP-CHILD */
static BOOL WH_StateInSetChildWEPKey(void);
static void WH_StateOutSetChildWEPKey(void *arg);
static BOOL WH_StateInStartChild(void);
static void WH_StateOutStartChild(void *arg);
static BOOL WH_StateInStartChildMP(void);
static void WH_StateOutStartChildMP(void *arg);

/* MP + key sharing */
#if 0
static BOOL WH_StateInStartParentKeyShare(void);
static BOOL WH_StateInEndParentKeyShare(void);
static BOOL WH_StateInStartChildKeyShare(void);
static BOOL WH_StateInEndChildKeyShare(void);
#endif

/* MP + SetMPData */
static BOOL WH_StateInSetMPData(void *data, u16 datasize, int port, WHSendCallbackFunc callback);
static void WH_StateOutSetMPData(void *arg);
static void WH_PortReceiveCallback(void *arg);

/* MP-PARENT -> IDLE */
static BOOL WH_StateInEndParentMP(void);
static void WH_StateOutEndParentMP(void *arg);
static BOOL WH_StateInEndParent(void);
static void WH_StateOutEndParent(void *arg);
static BOOL WH_StateInDisconnectChildren(u16 bitmap);
static void WH_StateOutDisconnectChildren(void *arg);

/* MP-CHILD -> IDLE */
static BOOL WH_StateInEndChildMP(void);
static void WH_StateOutEndChildMP(void *arg);
static BOOL WH_StateInEndChild(void);
static void WH_StateOutEndChild(void *arg);

/* IDLE -> STOP -> READY -> (stateless) */
static BOOL WH_StateInPowerOff(void);
static void WH_StateOutPowerOff(void *arg);
static BOOL WH_StateInDisable(void);
static void WH_StateOutDisable(void *arg);
static void WH_StateOutEnd(void *arg);

/* X -> IDLE */
static BOOL WH_StateInReset(void);
static void WH_StateOutReset(void *arg);


/* ======================================================================
   debug codes
   ====================================================================== */
#if GFL_NET_DEBUG
#define WMHIGH_DEBUG   //wh関連のデバッグ出力
#endif


#ifdef PM_DEBUG
#define _DEBUG_LIFETIME  (0)    // デバッグ時にライフタイムを無効にしたい時に使用
#else
#define _DEBUG_LIFETIME  (0)    // 本番では０
#endif



#if defined(WMHIGH_DEBUG)

#define WH_TRACE    if(wh_trace) wh_trace

#define WH_TRACE_STATE(state) OS_TPrintf("%s sSysState = %d\n", __func__, state)

#define WH_REPORT_FAILURE(result)                \
    do{ OS_TPrintf("Failed in %s, %s = %s\n",      \
                  __func__,                      \
                  #result,                       \
                  WH_GetWMErrCodeName(result));  \
        WH_SetError(result); }while(0)

#define WH_ASSERT(exp) \
    (void) ((exp) || (OSi_Panic(__FILE__, __LINE__, "Failed assertion " #exp), 0))

#else  /* defined(WMHIGH_DEBUG) */

#define WH_TRACE_STATE(state)     ((void)0)             /* */
#define WH_TRACE(...)               ((void)0)
#define WH_REPORT_FAILURE(result)   WH_SetError(result)
#define WH_ASSERT(exp)              ((void) 0)

#endif



#if defined(WMHIGH_DEBUG)
static const char *sStateNames[] = {
    "WM_STATE_READY",
    "WM_STATE_STOP",
    "WM_STATE_IDLE",
    "WM_STATE_CLASS1",
    "WM_STATE_TESTMODE",
    "WM_STATE_SCAN",
    "WM_STATE_CONNECT",
    "WM_STATE_PARENT",
    "WM_STATE_CHILD",
    "WM_STATE_MP_PARENT",
    "WM_STATE_MP_CHILD",
    "WM_STATE_DCF_CHILD",
};

static const char *WH_GetWMErrCodeName(int result)
{
    static const char *errnames[] = {
        "WM_ERRCODE_SUCCESS",
        "WM_ERRCODE_FAILED",
        "WM_ERRCODE_OPERATING",
        "WM_ERRCODE_ILLEGAL_STATE",
        "WM_ERRCODE_WM_DISABLE",
        "WM_ERRCODE_NO_DATASET",
        "WM_ERRCODE_INVALID_PARAM",
        "WM_ERRCODE_NO_CHILD",
        "WM_ERRCODE_FIFO_ERROR",
        "WM_ERRCODE_TIMEOUT",
        "WM_ERRCODE_SEND_QUEUE_FULL",
        "WM_ERRCODE_NO_ENTRY",
        "WM_ERRCODE_OVER_MAX_ENTRY",
        "WM_ERRCODE_INVALID_POLLBITMAP",
        "WM_ERRCODE_NO_DATA",
        "WM_ERRCODE_SEND_FAILED",
        "WM_ERRCODE_DCF_TEST",
        "WM_ERRCODE_WL_INVALID_PARAM",
        "WM_ERRCODE_WL_LENGTH_ERR",
        "WM_ERRCODE_FLASH_ERROR",

        "WH_ERRCODE_DISCONNECTED",
        "WH_ERRCODE_PARENT_NOT_FOUND",
        "WH_ERRCODE_NO_RADIO",
    };

    if (0 <= result && result < sizeof(errnames) / sizeof(char *))
    {
        return errnames[result];
    }
    else
    {
        return "N/A";
    }
}

static const char *WH_GetWMStateCodeName(u16 code)
{
    static const char *statenames[] = {
        "WM_STATECODE_PARENT_START",
        "N/A",
        "WM_STATECODE_BEACON_SENT",
        "WM_STATECODE_SCAN_START",
        "WM_STATECODE_PARENT_NOT_FOUND",
        "WM_STATECODE_PARENT_FOUND",
        "WM_STATECODE_CONNECT_START",
        "WM_STATECODE_CONNECTED",
        "WM_STATECODE_BEACON_LOST",
        "WM_STATECODE_DISCONNECTED",
        "WM_STATECODE_MP_START",
        "WM_STATECODE_MPEND_IND",
        "WM_STATECODE_MP_IND",
        "WM_STATECODE_MPACK_IND",
        "WM_STATECODE_DCF_START",
        "WM_STATECODE_DCF_IND",
        "WM_STATECODE_BEACON_RECV",
        "WM_STATECODE_DISASSOCIATE",
        "WM_STATECODE_REASSOCIATE",
        "WM_STATECODE_AUTHENTICATE",
        "WM_STATECODE_PORT_SEND",
        "WM_STATECODE_PORT_RECV",
        "WM_STATECODE_FIFO_ERROR",
        "WM_STATECODE_INFORMATION",
        "WM_STATECODE_UNKNOWN",
        "WM_STATECODE_PORT_INIT",
        "WM_STATECODE_DISCONNECTED_FROM_MYSELF",
    };

    if (0 <= code && code < sizeof(statenames) / sizeof(char *))
    {
        return statenames[code];
    }
    else
    {
        return "N/A";
    }
}

static const char *WH_GetWMApiidName(u16 apiid)
{
    static const char *apinames[] = {
        "WM_APIID_INITIALIZE",
        "WM_APIID_RESET",
        "WM_APIID_END",

        "WM_APIID_ENABLE",
        "WM_APIID_DISABLE",
        "WM_APIID_POWER_ON",
        "WM_APIID_POWER_OFF",

        "WM_APIID_SET_P_PARAM",
        "WM_APIID_START_PARENT",
        "WM_APIID_END_PARENT",
        "WM_APIID_START_SCAN",
        "WM_APIID_END_SCAN",
        "WM_APIID_START_CONNECT",
        "WM_APIID_DISCONNECT",
        "WM_APIID_START_MP",
        "WM_APIID_SET_MP_DATA",
        "WM_APIID_END_MP",
        "WM_APIID_START_DCF",
        "WM_APIID_SET_DCF_DATA",
        "WM_APIID_END_DCF",
        "WM_APIID_SET_WEPKEY",
        "WM_APIID_START_KS",
        "WM_APIID_END_KS",
        "WM_APIID_GET_KEYSET",
        "WM_APIID_SET_GAMEINFO",
        "WM_APIID_SET_BEACON_IND",
        "WM_APIID_START_TESTMODE",
        "WM_APIID_STOP_TESTMODE",
        "WM_APIID_VALARM_MP",
        "WM_APIID_SET_LIFETIME",
        "WM_APIID_MEASURE_CHANNEL",
        "WM_APIID_INIT_W_COUNTER",
        "WM_APIID_GET_W_COUNTER",
        "WM_APIID_SET_ENTRY",
        "WM_APIID_AUTO_DEAUTH",
        "WM_APIID_SET_MP_PARAMETER",
        "WM_APIID_SET_BEACON_PERIOD",
        "WM_APIID_AUTO_DISCONNECT",

        "WM_APIID_KICK_MP_PARENT",
        "WM_APIID_KICK_MP_CHILD",
        "WM_APIID_KICK_MP_RESUME",
    };
    static const char *apinames_indicates[] = {
        "WM_APIID_INDICATION",
        "WM_APIID_PORT_SEND",
        "WM_APIID_PORT_RECV",
        "WM_APIID_READ_STATUS"
    };
    static const char *apiname_unknown = "WM_APIID_UNKNOWN";

    if (0 <= apiid && apiid < sizeof(apinames) / sizeof(char *))
    {
        return apinames[apiid];
    }
    else if (WM_APIID_INDICATION <= apiid &&
             apiid < WM_APIID_INDICATION + sizeof(apinames_indicates) / sizeof(char *))
    {
        return apinames_indicates[apiid - WM_APIID_INDICATION];
    }
    else if (apiid == WM_APIID_UNKNOWN)
    {
        return apiname_unknown;
    }
    else
    {
        return "N/A";
    }
}

static void WH_OutputWMState(void)
{
    WMStatus s;

    if (WM_ReadStatus(&s) == WM_ERRCODE_SUCCESS)
    {
        WH_TRACE("state = %s\n", WH_GetWMStateCodeName(s.state));
        WH_TRACE("apiid = %s\n", WH_GetWMApiidName(s.BusyApiid));
    }
    else
    {
        WH_TRACE("WM not Initialized\n");
    }
}

static void WH_OutputBitmap(u16 bitmap)
{
    u16 i;
    for (i = 15; i >= 0; --i)
    {
        if ((bitmap >> i) & 0x01)
        {
            OS_TPrintf("o");
        }
        else
        {
            OS_TPrintf("-");
        }
    }

    OS_TPrintf("\n");
}

static void WH_ChangeSysState(int state)
{
    static const char *statenames[] = {
        "WH_SYSSTATE_STOP",
        "WH_SYSSTATE_IDLE",
        "WH_SYSSTATE_SCANNING",
        "WH_SYSSTATE_BUSY",
        "WH_SYSSTATE_CONNECTED",
        "WH_SYSSTATE_DATASHARING",
        "WH_SYSSTATE_KEYSHARING",
        "WH_SYSSTATE_MEASURECHANNEL",
        "WH_SYSSTATE_CONNECT_FAIL",
        "WH_SYSSTATE_ERROR",
    };
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    WH_TRACE("%s -> ", statenames[pNetWH->sSysState]);
    pNetWH->sSysState = state;
    WH_TRACE("%s\n", statenames[pNetWH->sSysState]);
}

#else

#define WH_GetWMErrCodeName(result)    ("")
#define WH_GetWMStateCodeName(result)  ("")
#define WH_GetWMApiidName(result)      ("")

static void WH_OutputWMState(void)
{;
}
static void WH_OutputBitmap(void)
{;
}

static void WH_ChangeSysState(int state)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sSysState = state;
}

#endif

static void WH_SetError(int code)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    // 既にシステムが ERROR 状態になっている場合は、上書きしない。
    if (pNetWH->sSysState == WH_SYSSTATE_ERROR || pNetWH->sSysState == WH_SYSSTATE_FATAL)
    {
        return;
    }
#ifdef DEBUG_ONLY_FOR_ohno
    OS_TPrintf("sErrCode 設定 %d\n",code);
#endif
    pNetWH->sErrCode = code;
}



/* ======================================================================
   state functions
   ====================================================================== */

/* ----------------------------------------------------------------------
   state : setparentparam
  ---------------------------------------------------------------------- */
static BOOL WH_StateInSetParentParam(void)
{
    // この状態では、親機の持っているゲーム情報を ARM7 に渡しています。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    WH_TRACE_STATE(pNetWH->sSysState);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WM_SetParentParameter(WH_StateOutSetParentParam, &pNetWH->sParentParam);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutSetParentParam(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }
#if 0
    if (pNetWH->sParentWEPKeyGenerator != NULL)
    {
        // WEP Key Generator が設定されていれば、WEP Key の設定へ
        if (!WH_StateInSetParentWEPKey())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
    else
#endif
    {
        // 正常に進行していれば次は StartParent 状態へ。
        if (!WH_StateInStartParent())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
}
#if 0
static BOOL WH_StateInSetParentWEPKey(void)
{
    u16 wepmode;
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    wepmode = (*pNetWH->sParentWEPKeyGenerator)(pNetWH->sWEPKey, &pNetWH->sParentParam);
    result = WM_SetWEPKey(WH_StateOutSetParentWEPKey, wepmode, pNetWH->sWEPKey);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutSetParentWEPKey(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    // 正常に進行していれば次は StartParent 状態へ。
    if (!WH_StateInStartParent())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}

#endif
/* ----------------------------------------------------------------------
   state : StartParent
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartParent(void)
{
    // この状態では StartParent 関数を呼び、親機としての設定を開始します。

    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if ( (pNetWH->sSysState == WH_SYSSTATE_CONNECTED)
         || (pNetWH->sSysState == WH_SYSSTATE_KEYSHARING) 
         || (pNetWH->sSysState == WH_SYSSTATE_DATASHARING) )
    {
        // 以上の場合には既に親としての設定は済んでいるはず。
        return TRUE;
    }
    {  // ユニオンルーム話しかけ対策
        WMStatus* status = (WMStatus*)WMi_GetStatusAddress();
        DC_InvalidateRange(&status->wep_flag, sizeof(status->wep_flag));
        status->wep_flag = FALSE;
        DC_FlushRange(&status->wep_flag, sizeof(status->wep_flag));
    }
    result = WM_StartParent(WH_StateOutStartParent);

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    pNetWH->sMyAid = 0;
    pNetWH->sConnectBitmap = WH_BITMAP_EMPTY;

    return TRUE;
}

static void WH_StateOutStartParent(void *arg)
{
    // StartParent で設定したコールバック関数は、
    // 1) ビーコンが送信された
    // 2) 新しく子機が接続した
    // 3) StartParent が終了した
    // 4) 子機の切断を検知
    // と、何通りかのケースで呼ばれるので、区別する必要があります。

    WMStartParentCallback *cb = (WMStartParentCallback *)arg;
    const u16 target_bitmap = (u16)(1 << cb->aid);
    BOOL bConnect=TRUE;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    switch (cb->state)
    {
        //-----------------------------------
        // ビーコン送信通知
    case WM_STATECODE_BEACON_SENT:
        //NET_PRINT("ビーコン送信\n");
        pNetWH->stateBeaconSentNum++;

        break;

        //-----------------------------------
        // 子機の接続を通知
    case WM_STATECODE_CONNECTED:
        {
            // cb->macAddress に接続してきた子機の MAC アドレスが入っています。
            // cb->ssid は子機が WM_StartConnect の ssid 引数にセットしたデータです。
            WH_TRACE("StartParent - new child (aid %x) connected\n", cb->aid);

//            OS_TPrintf("ssid my %d  child %d\n",CommStateGetServiceNo(),cb->ssid[0]);

#if 1 // 改良 比較関数を渡す必要がある @@OO
            if(pNetWH->connectCheckCallBack){
                bConnect = pNetWH->connectCheckCallBack(cb->aid,&cb->ssid[0]);
            }
            if((pNetWH->bPauseConnectSystem == TRUE  ) ||
               (pNetWH->bPauseConnect == TRUE) ||
               (!bConnect) ){
#endif
#if 0
            if((pNetWH->bPauseConnectSystem == TRUE  ) ||
                (pNetWH->bPauseConnect == TRUE) ||
                (WH_GetConnectNum() >= pNetWH->maxEntry) ||
               (cb->ssid[0] != CommStateGetServiceNo()) ||
               (0 != GFL_STD_MemComp(SSID,&cb->ssid[1],sizeof(SSID)))){
#endif
                
                WMErrCode result;
                // 接続を切断します。
                OS_TPrintf("切断 %d %d \n",pNetWH->maxEntry,WH_GetConnectNum());
                
                result = WM_Disconnect(NULL, cb->aid);
                if (result != WM_ERRCODE_OPERATING)
                {
                    WH_REPORT_FAILURE(result);
                    WH_ChangeSysState(WH_SYSSTATE_ERROR);
                }
                break;
            }
/*
            // 接続してきた子機が接続許可条件を満たしているかどうかをチェック
            if (pNetWH->sJudgeAcceptFunc != NULL)
            {
                if (!pNetWH->sJudgeAcceptFunc(cb))
                {
                    WMErrCode result;
                    // 接続を切断します。
                    result = WM_Disconnect(NULL, cb->aid);
                    if (result != WM_ERRCODE_OPERATING)
                    {
                        WH_REPORT_FAILURE(result);
                        WH_ChangeSysState(WH_SYSSTATE_ERROR);
                    }
                    break;
                }
            }
   */
            pNetWH->sConnectBitmap |= target_bitmap;

            // 子機接続時のコールバック
            if(pNetWH->connectCallBack){
                pNetWH->connectCallBack(cb->aid);
            }

        }
        break;

        //-----------------------------------
        // 子機の切断を通知
    case WM_STATECODE_DISCONNECTED:
        {
            WH_TRACE("StartParent - child (aid %x) disconnected\n", cb->aid);
            OS_TPrintf("disconnect %d\n",cb->aid);
            // cb->macAddress には, 切断された子機の MAC アドレスが入っています。
            pNetWH->sConnectBitmap &= ~target_bitmap;
            
            if(pNetWH->disconnectCallBack){
            pNetWH->disconnectCallBack(cb->aid);
            }
            
        }
        break;

        //-----------------------------------
        // 自ら子機を切断した
    case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        {
            WH_TRACE("StartParent - child (aid 0x%x) disconnected from myself\n", cb->aid);
            // 自ら切断した場合は処理を行いません
            // cb->macAddress には, 切断された子機の MAC アドレスが入っています。
        }
        break;

        //-----------------------------------
        // StartParentの処理が終了
    case WM_STATECODE_PARENT_START:
        {
            // MP 通信状態に移行します。
            if (!WH_StateInStartParentMP())
            {
                WH_ChangeSysState(WH_SYSSTATE_ERROR);
            }
        }
        break;

        //-----------------------------------
    default:
        WH_TRACE("unknown indicate, state = %d\n", cb->state);
    }
}

/* ----------------------------------------------------------------------
   state : StartParentMP
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartParentMP(void)
{
    // WM_Start 関数を呼び、 MP 通信プロトコルによる接続を開始します。

    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if ((pNetWH->sSysState == WH_SYSSTATE_CONNECTED)
        || (pNetWH->sSysState == WH_SYSSTATE_KEYSHARING) || (pNetWH->sSysState == WH_SYSSTATE_DATASHARING))
    {
        return TRUE;
    }

    WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
    result = WM_StartMPEx(WH_StateOutStartParentMP,
                          (u16 *)pNetWH->sRecvBuffer,
                          (u16)pNetWH->sRecvBufferSize,
                          (u16 *)pNetWH->sSendBuffer,
                          (u16)pNetWH->sSendBufferSize,
                          1,
                          0,//        u16             defaultRetryCount ,
                          FALSE,//        BOOL            minPollBmpMode ,
                          FALSE,//        BOOL            singlePacketMode ,
                          FALSE,//        BOOL            fixFreqMode ,
                          FALSE//        BOOL            ignoreFatalError
                          );
/*
    result = WM_StartMP(WH_StateOutStartParentMP,
                          (u16 *)pNetWH->sRecvBuffer,
                          (u16)pNetWH->sRecvBufferSize,
                          (u16 *)pNetWH->sSendBuffer,
                          (u16)pNetWH->sSendBufferSize,
                          1
                          );
   */
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutStartParentMP(void *arg)
{
    // StartMP で指定したコールバック関数は、
    // 1) StartMP によるモード開始時
    // 2) MP シーケンス完了（親機のみ）
    // 3) MP 受信（子機のみ）
    // 4) MP シーケンス完了通知(ACK受信)を検出（子機のみ）
    // の４通りのケースで呼ばれるため、区別する必要があります。

    WMstartMPCallback *cb = (WMstartMPCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    // WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_MP_START:
        // StartMP 正常終了の通知。
        // これ以降、送受信可能になります。

        GF_ASSERT(pNetWH->sConnectMode != WH_CONNECTMODE_KS_PARENT);
        GF_ASSERT(pNetWH->sConnectMode != WH_CONNECTMODE_DS_PARENT);

        WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
        break;

    case WM_STATECODE_MPEND_IND:
        // 親機の受信完了通知。

        // cb->recvBuf で子機から受信したデータフレームの内容を受け取れますが、
        // 通常のデータ受信は WM_SetPortCallback でコールバックを設定してください。
        // また、データシェアリング・キーシェアリングを使う場合は、
        // 受信処理は内部的に行われますので、WM_SetPortCallback を使う必要もありません。
        break;

    case WM_STATECODE_MP_IND:
    case WM_STATECODE_MPACK_IND:
        // 親機ならここへは来ないはずです。

    default:
        WH_TRACE("unknown indicate, state = %d\n", cb->state);
        break;
    }
}

/* ----------------------------------------------------------------------
   state : EndParentMP
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndParentMP(void)
{
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    // これ以降、送受信不可能になります。
    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WM_EndMP(WH_StateOutEndParentMP);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndParentMP(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_Reset();
        return;
    }

    // 自動的に、終了処理を開始します。
    if (!WH_StateInEndParent())
    {
        WH_TRACE("WH_StateInEndParent failed\n");
        WH_Reset();
        return;
    }
}

/* ----------------------------------------------------------------------
   state : EndParent
   ---------------------------------------------------------------------- */
static BOOL WH_StateInEndParent(void)
{
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    // ここで、親機としての動作を終了します。
    // 接続中の子機がいる場合は、個別に認証を切断した後
    // 親機としての活動が停止されます。
    result = WM_EndParent(WH_StateOutEndParent);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndParent(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    // ここで、親機としての切断処理は完了し アイドリング（待機中）状態に戻ります。
    WH_ChangeSysState(WH_SYSSTATE_IDLE);
}

/* ----------------------------------------------------------------------
   Name:        WH_ChildConnectAuto
   Description: 子機接続シーケンスを開始します。
                ただし、WH_ParentConnect や WH_ChildConnect で指定する
                各種設定を内部の自動処理に任せます。
   Arguments:   mode    - WH_CONNECTMODE_MP_CHILD ならば子機としてMP開始。
                          WH_CONNECTMODE_DS_CHILD ならば子機としてDataSharing開始。
                          WH_CONNECTMODE_KS_CHILD ならば子機としてKeySharing開始。

                macAddr - 接続する親機のMACアドレスを指定
                          0xFFFFFFならばすべての親機を検索する。
                          
                channel - 親を検索するチャンネルを指定
                          0ならばすべてのチャンネルを検索する。
                          
   ---------------------------------------------------------------------- */
BOOL WH_ChildConnectAuto(int mode, const u8 *macAddr, u16 channel,WHStartScanCallbackFunc sScanCallback)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    // WM_StartMP() 用の送受信バッファサイズ計算
    // 事前に静的にバッファを確保したい場合は WM_SIZE_MP_* 関数マクロを、
    // 動的に確保して構わない場合は、親子接続後で WM_StartMP() を呼び出す直前に
    // WM_GetReceiveBufferSize() API を用います。
    // 同様に事前に静的にバッファを確保したい場合は WM_SIZE_MP_* 関数マクロを、
    // 動的に確保して構わない場合は、親子接続後で WM_StartMP() を呼び出す直前に
    // WM_GetSendBufferSize() API を用います。
    pNetWH->sRecvBufferSize = pNetWH->sChildRecvBufferSize;//WH_CHILD_RECV_BUFFER_SIZE;
    pNetWH->sSendBufferSize = pNetWH->sChildSendBufferSize;//WH_CHILD_SEND_BUFFER_SIZE;

    WH_TRACE("recv buffer size = %d\n", pNetWH->sRecvBufferSize);
    WH_TRACE("send buffer size = %d\n", pNetWH->sSendBufferSize);

    WH_ChangeSysState(WH_SYSSTATE_SCANNING);

    // 子機モードで検索開始。
    pNetWH->sBssDesc.channel = 1;
    *(u16 *)(&pNetWH->sScanParam.bssid[4]) = *(u16 *)(macAddr + 4);
    *(u16 *)(&pNetWH->sScanParam.bssid[2]) = *(u16 *)(macAddr + 2);
    *(u16 *)(&pNetWH->sScanParam.bssid[0]) = *(u16 *)(macAddr + 0);

    pNetWH->sConnectMode = mode;


    pNetWH->sScanCallback = sScanCallback;
    pNetWH->sChannelIndex = channel;
    pNetWH->sScanParam.channel = 0;
    pNetWH->sAutoConnectFlag = TRUE;

//    pNetWH->sParentWEPKeyGenerator = NULL;
//    pNetWH->sChildWEPKeyGenerator = NULL;

    
    if (!WH_StateInStartScan())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_StartScan

  Description:  親機のビーコンを取得する関数

  Arguments:    callback - 親機発見時に返すコールバックを設定する。
                
                macAddr  - 接続する親機のMACアドレスを指定
                           0xFFFFFFならばすべての親機を検索する。
                           
                channel  - 親を検索するチャンネルを指定
                           0ならばすべてのチャンネルを検索する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL WH_StartScan(WHStartScanCallbackFunc callback, const u8 *macAddr, u16 channel)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);
    WH_ASSERT(pNetWH->sSysState != WH_SYSSTATE_CONNECTED);

    WH_ChangeSysState(WH_SYSSTATE_SCANNING);

    pNetWH->sScanCallback = callback;
    pNetWH->sChannelIndex = channel;
    pNetWH->sScanParam.channel = 0;
    pNetWH->sAutoConnectFlag = FALSE;          // 自動接続はしない

    // 検索するMACアドレスの条件を設定
    *(u16 *)(&pNetWH->sScanParam.bssid[4]) = *(u16 *)(macAddr + 4);
    *(u16 *)(&pNetWH->sScanParam.bssid[2]) = *(u16 *)(macAddr + 2);
    *(u16 *)(&pNetWH->sScanParam.bssid[0]) = *(u16 *)(macAddr);

    if (!WH_StateInStartScan())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
  state : StartScan
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartScan(void)
{
    // この状態の時、親機を探索します。
    WMErrCode result;
    u16 chanpat;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    WH_ASSERT(pNetWH->sSysState == WH_SYSSTATE_SCANNING);

    chanpat = WM_GetAllowedChannel();

    // 無線が使用可能かどうかをチェックする
    if (chanpat == 0x8000)
    {
        // 0x8000 が返ってきた場合は、無線が初期化されていないなど
        // 無線ライブラリの状態異常を表しているのでエラーにします。
        WH_REPORT_FAILURE(WM_ERRCODE_ILLEGAL_STATE);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return FALSE;
    }
    if (chanpat == 0)
    {
        // 無線が使えない状態。
        WH_REPORT_FAILURE(WH_ERRCODE_NO_RADIO);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return FALSE;
    }

    if (pNetWH->sChannelIndex == 0)
    {
        /* 現在の指定から昇順に、可能なチャンネルを検索します */
        while (TRUE)
        {
            pNetWH->sScanParam.channel++;
            if (pNetWH->sScanParam.channel > 16)
            {
                pNetWH->sScanParam.channel = 1;
            }

            if (chanpat & (0x0001 << (pNetWH->sScanParam.channel - 1)))
            {
                break;
            }
        }
    }
    else
    {
        pNetWH->sScanParam.channel = (u16)pNetWH->sChannelIndex;
    }
    pNetWH->sScanParam.maxChannelTime = WM_GetDispersionScanPeriod() / 3;
    pNetWH->sScanParam.scanBuf = &pNetWH->sBssDesc;

//    NET_PRINT("WM_StartScan %d %d\n",pNetWH->sScanParam.channel,pNetWH->sScanParam.maxChannelTime);
    result = WM_StartScan(WH_StateOutStartScan, &pNetWH->sScanParam);

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}


static void WH_StateOutStartScan(void *arg)
{
    WMstartScanCallback *cb = (WMstartScanCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    // スキャンコマンドに失敗した場合
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    if (pNetWH->sSysState != WH_SYSSTATE_SCANNING)
    {

        pNetWH->sAutoConnectFlag = FALSE; // 自動接続はキャンセル

        // 状態が変更されていればスキャン終了
        if (!WH_StateInEndScan())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_SCAN_START:
        return;

    case WM_STATECODE_PARENT_NOT_FOUND:
        break;

    case WM_STATECODE_PARENT_FOUND:
        // 親機が見つかった場合
        // GUIDELINE : ガイドライン準拠ポイント(6.3.5)
        // ggid を比較し、違っていたら失敗とします。
        // まず、WMBssDesc.gameInfoLength を確認し、
        // ggid に有効な値が入っていることから調べる必要があります。

        WH_TRACE("WH_StateOutStartScan : MAC=%02x%02x%02x%02x%02x%02x ",
                 cb->macAddress[0],
                 cb->macAddress[1],
                 cb->macAddress[2], cb->macAddress[3], cb->macAddress[4], cb->macAddress[5]);

        // BssDescの情報がARM7側から書き込まれているため
        // バッファに設定されたBssDescのキャッシュを破棄
        DC_InvalidateRange(&pNetWH->sBssDesc, sizeof(WMbssDesc));

#if 0
        // GGIDコールバックが必要ならば呼び出し
        if (pNetWH->sGGIDScanCallback != NULL && cb->gameInfoLength >= 8) {
            _GF_BSS_DATA_INFO* pGF = (_GF_BSS_DATA_INFO*)cb->gameInfo.userGameInfo;
            pNetWH->sGGIDScanCallback(cb->gameInfo.ggid, pGF->serviceNo);
        }
#endif
        if (cb->gameInfoLength < 8 || cb->gameInfo.ggid != pNetWH->sParentParam.ggid)
        {
            // GGIDが違っていれば無視する
            WH_TRACE("not my parent ggid \n");
            break;
        }

        // エントリーフラグが立っていなければ子機を受付中でないので無視する
        // またマルチブートフラグが立っている場合は、DSダウンロード親機であるので無視する。
        if ( ( cb->gameInfo.gameNameCount_attribute & (WM_ATTR_FLAG_ENTRY | WM_ATTR_FLAG_MB) )
             != WM_ATTR_FLAG_ENTRY )
        {
            WH_TRACE("not recieve entry\n");
            break;
        }

        WH_TRACE("parent find\n");

        // コールバックが必要ならば呼び出し
        if (pNetWH->sScanCallback != NULL)
        {
            pNetWH->sScanCallback(&pNetWH->sBssDesc);
        }

        // 見つかった親機に自動接続のためスキャン終了
        if (pNetWH->sAutoConnectFlag)
        {
            if (!WH_StateInEndScan())
            {
                WH_ChangeSysState(WH_SYSSTATE_ERROR);
            }
            return;
        }
        break;
    }

    // チャンネルを変更して再スキャンを開始します。
    if (!WH_StateInStartScan())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}

/* ----------------------------------------------------------------------
   state : EndScan
  ---------------------------------------------------------------------- */

/*---------------------------------------------------------------------------*
  Name:         WH_EndScan

  Description:  親機のビーコンを取得する関数

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL WH_EndScan(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if (pNetWH->sSysState != WH_SYSSTATE_SCANNING)
    {
        return FALSE;
    }
    pNetWH->sAutoConnectFlag = FALSE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    return TRUE;
}


static BOOL WH_StateInEndScan(void)
{
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    // この状態では、スキャンの終了処理を行います。
    result = WM_EndScan(WH_StateOutEndScan);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndScan(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    WH_ChangeSysState(WH_SYSSTATE_IDLE);

    if (!pNetWH->sAutoConnectFlag)
    {
        return;
    }
    {
        // スキャン終了処理が終了したので、そのまま子機としての活動を
        // 開始します。
        if (!WH_StateInStartChild())
        {
            WH_TRACE("WH_StateOutEndScan : startchild failed\n");
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
}
#if 0
static BOOL WH_StateInSetChildWEPKey(void)
{
    u16 wepmode;
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    wepmode = (*pNetWH->sChildWEPKeyGenerator)(pNetWH->sWEPKey, &pNetWH->sBssDesc);
    result = WM_SetWEPKey(WH_StateOutSetChildWEPKey, wepmode, pNetWH->sWEPKey);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutSetChildWEPKey(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    // 子機として親機に接続します
    if (!WH_StateInStartChild())
    {
        WH_TRACE("WH_StateOutSetChildWEPKey : startchild failed\n");
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}
#endif

/* ----------------------------------------------------------------------
   state : StartChild
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartChild(void)
{
    u8 ssid_data[32];
    u8* pSsidData = NULL;
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if ((pNetWH->sSysState == WH_SYSSTATE_CONNECTED)
        || (pNetWH->sSysState == WH_SYSSTATE_KEYSHARING)
        || (pNetWH->sSysState == WH_SYSSTATE_DATASHARING))
    {
        // 既に接続済み。
        WH_TRACE("WH_StateInStartChild : already connected?\n");
        return TRUE;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

//    result = WM_StartConnectEx(WH_StateOutStartChild, &pNetWH->sBssDesc, NULL, TRUE,
//                               (u16)((pNetWH->sChildWEPKeyGenerator!=NULL) ? WM_AUTHMODE_SHARED_KEY : WM_AUTHMODE_OPEN_SYSTEM));

    //ssid送信
#if 0
    GFL_STD_MemCopy(SSID,&ssid_data[1],sizeof(SSID));
    ssid_data[0] = CommStateGetServiceNo();
    OS_TPrintf("ssid  %d %s %d\n",ssid_data[0],SSID,sizeof(SSID));
    result = WM_StartConnectEx(WH_StateOutStartChild, &pNetWH->sBssDesc,
                               ssid_data, TRUE, WM_AUTHMODE_OPEN_SYSTEM);
#else

    pSsidData = GFI_NET_GetSSID();
    result = WM_StartConnectEx(WH_StateOutStartChild, &pNetWH->sBssDesc,
                               pSsidData, TRUE, WM_AUTHMODE_OPEN_SYSTEM);
#endif
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutStartChild(void *arg)
{
    // StartConnect で設定されたコールバックは、
    // 1) 接続処理の開始時
    // 2) 認証終了時
    // 3) 接続完了後、親機側から切断された時
    // 4) 最大台数以上の接続をしようとした時
    // という複数のケースで呼ばれるので、各々区別する必要があります。
    // この関数の場合、次へ進んで良いのは 2) のときのみです。

    WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);
    NET_PRINT("errcode %d %d\n",cb->errcode,cb->state);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);

        if (cb->errcode == WM_ERRCODE_OVER_MAX_ENTRY)
        {
            // GUIDELINE : ガイドライン準拠ポイント(6.3.7)
            // 自分（子機）が親機の対応台数からあふれてしまっている場合。
            // ここでは致命的エラーとします。
            // この時は、メイン側で何か表示する必要があります。
            // エラーコードが WM_ERRCODE_OVER_MAX_ENTRY だったかどうかは
            // WH_GetLastError 関数でチェック出来ます。
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
            return;
        }
        else if (cb->errcode == WM_ERRCODE_NO_ENTRY)
        {
            // 自分（子機）が接続しようと試みた親機が
            // エントリーを受け付けていない場合。
            // ここでは致命的エラーとします。
            // この時は、メイン側で何か表示する必要があります。
            // エラーコードが WM_ERRCODE_NO_ENTRY だったかどうかは
            // WH_GetLastError 関数でチェック出来ます。
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
            return;
        }
        else if (cb->errcode == WM_ERRCODE_FAILED)
        {
            // 自分（子機）が接続しようと試みた親機がいなくなってしまった等の
            // 理由で、接続要求がタイムアウトした場合。
            // ここでは致命的エラーとします。
            // この時は、メイン側でリセットした上で接続をリトライするか、
            // もしくは何か表示する必要があります。
            // エラーコードが WM_ERRCODE_FAILED だったかどうかは
            // WH_GetLastError 関数でチェック出来ます。
            WH_ChangeSysState(WH_SYSSTATE_CONNECT_FAIL);
            return;
        }
        else
        {
            // 親機のチャンネル設定が不正な場合にはWM_ERRCODE_INVALID_PARAMが
            // 返る可能性もある。
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
        return;
    }

    if (cb->state == WM_STATECODE_BEACON_LOST)
    {
        // 接続中の親機からのビーコンを 16 回連続で受信失敗しました。
        // ビーコンを見失うと、V ブランク同期が崩れてしまう可能性と、
        // 親機が次のセッションを開始している(TGID が変わった)ことに
        // 気づけない可能性があります。

        // このデモでは特に何も行いません。
        return;
    }

    if (cb->state == WM_STATECODE_CONNECTED)
    {
        if(pNetWH->bDisconnectChild){
            OS_TPrintf("comm>>親機を切断します\n");
            WH_SetError(WH_ERRCODE_DISCONNECTED);
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
            return;
        }
        else{
            // 認証終了時。
            // cb->aid に自分に割り振られた AID が入っています。
            WH_TRACE("Connect to Parent\n");
            OS_TPrintf("comm>>親機に接続できました\n");
            WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
            if (!WH_StateInStartChildMP())
            {
                /* FIXME : ここは BUSY のまま置いておいて良いのか? */
                WH_TRACE("WH_StateInStartChildMP failed\n");
                WH_ChangeSysState(WH_SYSSTATE_BUSY);
                return;
            }
            
            // 自分の aid を保存しておく。
            pNetWH->sMyAid = cb->aid;
            return;
            
        }
    }
    else if (cb->state == WM_STATECODE_CONNECT_START)
    {
        // 接続処理の開始時。
        // 親機がいなくなっていた場合などに WM_ERRCODE_FAILED が、
        // 親機がエントリーを受け付けていない場合は WM_ERRCODE_NO_ENTRY が、
        // 親機の接続数がいっぱいの場合は WM_ERRCODE_OVER_MAX_ENTRY が、
        // それぞれ cb->errcode に返っています。
        // ここでは何も行わず、認証の終了を待ちます。
        return;

    }
    else if (cb->state == WM_STATECODE_DISCONNECTED)
    {
        // GUIDELINE : ガイドライン準拠ポイント(6.3.1)
        // 親機から切断されてしまった場合。
        // 上(6.3.7)と同様の処理（何か表示する）が必要です。
        // この時のエラーコードは、 WH_ERRCODE_DISCONNECTED です
        // （`WM_'ではない事に注意）。
        // こうなる場合（親から切断）は、普通のゲーム中にもありえる
        // でしょうが、ここではとりあえずエラーの一種としておいて
        // メイン側で対応します。

        WH_TRACE("Disconnected from Parent\n");
        OS_TPrintf("親機から切断された\n");
        WH_SetError(WH_ERRCODE_DISCONNECTED);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }
    else if (cb->state == WM_STATECODE_DISCONNECTED_FROM_MYSELF)
    {
        // 自ら切断した場合は処理を行いません
        return;
    }

    WH_TRACE("unknown state %d, %s\n", cb->state, WH_GetWMStateCodeName(cb->state));
    WH_ChangeSysState(WH_SYSSTATE_ERROR);
}

/* ----------------------------------------------------------------------
   state : StartChildMP
   ---------------------------------------------------------------------- */
static BOOL WH_StateInStartChildMP(void)
{
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);
/*
    result = WM_StartMP(WH_StateOutStartChildMP,
                        (u16 *)pNetWH->sRecvBuffer,
                        (u16)pNetWH->sRecvBufferSize, (u16 *)pNetWH->sSendBuffer,
                        (u16)pNetWH->sSendBufferSize, 1);
*/

    result = WM_StartMPEx(WH_StateOutStartChildMP,
                          (u16 *)pNetWH->sRecvBuffer,
                          (u16)pNetWH->sRecvBufferSize,
                          (u16 *)pNetWH->sSendBuffer,
                          (u16)pNetWH->sSendBufferSize,
                          1,
                          0,//        u16             defaultRetryCount ,
                          FALSE,//        BOOL            minPollBmpMode ,
                          FALSE,//        BOOL            singlePacketMode ,
                          FALSE,//        BOOL            fixFreqMode ,
                          FALSE//        BOOL            ignoreFatalError
                          );

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutStartChildMP(void *arg)
{
    WMstartMPCallback *cb = (WMstartMPCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    // WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {

        if (cb->errcode == WM_ERRCODE_SEND_FAILED)
        {
            // WM_STATECODE_MPACK_IND において
            // MPACK フレームにより親機の受信エラーが通知された場合。
            // 再送処理などは別の場所で行われているため、
            // ここでは特に何もする必要はありません。デバッグ用です。
            return;

        }
        else if (cb->errcode == WM_ERRCODE_TIMEOUT)
        {
            // MP フレームの受信後、一定時間経っても MPACK フレームが
            // 受信できなかった場合。(cb->state == WM_STATECODE_MPACK_IND)
            // 再送処理などは別の場所で行われているため、
            // ここでは特に何もする必要はありません。デバッグ用です。
            return;

        }
        else if (cb->errcode == WM_ERRCODE_INVALID_POLLBITMAP)
        {
            // WM_STATECODE_MP_IND, WM_STATECODE_MPACK_IND において
            // 自分宛てでないフレームを受け取った場合。
            // 3 台以上の通信でしばしば発生するため、
            // 致命的エラーにしてはいけません。
            // ここでは特に何もする必要はありません。デバッグ用です。
            return;
        }

        WH_REPORT_FAILURE(cb->errcode);

        WH_ChangeSysState(WH_SYSSTATE_ERROR);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_MP_START:
        // StartMP が正常終了した通知。
        // これ以降、送受信可能となります。
        GF_ASSERT(pNetWH->sConnectMode != WH_CONNECTMODE_KS_CHILD);
        GF_ASSERT(pNetWH->sConnectMode != WH_CONNECTMODE_DS_CHILD);
        WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
        break;

    case WM_STATECODE_MP_IND:
        // 子機の受信完了。

        // cb->recvBuf で親機から受信したデータフレームの内容を受け取れますが、
        // 通常のデータ受信は WM_SetPortCallback でコールバックを設定してください。
        // また、データシェアリング・キーシェアリングを使う場合は、
        // 受信処理は内部的に行われますので、WM_SetPortCallback を使う必要もありません。

        break;

    case WM_STATECODE_MPACK_IND:
        // MPACK フレームの受信通知。デバッグ用です。
        break;

    case WM_STATECODE_MPEND_IND:
        // 子機ならここへは来ないはずです。

    default:
        WH_TRACE("unknown indicate, state = %d\n", cb->state);
        break;
    }
}

/* ----------------------------------------------------------------------
   state : StartChildKeyShare
  ---------------------------------------------------------------------- */
#if 0
static BOOL WH_StateInStartChildKeyShare(void)
{
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (pNetWH->sSysState == WH_SYSSTATE_KEYSHARING)
    {
        // 既にキーシェアリングしています。
        return TRUE;
    }

    if (pNetWH->sSysState != WH_SYSSTATE_CONNECTED)
    {
        // 接続していません。
        return FALSE;
    }

    WH_ChangeSysState(WH_SYSSTATE_KEYSHARING);
    result = WM_StartKeySharing(&pNetWH->sWMKeySetBuf, WH_DS_PORT);

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
   state : EndChildKeyShare
   ---------------------------------------------------------------------- */
static BOOL WH_StateInEndChildKeyShare(void)
{
    // キーシェアリングを終了します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (pNetWH->sSysState != WH_SYSSTATE_KEYSHARING)
    {
        return FALSE;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_EndKeySharing(&pNetWH->sWMKeySetBuf);

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    if (!WH_StateInEndChildMP())
    {
        return FALSE;
    }

    return TRUE;
}
#endif

/* ----------------------------------------------------------------------
   state : EndChildMP
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndChildMP(void)
{
    // MP 通信を終了します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_EndMP(WH_StateOutEndChildMP);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutEndChildMP(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        (void)WH_Finalize();
        return;
    }

    if (!WH_StateInEndChild())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}

/* ----------------------------------------------------------------------
  state : EndChild
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndChild(void)
{
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    // 親機との接続を切断します。
    result = WM_Disconnect(WH_StateOutEndChild, 0);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_Reset();
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndChild(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }
    // ここで、子機としての切断処理は完了し アイドリング（待機中）状態に戻ります。
    WH_ChangeSysState(WH_SYSSTATE_IDLE);
}

/* ----------------------------------------------------------------------
  state : Reset
  ---------------------------------------------------------------------- */
static BOOL WH_StateInReset(void)
{
    // この状態は、親機子機共通です。
    // システムを初期状態に戻します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_Reset(WH_StateOutReset);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutReset(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        WH_REPORT_FAILURE(cb->errcode);
        NET_PRINT("WH_StateOutReset でエラーが出た\n");
        return;
    }
    // Reset は次の状態を開始せず、アイドリング（待機中）状態にします。
    WH_ChangeSysState(WH_SYSSTATE_IDLE);
}

/* ----------------------------------------------------------------------
   disconnect
  ---------------------------------------------------------------------- */
static BOOL WH_StateInDisconnectChildren(u16 bitmap)
{
    // この状態では、引数で指定した子機との接続を切断します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    result = WM_DisconnectChildren(WH_StateOutDisconnectChildren, bitmap);

    if (result == WM_ERRCODE_NO_CHILD)
    {
        return FALSE;
    }

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    {
        OSIntrMode enabled = OS_DisableInterrupts();
        pNetWH->sConnectBitmap &= ~bitmap;
        (void)OS_RestoreInterrupts(enabled);
    }
    return TRUE;
}

static void WH_StateOutDisconnectChildren(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }
}

/* ----------------------------------------------------------------------
   power off
  ---------------------------------------------------------------------- */
static BOOL WH_StateInPowerOff(void)
{
    // 無線ハードウェアへの電力供給を終了します。
    // この状態は、親機子機共通です。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    result = WM_PowerOff(WH_StateOutPowerOff);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutPowerOff(void *arg)
{
    // 電源切断後状態です。
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    if (!WH_StateInDisable())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
    }
}

/* ----------------------------------------------------------------------
   disable
  ---------------------------------------------------------------------- */
static BOOL WH_StateInDisable(void)
{
    // 無線ハードウェアの使用終了を通知します。
    // この状態は、親機子機共通です。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    result = WM_Disable(WH_StateOutDisable);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutDisable(void *arg)
{
    // 全て終了しました。
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
    }
}

/* ----------------------------------------------------------------------
  state : SetMPData
  ---------------------------------------------------------------------- */
static BOOL WH_StateInSetMPData(void *data, u16 datasize, int port, WHSendCallbackFunc callback)
{
    // この状態は、親機子機共通です。
    // データをセットし、送信します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    // WH_TRACE_STATE(pNetWH->sSysState);

    DC_FlushRange(pNetWH->sSendBuffer, (u32)pNetWH->sSendBufferSize);
    /* PXI操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();


    result = WM_SetMPDataToPortEx(WH_StateOutSetMPData,
                                  (void *)callback,
                                  data, datasize, 0xffff, port, WH_DATA_PRIO);
    
    
    if (result != WM_ERRCODE_OPERATING) {
        WH_TRACE("WH_StateInSetMPData failed - %s\n", WH_GetWMErrCodeName(result));
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutSetMPData(void *arg)
{
    WMPortSendCallback *cb = (WMPortSendCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    // WH_TRACE_STATE(pNetWH->sSysState);

    // この callback が呼ばれるまでは、SetMPDataToPort で
    // 設定した送信データのメモリ領域を上書きしてはいけません。

    // 0～7番 port を使った場合は、送信に失敗した場合に
    // WM_ERRCODE_SEND_FAILED が返ります。
    // また、送信キューが一杯だった場合には
    // WM_ERRCODE_SEND_QUEUE_FULL が返ります。


    if (cb->errcode != WM_ERRCODE_SUCCESS && cb->errcode != WM_ERRCODE_SEND_FAILED)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    if (cb->arg != NULL)
    {
        WHSendCallbackFunc callback = (WHSendCallbackFunc) cb->arg;
        // 必要に応じて、WHSendCallbackFunc 型を変更し、
        // cb->aid や cb->data などを受け取るようにしてください。
        (*callback) ((cb->errcode == WM_ERRCODE_SUCCESS));
    }
}

static void WH_PortReceiveCallback(void *arg)
{
    WMPortRecvCallback *cb = (WMPortRecvCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
    }
    else if (pNetWH->sReceiverFunc != NULL)
    {
        if (cb->state == WM_STATECODE_PORT_INIT)
        {
            // 初期化時には何もしません。
            // cb->aidBitmap に初期化時点で接続している相手の aid が
            // 設定されています。
        }
        else if (cb->state == WM_STATECODE_PORT_RECV)
        {
            // データを受信したので、コールバックを呼びます。
            (*pNetWH->sReceiverFunc) (cb->aid, cb->data, cb->length);
        }
        else if (cb->state == WM_STATECODE_DISCONNECTED)
        {
            // 切断された旨を NULL 送信で通知します。
            (*pNetWH->sReceiverFunc) (cb->aid, NULL, 0);
        }
        else if (cb->state == WM_STATECODE_DISCONNECTED_FROM_MYSELF)
        {
            // 過去との互換のため、自ら切断した場合は通知しません。
        }
        else if (cb->state == WM_STATECODE_CONNECTED)
        {
            // 接続された場合は何もしません。
            // cb->aid に接続してきた相手の aid が、
            // cb->macAddress に相手の MAC アドレスが、
            // 設定されています。
        }
    }
}


/* ----------------------------------------------------------------------
  state : End
  ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
  state : WM_End
  ---------------------------------------------------------------------- */
static void WH_StateOutEnd(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }
    WVR_TerminateAsync(NULL,NULL);  // イクニューモン切断
    WH_ChangeSysState(WH_SYSSTATE_STOP);
}


/* ======================================================================
   Public Interfaces
   ====================================================================== */


/**************************************************************************
 * 以下は、WH の各種設定値を変更する関数です。
 **************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WH_SetGgid

  Description:  ゲームグループIDを設定します。
                親機の接続前に呼び出します。

  Arguments:    ggid    設定するゲームグループID.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetGgid(u32 ggid)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sParentParam.ggid = ggid;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetUserGameInfo

  Description:  ユーザ定義の親機情報を設定します。
                親機の接続前に呼び出します。

  Arguments:    userGameInfo  ユーザ定義の親機情報へのポインタ
                length        ユーザ定義の親機情報のサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetUserGameInfo( u16* userGameInfo, u16 length )
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    //SDK_ASSERT( length <= WM_SIZE_USER_GAMEINFO );
    //SDK_ASSERT( (userGameInfo != NULL) || (length > 0) );
    //SDK_ASSERT( pNetWH->sSysState == WH_SYSSTATE_IDLE );
    
    // beacon にユーザ定義のデータを載せる場合はここに指定します。
    // 子機の親機選択画面で親機の Nickname を表示させたい場合などは
    // ここに情報をセットして親機から子機に伝えることになります。
    pNetWH->sParentParam.userGameInfo = userGameInfo;
    pNetWH->sParentParam.userGameInfoLength = length;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetDebugOutput

  Description:  デバッグ文字列出力用の関数を設定します。

  Arguments:    func    設定するデバッグ文字列出力用の関数.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetDebugOutput(void (*func) (const char *, ...))
{
    OSIntrMode enabled = OS_DisableInterrupts();
    wh_trace = func;
    (void)OS_RestoreInterrupts(enabled);
}


/**************************************************************************
 * 以下は、WM ライブラリの状態を取得するラッパー関数です。
 **************************************************************************/

/* ----------------------------------------------------------------------
  Name:        WH_GetLinkLevel
  Description: 電波の受信強度を取得します。
  Arguments:   none.
  Returns:     WMLinkLevel の数値を返します。
  ---------------------------------------------------------------------- */
int WH_GetLinkLevel(void)
{
    return (int)WM_GetLinkLevel();
}

/* ----------------------------------------------------------------------
   Name:        WH_GetAllowedChannel
   Description: 接続に使用出来るチャンネルのビットパターンを取得します。
   Arguments:   none.
   Returns:     channel pattern
   ---------------------------------------------------------------------- */
u16 WH_GetAllowedChannel(void)
{
    return WM_GetAllowedChannel();
}


/**************************************************************************
 * 以下は、WH の状態を取得する関数です。
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_GetBitmap
   Description: 接続状態を示すビットパターンを取得します。
   Arguments:   none.
   Returns:     bitmap pattern
   ---------------------------------------------------------------------- */
u16 WH_GetBitmap(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->sConnectBitmap;
}


/* ----------------------------------------------------------------------
   Name:        WH_GetBitmap
   Description: 接続状態を示すビットパターンを取得します。
   Arguments:   none.
   Returns:     bitmap pattern
   ---------------------------------------------------------------------- */
static u16 WH_GetConnectNum(void)
{
    int num=0,i;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    u16 bitmap = pNetWH->sConnectBitmap;
    
    for(i = 0;i < 16; i++){
        if(bitmap & 0x01){
            num++;
        }
        bitmap = bitmap >> 1;
    }
    return num;
}


/* ----------------------------------------------------------------------
   Name:        WH_GetSystemState
   Description: WH の内部状態を取得します。
   Arguments:   none.
   Returns:     内部状態（WH_SYSSTATE_XXXX）。
   ---------------------------------------------------------------------- */
int WH_GetSystemState(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->sSysState;
}

/* ----------------------------------------------------------------------
   Name:        WH_GetConnectMode
   Description: 接続情報を取得します。
   Arguments:   none.
   Returns:     接続情報（WH_CONNECTMODE_XX_XXXX）。
   ---------------------------------------------------------------------- */
int WH_GetConnectMode(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->sConnectMode;
}

/* ----------------------------------------------------------------------
   Name:        WH_GetLastError
   Description: 最も最近に起きたエラーのコードを取得します。
   Arguments:   none.
   Returns:     エラーコード。
   ---------------------------------------------------------------------- */
int WH_GetLastError(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->sErrCode;
}

/*---------------------------------------------------------------------------*
  Name:         WH_PrintBssDesc

  Description:  WMBssDesc 構造体のメンバをデバッグ出力する。

  Arguments:    info    デバッグ出力したいBssDescへのポインタ.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_PrintBssDesc(WMBssDesc *info)
{
#pragma unused( info )
    u16 i;

    OS_TPrintf("length = %04x\n", info->length);
    OS_TPrintf("rssi   = %04x\n", info->rssi);
    OS_TPrintf("bssid = %02x%02x%02x%02x%02x%02x\n", info->bssid[0], info->bssid[1], info->bssid[2],
               info->bssid[3], info->bssid[4], info->bssid[5]);
    OS_TPrintf("ssidLength = %04x\n", info->ssidLength);
    OS_TPrintf("ssid = ");
    for (i = 0; i < 32; i++)
    {
        OS_TPrintf("%02x", info->ssid[i]);
    }
    OS_TPrintf("\n");
    OS_TPrintf("capaInfo        = %04x\n", info->capaInfo);
    OS_TPrintf("rateSet.basic   = %04x\n", info->rateSet.basic);
    OS_TPrintf("rateSet.support = %04x\n", info->rateSet.support);
    OS_TPrintf("beaconPeriod    = %04x\n", info->beaconPeriod);
    OS_TPrintf("dtimPeriod      = %04x\n", info->dtimPeriod);
    OS_TPrintf("channel         = %04x\n", info->channel);
    OS_TPrintf("cfpPeriod       = %04x\n", info->cfpPeriod);
    OS_TPrintf("cfpMaxDuration  = %04x\n", info->cfpMaxDuration);
    OS_TPrintf("gameInfoLength  = %04x\n", info->gameInfoLength);
//    OS_TPrintf("gameInfo.version = %04x\n", info->gameInfo.version);
    OS_TPrintf("gameInfo.ggid   = %08x\n", info->gameInfo.ggid);
    OS_TPrintf("gameInfo.tgid   = %04x\n", info->gameInfo.tgid);
    OS_TPrintf("gameInfo.userGameInfoLength = %02x\n", info->gameInfo.userGameInfoLength);
    OS_TPrintf("gameInfo.gameNameCount_attribute = %02x\n", info->gameInfo.gameNameCount_attribute);
    OS_TPrintf("gameInfo.parentMaxSize   = %04x\n", info->gameInfo.parentMaxSize);
    OS_TPrintf("gameInfo.childMaxSize    = %04x\n", info->gameInfo.childMaxSize);
}


/**************************************************************************
 * 以下は、チャンネルに関する処理を行う関数です。
 **************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WH_StartMeasureChannel

  Description:  チャンネル電波使用率の調査を開始

  Arguments:    None.

  Returns:     シーケンス開始に成功すれば真。
 *---------------------------------------------------------------------------*/
BOOL WH_StartMeasureChannel(void)
{
#define MAX_RATIO 100                  // チャンネル使用率は0～100の範囲
    u16 result;
    u8  macAddr[6];
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    OS_GetMacAddress(macAddr);
    RAND_INIT(OS_GetVBlankCount() + *(u16 *)&macAddr[0] + *(u16 *)&macAddr[2] + *(u16 *)&macAddr[4]);   // 乱数初期化
    RAND();

    pNetWH->sChannel = 0;
    pNetWH->sChannelBusyRatio = MAX_RATIO + 1;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WH_StateInMeasureChannel(1);

    if (result == WH_ERRCODE_NOMORE_CHANNEL)
    {
        // 使用できるチャンネルが1つもない
        // そもそも電波が使えない特殊な状況に
        // あるということなので、ここではエラーで返しています
        // （本来はここで何か表示する必要があります）。
        WH_REPORT_FAILURE(WH_ERRCODE_NOMORE_CHANNEL);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return FALSE;
    }

    if (result != WM_ERRCODE_OPERATING)
    {
        // エラー終了
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_StateInMeasureChannel

  Arguments:    channel     検索を開始するチャンネル番号

  Returns:      WM_ERRCODE_SUCCESS        - 検索中
                WM_ERRCODE_NOMORE_CHANNEL - もう検索するチャンネルがない
                WM_ERRCODE_API_ERR        - WM_MeasureChannelのAPI呼び出し失敗
 *---------------------------------------------------------------------------*/
static u16 WH_StateInMeasureChannel(u16 channel)
{
    u16 allowedChannel;
    u16 result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    allowedChannel = WM_GetAllowedChannel();

    if (allowedChannel == 0x8000)
    {
        // 0x8000 が返ってきた場合は、無線が初期化されていないなど
        // 無線ライブラリの状態異常を表しているのでエラーにします。
        WH_REPORT_FAILURE(WM_ERRCODE_ILLEGAL_STATE);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return WM_ERRCODE_ILLEGAL_STATE;
    }
    if (allowedChannel == 0)
    {
        // 0が返ってきた場合、そもそも電波が使えない特殊な状況に
        // あるということなので、使用できるチャンネルがない事を返します。
        WH_REPORT_FAILURE(WH_ERRCODE_NO_RADIO);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return WH_ERRCODE_NOMORE_CHANNEL;
    }

    while (((1 << (channel - 1)) & allowedChannel) == 0)
    {
        channel++;
        if (channel > 16)
        {
            /* 許可されたチャンネルをすべて調べ終わった場合 */
            return WH_ERRCODE_NOMORE_CHANNEL;
        }
    }

    result = WHi_MeasureChannel(WH_StateOutMeasureChannel, channel);
    if (result != WM_ERRCODE_OPERATING)
    {
        return result;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WH_StateOutMeasureChannel

  Arguments:    arg     検索を結果を通知するWMMeasureChannelCallback構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WH_StateOutMeasureChannel(void *arg)
{
    u16 result;
    u16 channel;
    WMMeasureChannelCallback *cb = (WMMeasureChannelCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        // 何か不都合があった場合。
        // MeasureChannel で失敗するようならどのみち電波を使えない、と
        // 考えられるので、エラー状態にします。
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return;
    }

    WH_TRACE("channel %d bratio = %x\n", cb->channel, cb->ccaBusyRatio);

    channel = cb->channel;

    /* より使用率の低いチャンネルを取得 (初期値 101% なので先頭は必ず選択) */
    if (pNetWH->sChannelBusyRatio > cb->ccaBusyRatio)
    {
        pNetWH->sChannelBusyRatio = cb->ccaBusyRatio;
        pNetWH->sChannelBitmap = (u16)(1 << (channel - 1));
    }
    else if (pNetWH->sChannelBusyRatio == cb->ccaBusyRatio)
    {
        pNetWH->sChannelBitmap |= 1 << (channel - 1);
    }

    result = WH_StateInMeasureChannel(++channel);

    if (result == WH_ERRCODE_NOMORE_CHANNEL)
    {
        // チャンネル検索終了
        WH_ChangeSysState(WH_SYSSTATE_MEASURECHANNEL);
        return;
    }

    if (result != WM_ERRCODE_OPERATING)
    {
        // エラー終了
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }
}

/* ----------------------------------------------------------------------
  電波使用率のチェック
  ---------------------------------------------------------------------- */
static WMErrCode WHi_MeasureChannel(WMCallbackFunc func, u16 channel)
{
#define WH_MEASURE_TIME         30     // 1フレームに一回通信している電波を拾えるだけの間隔(ms)
#define WH_MEASURE_CS_OR_ED     3      // キャリアセンスとED値の論理和
#define WH_MEASURE_ED_THRESHOLD 17     // 実験データによる経験的に有効と思われるお勧めED閾値

    /*
     * 電波使用率取得パラメータとして、
     * 実験による経験的に有効と思われる値を入れています。
     */
    return WM_MeasureChannel(func,     /* コールバック設定 */
                             WH_MEASURE_CS_OR_ED,       /* CS or ED */
                             WH_MEASURE_ED_THRESHOLD,   /* 第2引数がキャリアセンスのみの場合は無効 */
                             channel,  /* 今回の検索チャンネル */
                             WH_MEASURE_TIME);  /*１チャンネルあたりの調査時間[ms] */
}


/*---------------------------------------------------------------------------*
  Name:         WH_GetMeasureChannel

  Description:  利用可能な中から一番使用率の低いチャンネルを返します。
                WH_MeasureChannelの動作が完了しWH_SYSSTATE_MEASURECHANNEL状態
                になっている必要があります。
                この関数がコールされると一番使用率の低いチャンネルを返し
                WH_SYSSTATE_IDLE状態に遷移します。
                
  Arguments:    None.

  Returns:      もっとも使用率の低い利用可能なチャンネル番号.
 *---------------------------------------------------------------------------*/
u16 WH_GetMeasureChannel(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_ASSERT(pNetWH->sSysState == WH_SYSSTATE_MEASURECHANNEL);

    WH_ChangeSysState(WH_SYSSTATE_IDLE);
    pNetWH->sChannel = (u16)SelectChannel(pNetWH->sChannelBitmap);
    WH_TRACE("decided channel = %d\n", pNetWH->sChannel);
    return pNetWH->sChannel;
}


/*---------------------------------------------------------------------------*
  Name:         SelectChannel

  Description:  最も電波使用率の低かったチャンネルを取得します。
                最も電波使用率の低いチャンネルが複数あった場合には、
                乱数を使用する。
                
  Arguments:    チャンネルビットマップ.

  Returns:      もっとも使用率の低い利用可能なチャンネル番号.
 *---------------------------------------------------------------------------*/
static s16 SelectChannel(u16 bitmap)
{
    s16 i;
    s16 channel = 0;
    u16 num = 0;
    u16 select;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();

    for (i = 0; i < 16; i++)
    {
        if (bitmap & (1 << i))
        {
            channel = (s16)(i + 1);
            num++;
        }
    }

    if (num <= 1)
    {
        return channel;
    }

    // 同じ電波使用率のチャンネルが複数存在した場合
    select = (u16)(((RAND() & 0xFF) * num) / 0x100);

    channel = 1;

    for (i = 0; i < 16; i++)
    {
        if (bitmap & 1)
        {
            if (select == 0)
            {
                return (s16)(i + 1);
            }
            select--;
        }
        bitmap >>= 1;
    }

    return 0;
}


/**************************************************************************
 * 以下は、無線を初期化して通信可能状態まで遷移する関数です。
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_CreateHandle
   Description: メモリーを確保し初期化します
   Arguments:   作業領域.
   Returns:     メモリー
   ---------------------------------------------------------------------- */

static void* _pMem = NULL;

GFL_NETWM* WH_CreateHandle(HEAPID heapID, void* pHeap, int maxNum, int maxByte)
{
    GFL_NETWM* pWmInfo = (GFL_NETWM*)pHeap;
    u32 addr;
    int allocSize;
    
    if(pHeap!=NULL){
        return (GFL_NETWM*)pHeap;
    }
    _pMem = GFL_HEAP_AllocMemory(heapID, sizeof(GFL_NETWM)+32);
    GFL_STD_MemClear(_pMem, sizeof(GFL_NETWM)+32);
    
    addr = (u32)_pMem;
    if(addr % 32){
        addr += 32 - (addr % 32);
    }
    pWmInfo = (GFL_NETWM*)addr;
    addr = (u32)&pWmInfo->sParentParam;// ATTRIBUTE_ALIGN(32);
    GF_ASSERT((addr%32)==0);
    addr = (u32)&pWmInfo->sWmBuffer;// ATTRIBUTE_ALIGN(32);
    GF_ASSERT((addr%32)==0);
    addr = (u32)&pWmInfo->sBssDesc;// ATTRIBUTE_ALIGN(32);
    GF_ASSERT((addr%32)==0);
    addr = (u32)&pWmInfo->sScanParam;// ATTRIBUTE_ALIGN(32);
    GF_ASSERT((addr%32)==0);

    pWmInfo->parentMPSize = GFL_NET_DATA_HEADER + maxNum * maxByte;
    pWmInfo->childMPSize = maxByte;

    // 送受信バッファは親の分サイズ確保
    {
        int sendSize = pWmInfo->parentMPSize;
        sendSize = WM_SIZE_MP_PARENT_SEND_BUFFER( sendSize, FALSE );
        pWmInfo->sParentSendBufferSize=sendSize;
        pWmInfo->sChildRecvBufferSize = WM_SIZE_MP_CHILD_RECEIVE_BUFFER(sendSize,FALSE);
    }
    {
        int recvSize = WM_SIZE_MP_PARENT_RECEIVE_BUFFER( pWmInfo->childMPSize, maxNum, FALSE );//32byteアライメント
        pWmInfo->sParentRecvBufferSize = recvSize;
        pWmInfo->sChildSendBufferSize =  WM_SIZE_MP_CHILD_SEND_BUFFER( pWmInfo->childMPSize, FALSE );
    }
    {
        allocSize = pWmInfo->sParentSendBufferSize > pWmInfo->sChildSendBufferSize ? pWmInfo->sParentSendBufferSize : pWmInfo->sChildSendBufferSize;
        pWmInfo->pSendBufferBK = GFL_HEAP_AllocMemory(heapID, allocSize+32);
        addr = (u32)pWmInfo->pSendBufferBK;
        if(addr % 32){
            addr += 32 - (addr % 32);
        }
        pWmInfo->sSendBuffer = (u8*)addr;

    }
    {
        allocSize = pWmInfo->sParentRecvBufferSize > pWmInfo->sChildRecvBufferSize ? pWmInfo->sParentRecvBufferSize : pWmInfo->sChildRecvBufferSize;
        pWmInfo->pRecvBufferBK = GFL_HEAP_AllocMemory(heapID,allocSize+32);
        addr = (u32)pWmInfo->pRecvBufferBK;
        if(addr % 32){
            addr += 32 - (addr % 32);
        }
        pWmInfo->sRecvBuffer = (u8*)addr;
    }
    return (GFL_NETWM*)pWmInfo;
}


/* ----------------------------------------------------------------------
   Name:        WH_DestroyHandle
   Description: メモリーを開放処理します
   Arguments:   作業領域.
   Returns:     メモリー
   ---------------------------------------------------------------------- */
void WH_DestroyHandle(GFL_NETWM* pWmInfo)
{
    GFL_HEAP_FreeMemory(pWmInfo->pRecvBufferBK);
    GFL_HEAP_FreeMemory(pWmInfo->pSendBufferBK);
    GFL_HEAP_FreeMemory(_pMem);
}

/* ----------------------------------------------------------------------
   Name:        WH_Initialize
   Description: 初期化作業を行い、初期化シーケンスを開始します。
   Arguments:   作業領域.
   Returns:     シーケンス開始に成功すれば真。
   ---------------------------------------------------------------------- */
BOOL WH_Initialize(void* pHeap, BOOL bNet)
{
    GFL_NETWM* pWmInfo = (GFL_NETWM*)pHeap;

    
    pWmInfo->sRecvBufferSize = 0;
    pWmInfo->sSendBufferSize = 0;

    pWmInfo->sReceiverFunc = NULL;
    pWmInfo->sMyAid = 0;
    pWmInfo->sConnectBitmap = WH_BITMAP_EMPTY;
    pWmInfo->sErrCode = WM_ERRCODE_SUCCESS;

    pWmInfo->sSysState = WH_SYSSTATE_STOP;

    pWmInfo->sParentParam.userGameInfo = NULL;
    pWmInfo->sParentParam.userGameInfoLength = 0;

    // 接続子機のユーザ判定関数をNULL (multiboot)
    pWmInfo->sJudgeAcceptFunc = NULL;
    pWmInfo->maxEntry = GFL_NET_MACHINE_MAX;
    
    pWmInfo->bDisconnectChild = FALSE;
    pWmInfo->bPauseConnect = FALSE;

    if(!bNet){
        return TRUE;
    }
    
    // 初期化シーケンス開始。
    if (!WH_StateInInitialize()) {
        WH_SetError(WM_ERRCODE_FAILED);
        return FALSE;
    }
    return TRUE;
}


/* ----------------------------------------------------------------------
   Indicate handler
   ---------------------------------------------------------------------- */
static void WH_IndicateHandler(void *arg)
{
    WMindCallback *cb = (WMindCallback *)arg;

    if (cb->errcode == WM_ERRCODE_FIFO_ERROR)
    {
        // 復旧不能なエラーが発生した場合です。
        // プレイヤーにその旨を通知する画面などを表示し、
        // プログラムを停止させて下さい。
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        //OS_Panic("Wireless FATAL error occured.\n");
        WH_SetError(WH_ERRCODE_FATAL);
//        CommFatalErrorFunc(0);  // 割り込み中に画面表示をできないので移動
    }
}

/* ----------------------------------------------------------------------
   state : Initialize
   ---------------------------------------------------------------------- */
static BOOL WH_StateInInitialize(void)
{
    // 初期化シーケンスを開始します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

#ifndef WH_USE_DETAILED_INITIALIZE
    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_Initialize(&pNetWH->sWmBuffer, WH_StateOutInitialize, WH_DMA_NO);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }

#else
    // WM_Init は同期関数です。
    result = WM_Init(&pNetWH->sWmBuffer, WH_DMA_NO);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    if (!WH_StateInEnable())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }
#endif

    return TRUE;
}


#ifndef WH_USE_DETAILED_INITIALIZE

static void WH_StateOutInitialize(void *arg)
{
    // 電源投入後状態です。
    WMErrCode result;
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return;
    }
    // 不定期に発生する通知を受け取るコールバック関数を設定します。
    result = WM_SetIndCallback(WH_IndicateHandler);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
#if ERROR_CHECK
        pNetWH->errorCallBack(0,GFL_NET_ERROR_RESET_SAVEPOINT);
#endif
        return;
    }

    // システム状態をアイドリング（待機中）に変更。
    WH_ChangeSysState(WH_SYSSTATE_IDLE);

    // 次の状態をセットしないので、ここでシーケンスはいったん終了です。
    // この状態で WH_Connect が呼ばれると接続作業に移行します。
}

#else

/* ----------------------------------------------------------------------
   enable
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEnable(void)
{
    // ハードウェアを使用可能にします（使用許可を得ます）。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    result = WM_Enable(WH_StateOutEnable);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutEnable(void *arg)
{
    // ハードウェアの使用が許可されたら、電源投入状態へ移行します。
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    if (!WH_StateInPowerOn())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }
}

/* ----------------------------------------------------------------------
   power on
  ---------------------------------------------------------------------- */
static BOOL WH_StateInPowerOn(void)
{
    // 無線ハードウェアが使用可能になったので、電力供給を開始します。
    WMErrCode result;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    result = WM_PowerOn(WH_StateOutPowerOn);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutPowerOn(void *arg)
{
    // 電源投入後状態です。
    WMErrCode result;
    WMCallback *cb = (WMCallback *)arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_TRACE_STATE(pNetWH->sSysState);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    // 不定期に発生する通知を受け取るコールバック関数を設定します。
    result = WM_SetIndCallback(WH_IndicateHandler);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    NET_PRINT(" システム状態をアイドリング（待機中）に変更。\n");
    WH_ChangeSysState(WH_SYSSTATE_IDLE);

    // 次の状態をセットしないので、ここでシーケンスはいったん終了です。
    // この状態で WH_Connect が呼ばれると接続作業に移行します。
}

#endif // #ifdef WH_USE_DETAILED_INITIALIZE


static void _lifeTimeCallback(void* arg)
{
    WMCallback* pCallBack = (WMCallback*)arg;

    
    OS_TPrintf("lifetime 通過 %d \n",pCallBack->apiid);
    // WM_SetLifeTimeに設定したコールバック
}


#define _LIFE_TIME (15)
/* ----------------------------------------------------------------------
  Name:        WH_ParentConnect
  Description: 接続シーケンスを開始します。
  Arguments:   mode    - WH_CONNECTMODE_MP_PARENT ならば親機としてMP開始。
                         WH_CONNECTMODE_DS_PARENT ならば親機としてDataSharing開始。
                         WH_CONNECTMODE_KS_PARENT ならば親機としてKeySharing開始。
               tgid    - 親機通信tgid
               channel - 親機通信channel
               maxEntry - 子機最大数 サービスによって分ける
               beaconPeriod - ビーコン間隔 サービスによって指定する
               bEntry    - 子機の新規を受け付けるかどうか
  Returns:     接続シーケンス開始に成功すれば真。
  ---------------------------------------------------------------------- */
BOOL WH_ParentConnect(int mode, u16 tgid, u16 channel,u16 maxEntry,u16 beaconPeriod,BOOL bEntry)
{
    // 待機状態になければ接続シーケンスを開始出来ません。
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_ASSERT(pNetWH->sSysState == WH_SYSSTATE_IDLE);

    // ライフタイムの設定  この設置を有効にすると ライフタイムを無効にできる
#if _DEBUG_LIFETIME
    WM_SetLifeTime(_lifeTimeCallback,0xffff,0xffff,0xffff,0xffff);
#endif  //_DEBUG_LIFETIME
    
    // WM_StartMP() 用の送受信バッファサイズ計算
    // 事前に静的にバッファを確保したい場合は WM_SIZE_MP_* 関数マクロを、
    // 動的に確保して構わない場合は、親子接続後で WM_StartMP() を呼び出す直前に
    // WM_GetReceiveBufferSize() API を用います。
    // 同様に事前に静的にバッファを確保したい場合は WM_SIZE_MP_* 関数マクロを、
    // 動的に確保して構わない場合は、親子接続後で WM_StartMP() を呼び出す直前に
    // WM_GetSendBufferSize() API を用います。
    pNetWH->sRecvBufferSize = pNetWH->sParentRecvBufferSize;//WH_PARENT_RECV_BUFFER_SIZE;
    pNetWH->sSendBufferSize = pNetWH->sParentSendBufferSize;//WH_PARENT_SEND_BUFFER_SIZE;
    
    WH_TRACE("recv buffer size = %d\n", pNetWH->sRecvBufferSize);
    WH_TRACE("send buffer size = %d\n", pNetWH->sSendBufferSize);
    
    pNetWH->sConnectMode = mode;
    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    pNetWH->sParentParam.tgid = tgid;
    pNetWH->sParentParam.channel = channel;
    pNetWH->sParentParam.beaconPeriod = beaconPeriod;

    GF_ASSERT(mode == WH_CONNECTMODE_MP_PARENT);
    pNetWH->sParentParam.parentMaxSize = pNetWH->parentMPSize;
    pNetWH->sParentParam.childMaxSize = pNetWH->childMPSize;
    pNetWH->sParentParam.maxEntry = maxEntry;
    pNetWH->sParentParam.CS_Flag = 0;
    pNetWH->sParentParam.multiBootFlag = 0;
    pNetWH->sParentParam.entryFlag = bEntry;
    pNetWH->sParentParam.KS_Flag = (u16)((mode == WH_CONNECTMODE_KS_PARENT) ? 1 : 0);

    switch (mode)
    {
    case WH_CONNECTMODE_MP_PARENT:
    case WH_CONNECTMODE_KS_PARENT:
    case WH_CONNECTMODE_DS_PARENT:
        // 親機モードで接続開始。
        return WH_StateInSetParentParam();
    default:
        break;
    }

    WH_TRACE("unknown connect mode %d\n", mode);
    return FALSE;
}


/* ----------------------------------------------------------------------
  Name:        WH_ChildConnect
  Description: 接続シーケンスを開始します。
  Arguments:   mode -    WH_CONNECTMODE_MP_CHILD ならば子機としてMP開始。
                         WH_CONNECTMODE_DS_CHILD ならば子機としてDataSharing開始。
                         WH_CONNECTMODE_KS_CHILD ならば子機としてKeySharing開始。
               bssDesc - 接続する親機のbssDesc
                
  Returns:     接続シーケンス開始に成功すれば真。
  ---------------------------------------------------------------------- */
BOOL WH_ChildConnect(int mode, WMBssDesc *bssDesc)
{
    // 待機状態になければ接続シーケンスを開始出来ません。
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_ASSERT(pNetWH->sSysState == WH_SYSSTATE_IDLE);

    // ライフタイムの設定
    //上手に設定しないと、ひとつの子機がおちたら全部切断してしまったりするバグになる k.ohno
#if _DEBUG_LIFETIME
    WM_SetLifeTime(_lifeTimeCallback,0xffff,0xffff,0xffff,0xffff);
#endif  //_DEBUG_LIFETIME

    // WM_StartMP() 用の送受信バッファサイズ計算
    // 事前に静的にバッファを確保したい場合は WM_SIZE_MP_* 関数マクロを、
    // 動的に確保して構わない場合は、親子接続後で WM_StartMP() を呼び出す直前に
    // WM_GetReceiveBufferSize() API を用います。
    // 同様に事前に静的にバッファを確保したい場合は WM_SIZE_MP_* 関数マクロを、
    // 動的に確保して構わない場合は、親子接続後で WM_StartMP() を呼び出す直前に
    // WM_GetSendBufferSize() API を用います。
    pNetWH->sRecvBufferSize = pNetWH->sChildRecvBufferSize;//WH_CHILD_RECV_BUFFER_SIZE;
    pNetWH->sSendBufferSize = pNetWH->sChildSendBufferSize;//WH_CHILD_SEND_BUFFER_SIZE;
//    pNetWH->sRecvBufferSize = WH_CHILD_RECV_BUFFER_SIZE;
//    pNetWH->sSendBufferSize = WH_CHILD_SEND_BUFFER_SIZE;

    WH_TRACE("recv buffer size = %d\n", pNetWH->sRecvBufferSize);
    WH_TRACE("send buffer size = %d\n", pNetWH->sSendBufferSize);

    pNetWH->sConnectMode = mode;
    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    switch (mode)
    {
    case WH_CONNECTMODE_MP_CHILD:
    case WH_CONNECTMODE_KS_CHILD:
    case WH_CONNECTMODE_DS_CHILD:
        // 子機モードで接続開始。
        // 保存されていた親機のBssDescを使用してスキャン無しで接続する。
        GFL_STD_MemCopy(bssDesc, &pNetWH->sBssDesc,  sizeof(pNetWH->sBssDesc));
        DC_FlushRange(&pNetWH->sBssDesc, sizeof(pNetWH->sBssDesc));
        DC_WaitWriteBufferEmpty();
#if 0
        if (pNetWH->sChildWEPKeyGenerator != NULL)
        {
            // WEP Key Generator が設定されていれば、WEP Key の設定へ
            return WH_StateInSetChildWEPKey();
        }
        else
#endif
        {
            return WH_StateInStartChild();
        }
    default:
        break;
    }

    WH_TRACE("unknown connect mode %d\n", mode);
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetJudgeAcceptFunc

  Description:  子機の接続受け入れを判定するための関数をセットします。

  Arguments:    子機の接続判定関数を設定.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetJudgeAcceptFunc(WHJudgeAcceptFunc func)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sJudgeAcceptFunc = func;
}


/**************************************************************************
 * 以下は、WH_DATA_PORT ポートを使用する直接的な MP 通信の関数です。
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_SetReceiver
   Description: WH_DATA_PORT ポートにデータ受信コールバックを設定します。
   Arguments:   proc - データ受信コールバック
                port   データ送信ポート
   Returns:     none.
   ---------------------------------------------------------------------- */
void WH_SetReceiver(WHReceiverFunc proc, int port)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sReceiverFunc = proc;
    if (WM_SetPortCallback(port, WH_PortReceiveCallback, NULL) != WM_ERRCODE_SUCCESS)
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        WH_TRACE("WM not Initialized\n");
        while(1){}
    }
}

/* ----------------------------------------------------------------------
   Name:        WH_SendData
   Description: WH_DATA_PORT ポートにデータ送信を開始します。
               （MP通信用。データシェアリング中などに呼ぶ必要はありません）
   Arguments:   size - データサイズ
   Returns:     送信開始に成功すれば真。
   ---------------------------------------------------------------------- */

BOOL WH_SendData(void *data, u16 size, int port, WHSendCallbackFunc callback)
{
    return WH_StateInSetMPData(data, size, port, callback);
}

/**************************************************************************
 * 以下は、通信を終了して初期化状態まで遷移させる関数です。
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_Reset
   Description: リセットシーケンスを開始します。
                この関数を呼ぶと、今の状態に構わずリセットします。
        エラーからの強制復帰用です。
   Arguments:   none.
   Returns:     処理開始に成功すれば真。
   ---------------------------------------------------------------------- */
void WH_Reset(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if(WH_SYSSTATE_SCANNING == pNetWH->sSysState){
        OS_TPrintf("停止しました\n");
        while(1){}
    }
    
    if (!WH_StateInReset())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
    }
}

/* ----------------------------------------------------------------------
   Name:        WH_Finalize
   Description: 後処理・終了シーケンスを開始します。
                この関数を呼ぶと、今の状態を見て適切な終了シーケンスを
                実行します。
                通常の終了処理には（WH_Resetではなく）この関数を使用します。
   Arguments:   None.
   Returns:     None.
   ---------------------------------------------------------------------- */
void WH_Finalize(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if (pNetWH->sSysState == WH_SYSSTATE_IDLE)
    {
        WH_TRACE("already WH_SYSSTATE_IDLE\n");
        return;
    }

    WH_TRACE("WH_Finalize, state = %d\n", pNetWH->sSysState);


    if ((pNetWH->sSysState != WH_SYSSTATE_KEYSHARING)
        && (pNetWH->sSysState != WH_SYSSTATE_DATASHARING)
        && (pNetWH->sSysState != WH_SYSSTATE_CONNECTED))
    {
        // 接続していない・エラー状態などの場合はリセットしておく。
        WH_ChangeSysState(WH_SYSSTATE_BUSY);
        WH_Reset();
        return;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    switch (pNetWH->sConnectMode)
    {
    case WH_CONNECTMODE_KS_CHILD:
#if 0
        if (!WH_StateInEndChildKeyShare())
        {
            WH_Reset();
        }
#endif
        break;

    case WH_CONNECTMODE_DS_CHILD:
    case WH_CONNECTMODE_MP_CHILD:
        if (!WH_StateInEndChildMP())
        {
            WH_Reset();
        }
        break;

    case WH_CONNECTMODE_KS_PARENT:
/*        if (!WH_StateInEndParentKeyShare())
        {
            WH_Reset();
        }*/
        break;

    case WH_CONNECTMODE_DS_PARENT:
    case WH_CONNECTMODE_MP_PARENT:
        if (!WH_StateInEndParentMP())
        {
            WH_Reset();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WH_End

  Description:  無線通信を終了する。

  Arguments:    None.

  Returns:      成功すれば真。
 *---------------------------------------------------------------------------*/
BOOL WH_End(void)
{
    int err;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    WH_ASSERT(pNetWH->sSysState == WH_SYSSTATE_IDLE);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    err = WM_End(WH_StateOutEnd);
    if (err != WM_ERRCODE_OPERATING)
    {
        NET_PRINT(" WH_End エラー%d\n",err);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);

        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_GetCurrentAid

  Description:  現在の自分のAIDを取得します。
                子機は接続・切断時に変化する可能性があります。

  Arguments:    None.

  Returns:      AIDの値
 *---------------------------------------------------------------------------*/
u16 WH_GetCurrentAid(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->sMyAid;
}


void WH_SetMaxEntry(int maxEntry)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if(pNetWH){
        pNetWH->maxEntry = maxEntry;
    }
}

#if 0
/*---------------------------------------------------------------------------*
  Name:         WH_SetParentWEPKeyGenerator

  Description:  WEP Key を生成する関数を設定します。
                この関数を呼び出すと、接続時の認証に WEP が使われます。
                ゲームアプリケーションごとのアルゴリズムで
                接続前に親子で同一の値を設定します。
                この関数は親機用です。

  Arguments:    func    WEP Key を生成する関数へのポインタ
                        NULL を指定すると WEP Key を使用しない

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetParentWEPKeyGenerator(WHParentWEPKeyGeneratorFunc func)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sParentWEPKeyGenerator = func;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetChildWEPKeyGenerator

  Description:  WEP Key を生成する関数を設定します。
                この関数を呼び出すと、接続時の認証に WEP が使われます。
                ゲームアプリケーションごとのアルゴリズムで
                接続前に親子で同一の値を設定します。
                この関数は子機用です。

  Arguments:    func    WEP Key を生成する関数へのポインタ
                        NULL を指定すると WEP Key を使用しない

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetChildWEPKeyGenerator(WHChildWEPKeyGeneratorFunc func)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sChildWEPKeyGenerator = func;
}
#endif
/*---------------------------------------------------------------------------*
  Name:         WH_IsSysStateIdle
  Description:  アイドル状態になったのを確認して
                次の状態に進むので、その確認のために必要  k.ohno追加
  Arguments:    none
  Returns:      WH_SYSSTATE_IDLEならTRUE
 *---------------------------------------------------------------------------*/
BOOL WH_IsSysStateIdle(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
   return (pNetWH->sSysState == WH_SYSSTATE_IDLE);
}

/*---------------------------------------------------------------------------*
  Name:         WH_IsSysStateBusy
  Description:  BUSY状態になっている場合RESETがきかないので
                次の状態に進むので、その確認のために必要  k.ohno追加
  Arguments:    none
  Returns:      WH_SYSSTATE_BUSYならTRUE
 *---------------------------------------------------------------------------*/
BOOL WH_IsSysStateBusy(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
   return (pNetWH->sSysState == WH_SYSSTATE_BUSY);
}

/*---------------------------------------------------------------------------*
  Name:         WH_IsSysStateError
  Description:  ERROR状態になっている場合RESETがきかないので
                次の状態に進むので、その確認のために必要  k.ohno追加
  Arguments:    none
  Returns:      WH_SYSSTATE_ERRORならTRUE
 *---------------------------------------------------------------------------*/
BOOL WH_IsSysStateError(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
   return (pNetWH->sSysState == WH_SYSSTATE_ERROR);
}

/*---------------------------------------------------------------------------*
  Name:         WH_IsSysStateScan
  Description:  子機の検索状態の時かどうかを確認
                親子機切り替え時に、子機がスキャン中の場合、無理なリセットを
                かけたらエラーになった為
  Arguments:    none
  Returns:      WH_SYSSTATE_SCANNINGならTRUE
 *---------------------------------------------------------------------------*/
BOOL WH_IsSysStateScan(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if(pNetWH){
        return (pNetWH->sSysState == WH_SYSSTATE_SCANNING);
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         WHSetGameInfo
  Description:  ビーコンの中身を変更する
                接続時にしか働かない
  Arguments:    
  Returns:      none
 *---------------------------------------------------------------------------*/

void WHSetGameInfo(void* pBuff, int size, int ggid, int tgid)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if(pNetWH->sSysState == WH_SYSSTATE_CONNECTED){
        NET_PRINT("ビーコンをセットした %d\n",tgid);
        WM_SetGameInfo(NULL, pBuff, size,
                       ggid, tgid, WM_ATTR_FLAG_ENTRY);
    }
    else{
        NET_PRINT("ビーコンをセットできなかった %d \n",tgid);
    }
}

/*---------------------------------------------------------------------------*
  Name:         _callBackSetEntry
  Description:  WM_SetEntryのコールバック
  Arguments:    none
  Returns:      none
 *---------------------------------------------------------------------------*/

static void _callBackSetEntry(void* arg)
{
    WMCallback* pWMCB = arg;
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    
    if(pWMCB->errcode == WM_ERRCODE_SUCCESS){
        pNetWH->bSetEntry = TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WHSetEntry
  Description:  子機接続を受け付けるかどうかを制限する
  Arguments:    TRUE で受け付ける FALSEで受け付けない
  Returns:      成功したらTRUEだが 実際はWHIsSetEntryEndを待たねばならない
 *---------------------------------------------------------------------------*/

BOOL WHSetEntry(BOOL bEnable)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->bSetEntry = FALSE;
    if(pNetWH->sSysState == WH_SYSSTATE_CONNECTED){
        if(WM_ERRCODE_OPERATING == WM_SetEntry( _callBackSetEntry, bEnable)){
            return TRUE;
        }
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         WHIsSetEntryEnd
  Description:  子機受付状態関数が終わったかどうか
  Arguments:    none
  Returns:      none
 *---------------------------------------------------------------------------*/

BOOL WHIsSetEntryEnd(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->bSetEntry;
}

/*---------------------------------------------------------------------------*
  Name:         WHIsParentBeaconSent
  Description:  ビーコンを投げ終わった時にTRUEになります
  Returns:      both
 *---------------------------------------------------------------------------*/

BOOL WHIsParentBeaconSent(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if(pNetWH){
        return (pNetWH->stateBeaconSentNum != 0);
    }
    return FALSE;
}

void WH_ParentDataInit(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->stateBeaconSentNum = 0;
}

/*---------------------------------------------------------------------------*
  Name:         WHSetLifeTime
  Description:  ライフタイムを小さくする または元に戻す
  Arguments:    bMinimum 小さくする場合TRUE
  Returns:      none
 *---------------------------------------------------------------------------*/

void WHSetLifeTime(BOOL bMinimum)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    if(!pNetWH){
        return;
    }
    if(bMinimum){
        WM_SetLifeTime(_lifeTimeCallback,0xffff,1,5,1);
    }
    else{
#if _DEBUG_LIFETIME
        WM_SetLifeTime(_lifeTimeCallback,0xffff,0xffff,0xffff,0xffff);
#else
        WM_SetLifeTime(_lifeTimeCallback,0xffff,40,5,40);
#endif  //_DEBUG_LIFETIME
    }
}


/*---------------------------------------------------------------------------*
  Name:         WHSetGGIDScanCallback
  Description:  GGIDを検索する為のコールバックセット
  Arguments:    コールバック
  Returns:      none
 *---------------------------------------------------------------------------*/
#if 0
void WHSetGGIDScanCallback(fGGIDCallBack callback)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->sGGIDScanCallback = callback;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         WHSetDisconnectCallBack
  Description:  接続が切れた際に呼ばれるコールバック関数登録
  Arguments:    コールバック
  Returns:      none
 *---------------------------------------------------------------------------*/
void WHSetDisconnectCallBack(WHdisconnectCallBack callBack)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->disconnectCallBack = callBack;
}

/*---------------------------------------------------------------------------*
  Name:         WHSetConnectCallBack
  Description:  子機接続時のコールバック登録関数
  Arguments:    callBack コールバック関数
  Returns:      none
 *---------------------------------------------------------------------------*/

void WHSetConnectCallBack(WHdisconnectCallBack callBack)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->connectCallBack = callBack;
}

/*---------------------------------------------------------------------------*
  Name:         WHParentConnectPause
  Description:  親機にくる接続を止める もしくは解除
  Arguments:    止める もしくは解除
  Returns:      none
 *---------------------------------------------------------------------------*/

void WHParentConnectPause(BOOL bPause)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->bPauseConnect = bPause;
}

/*---------------------------------------------------------------------------*
  Name:         WHParentConnectPause
  Description:  親機にくる接続を止める もしくは解除を得る
  Arguments:    止める もしくは解除
  Returns:      none
 *---------------------------------------------------------------------------*/

BOOL WHGetParentConnectPause(void)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    return pNetWH->bPauseConnect;
}

/*---------------------------------------------------------------------------*
  Name:         WHParentConnectPauseSystem  システム専用
  Description:  親機にくる接続を止める もしくは解除
  Arguments:    止める もしくは解除
  Returns:      none
 *---------------------------------------------------------------------------*/

void WHParentConnectPauseSystem(BOOL bPause)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->bPauseConnectSystem = bPause;
}


/*---------------------------------------------------------------------------*
  Name:         WHChildConnectPause
  Description:  子機が接続と同時に切断
  Arguments:    止める もしくは解除
  Returns:      none
 *---------------------------------------------------------------------------*/

void WHChildConnectPause(BOOL bDisconnect)
{
    GFL_NETWM* pNetWH = _GFL_NET_WLGetNETWH();
    pNetWH->bDisconnectChild = bDisconnect;
}

