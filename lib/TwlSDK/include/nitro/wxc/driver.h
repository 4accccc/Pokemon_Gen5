/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     driver.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_WXC_DRIVER_H_
#define	NITRO_WXC_DRIVER_H_

#include    <nitro.h>


/*****************************************************************************/
/* constant */

/* ワイヤレス基本設定 */
#define     WXC_DEFAULT_PORT             4
#define     WXC_DEFAULT_PORT_PRIO        2

/* 以前は 150[ms] だったが現在はビーコン間隔(90[ms]) + 20[ms] に */
#define     WXC_SCAN_TIME_MAX            (WXC_BEACON_PERIOD + 20)

#define     WXC_MAX_RATIO                100

/* ワイヤレスドライバからのイベントコールバック種別 */
typedef enum
{
    /* ワイヤレス終了 (引数は常に NULL) */
    WXC_DRIVER_EVENT_STATE_END,
    /* STOP ステートに遷移完了 (引数は常に NULL) */
    WXC_DRIVER_EVENT_STATE_IDLE,
    /* IDLE ステートに遷移完了 (引数は常に NULL) */
    WXC_DRIVER_EVENT_STATE_STOP,
    /* MP_PARENT ステートに遷移完了 (引数は常に NULL) */
    WXC_DRIVER_EVENT_STATE_PARENT,
    /* MP_CHILD ステートに遷移完了  (引数は常に NULL) */
    WXC_DRIVER_EVENT_STATE_CHILD,
    /* ビーコン更新タイミング (引数は WMParentParam ポインタ) */
    WXC_DRIVER_EVENT_BEACON_SEND,
    /* ビーコン検出タイミング (引数は WMBssDesc ポインタ) */
    WXC_DRIVER_EVENT_BEACON_RECV,
    /* MP パケット送信 (引数は WXCPacketInfo ポインタ) */
    WXC_DRIVER_EVENT_DATA_SEND,
    /* MP パケット受信 (引数は const WXCPacketInfo ポインタ) */
    WXC_DRIVER_EVENT_DATA_RECV,
    /* 接続前の通知 (引数は WMBssDesc ポインタ) */
    WXC_DRIVER_EVENT_CONNECTING,
    /* 接続の検知 (引数は) */
    WXC_DRIVER_EVENT_CONNECTED,
    /* 切断の検知 (引数は WMStartParentCallback ポインタ) */
    WXC_DRIVER_EVENT_DISCONNECTED,

    WXC_DRIVER_EVENT_MAX
}
WXCDriverEvent;

/* ドライバの内部状態 */
typedef enum WXCDriverState
{
    WXC_DRIVER_STATE_END,              /* 初期化前 (driver = NULL) */
    WXC_DRIVER_STATE_BUSY,             /* 状態遷移中 */
    WXC_DRIVER_STATE_STOP,             /* STOP ステートで安定中 */
    WXC_DRIVER_STATE_IDLE,             /* IDLE ステートで安定中 */
    WXC_DRIVER_STATE_PARENT,           /* MP_PARENT ステートで安定中 */
    WXC_DRIVER_STATE_CHILD,            /* MP_CHILD ステートで安定中 */
    WXC_DRIVER_STATE_ERROR             /* 自動復帰手段の無いエラー */
}
WXCDriverState;


/*****************************************************************************/
/* declaration */

/*---------------------------------------------------------------------------*
  Name:         WXCDriverEventFunc

  Description:  ワイヤレスドライバからのイベントコールバックプロトタイプ

  Arguments:    event         通知するイベント
                arg           イベントごとに定義された引数

  Returns:      イベントごとに定義された u32 のイベント結果値.
 *---------------------------------------------------------------------------*/
typedef u32 (*WXCDriverEventFunc) (WXCDriverEvent event, void *arg);


/* WXCライブラリ内部ワイヤレスドライバ構造体 */
typedef struct WXCDriverWork
{

    /* WM 内部ワーク */
    u8      wm_work[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
    u8      mp_send_work[WM_SIZE_MP_PARENT_SEND_BUFFER(WM_SIZE_MP_DATA_MAX, FALSE)]
        ATTRIBUTE_ALIGN(32);
    u8     
        mp_recv_work[WM_SIZE_MP_PARENT_RECEIVE_BUFFER(WM_SIZE_MP_DATA_MAX, WM_NUM_MAX_CHILD, FALSE)]
        ATTRIBUTE_ALIGN(32);
    u8      current_send_buf[WM_SIZE_MP_DATA_MAX] ATTRIBUTE_ALIGN(32);
    u16     wm_dma;                    /* WM 用 DMA チャンネル */
    u16     current_channel;           /* 現在のチャンネル(Measure/Start) */
    u16     own_aid;                   /* 自身の AID */
    u16     peer_bitmap;               /* 接続相手のビットマップ */
    u16     send_size_max;             /* MP 送信サイズ */
    u16     recv_size_max;             /* MP 送信サイズ */
    BOOL    send_busy;                 /* 前回の MP 完了待ち　 */

    /* 状態遷移情報 */
    WXCDriverState state;
    WXCDriverState target;
    WXCDriverEventFunc callback;

    /* 親機制御情報 */
    WMParentParam *parent_param;
    BOOL    need_measure_channel;
    int     measure_ratio_min;

    /* 子機制御情報 */
    int     scan_found_num;
    u8      padding1[20];
    WMBssDesc target_bss[1] ATTRIBUTE_ALIGN(32);
    u8      scan_buf[WM_SIZE_SCAN_EX_BUF] ATTRIBUTE_ALIGN(32);
    WMScanExParam scan_param[1] ATTRIBUTE_ALIGN(32);
    u8      ssid[24];
    u8      padding2[4];

}
WXCDriverWork;


/*****************************************************************************/
/* function */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         WXC_InitDriver

  Description:  ワイヤレスを初期化して IDLE へ移行開始する.

  Arguments:    driver        内部ワークメモリバッファとして使用する
                              WXCDriverWork 構造体へのポインタ
                              32バイトアラインメントされている必要がある
                pp            親機設定
                func          イベント通知コールバック
                dma           ワイヤレスに割り当てる DMA チャンネル

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InitDriver(WXCDriverWork * driver, WMParentParam *pp, WXCDriverEventFunc func, u32 dma);

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverTarget

  Description:  特定の状態を目標に遷移を開始する.

  Arguments:    driver        WXCDriverWork 構造体
                target        遷移先のステート

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_SetDriverTarget(WXCDriverWork * driver, WXCDriverState target);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetDriverState

  Description:  現在の遷移状態を取得する.

  Arguments:    None.

  Returns:      現在の遷移状態.
 *---------------------------------------------------------------------------*/
static inline WXCDriverState WXC_GetDriverState(const WXCDriverWork * driver)
{
    return driver->state;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverSsid

  Description:  接続時の SSID を設定する.

  Arguments:    driver        WXCDriverWork 構造体.
                buffer        設定すSSIDデータ.
                length        SSIDデータ長.
                              WM_SIZE_CHILD_SSID 以下である必要がある.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetDriverSsid(WXCDriverWork * driver, const void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetDriverTarget

  Description:  現在の目標遷移先状態を取得する.

  Arguments:    None.

  Returns:      現在の目標遷移先状態.
 *---------------------------------------------------------------------------*/
static inline WXCDriverState WXC_GetDriverTarget(const WXCDriverWork * driver)
{
    return driver->target;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsParentMode

  Description:  現在のワイヤレス状態が親機モードか判定.
                WXC_STATE_ACTIVE の状態に限り有効.

  Arguments:    None.

  Returns:      ワイヤレス状態が親機モードなら TRUE, 子機モードなら FALSE.
 *---------------------------------------------------------------------------*/
static inline BOOL WXCi_IsParentMode(const WXCDriverWork * driver)
{
    return (driver->state == WXC_DRIVER_STATE_PARENT);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetParentBssDesc

  Description:  接続先の WMBssDesc 構造体を取得. (子機の場合のみ有効)

  Arguments:    None.

  Returns:      接続先の WMBssDesc 構造体
 *---------------------------------------------------------------------------*/
static inline const WMBssDesc *WXCi_GetParentBssDesc(const WXCDriverWork * driver)
{
    return driver->target_bss;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_Stop

  Description:  ワイヤレス状態を STOP へ移行開始する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_Stop(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_STOP);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartParent

  Description:  ワイヤレス状態を IDLE から MP_PARENT へ移行開始する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_StartParent(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_PARENT);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartChild

  Description:  ワイヤレス状態を IDLE から MP_CHILD へ移行開始する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_StartChild(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_CHILD);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_Reset

  Description:  ワイヤレス状態をリセットして IDLE へ移行開始する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_Reset(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_IDLE);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_End

  Description:  ワイヤレスを終了する.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_End(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_END);
}


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_DRIVER_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
