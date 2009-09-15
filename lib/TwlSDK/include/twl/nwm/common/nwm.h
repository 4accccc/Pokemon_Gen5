/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - include
  File:     nwm.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_NWM_ARM9_NWM_H__
#define LIBRARIES_NWM_ARM9_NWM_H__

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>
#include <nitro/wm.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define NWM_NUM_MAX_AP_AID              2007   // 無線ルータから与えられうる AssociationID の最大値

#define NWM_SIZE_SSID                     32
#define NWM_SIZE_MACADDR                   6
#define NWM_SIZE_BSSID                    NWM_SIZE_MACADDR

#define NWM_SIZE_WEP_40BIT                 5
#define NWM_SIZE_WEP_104BIT               13
#define NWM_SIZE_WEP_128BIT               16

#define NWM_SIZE_WEP                      20   // WM(DS接続設定) との互換性確保のため

#define NWM_WEPMODE_OPEN                   WM_WEPMODE_NO
#define NWM_WEPMODE_40BIT                  WM_WEPMODE_40BIT
#define NWM_WEPMODE_104BIT                 WM_WEPMODE_104BIT
#define NWM_WEPMODE_128BIT                 WM_WEPMODE_128BIT

#define NWM_WPAMODE_WPA_TKIP               (WM_WEPMODE_128BIT + 1)
#define NWM_WPAMODE_WPA2_TKIP              (WM_WEPMODE_128BIT + 2)
#define NWM_WPAMODE_WPA_AES                (WM_WEPMODE_128BIT + 3)
#define NWM_WPAMODE_WPA2_AES               (WM_WEPMODE_128BIT + 4)


#define NWM_BSS_DESC_SIZE                 64   // NWM_StartScan()で引き渡す、親機情報格納用バッファのサイズ
#define NWM_BSS_DESC_SIZE_MAX            512
#define NWM_FRAME_SIZE_MAX              1522   // 最大データフレームサイズ(1500(MTU) + 22(802.3 header))
#define NWM_SCAN_NODE_MAX                 32   // 1回のScanで発見可能な最大親機台数
#define NWM_SIZE_SCANBUF_MAX           65535   // 最大のバッファサイズ

#define NWM_SYSTEM_BUF_SIZE           (0x00003000)

// bitmask for capability information
#define NWM_CAPABILITY_ESS_MASK         0x0001
#define NWM_CAPABILITY_ADHOC_MASK       0x0002
#define NWM_CAPABILITY_CP_POLLABLE_MASK 0x0004
#define NWM_CAPABILITY_CP_POLL_REQ_MASK 0x0008
#define NWM_CAPABILITY_PRIVACY_MASK     0x0010
#define NWM_CAPABILITY_SPREAMBLE_MASK   0x0020
#define NWM_CAPABILITY_PBCC_MASK        0x0040
#define NWM_CAPABILITY_CH_AGILITY_MASK  0x0800
#define NWM_CAPABILITY_SPECTRUM_MASK    0x0100
#define NWM_CAPABILITY_QOS_MASK         0x0200
#define NWM_CAPABILITY_SSLOTTIME_MASK   0x0400
#define NWM_CAPABILITY_APSD_MASK        0x0800


#define NWM_SCANTYPE_PASSIVE    0
#define NWM_SCANTYPE_ACTIVE     1

#define NWM_DEFAULT_PASSIVE_SCAN_PERIOD 105 /* NWM では WM よりもデフォルト値が大きいことに注意 */
#define NWM_DEFAULT_ACTIVE_SCAN_PERIOD   30 /* NWM では アクティブスキャンとパッシブスキャンのデフォルト値が違うことに注意 */

#define NWM_RATESET_1_0M                0x0001
#define NWM_RATESET_2_0M                0x0002
#define NWM_RATESET_5_5M                0x0004
#define NWM_RATESET_6_0M                0x0008
#define NWM_RATESET_9_0M                0x0010
#define NWM_RATESET_11_0M               0x0020
#define NWM_RATESET_12_0M               0x0040
#define NWM_RATESET_18_0M               0x0080
#define NWM_RATESET_24_0M               0x0100
#define NWM_RATESET_36_0M               0x0200
#define NWM_RATESET_48_0M               0x0400
#define NWM_RATESET_54_0M               0x0800
#define NWM_RATESET_11B_MASK            ( NWM_RATESET_1_0M | NWM_RATESET_2_0M | NWM_RATESET_5_5M | NWM_RATESET_11_0M )
#define NWM_RATESET_11G_MASK            ( NWM_RATESET_1_0M | NWM_RATESET_2_0M | NWM_RATESET_5_5M | NWM_RATESET_11_0M     \
                                        | NWM_RATESET_6_0M | NWM_RATESET_9_0M | NWM_RATESET_12_0M | NWM_RATESET_18_0M    \
                                        | NWM_RATESET_24_0M | NWM_RATESET_36_0M | NWM_RATESET_48_0M | NWM_RATESET_54_0M )

#define NWM_WPA_PSK_LENGTH               32
#define NWM_WPA_PASSPHRASE_LENGTH_MAX    64


/* リンクレベル */
#define NWM_RSSI_INFRA_LINK_LEVEL_1  12
#define NWM_RSSI_INFRA_LINK_LEVEL_2  17
#define NWM_RSSI_INFRA_LINK_LEVEL_3  22

#define NWM_RSSI_ADHOC_LINK_LEVEL_1  12 /* [TODO] TBD */
#define NWM_RSSI_ADHOC_LINK_LEVEL_2  17 /* [TODO] TBD */
#define NWM_RSSI_ADHOC_LINK_LEVEL_3  22 /* [TODO] TBD */

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/


// 各APIのID
typedef enum NWMApiid
{
    NWM_APIID_INIT = 0,                 // NWM_Init()
    NWM_APIID_RESET,                    // NWM_Reset()
    NWM_APIID_END,                      // NWM_End()

    NWM_APIID_LOAD_DEVICE,              // NWM_LoadDevice()
    NWM_APIID_UNLOAD_DEVICE,            // NWM_UnloadDevice()
    NWM_APIID_OPEN,                     // NWM_Open()
    NWM_APIID_CLOSE,                    // NWM_Close()

    NWM_APIID_START_SCAN,               // NWM_StartScan()
    NWM_APIID_CONNECT,                  // NWM_Connect()
    NWM_APIID_DISCONNECT,               // NWM_Disconnect()
    NWM_APIID_SET_RECEIVING_BUF,        // NWM_SetReceivingFrameBuffer()
    NWM_APIID_SEND_FRAME,               // NWM_SendFrame()
    NWM_APIID_UNSET_RECEIVING_BUF,      // NWM_UnsetReceivingFrameBuffer()
    NWM_APIID_SET_WEPKEY,               // NWM_SetWEPKey()
    NWM_APIID_SET_PS_MODE,              // NWM_SetPowerSaveMode()

    NWM_APIID_SET_WPA_KEY,              //
    NWM_APIID_SET_WPA_PARAMS,           //

    NWM_APIID_CREATE_QOS,               //
    NWM_APIID_SET_WPA_PSK,              // NWM_SetWPAPSK()
    NWM_APIID_INSTALL_FIRMWARE,         // NWMi_InstallFirmware()
    NWM_APIID_ASYNC_KIND_MAX,           //  : 非同期処理の種類
  
    NWM_APIID_INDICATION = 128,         //  : Indicationコールバック用

    NWM_APIID_UNKNOWN = 255             //  : 不明のコマンド番号時にARM7から返す値
}
NWMApiid;


// APIのリザルトコード
typedef enum NWMRetCode
{
    NWM_RETCODE_SUCCESS         =  0,
    NWM_RETCODE_FAILED          =  1,
    NWM_RETCODE_OPERATING       =  2,
    NWM_RETCODE_ILLEGAL_STATE   =  3,
    NWM_RETCODE_NWM_DISABLE     =  4,
    NWM_RETCODE_INVALID_PARAM   =  5,
    NWM_RETCODE_FIFO_ERROR      =  6,
    NWM_RETCODE_FATAL_ERROR     =  7,   // ソフトウェアでの対応が不可能なエラー
    NWM_RETCODE_NETBUF_ERROR    =  8,
    NWM_RETCODE_WMI_ERROR       =  9,
    NWM_RETCODE_SDIO_ERROR      = 10,
    NWM_RETCODE_RECV_IND        = 11,
    NWM_RETCODE_INDICATION      = 12,   // internal use only
  
    NWM_RETCODE_MAX
} NWMRetCode;

// NWM のステートコード
typedef enum NWMState
{
    NWM_STATE_NONE            = 0x0000,
    NWM_STATE_INITIALIZED     = 0x0001,    // INITIALIZED state
    NWM_STATE_LOADED          = 0x0002,    // LOADED state
    NWM_STATE_DISCONNECTED    = 0x0003,    // DISCONNECTED state
    NWM_STATE_INFRA_CONNECTED = 0x0004,    // CONNECTED STA (infrastructure) state
    NWM_STATE_ADHOC_CONNECTED = 0x0005     // CONNECTED STA (Ad Hoc) state
}
NWMState;

typedef enum NWMReasonCode
{
    NWM_REASON_API_SUCCESS          = 0,
  
    /* for infra mode */
    NWM_REASON_NO_NETWORK_AVAIL     = 1,
    NWM_REASON_LOST_LINK            = 2,
    NWM_REASON_DISCONNECT_CMD       = 3,
    NWM_REASON_BSS_DISCONNECTED     = 4,
    NWM_REASON_AUTH_FAILED          = 5,
    NWM_REASON_ASSOC_FAILED         = 6,
    NWM_REASON_NO_RESOURCES_AVAIL   = 7,
    NWM_REASON_CSERV_DISCONNECT     = 8,
    NWM_REASON_INVAILD_PROFILE      = 9,

    NWM_REASON_WEP_KEY_ERROR        =10,

    /* for WPA supplicant */
    NWM_REASON_WPA_KEY_ERROR        =11,
    NWM_REASON_TKIP_MIC_ERROR       =12,
    
    /* for Wireless QoS (802.11e) */
    NWM_REASON_NO_QOS_RESOURCES_AVAIL   = 13,
    
    NWM_REASON_UNKNOWN

} NWMReasonCode;

typedef enum NWMAuthMode
{
  NWM_AUTHMODE_OPEN,    /* dot11 authentication */
  NWM_AUTHMODE_SHARED,  /* dot11 authentication */
  NWM_AUTHMODE_WPA_PSK_TKIP         = NWM_WPAMODE_WPA_TKIP,   /* WCM_WEPMODE_* と被らないように注意 */
  NWM_AUTHMODE_WPA2_PSK_TKIP        = NWM_WPAMODE_WPA2_TKIP,
  NWM_AUTHMODE_WPA_PSK_AES          = NWM_WPAMODE_WPA_AES,
  NWM_AUTHMODE_WPA2_PSK_AES         = NWM_WPAMODE_WPA2_AES
}
NWMAuthMode, NWMauthMode;


// Element ID of Infomation Elements
typedef enum NWMElementID {
  NWM_ELEMENTID_SSID               = 0,
  NWM_ELEMENTID_SUPPORTED_RATES    = 1,
  NWM_ELEMENTID_FH_PARAMETER_SET   = 2,
  NWM_ELEMENTID_DS_PARAMETER_SET   = 3,
  NWM_ELEMENTID_CF_PARAMETER_SET   = 4,
  NWM_ELEMENTID_TIM                = 5,
  NWM_ELEMENTID_IBSS_PARAMETER_SET = 6,
  NWM_ELEMENTID_COUNTRY            = 7,
  NWM_ELEMENTID_HP_PARAMETERS      = 8,
  NWM_ELEMENTID_HP_TABLE           = 9,
  NWM_ELEMENTID_REQUEST            = 10,
  NWM_ELEMENTID_QBSS_LOAD          = 11,
  NWM_ELEMENTID_EDCA_PARAMETER_SET = 12,
  NWM_ELEMENTID_TSPEC              = 13,
  NWM_ELEMENTID_TRAFFIC_CLASS      = 14,
  NWM_ELEMENTID_SCHEDULE           = 15,
  NWM_ELEMENTID_CHALLENGE_TEXT     = 16,

  NWM_ELEMENTID_POWER_CONSTRAINT   = 32,
  NWM_ELEMENTID_POWER_CAPABILITY   = 33,
  NWM_ELEMENTID_TPC_REQUEST        = 34,
  NWM_ELEMENTID_TPC_REPORT         = 35,
  NWM_ELEMENTID_SUPPORTED_CHANNELS = 36,
  NWM_ELEMENTID_CH_SWITCH_ANNOUNCE = 37,
  NWM_ELEMENTID_MEASURE_REQUEST    = 38,
  NWM_ELEMENTID_MEASURE_REPORT     = 39,
  NWM_ELEMENTID_QUIET              = 40,
  NWM_ELEMENTID_IBSS_DFS           = 41,
  NWM_ELEMENTID_ERP_INFORMATION    = 42,
  NWM_ELEMENTID_TS_DELAY           = 43,
  NWM_ELEMENTID_TCLASS_PROCESSING  = 44,
  NWM_ELEMENTID_HT_CAPABILITY      = 45,
  NWM_ELEMENTID_QOS_CAPABILITY     = 46,
  NWM_ELEMENTID_RSN                = 48,
  NWM_ELEMENTID_EX_SUPPORTED_RATES = 50,
  NWM_ELEMENTID_HT_INFORMATION     = 61,
  
  NWM_ELEMENTID_VENDOR             = 221,
  NWM_ELEMENTID_NINTENDO           = 221
}
NWMElementID;

typedef enum NWMPowerMode {
  NWM_POWERMODE_ACTIVE,
  NWM_POWERMODE_STANDARD,
  NWM_POWERMODE_UAPSD
} NWMPowerMode;

typedef enum NWMAccessCategory {
  NWM_AC_BE,          /* best effort */
  NWM_AC_BK,          /* background */
  NWM_AC_VI,          /* video */
  NWM_AC_VO,          /* voice */
  NWM_AC_NUM
} NWMAccessCategory;

typedef enum NWMNwType
{
  NWM_NWTYPE_INFRA,
  NWM_NWTYPE_ADHOC,
  NWM_NWTYPE_WPS,
  NWM_NWTYPE_NUM
} NWMNwType;

typedef enum NWMFramePort
{
    NWM_PORT_IPV4_ARP, /* for TCP/IP */
    NWM_PORT_EAPOL,    /* for WPA supplicant */
    NWM_PORT_OTHERS,
    NWM_PORT_NUM
} NWMFramePort, NWMframePort;

typedef void (*NWMCallbackFunc) (void *arg);     // NWM APIのコールバック型

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

typedef struct NWMBssDesc
{
    u16     length;                    // 2
    s16     rssi;                      // 4
    u8      bssid[NWM_SIZE_BSSID];     // 10
    u16     ssidLength;                // 12
    u8      ssid[NWM_SIZE_SSID];       // 44
    u16     capaInfo;                  // 46
    struct
    {
        u16     basic;                 // 48
        u16     support;               // 50
    }
    rateSet;
    u16     beaconPeriod;              // 52
    u16     dtimPeriod;                // 54
    u16     channel;                   // 56
    u16     cfpPeriod;                 // 58
    u16     cfpMaxDuration;            // 60
    u16     reserved;                  // 62 just for compatibility with WM (must be 0)
    u16     elementCount;              // 64
    u16     elements[1];
} NWMBssDesc, NWMbssDesc;


typedef struct NWMInfoElements
{
  u8  id;
  u8  length;
  u16 element[1];
}
NWMInfoElements;


//---------------------------------------
// NWM  Scan パラメータ構造体
typedef struct NWMScanParam
{
    NWMBssDesc *scanBuf;                   // 親機情報を格納するためのバッファ
    u16         scanBufSize;               // scanBufのサイズ
    u16         channelList;               // Scanを行うchannelリスト(複数指定可)
    u16         channelDwellTime;          // 1チャンネルあたりのScan時間(ms)
    u16         scanType;                  // passive or active
    u8          bssid[NWM_SIZE_BSSID];     // Scan対象MacAddress(0xffなら全親機対象)
    u16         ssidLength;                // Scan対象のSSID長(0の場合は全ノードが対象になる)
    u8          ssid[NWM_SIZE_SSID];       // Scan対象のSSID
    u16         rsv[6];
} NWMScanParam, NWMscanParam;

// NWM  Scan パラメータ構造体
typedef struct NWMScanExParam
{
    NWMBssDesc *scanBuf;                   // 親機情報を格納するためのバッファ
    u16         scanBufSize;               // scanBufのサイズ
    u16         channelList;               // Scanを行うchannelリスト(複数指定可)
    u16         channelDwellTime;          // 1チャンネルあたりのScan時間(ms)
    u16         scanType;                  // passive or active
    u8          bssid[NWM_SIZE_BSSID];     // Scan対象MacAddress(0xffなら全親機対象)
    u16         ssidLength;                // Scan対象のSSID長(0の場合は全ノードが対象になる)
    u8          ssid[NWM_SIZE_SSID];       // Scan対象のSSID
    u16         ssidMatchLength;           // スキャン対象の SSID マッチング長
    u16         rsv[5];
} NWMScanExParam, NWMscanExParam;


//---------------------------------------
// NWM  WPA parameter structure

typedef struct NWMWpaParam {
    u16   auth;     // NWMAuthMode (can use PSK only)
    u8    psk[NWM_WPA_PSK_LENGTH];
} NWMWpaParam;

// Frame受信バッファ構造体
typedef struct NWMRecvFrameHdr
{
  u8   da[NWM_SIZE_MACADDR];
  u8   sa[NWM_SIZE_MACADDR];
  u8   pid[2];
  u8   frame[2];
} NWMRecvFrameHdr;

//==========================================================================================

// 通常のコールバックの引数
typedef struct NWMCallback
{
    u16     apiid;
    u16     retcode;

} NWMCallback;

// NWM_StartScan()のコールバックの引数
typedef struct NWMStartScanCallback
{
    u16     apiid;
    u16     retcode;
    u32     channelList;               // 発見・未発見に関わらず、スキャンしたチャンネルリスト
    u8      reserved[2];               // padding
    u16     bssDescCount;              // 発見された親機の数
    u32     allBssDescSize;
    NWMBssDesc *bssDesc[NWM_SCAN_NODE_MAX];  // 親機情報の先頭アドレス
    u16     linkLevel[NWM_SCAN_NODE_MAX];   // 受信電波強度

} NWMStartScanCallback, NWMstartScanCallback;

// NWM_Connect()のコールバックの引数
typedef struct NWMConnectCallback
{
    u16     apiid;
    u16     retcode;
    u16     channel;
    u8      bssid[NWM_SIZE_BSSID];
    s8      rssi;
    u8      padding;
    u16     aid;                       // CONNECTEDのときのみ、自身に割り当てられたAID
    u16     reason;                    // 切断時のreason. NWMReasonCode で定義される。
    u16     listenInterval;
    u8      networkType;
    u8      beaconIeLen;
    u8      assocReqLen;
    u8      assocRespLen;
    u8      assocInfo[2]; /* this field consists of beaconIe, assocReq, assocResp */
} NWMConnectCallback, NWMconnectCallback;

// NWM_Disconnect()のコールバックの引数
typedef struct NWMDisconnectCallback
{
    u16   apiid;
    u16   retcode;
    u16   reason;
    u16   rsv;
} NWMDisconnectCallback, NWMdisconnectCallback;

// NWM_SendFrame()のコールバックの引数
typedef struct NWMSendFrameCallback
{
    u16     apiid;
    u16     retcode;
    NWMCallbackFunc callback;
} NWMSendFrameCallback;

// NWM_SetReceivingFrameBuffer()のコールバックの引数
typedef struct NWMReceivingFrameCallback
{
  u16   apiid;
  u16   retcode;
  u16   port;
  u16   rssi;
  u32   length;
  NWMRecvFrameHdr *recvBuf;

} NWMReceivingFrameCallback;


/*===========================================================================
  NWM APIs
  ===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         NWM_Init

  Description:  NWMライブラリの初期化処理を行う。
                ARM9側の初期化のみを行う同期関数。

  Arguments:    sysBuf     -   呼び出し元によって確保されたバッファへのポインタ。

                bufSize    -   呼び出し元によって確保されたバッファのサイズ。

                dmaNo      -   NWMが使用するDMA番号。

  Returns:      NWMRetCode -   処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Init(void* sysBuf, u32 bufSize, u8 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         NWM_Reset

  Description:  TWL無線ドライバを起動し直し、TWL無線モジュールをリセットする。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Reset(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_LoadDevice

  Description:  TWL無線モジュールを起動する。
                内部状態は NWM_STATE_INITIALIZED 状態から NWM_STATE_LOADED 状態
                に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_LoadDevice(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_UnloadDevice

  Description:  TWL無線モジュールをシャットダウンする。
                内部状態は NWM_STATE_LOADED 状態から NWM_STATE_INITIALIZED 状態
                に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_UnloadDevice(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_Open

  Description:  TWL無線を使用可能状態をONにする。
                内部状態は NWM_STATE_LOADED 状態から NWM_STATE_DISCONNECTED 状態
                に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Open(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_Close

  Description:  TWL無線を使用可能状態をOFFにする。
                内部状態は NWM_STATE_DISCONNECTED  状態から
                NWM_STATE_LOADED 状態に遷移される。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Close(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_End

  Description:  NWMライブラリの終了処理を行う。
                ARM9側の終了のみを行う同期関数。

  Arguments:    None

  Returns:      NWMRetCode   -  処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_End(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_StartScan

  Description:  APのスキャンを開始する。
                一度の呼び出しで複数台の親機情報を取得できる。
                スキャン中とスキャン後での状態遷移はない。

  Arguments:    callback     -  非同期処理が完了した時に呼び出されるコールバック関数。
                                引数は NWMStartScanCallback 構造体の形で返される。
                param        -  スキャン設定情報を示す構造体へのポインタ。
                                スキャン結果の情報は param->scanBuf に ARM7 が直接書き出す
                                ので、キャッシュラインに合わせておく必要がある。
                                個々のパラメータは NWMScanParam 構造体を参照。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_StartScan(NWMCallbackFunc callback, const NWMScanParam *param);

/*---------------------------------------------------------------------------*
  Name:         NWM_StartScanEx

  Description:  APのスキャンを開始する。
                一度の呼び出しで複数台の親機情報を取得できる。
                スキャン中とスキャン後での状態遷移はない。
                スキャン結果のSSIDの部分マッチングをすることができる。

  Arguments:    callback     -  非同期処理が完了した時に呼び出されるコールバック関数。
                                引数は NWMStartScanCallback 構造体の形で返される。
                param        -  スキャン設定情報を示す構造体へのポインタ。
                                ssidMatchLength で指定した長さだけ、スキャン対象のSSIDと
                                無線モジュールからのスキャン結果を照合し、
                                該当するもののみスキャン結果として書き出す。
                                スキャン結果の情報は param->scanBuf に ARM7 が直接書き出す
                                ので、キャッシュラインに合わせておく必要がある。
                                個々のパラメータは NWMScanExParam 構造体を参照。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_StartScanEx(NWMCallbackFunc callback, const NWMScanExParam *param);

/*---------------------------------------------------------------------------*
  Name:         NWM_Connect

  Description:  APへの接続を開始する。DS親機への接続は不可。

  Arguments:    callback     -  非同期処理が完了した時に呼び出されるコールバック関数。
                                引数は NWMConnectCallback 構造体の形で返される。
                nwType       -  NWMNwType で定義される値をとる。
                                通常は NWM_NWTYPE_INFRA を使用してください。
                pBdesc       -  接続するAPの情報。
                                NWM_StartScanにて取得した構造体を指定する。
                                この構造体の実体は強制的にキャッシュストアされる。
                                個々のパラメータは NWMBssDesc 構造体を参照。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Connect(NWMCallbackFunc callback, u8 nwType, const NWMBssDesc *pBdesc);

/*---------------------------------------------------------------------------*
  Name:         NWM_Disconnect

  Description:  APへの接続を切断する。

  Arguments:    callback     -  非同期処理が完了した時に呼び出されるコールバック関数。
                                引数は NWMDisconnectCallback 構造体の形で返される。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Disconnect(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetWEPKey

  Description:  WEP暗号モード、WEP暗号キーを設定する。
                NWM_Connectを呼ぶ前に設定しておく必要がある。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                wepmode     -   NWM_WEPMODE_OPEN  : 暗号機能なし
                                NWM_WEPMODE_40BIT : RC4(40bit)暗号モード
                                NWM_WEPMODE_104BIT: RC4(104bit)暗号モード
                                NWM_WEPMODE_128BIT: RC4(128bit)暗号モード
                wepkeyid    -   4つ指定したwepkeyのどれをデータ送信に使用するかを選択します。
                                0～3で指定します。
                wepkey      -   暗号キーデータ( 80バイト )へのポインタ。
                                キーデータは4つのデータで構成され、それぞれ20バイト。
                                各20バイトのうち、
                                 40ビットモードでは  5 バイト
                                104ビットモードでは 13 バイト
                                128ビットモードでは 16 バイト
                                のデータが使用される。
                                また、このデータの実体は強制的にキャッシュストアされる。
                authMode    -   接続時の認証方式。
                                NWM_AUTHMODE_OPEN  : オープン認証
                                NWM_AUTHMODE_SHARED: 共有鍵認証

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetWEPKey(NWMCallbackFunc callback, u16 wepmode, u16 wepkeyid, const u8 *wepkey, u16 authMode);

/*---------------------------------------------------------------------------*
  Name:         NWM_Passphrase2PSK

  Description:  WPAパスフレーズから、PSKを計算する。同期関数。

  Arguments:    passphrase   -   WPAパスフレーズを示すバッファへのポインタ。
                                 最大63バイト。
                ssid         -   接続先APのSSIDを示すバッファへのポインタ
                ssidlen      -   接続先APのSSIDのサイズ
                psk          -   計算されたPSKを格納するバッファへのポインタ。
                                 32バイト固定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWM_Passphrase2PSK(const u8 passphrase[NWM_WPA_PASSPHRASE_LENGTH_MAX], const u8 *ssid, u8 ssidlen, u8 psk[NWM_WPA_PSK_LENGTH]);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetWPAPSK

  Description:  WPA暗号モード、WPA暗号キーを設定する。
                NWM_Connectを呼ぶ前に設定しておく必要がある。

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                param       -   WPA設定情報を示す構造体へのポインタ。
                                個々のパラメータは NWMWpaParam 構造体を参照。

  Returns:      NWMRetCode  -   処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetWPAPSK(NWMCallbackFunc callback, NWMWpaParam *param);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetReceivingFrameBuffer

  Description:  受信バッファの設定を行う。

  Arguments:    callback     -  非同期処理が完了した時、およびデータを受信した時に
                                呼び出されるコールバック関数。
                                引数は NWMReceivingFrameCallback 構造体の形で返される。
                                データ受信時は、NWM_RETCODE_RECV_IND を伴う。
                                
                recvBuf      -  データ受信バッファへのポインタ。
                                ARM7が直接データを書き出すので、キャッシュに注意してください。
                recvBufSize  -  データ受信バッファのサイズ。受信バッファ数は、このサイズから
                                1536(0x600)バイトを割った整数になります。(小数点以下切捨て)
                                受信取りこぼしを避けるため、3072(0xC00)バイト以上確保してください。
                protocol     -  どのプロトコルに対しての受信バッファかを指定します。
                                NWM_PORT_IPV4_ARP: IPv4のTCP/IPデータおよびARPデータ
                                                   (通常はこちらをお使いください。)
                                NWM_PORT_EAPOL   : EAPOLデータ(WPS用)
                                NWM_PORT_OTHERS  : その他のプロトコル

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetReceivingFrameBuffer(NWMCallbackFunc callback, u8* recvBuf, u16 recvBufSize, u16 protocol);

/*---------------------------------------------------------------------------*
  Name:         NWM_SendFrame

  Description:  無線モジュールへの送信データのセットを行う。
                送信が完了したことを保証するものではないことに注意。

  Arguments:    callback     -  非同期処理が完了した時に呼び出されるコールバック関数。
                                引数は NWMSendFrameCallback 構造体の形で返される。
                destAddr     -  送信対象のBSSIDを示すバッファへのポインタ。
                sendFrame    -  送信データへのポインタ。
                                送信データの実体は強制的にキャッシュストア
                                される点に注意。
                sendFrameSize-  送信データのサイズ。

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SendFrame(NWMCallbackFunc callback, const u8 *destAddr,
                         const u16 *sendFrame, u16 sendFrameSize);

/*---------------------------------------------------------------------------*
  Name:         NWM_UnsetReceivingFrameBuffer

  Description:  受信バッファの設定を解除する。

  Arguments:    callback     -  非同期処理が完了した時、およびデータを受信した時に
                                呼び出されるコールバック関数。
                                
                protocol     -  どのプロトコルに対しての受信バッファかを指定します。
                                NWM_PORT_IPV4_ARP: IPv4のTCP/IPデータおよびARPデータ
                                                   (通常はこちらをお使いください。)
                                NWM_PORT_EAPOL   : EAPOLデータ(WPS用)
                                NWM_PORT_OTHERS  : その他のプロトコル

  Returns:      NWMRetCode   -  処理結果を返す。非同期処理が正常に開始された場合に
                                NWM_RETCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_UnsetReceivingFrameBuffer(NWMCallbackFunc callback, u16 protocol);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetPowerSaveMode

  Description:  PowerSaveModeを変更する

  Arguments:    callback    -   非同期処理が完了した時に呼び出されるコールバック関数。
                powerSave   -   省電力モードを指定する。
                                NWM_POWERMODE_ACTIVE  : 省電力モードOFF
                                NWM_POWERMODE_STANDARD: 802.11標準の省電力モード
                                NWM_POWERMODE_UAPSD   : 802.11eで定められる拡張省電力モード
                                                        (Unscheduled Automatic Power Save Delivery)

  Returns:      NWMRetCode  -   処理結果を返す。非同期処理が正常に開始された場合に
                                WM_ERRCODE_OPERATINGが返され、その後コールバックに
                                非同期処理の結果が改めて渡される。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetPowerSaveMode(NWMCallbackFunc callback, NWMPowerMode powerSave);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetMacAddress

  Description:  TWL無線モジュールから取得した自機のMACアドレスを取得する。
                同期関数。

  Arguments:    macAddr      -   自機のMACアドレスを格納するバッファへのポインタ。

  Returns:      NWMRetCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_GetMacAddress(u8* macAddr);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetBssDesc

  Description:  NWM_StartScan によるスキャン結果のバッファから、BSS情報を取得する。
                同期関数。

  Arguments:    bssbuf       -   スキャン結果を示すバッファへのポインタ
                bsssize      -   スキャン結果を示すバッファのサイズ
                index        -   取得するBSS情報のインデックス

  Returns:      NWMBssDesc   -   index番目に入っているBSS情報を返す。
                                 取得できなかった場合はNULLを返す。
 *---------------------------------------------------------------------------*/
NWMBssDesc* NWM_GetBssDesc(void* bssbuf, u32 bsssize, int index);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetAllowedChannel

  Description:  通信時に使用を許可されたチャンネルを取得する。同期関数。
                NWM_STATE_DISCONNECTED 以降(NWM_Open 完了後)の状態で呼び出してください。

  Arguments:    None.

  Returns:      u16 -   使用許可チャンネルのビットフィールドを返す。最下位ビットが
                        1チャンネル、最上位ビットが16チャンネルを示す。ビットが1の
                        チャンネルが使用許可、ビットが0のチャンネルは使用禁止。
                        通常は 1～13 チャンネルの内いくつかのビットが1になった値が
                        返される。0x0000が返された場合は使用を許可されたチャンネルが
                        存在しないため、無線機能そのものを使用禁止である。
                        また、未初期化時など関数に失敗した場合は0x8000が返される。
 *---------------------------------------------------------------------------*/
u16 NWM_GetAllowedChannel(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_CalcLinkLevel

  Description:  nwm_common_private.h にて定義されている閾値からリンクレベルを算出

  Arguments:    s16          -   Atheros ドライバから通知されるRSSI値

  Returns:      u16          -   WM と同様のリンクレベル
 *---------------------------------------------------------------------------*/
u16 NWM_CalcLinkLevel(s16 rssi);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetDispersionScanPeriod

  Description:  STA として AP もしくは DS 親機を探索する際に設定すべき探索限度時間を取得する。

  Arguments:    u16 scanType - スキャンタイプ、NWM_SCANTYPE_PASSIVE or NWM_SCANTYPE_ACTIVE
  
  Returns:      u16 -   設定すべき探索限度時間(ms)。
 *---------------------------------------------------------------------------*/
u16 NWM_GetDispersionScanPeriod(u16 scanType);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetState

  Description:  NWM の状態を取得する。

  Arguments:    None.
  
  Returns:      u16 -   NWM の状態。NWMState 列挙型で表される。
 *---------------------------------------------------------------------------*/
u16 NWM_GetState(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetInfoElements

  Description:  BSS情報から、指定した情報要素(IE)を取得する。

  Arguments:    bssDesc           -   取得対象となるBSS情報。
                elementID         -   802.11で定められる情報要素のElementID。

  Returns:      NWMInfoElements   -   指定した情報要素が存在すれば、
                                      該当する情報要素へのポインタを返す。
                                      取得できなかった場合はNULLを返す。
 *---------------------------------------------------------------------------*/
NWMInfoElements* NWM_GetInfoElements(NWMBssDesc *bssDesc, u8 elementID);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetVenderInfoElements

  Description:  BSS情報から、指定したベンダ情報要素(IE)を取得する。
                WPA等の情報要素取得に用いる。

  Arguments:    bssDesc           -   取得対象となるBSS情報。
                elementID         -   802.11で定められる情報要素のElementID。
                                      必ずNWM_ELEMENTID_VENDOR として指定する。
                ouiType           -   OUI(3バイト)とType(1バイト)の配列。
                                      WPAの場合、0x00 0x50 0xf2 0x01

  Returns:      NWMInfoElements   -   指定したベンダ情報要素が存在すれば、
                                      該当するベンダ情報要素へのポインタを返す。
                                      取得できなかった場合はNULLを返す。
 *---------------------------------------------------------------------------*/
NWMInfoElements* NWM_GetVenderInfoElements(NWMBssDesc *bssDesc, u8 elementID, const u8 ouiType[4]);


#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* LIBRARIES_NWM_ARM9_NWM_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
