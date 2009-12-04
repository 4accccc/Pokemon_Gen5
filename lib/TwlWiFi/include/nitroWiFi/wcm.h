/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - include
  File:     wcm.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-10#$
  $Rev: 1108 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#ifndef NITROWIFI_WCM_H_
#define NITROWIFI_WCM_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <nitro/types.h>
#include <nitro/wm.h>
#ifdef SDK_TWL
#include <twl/nwm.h>
#endif

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/

// WCM ライブラリが必要とするワークバッファのサイズ
#define WCM_WORK_SIZE_WM            8960            // TBD
#define WCM_WORK_SIZE_NWM           18432           // 0x4800 TBD

#ifdef SDK_TWL
#define WCM_WORK_SIZE               WCM_WORK_SIZE_NWM
#else
#define WCM_WORK_SIZE               WCM_WORK_SIZE_WM
#endif

// 処理結果コード
#define WCM_RESULT_SUCCESS          0               // 成功
#define WCM_RESULT_FAILURE          1               // 失敗
#define WCM_RESULT_PROGRESS         2               // 既に同じ非同期処理を実行中
#define WCM_RESULT_ACCEPT           3               // 非同期処理要求を受諾( 必ずコールバックが発生 )
#define WCM_RESULT_REJECT           4               // 非同期処理要求を拒否( ARM7 への要求伝達に失敗 )
#define WCM_RESULT_WMDISABLE        5               // ARM7 側で WM ライブラリが動作していない
#define WCM_RESULT_NOT_ENOUGH_MEM   6               // ワークバッファのサイズが足りない
#define WCM_RESULT_FATAL_ERROR      7               // 復旧不可能なエラー

// ライブラリ内部フェーズ
#define WCM_PHASE_NULL              0               // 初期化前
#define WCM_PHASE_WAIT              1               // 初期化直後の状態( 要求待ち )
#define WCM_PHASE_WAIT_TO_IDLE      2               // 初期化直後の状態 から 無線機能の起動シーケンス中
#define WCM_PHASE_IDLE              3               // 無線機能アイドル状態
#define WCM_PHASE_IDLE_TO_WAIT      4               // アイドル状態 から 無線機能の停止シーケンス中
#define WCM_PHASE_IDLE_TO_SEARCH    5               // アイドル状態 から AP 自動探索状態への移行シーケンス中
#define WCM_PHASE_SEARCH            6               // AP 自動探索状態
#define WCM_PHASE_SEARCH_TO_IDLE    7               // AP 自動探索状態 から アイドル状態への移行シーケンス中
#define WCM_PHASE_IDLE_TO_DCF       8               // アイドル状態 から AP への無線接続シーケンス中
#define WCM_PHASE_DCF               9               // AP と無線接続された DCF 通信可能状態
#define WCM_PHASE_DCF_TO_IDLE       10              // DCF 通信状態 から 無線接続を切断するシーケンス中
#define WCM_PHASE_FATAL_ERROR       11              // 復旧不可能なエラーが発生し、全ての処理が受け付けられない状態
#define WCM_PHASE_IRREGULAR         12              // 状態遷移シーケンスの途中で問題が発生した状態
#define WCM_PHASE_TERMINATING       13              // WCM ライブラリの強制停止シーケンス中

// 通知種別
#define WCM_NOTIFY_COMMON           0               // デフォルトの通知種別。
#define WCM_NOTIFY_STARTUP          1               // WCM_StartupAsync 関数の結果通知種別
#define WCM_NOTIFY_CLEANUP          2               // WCM_CleanupAsync 関数の結果通知種別
#define WCM_NOTIFY_BEGIN_SEARCH     3               // WCM_SearchAsync 関数、WCM_BeginSearchAsync 関数による探索開始処理の結果通知種別
#define WCM_NOTIFY_END_SEARCH       4               // WCM_SearchAsync 関数、WCM_EndSearchAsync 関数による探索停止処理の結果通知種別
#define WCM_NOTIFY_CONNECT          5               // WCM_ConnectAsync 関数の結果通知種別
#define WCM_NOTIFY_DISCONNECT       6               // WCM_DisconnectAsync 関数の結果通知種別・AP からの切断通知時の種別
#define WCM_NOTIFY_FOUND_AP         7               // 自動探索処理時の AP 発見時の通知種別
#define WCM_NOTIFY_SEARCH_AROUND    8               // 自動探索処理が一巡した時の通知種別 ( オプション設定時 )
#define WCM_NOTIFY_TERMINATE        9               // WCM_TerminateAsync 関数の結果通知種別

// AP 自動探索結果保持バッファのロック種別
#define WCM_APLIST_UNLOCK   0                       // 開錠 - 発見された AP 情報を元に自動的にリストが編集される
#define WCM_APLIST_LOCK     1                       // 施錠 - 発見された AP 情報はリストに反映されない

// AP 自動探索結果保持バッファの満杯時動作種別
#define WCM_APLIST_MODE_IGNORE      0               // 無視 - 発見された AP 情報は破棄される
#define WCM_APLIST_MODE_EXCHANGE    1               // 交換 - 最も古い AP 情報と入れ替えられる

// WEP 暗号化モード種別
#define WCM_WEPMODE_NONE        0                      // 暗号化なし
#define WCM_WEPMODE_40          1                      // RC4 (  40 ビット ) 暗号化モード
#define WCM_WEPMODE_104         2                      // RC4 ( 104 ビット ) 暗号化モード
#define WCM_WEPMODE_128         3                      // RC4 ( 128 ビット ) 暗号化モード

#ifdef SDK_TWL /* 以下の設定は TWL でのみ使用可能 */
#define WCM_WEPMODE_WPA_TKIP    NWM_WPAMODE_WPA_TKIP  // WPA-PSK(TKIP) 暗号化モード
#define WCM_WEPMODE_WPA2_TKIP   NWM_WPAMODE_WPA2_TKIP // WPA2-PSK(TKIP) 暗号化モード
#define WCM_WEPMODE_WPA_AES     NWM_WPAMODE_WPA_AES   // WPA-PSK(AES) 暗号化モード
#define WCM_WEPMODE_WPA2_AES    NWM_WPAMODE_WPA2_AES  // WPA2-PSK(AES) 暗号化モード
#endif                          

#define WCM_PRIVACYMODE_NONE        0
#define WCM_PRIVACYMODE_WEP40       WCM_WEPMODE_40
#define WCM_PRIVACYMODE_WEP104      WCM_WEPMODE_104
#define WCM_PRIVACYMODE_WEP128      WCM_WEPMODE_128
#define WCM_PRIVACYMODE_WEP         0x7f                   // 鍵長が不明な WEP。
                                                           // WCM ライブラリ内では使用できない定義

#ifdef SDK_TWL /* 以下の設定は TWL でのみ使用可能 */
#define WCM_PRIVACYMODE_WPA_TKIP    WCM_WEPMODE_WPA_TKIP
#define WCM_PRIVACYMODE_WPA2_TKIP   WCM_WEPMODE_WPA2_TKIP
#define WCM_PRIVACYMODE_WPA_AES     WCM_WEPMODE_WPA_AES
#define WCM_PRIVACYMODE_WPA2_AES    WCM_WEPMODE_WPA2_AES
#endif

// 各種制御用定数
#define WCM_CAM_LIFETIME            80              // CAM ライフタイム( 8 秒 )
#define WCM_BSSID_SIZE              WM_SIZE_BSSID   // BSS-ID のサイズ ( 6 バイト )
#define WCM_ESSID_SIZE              WM_SIZE_SSID    // ESS-ID の最大サイズ ( 32 バイト )
#define WCM_APLIST_SIZE             192             // AP 情報ブロックのデータ領域サイズ
#define WCM_APLIST_EX_SIZE          16              // AP 情報ブロックの制御領域サイズ
#define WCM_APLIST_BLOCK_SIZE       (WCM_APLIST_EX_SIZE + WCM_APLIST_SIZE)
#define WCM_WEP_SIZE                WM_SIZE_WEPKEY  // 80 バイト
#define WCM_WEP_EX_SIZE             (1 + 1 + 14)    // WCMWepDesc.mode + .keyId + .reserved
#define WCM_WEP_STRUCT_SIZE         (WCM_WEP_SIZE + WCM_WEP_EX_SIZE)    // == sizeof( WCMWepDesc )
#define WCM_DCF_RECV_EXCESS_SIZE    (sizeof(WMDcfRecvBuf) - 4)          // 44 バイト
#define WCM_DCF_SEND_EXCESS_SIZE    0   // 送信バッファにヘッダ格納用領域は不要
#define WCM_DCF_RECV_SIZE           (WM_DCF_MAX_SIZE + WCM_DCF_RECV_EXCESS_SIZE)
#define WCM_DCF_RECV_BUF_SIZE       (WCM_ROUNDUP32(WCM_DCF_RECV_SIZE))  // 一回の DCF 受信に必要なバッファサイズ
#define WCM_DCF_SEND_BUF_SIZE       (WCM_ROUNDUP32(WM_DCF_MAX_SIZE))    // 一回の DCF 送信に必要なバッファサイズ
#define WCM_ETC_BUF_SIZE            108     // 各種制御用変数群のサイズ
#define WCM_ADDITIONAL_RATESET      0x0003  // サポートレートセットに固定で追加( 1M or 2M )
#define WCM_CAMOUFLAGE_RATESET      0x0024  // サポートレートセットに偽装追加( 5.5M and 11M )

#ifdef SDK_TWL /* 以下の設定は TWL でのみ使用可能 */
#define WCM_WPA_PASSPHRASE_LENGTH_MAX  NWM_WPA_PASSPHRASE_LENGTH_MAX //WPA パスフレーズの最大 (64バイト)
#define WCM_WPA_PSK_SIZE  NWM_WPA_PSK_LENGTH //32バイト
#endif
/*
                                     * サポートレートセットは 16 ビットのビットフィールドで指定する形である。
                                     * 各ビットとレートの対応、及び実際にエレメントに格納される数値の対応は以下の通り。
                                     *
                                     *   1M   2M 5.5M   6M   9M  11M  12M  18M  24M  36M  48M  54M
                                     * {  2,   4,  11,  12,  18,  22,  24,  36,  48,  72,  96, 108, 0, 0, 0, 0 }
                                     */
#define WCM_AID_MIN 1                       // 最小の Auth-ID
#define WCM_AID_MAX 2007                    // 最大の Auth-ID
extern const u8 WCM_Bssid_Any[WCM_BSSID_SIZE];
extern const u8 WCM_Essid_Any[WCM_ESSID_SIZE];

// 任意の BSS-ID を示す定数
#define WCM_BSSID_ANY   ((void*)WCM_Bssid_Any)

// 任意の ESS-ID を示す定数
#define WCM_ESSID_ANY   ((void*)WCM_Essid_Any)

/*-------- オプション用ビット配置定義 --------*/

// スキャン時のチャンネル
#define WCM_OPTION_TEST_CHANNEL     0x00008000
#define WCM_OPTION_FILTER_CHANNEL   0x00003ffe
#define WCM_OPTION_MASK_CHANNEL     (WCM_OPTION_TEST_CHANNEL | WCM_OPTION_FILTER_CHANNEL)
#define WCM_OPTION_CHANNEL_1        (WCM_OPTION_TEST_CHANNEL | 0x00000002)
#define WCM_OPTION_CHANNEL_2        (WCM_OPTION_TEST_CHANNEL | 0x00000004)
#define WCM_OPTION_CHANNEL_3        (WCM_OPTION_TEST_CHANNEL | 0x00000008)
#define WCM_OPTION_CHANNEL_4        (WCM_OPTION_TEST_CHANNEL | 0x00000010)
#define WCM_OPTION_CHANNEL_5        (WCM_OPTION_TEST_CHANNEL | 0x00000020)
#define WCM_OPTION_CHANNEL_6        (WCM_OPTION_TEST_CHANNEL | 0x00000040)
#define WCM_OPTION_CHANNEL_7        (WCM_OPTION_TEST_CHANNEL | 0x00000080)
#define WCM_OPTION_CHANNEL_8        (WCM_OPTION_TEST_CHANNEL | 0x00000100)
#define WCM_OPTION_CHANNEL_9        (WCM_OPTION_TEST_CHANNEL | 0x00000200)
#define WCM_OPTION_CHANNEL_10       (WCM_OPTION_TEST_CHANNEL | 0x00000400)
#define WCM_OPTION_CHANNEL_11       (WCM_OPTION_TEST_CHANNEL | 0x00000800)
#define WCM_OPTION_CHANNEL_12       (WCM_OPTION_TEST_CHANNEL | 0x00001000)
#define WCM_OPTION_CHANNEL_13       (WCM_OPTION_TEST_CHANNEL | 0x00002000)
#define WCM_OPTION_CHANNEL_ALL      (WCM_OPTION_TEST_CHANNEL | WCM_OPTION_FILTER_CHANNEL)
#define WCM_OPTION_CHANNEL_RDC      (WCM_OPTION_CHANNEL_1 | WCM_OPTION_CHANNEL_7 | WCM_OPTION_CHANNEL_13)

// DCF 通信時のパワーセーブ設定
#define WCM_OPTION_TEST_POWER   0x00020000
#define WCM_OPTION_FILTER_POWER 0x00010000
#define WCM_OPTION_MASK_POWER   (WCM_OPTION_TEST_POWER | WCM_OPTION_FILTER_POWER)
#define WCM_OPTION_POWER_SAVE   (WCM_OPTION_TEST_POWER | 0x00000000)
#define WCM_OPTION_POWER_ACTIVE (WCM_OPTION_TEST_POWER | 0x00010000)

// AP との認証モード
#define WCM_OPTION_TEST_AUTH        0x00080000
#define WCM_OPTION_FILTER_AUTH      0x00040000
#define WCM_OPTION_MASK_AUTH        (WCM_OPTION_TEST_AUTH | WCM_OPTION_FILTER_AUTH)
#define WCM_OPTION_AUTH_OPENSYSTEM  (WCM_OPTION_TEST_AUTH | 0x00000000)
#define WCM_OPTION_AUTH_SHAREDKEY   (WCM_OPTION_TEST_AUTH | 0x00040000)

// スキャン種別
#define WCM_OPTION_TEST_SCANTYPE    0x00200000
#define WCM_OPTION_FILTER_SCANTYPE  0x00100000
#define WCM_OPTION_MASK_SCANTYPE    (WCM_OPTION_TEST_SCANTYPE | WCM_OPTION_FILTER_SCANTYPE)
#define WCM_OPTION_SCANTYPE_PASSIVE (WCM_OPTION_TEST_SCANTYPE | 0x00000000)
#define WCM_OPTION_SCANTYPE_ACTIVE  (WCM_OPTION_TEST_SCANTYPE | 0x00100000)

// スキャン一巡時の通知可否設定
#define WCM_OPTION_TEST_ROUNDSCAN   0x00800000
#define WCM_OPTION_FILTER_ROUNDSCAN 0x00400000
#define WCM_OPTION_MASK_ROUNDSCAN   (WCM_OPTION_TEST_ROUNDSCAN | WCM_OPTION_FILTER_ROUNDSCAN)
#define WCM_OPTION_ROUNDSCAN_IGNORE (WCM_OPTION_TEST_ROUNDSCAN | 0x00000000)
#define WCM_OPTION_ROUNDSCAN_NOTIFY (WCM_OPTION_TEST_ROUNDSCAN | 0x00400000)

// 使用する無線デバイス
#define WCM_DEVICEID_DEFAULT        0x00000000  // NITRO: WM, HYBRID: NWM, TWL: NWM
#define WCM_DEVICEID_WM             0x00000001
#define WCM_DEVICEID_NWM            0x00000002

#define WCM_DEBUGFLAG_DUMP_SEND     0x00000001
#define WCM_DEBUGFLAG_DUMP_RECV     0x00000002

#define WCM_SIZE_MACADDR            6
#define WCM_SIZE_ETHHDR             14

// 取得した BssDesc に含まれる Infomation Elements
#define WCM_IE_ID_EX_SUPPORT_RATE   50
#define WCM_IE_ID_ERP_INFORMATION_A  42
#define WCM_IE_ID_ERP_INFORMATION_B  47
#define WCM_IE_ID_COUNTRY           7
#define WCM_IE_ID_RSN               48

#define WCM_IE_ID_VENDOR_SPECIFIC   221 /* for WPA */

/*---------------------------------------------------------------------------*
    マクロ 定義
 *---------------------------------------------------------------------------*/
#define WCM_ROUNDUP32(_x_)      (((_x_) + 0x01f) &~(0x01f))
#define WCM_ROUNDDOWN32(_x_)    ((_x_) &~(0x01f))
#define WCM_ROUNDUP4(_x_)       (((_x_) + 0x03) &~(0x03))
#define WCM_ROUNDDOWN4(_x_)     ((_x_) &~(0x03))

/*---------------------------------------------------------------------------*
    型 定義
 *---------------------------------------------------------------------------*/

typedef WMLinkLevel             WCMLinkLevel;

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// WCM ライブラリ動作設定
typedef struct WCMConfig
{
    s32     dmano;      // WM が利用する DMA 番号 ( 0 ～ 3 )
    void*   pbdbuffer;  // AP 情報保持領域アドレス
    s32     nbdbuffer;  // AP 情報保持領域サイズ ( バイト単位 )
    s32     nbdmode;    // AP 情報保持方法 ( WCM_APLIST_MODE_* )
} WCMConfig;

// WCM ライブラリ通知内 詳細パラメータ共用体
typedef union WCMNoticeParameter
{
    s32     n;          // 整数型
    void*   p;          // ポインタ型
} WCMNoticeParameter;

// WCM ライブラリ通知コールバックに渡される構造体
typedef struct WCMNotice
{
    s16 notify; // 通知種別 ( WCM_NOTIFY_* )
    s16 result; // 処理結果 ( WCM_RESULT_* )
    WCMNoticeParameter  parameter[3];   // 各種詳細パラメータ
} WCMNotice;

// WCM 通知コールバック関数 型定義
typedef void (*WCMNotify) (WCMNotice * arg);

// WEPキー設定構造体 (NitroWiFi との互換性のために残されている)
typedef struct WCMWepDesc
{
    u8  mode;                   // WEP 暗号化モード ( WCM_WEPMODE_* )
    u8  keyId;                  // WEP キー ID ( 0 ～ 3 )
    u8  key[WM_SIZE_WEPKEY];    // WEP キーデータ列[ 20 * 4 バイト ]
} WCMWepDesc;

// 暗号化設定構造体 (WCMWepDesc の機能を含みキャストが可能)
typedef struct WCMPrivacyDesc
{
    u8  mode;                   // 暗号化モード ( WCM_PRIVACYMODE_* )
    union
    {
        u8  keyId;              // mode <= WCM_PRIVACYMODE_WEP128 の時
        u8  keyLen;             // mode >  WCM_PRIVACYMODE_WEP128 の時 (WPA)
    };
    u8  key[WM_SIZE_WEPKEY];
    u8  _reserved[2];
} WCMPrivacyDesc;

// WMBssDesc, NWMBssDesc の共通フィールドを持つ BssDesc
#pragma  warn_padding off
typedef struct WCMBssDesc
{
    u16             length;                    // 2
    u16             rssi;                      // 4
    u8              bssid[WM_SIZE_BSSID];      // 10
    u16             ssidLength;                // 12
    u8              ssid[WM_SIZE_SSID];        // 44
    u16             capaInfo;                  // 46
    struct
    {
        u16             basic;                 // 48
        u16             support;               // 50
    }
    rateSet;
    u16             beaconPeriod;              // 52
    u16             dtimPeriod;                // 54
    u16             channel;                   // 56
    u16             cfpPeriod;                 // 58
    u16             cfpMaxDuration;            // 60
    u16             gameInfoLength;            // 62
    union
    {
        u16             otherElementCount;
        u16             elementCount;
    };                                         // 64
    union
    {
        WMGameInfo      gameInfo;
        u16             elements[1];
    };
} WCMBssDesc;
#pragma  warn_padding reset

// WCMBssDesc に含まれる Information Element
typedef struct WCMInfoElement
{
    u8  identifier;
    u8  length;
    union
    {
        u8              information[1];
        struct
        {
            u8          oui[3];
            u8          ouiType;
            u8          data[1];
        } vender;
    };
} WCMInfoElement;

/*---------------------------------------------------------------------------*
    関数 定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WCM_Init
                WCM_InitEx

  Description:  WCM ライブラリを初期化する。同期関数。

  Arguments:    buf     -   WCM ライブラリが内部的に使用するワークバッファへの
                            ポインタを指定する。初期化完了後は、WCM_Finish 関数
                            によって WCM ライブラリを終了するまで別の用途に用い
                            てはいけない。
                len     -   ワークバッファのサイズを指定する。WCM_WORK_SIZE より
                            小さいサイズを指定するとこの関数は失敗する。
                deviceId-   使用する無線デバイスのIDを指定する。

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_NOT_ENOUGH_MEM
 *---------------------------------------------------------------------------*/
s32         WCM_Init(void* buf, s32 len);
s32         WCM_InitEx(void* buf, s32 len, u8 deviceId);

/*---------------------------------------------------------------------------*
  Name:         WCM_Finish

  Description:  WCMライブラリを終了する。同期関数。WCM_Init関数によって指定した
                ワーク用バッファは解放される。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32         WCM_Finish(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_StartupAsync

  Description:  無線機能の起動シーケンスを開始する。
                内部的には WAIT から IDLE へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には notify で指定したコールバック関数により非同期的な
                処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_STARTUP となる。

  Arguments:    config  -   WCM の動作設定となる構造体へのポインタを指定。
                notify  -   非同期的な処理結果を通知するコールバック関数を指定。
                            このコールバック関数は以後の非同期的な関数の結果
                            通知に共通して使用される。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_WMDISABLE ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_StartupAsync(WCMConfig* config, WCMNotify notify);

/*---------------------------------------------------------------------------*
  Name:         WCM_CleanupAsync

  Description:  無線機能の停止シーケンスを開始する。
                内部的には IDLE から WAIT へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_CLEANUP となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_CleanupAsync(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_SearchAsync

  Description:  AP 自動探索開始シーケンス、もしくは停止シーケンスを開始する。
                パラメータによって WCM_BeginSearchAsync 関数を呼び出すか、
                WCM_EndSearchAsync 関数を呼び出すか振り分けるだけの関数。

  Arguments:    bssid   -   探索する AP の BSSID を指定する。NULL を指定した
                            場合には、探索の停止を指示したものとみなされる。
                essid   -   探索する AP の ESSID を指定する。NULL を指定した
                            場合には、探索の停止を指示したものとみなされる。
                option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。0 を指定した場合は
                            変更は行われない。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_SearchAsync(const void* bssid, const void* essid, u32 option);

/*---------------------------------------------------------------------------*
  Name:         WCM_BeginSearchAsync

  Description:  AP 自動探索開始シーケンスを開始する。既に自動探索状態であった
                場合に、探索条件の変更のみ行うことも可能。
                内部的には IDLE から SEARCH へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_BEGIN_SEARCH となる。

  Arguments:    bssid   -   探索する AP の BSSID を指定する。NULL もしくは
                            WCM_BSSID_ANY を指定した場合は任意の BSSID を持つ
                            AP を探索する。
                essid   -   探索する AP の ESSID を指定する。NULL もしくは
                            WCM_ESSID_ANY を指定した場合は任意の ESSID を持つ
                            AP を探索する。
                option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。0 を指定した場合は
                            変更は行わない。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_BeginSearchAsync(const void* bssid, const void* essid, u32 option);

/*---------------------------------------------------------------------------*
  Name:         WCM_EndSearchAsync

  Description:  AP 探索停止シーケンスを開始する。
                内部的には SEARCH から IDLE へフェーズの移行処理が行われる。
                非同期関数であり、動機的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_END_SEARCH となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_EndSearchAsync(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_ConnectAsync

  Description:  AP との無線接続シーケンスを開始する。
                内部的には IDLE から DCF へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_CONNECT となる。

  Arguments:    bssDesc -   接続する AP の無線ネットワーク情報を指定する。
                            WCM_SearchAsync によって得られた情報がそのまま指定
                            されることを想定。
                wepDesc -   WCMWepDesc 型の WEP キーによる暗号化設定情報を指定
                            する。NULL の場合は、WEP 暗号化なしという設定になる。
                option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。0 を指定した場合は
                            変更は行われない。

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_ConnectAsync(void* bssDesc, void* wepDesc, u32 option);

/*---------------------------------------------------------------------------*
  Name:         WCM_DisconnectAsync

  Description:  AP との無線接続を切断するシーケンスを開始する。
                内部的には DCF から IDLE へフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_StartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_DISCONNECT となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_DisconnectAsync(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_TerminateAsync

  Description:  無線機能の強制終了シーケンスを開始する。
                別の非同期処理が実行されていない限りはどのフェーズからでも実行
                可能であり、内部的には WAIT へとフェーズの移行処理が行われる。
                非同期関数であり、同期的な返り値が WCM_RESULT_ACCEPT であった
                場合には WCM_SartupAsync 関数で指定したコールバック関数により
                非同期的な処理結果が通知される。
                非同期処理結果の通知種別は WCM_NOTIFY_TERMINATE となる。

  Arguments:    None.

  Returns:      s32     -   以下の内のいずれかの同期的な処理結果が返される。
                            WCM_RESULT_SUCCESS , WCM_RESULT_FAILURE ,
                            WCM_RESULT_PROGRESS , WCM_RESULT_REJECT ,
                            WCM_RESULT_FATAL_ERROR , WCM_RESULT_ACCEPT
 *---------------------------------------------------------------------------*/
s32         WCM_TerminateAsync(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_GetPhase

  Description:  WCM ライブラリの内部状態 ( フェーズ ) を取得する。同期関数。

  Arguments:    None.

  Returns:      s32     -   現在の WCM ライブラリのフェーズを返す。
 *---------------------------------------------------------------------------*/
s32         WCM_GetPhase(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_UpdateOption

  Description:  WCM ライブラリのオプション設定を更新する。

  Arguments:    option  -   オプション設定フォーマットに従った 32 ビットの
                            オプション変更用変数を指定する。
                            0 を指定した場合は何も更新を行わない動作になる。

  Returns:      u32     -   変更を行う前のオプション変数を返す。
 *---------------------------------------------------------------------------*/
u32         WCM_UpdateOption(u32 option);

/*---------------------------------------------------------------------------*
  Name:         WCM_GetAllowedChannel

  Description:  現在使用している無線デバイスの使用許可チャンネルリストを取得する。

  Arguments:    None.

  Returns:      u16    - 1～13chのうち、使用を許可されたチャンネル群のビットフィールドを返します。
                         最下位ビットから順に 1 チャンネル, 2 チャンネル……に対応します。
 *---------------------------------------------------------------------------*/
u16         WCM_GetAllowedChannel(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_SetChannelScanTime

  Description:  AP の自動探索時に１つのチャンネルをスキャンする時間を設定する。

  Arguments:    msec    -   １チャンネルをスキャンする時間を ms 単位で指定。
                            10 ～ 1000 までの間で設定可能だが、この範囲外の値を
                            指定すると自動探索はデフォルトの設定時間で探索を行う
                            ようになる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void        WCM_SetChannelScanTime(u16 msec);

/*---------------------------------------------------------------------------*
  Name:         WCM_LockApList

  Description:  AP 情報保持リストのロック状態を変更する。
    NOTICE:     ロック中は割り込みによって AP を発見した場合でも AP 情報保持
                リストの内容は変更されず、発見した AP 情報は破棄される点に注意。

  Arguments:    lock    -   WCM_APLIST_LOCK     : ロックする。
                            WCM_APLIST_UNLOCK   : ロックを解放する。
                            その他              : WCM_APLIST_LOCK とみなす。

  Returns:      s32     -   呼び出し前のロック状態を返す。
                            WCM_APLIST_LOCK     : ロックされていた。
                            WCM_APLIST_UNLOCK   : ロックは解放されていた。
 *---------------------------------------------------------------------------*/
s32         WCM_LockApList(s32 lock);

/*---------------------------------------------------------------------------*
  Name:         WCM_ClearApList

  Description:  AP 情報保持リストの内容を消去して初期状態に戻す。
                ロック状態如何に関わらず情報保持領域をクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void        WCM_ClearApList(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_CountApList

  Description:  AP 情報保持リスト内で保持されている AP 情報管理ブロック数を
                取得する。
    NOTICE:     WCM_LockApList関数にてロックされていない場合には関数呼出し後に
                割り込みによってブロック数は増減する可能性がある点に注意。

  Arguments:    None.

  Returns:      s32     -   AP 情報管理ブロック数を返す。
                            正常にブロック数を取得できない場合には 0 を返す。
 *---------------------------------------------------------------------------*/
s32         WCM_CountApList(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_PointApList
                WCM_PointApListEx

  Description:  AP 情報保持リスト内から、指定された index が割り当てられている
                AP 情報を抽出する。
    NOTICE:     WCM_LockApList関数によってロックされていない場合には関数呼出し
                後に割り込みによって取得したポインタが示すバッファの内容が書き
                換わる可能性がある点に注意。

  Arguments:    index       -   index を指定する。WCM_CountApList関数によって
                                得られる数 未満の index を指定する必要がある。

  Returns:      WMBssDesc*  -   抽出した AP 情報へのポインタを返す。
                WCMBssDesc*      抽出に失敗した場合は NULL を返す。
 *---------------------------------------------------------------------------*/
WMBssDesc*  WCM_PointApList(s32 index);
WCMBssDesc* WCM_PointApListEx(s32 index);

/*---------------------------------------------------------------------------*
  Name:         WCM_PointApListLinkLevel

  Description:  AP 情報保持リスト内から、指定された index が割り当てられている
                AP 情報が登録された際の電波強度を抽出する。

  Arguments:    index       -   index を指定する。WCM_CountApList関数によって
                                得られる数 未満の index を指定する必要がある。

  Returns:      WMLinkLevel -   抽出した AP 情報登録時の電波強度を返す。
                                抽出に失敗した場合は WM_LINK_LEVEL_0 を返す。
 *---------------------------------------------------------------------------*/
WMLinkLevel WCM_PointApListLinkLevel(s32 index);

/*---------------------------------------------------------------------------*
  Name:         WCM_CompareBssID

  Description:  ２つの BSSID を比較する。同期関数。

  Arguments:    a       -   比較対象の BSSID へのポインタを指定する。
                b       -   比較対象の BSSID へのポインタを指定する。

  Returns:      BOOL    -   ２つの BSSID が同じならば TRUE を返す。
                            異なる BSSID である場合は FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_CompareBssID(const u8* a, const u8* b);

/*---------------------------------------------------------------------------*
  Name:         WCM_CompareEssID

  Description:  ２つの ESSID を比較する。同期関数。

  Arguments:    a       -   比較対象の ESSID へのポインタを指定する。
                b       -   比較対象の ESSID へのポインタを指定する。

  Returns:      BOOL    -   ２つの ESSID が同じならば TRUE を返す。
                            異なる ESSID である場合は FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_CompareEssID(const u8* a, const u8* b);

/*---------------------------------------------------------------------------*
  Name:         WCM_GetLinkLevel

  Description:  DCF 通信時のリンク強度を取得する。同期関数。

  Arguments:    None.

  Returns:      WMLinkLevel -   4段階に評価したリンク強度を返す。
 *---------------------------------------------------------------------------*/
WMLinkLevel WCM_GetLinkLevel(void);

/*---------------------------------------------------------------------------*
  Name:         WCM_IsInfrastructureNetwork

  Description:  AP サーチ結果として得られた情報がインフラストラクチャネットワーク
                のものであるかどうかを調べる。

  Arguments:    bssDesc -   調べる AP 情報 ( ネットワーク情報 )。

  Returns:      BOOL    -   インフラストラクチャネットワークである場合に TRUE を、
                            そうでない場合に FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_IsInfrastructureNetwork(WMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_IsAdhocNetwork

  Description:  AP サーチ結果として得られた情報がアドホックネットワークのもの
                であるかどうかを調べる。

  Arguments:    bssDesc -   調べる AP 情報 ( ネットワーク情報 )。

  Returns:      BOOL    -   アドホックネットワークである場合に TRUE を、
                            そうでない場合に FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_IsAdhocNetwork(WMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_IsPrivacyNetwork

  Description:  AP サーチ結果として得られた情報が WEP セキュリティの適用された
                ネットワークのものであるかどうかを調べる。

  Arguments:    bssDesc -   調べる AP 情報 ( ネットワーク情報 )。

  Returns:      BOOL    -   WEP セキュリティの適用されたネットワークである場合に
                            TRUE を、そうでない場合に FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_IsPrivacyNetwork(WMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_IsShortPreambleNetwork

  Description:  AP サーチ結果として得られた情報がショートプリアンブルを使用した
                ネットワークのものであるかどうかを調べる。

  Arguments:    bssDesc -   調べる AP 情報 ( ネットワーク情報 )。

  Returns:      BOOL    -   ショートプリアンブルを使用したネットワークである場合に
                            TRUE を、そうでない場合に FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_IsShortPreambleNetwork(WMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_IsLongPreambleNetwork

  Description:  AP サーチ結果として得られた情報がロングプリアンブルを使用した
                ネットワークのものであるかどうかを調べる。

  Arguments:    bssDesc -   調べる AP 情報 ( ネットワーク情報 )。

  Returns:      BOOL    -   ロングプリアンブルを使用したネットワークである場合に
                            TRUE を、そうでない場合に FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_IsLongPreambleNetwork(WMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_IsEssidHidden

  Description:  AP サーチ結果として得られた情報が ESS-ID を隠蔽しているかどうか
                調べる。

  Arguments:    bssDesc -   調べる AP 情報 ( ネットワーク情報 )。

  Returns:      BOOL    -   ESS-ID が隠蔽されている場合に TRUE を返す。
                            有効な ESS-ID が公開されている場合には FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL        WCM_IsEssidHidden(WMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_GetWorkSize

  Description:  指定した無線デバイスを使用するために必要なワークサイズを取得
                する。

  Arguments:    workSize     - 取得したワークサイズを格納するポインタを指定
                deviceId     - 使用する無線デバイスの ID

  Returns:      BOOL         - 成功の可否。
                               NITRO 動作時に新無線のワークサイズを取得しよう
                               とした場合も失敗となる。
 *---------------------------------------------------------------------------*/
BOOL        WCM_GetWorkSize(s32* workSize, u8 deviceId);

/*---------------------------------------------------------------------------*
  Name:         WCM_CalcLinkLevel

  Description:  WCMBssDesc の中の RSSI 値から指定した無線デバイスにおける
                リンク強度を求める

  Arguments:    rssi         - WCMBssDesc の中の RSSI 値

  Returns:      WCMLinkLevel - 4段階に評価したリンク強度を返す。
 *---------------------------------------------------------------------------*/
WCMLinkLevel WCM_CalcLinkLevel(u8 rssi);

/* 暫定追加 */
WCMInfoElement* WCM_GetInfoElement(WCMBssDesc* bssDesc, u8 elementId);
WCMInfoElement* WCM_GetVenderInfoElement(WCMBssDesc* bssDesc, const u8 oui[3]);

/*---------------------------------------------------------------------------*
  Name:         WCMi_BssDescCopy

  Description:  WCMBssDesc を不要な Infomation Elements を除いてコピーする。

  Arguments:    srcBssDesc  - コピー元の AP 情報
                destBssDesc - コピー先の AP 情報

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WCMi_BssDescCopy(WCMBssDesc* srcBssDesc, WCMBssDesc* destBssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCMi_GetGameInfo

  Description:  WCMBssDesc の中身を調査して MP 親機のビーコンだった場合 WMGameInfo を取得する

  Arguments:    bssDesc  - AP 情報

  Returns:      WMGameInfo
 *---------------------------------------------------------------------------*/
WMGameInfo* WCMi_GetGameInfo(const WCMBssDesc* bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WCM_GetWPAPSK

  Description:  WPAパスフレーズと接続対象の SSID から WPA-PSK(事前認証鍵)を生成
                ※ PSK の計算は接続設定時にのみ行い、接続には計算済みの PSK を使用する

  Arguments:    passphrase   - WPA パスフレーズ、8 文字以上 63 文字以下の ASCII 文字列
                ssid         - SSID
                ssidlen      - SSID の長さ(32未満の必要あり)
                psk          - 生成された PSK の格納先。32Byte

  Returns:      BOOL         - 成功の可否。
                               パスフレーズ長が異なる場合も失敗となる
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
BOOL        WCM_GetWPAPSK(const u8 passphrase[WCM_WPA_PASSPHRASE_LENGTH_MAX],
                          const u8 *ssid, u8 ssidlen, u8 psk[WCM_WPA_PSK_SIZE]);
#endif


/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* NITROWIFI_WCM_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
