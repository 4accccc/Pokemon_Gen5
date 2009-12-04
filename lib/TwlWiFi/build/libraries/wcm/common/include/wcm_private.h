/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - WCM - libraries
  File:     wcm_private.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 1094 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITROWIFI_WCM_PRIVATE_H_
#define NITROWIFI_WCM_PRIVATE_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <nitroWiFi/wcm.h>
#include <nitro/os.h>

#include "wcm_wmif.h"
#ifdef SDK_TWL
#include "wcm_nwmif.h"
#endif

#include "wcm_cpsif.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/

// リセット重複呼び出し管理フラグの状態
#define WCM_RESETTING_OFF   0
#define WCM_RESETTING_ON    1

// AP 情報保持リスト内の各ブロックの使用状態
#define WCM_APLIST_BLOCK_EMPTY  0
#define WCM_APLIST_BLOCK_OCCUPY 1

// Keep Alive 用 NULL パケット送信間隔(秒)
#define WCM_KEEP_ALIVE_SPAN ((OSTick) 70)

// デバッグ表示有無設定
#ifdef SDK_FINALROM
#define WCM_DEBUG   0
#else
#define WCM_DEBUG   1
#endif

// 接続時の失敗コード定義
#define WCM_CONNECT_STATUSCODE_ILLEGAL_RATESET  18

#if WCM_DEBUG
#define WCMi_Printf     OS_TPrintf("[WCM] %s(%d) Report: ", __FILE__, __LINE__); \
    OS_TPrintf
#define WCMi_Warning    OS_TPrintf("[WCM] %s(%d) Warning: ", __FILE__, __LINE__); \
    OS_TPrintf
#else
#define WCMi_Printf(...)    ((void)0)
#define WCMi_Warning(...)   ((void)0)
#endif

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// WCM 内部ワーク変数群をまとめた構造体
// TWL ビルドの時はサイズが増える。NITRO ビルドの場合、構造体メンバの順番は違う
// が必要なワークサイズそのものは NITRO-WiFi と同じ。
typedef struct WCMWork
{
    u8              sendBuf[WCM_DCF_SEND_BUF_SIZE];
    u8              recvBuf[WCM_DCF_RECV_BUF_SIZE * 2];
    
    // key を 32 バイト整合にするため、WCMPrivacyDesc と同じで
    // 変数の順番を変えただけの構造体を使う
    struct
    {
        u8  key[WM_SIZE_WEPKEY];
        u8  mode;                   // 暗号化モード ( WCM_PRIVACYMODE_* )
        union
        {
            u8  keyId;              // mode <= WCM_PRIVACYMODE_128 の時
            u8  keyLen;             // mode >  WCM_PRIVACYMODE_128 の時 (WPA)
        };
        u8  _reserved[2];
    } privacyDesc;
    
    OSAlarm         keepAliveAlarm;
    
    WCMConfig       config;
    WCMNotify       notify;
    
    u32             option;
    u16             maxScanTime;
    s16             notifyId;
    
    u8              apListLock;
    u8              resetting;
    
    u8              phase;
    u8              deviceId;
    
    // ここから 32 バイト整合
    WCMBssDesc      bssDesc;
    union
    {
        // 旧無線用
        struct
        {
            u8              work[WCM_ROUNDUP32( WM_SYSTEM_BUF_SIZE )];
            WMScanExParam   scanExParam;
            // 共通パラメータにしたいが、構造体サイズの制限によりこの場所に。
            u32             scanCount;
            u16             authId;
            // WL の結果コード
            u16             wlStatusOnConnectFail;
        } wm;
        
#ifdef SDK_TWL
        // 新無線用
        // deviceId == WCM_DEVICEID_NWM の時のみアクセスできる
        // それ以外はメモリが確保されていないかもしれない
        struct
        {
            u8              work[WCM_ROUNDUP32( NWM_SYSTEM_BUF_SIZE )];
            NWMScanExParam  scanExParam;
            u32             scanCount;
            u16             authId;
            // 鍵間違いをハンドリングするための疑似 WL の結果コード
            u16             pseudoStatusOnConnectFail;
        } nwm;
#endif
    };
} WCMWork;

// WCM CPS-IF 内部状態管理用構造体
typedef struct WCMCpsifWork
{
    u8  initialized;                // ワーク変数初期化確認用
    u8  reserved[3];                // パディング  (OSThreadQueue が4byte以上に)
    OSThreadQueue   sendQ;          // DCF 送信完了待ちブロック用 Thread Queue
    OSMutex         sendMutex;      // 複数スレッドによる送信の排他制御用 Mutex
    u16             sendResult;     // DCF 送信の非同期的な処理結果退避エリア
    u16             padding1;
    WCMRecvInd      recvCallback;   // DCF 受信コールバック先退避エリア
    WCMSendInd      sendCallback;   // DCF 送信コールバック先退避エリア
    u8              macAddress[6];
    u16             padding2;
} WCMCpsifWork;

// AP 情報保持リスト内のブロック構造体
typedef struct WCMApList
{
    u8      state;
    u8      reserved;
    u16     linkLevel;
    u32     index;
    void*   previous;
    void*   next;
    u32     data[WCM_ROUNDUP4( WCM_APLIST_SIZE ) / sizeof(u32) ];
} WCMApList;

// AP 情報保持リストの管理用ヘッダ構造体
typedef struct WCMApListHeader
{
    u32         count;
    WCMApList*  head;
    WCMApList*  tail;

} WCMApListHeader;

/*---------------------------------------------------------------------------*
    関数 定義
 *---------------------------------------------------------------------------*/

// WCM ライブラリの内部管理構造体へのポインタを取得
WCMWork*    WCMi_GetSystemWork(void);

// AP 情報発見時に呼び出される 保持リストへのエントリー追加(更新)要求
//void        WCMi_EntryApList(WMBssDesc* bssDesc, u16 linkLevel);
void        WCMi_EntryApList(WCMBssDesc* bssDesc, u16 linkLevel);

// Keep Alive 用アラームをリセットする関数
void        WCMi_ResetKeepAliveAlarm(void);

// DCF データ受信時に受信電波強度を一旦退避する関数
void        WCMi_ShelterRssi(u8 rssi);
u8          WCMi_GetRssiAverage(void);

// WCM ライブラリの内部状態 ( フェーズ ) を変更する関数
void        WCMi_SetPhase(u8 phase);

// アプリケーションへのコールバックによる通知を行う関数
void        WCMi_Notify(s16 result, void* para0, void* para1, s32 para2);
void        WCMi_NotifyEx(s16 notify, s16 result, void* para0, void* para1, s32 para2);

// 次にスキャンすべきチャンネルを決定する関数
u16         WCMi_GetNextScanChannel(u16 channel);

// メモリのダンプ関数
void        WCMi_DumpMemory(const u8* pDump, u32 len);
// デバッグ用フラグ
extern u32  WCM_DebugFlags;


/*
   cpsif.c
 */
// WCM_Init 関数から呼び出される CPS インターフェース初期化関数
void        WCMi_InitCpsif(void);

// DCF データ受信時に呼び出される CPS インターフェースへのコールバック関数
void        WCMi_CpsifRecvCallback(u8 *srcAdrs, u8 *destAdrs, u8 *data, s32 length);

// Keep Alive 用 NULL パケットを送信する関数
void        WCMi_CpsifSendNullPacket(void);

WCMCpsifWork* WCMi_GetCpsifWork(void);
BOOL        WCMi_CpsifTryLockMutexInIRQ(OSMutex* mutex);
void        WCMi_CpsifUnlockMutexInIRQ(OSMutex* mutex);
void        WCMi_CpsifSetMacAddress(u8* macAddress);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* NITROWIFI_WCM_PRIVATE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
