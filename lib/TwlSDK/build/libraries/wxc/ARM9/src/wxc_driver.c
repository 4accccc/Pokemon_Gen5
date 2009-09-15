/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_driver.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9661 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc/common.h>
#include <nitro/wxc/driver.h>


/*****************************************************************************/
/* declaration */

/* エラーチェック関数群 */
static void WXCi_RecoverWmApiFailure(WXCDriverWork *, WMApiid id, WMErrCode err);
static BOOL WXCi_CheckWmApiResult(WXCDriverWork * driver, WMApiid id, WMErrCode err);
static BOOL WXCi_CheckWmCallbackResult(WXCDriverWork *, void *arg);
static void WXCi_ErrorQuit(WXCDriverWork *);

/*
 * ワイヤレスステート間の連続遷移プロシージャ.
 * 最初に引数 NULL で呼び出し, 次々と同じ関数へコールバックする.
 * 目標の状態に達したらイベント通知コールバック発生.
 *
 * 備考:
 *   これらの関数は最終的にスレッド化を容易にするよう設計されているので,
 *   メンテナンスの際にこれら関数内の個々の WM 関数呼び出しを分割して
 *   かつ StateIn- StateOut- に分けるような変更をすべきでない.
 */
static void WXCi_InitProc(void *arg);  /* (end) -> STOP             */
static void WXCi_StartProc(void *arg); /* STOP -> IDLE              */
static void WXCi_StopProc(void *arg);  /* IDLE -> STOP              */
static void WXCi_EndProc(void *arg);   /* STOP  -> (end)            */
static void WXCi_ResetProc(void *arg); /* (any) -> IDLE             */
static void WXCi_StartParentProc(void *arg);    /* IDLE  -> MP_PARENT        */
static void WXCi_StartChildProc(void *arg);     /* IDLE  -> MP_CHILD         */
static void WXCi_ScanProc(void *arg);  /* IDLE -> SCAN -> IDLE      */
static void WXCi_MeasureProc(void *arg);        /* IDLE -> (measure) -> IDLE */

/* 状態遷移制御 */
static void WXCi_OnStateChanged(WXCDriverWork *, WXCDriverState state, void *arg);

/* その他のコールバックおよびインジケート */
static void WXCi_IndicateCallback(void *arg);
static void WXCi_PortCallback(void *arg);
static void WXCi_MPSendCallback(void *arg);
static void WXCi_ParentIndicate(void *arg);
static void WXCi_ChildIndicate(void *arg);

/*****************************************************************************/
/* constant */

/* SDK3.0RC以降では、自身の切断でもインジケートが発生するよう変更された */
#define VERSION_TO_INT(major, minor, relstep) \
    (((major) << 24) | ((minor) << 16) | ((relstep) << 0))
#if VERSION_TO_INT(SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_RELSTEP) < VERSION_TO_INT(3, 0, 20100)
#define WM_STATECODE_DISCONNECTED_FROM_MYSELF ((WMStateCode)26)
#endif


/*****************************************************************************/
/* variable */

/*
 * ワイヤレスドライバのワークメモリ.
 * ・これは WM コールバック内でのみ this として使用する目的で用意されている.
 *   (通常, 可能な限り WXCDriverWork を引数にとり呼び出し対象を明示すること)
 * ・グローバルポインタを直接参照すると半volatile扱いになり効率が悪いため,
 *   このポインタを多用する関数内では一旦ローカル変数にすくう方が良い.
 */
static WXCDriverWork *work;


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXCi_ErrorQuit

  Description:  エラーを検出した際のリセット処理.

  Arguments:    driver        WXCDriverWork 構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ErrorQuit(WXCDriverWork * driver)
{
    /* たいてい異常終了処理になるので BUSY を無視する */
    if (driver->state == WXC_DRIVER_STATE_BUSY)
    {
        driver->state = driver->target;
    }
    WXCi_End(driver);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_RecoverWmApiFailure

  Description:  WM 関数のエラーから状態回復の試行

  Arguments:    driver        WXCDriverWork 構造体
                id            WM 関数の種類
                err           関数から返されたエラーコード

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_RecoverWmApiFailure(WXCDriverWork * driver, WMApiid id, WMErrCode err)
{
    /* 念のためここでも成功判定 */
    if (err == WM_ERRCODE_SUCCESS)
    {
        return;
    }

    switch (id)
    {
    default:
        /* WXC で使用していないはずの関数 */
        OS_TWarning("WXC library error (unknown WM API : %d)\n", id);
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_ENABLE:
    case WM_APIID_POWER_ON:
    case WM_APIID_POWER_OFF:
    case WM_APIID_DISABLE:
    case WM_APIID_RESET:
        /* 初期化にすら失敗した場合は終了 */
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_MEASURE_CHANNEL:
    case WM_APIID_SET_P_PARAM:
    case WM_APIID_START_PARENT:
    case WM_APIID_START_MP:
    case WM_APIID_START_SCAN_EX:
    case WM_APIID_END_SCAN:
    case WM_APIID_DISCONNECT:
        /* 主要な状態遷移関数で予期せぬ失敗が発生した場合は終了 */
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_START_CONNECT:
        /* StartConnect 完了時の接続失敗はありえるので再度 IDLE へ */
        if ((err == WM_ERRCODE_FAILED) ||
            (err == WM_ERRCODE_NO_ENTRY) || (err == WM_ERRCODE_OVER_MAX_ENTRY))
        {
            /* この一瞬だけ BUSY を解除する */
            driver->state = WXC_DRIVER_STATE_CHILD;
            WXCi_Reset(driver);
        }
        /* それ以外は致命的エラーと判断して終了 */
        else
        {
            WXCi_ErrorQuit(driver);
        }
        break;

    case WM_APIID_INDICATION:
        /* FIFO_ERROR, FLASH_ERROR 等の致命的な通知を受信した場合は終了 */
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_SET_MP_DATA:
    case WM_APIID_PORT_SEND:
        /*
         * SetMPData が失敗するケース:
         *   ILLEGAL_STATE,INVALID_PARAM,FIFO_ERROR ... 常に.
         *   NO_CHILD ... 呼び出し時.
         *   SEND_QUEUE_FULL,SEND_FAILED ... コールバック時.
         * 再試行によって解決する問題なので無視する.
         */
        break;

    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CheckWmApiResult

  Description:  WM 関数の呼び出し返り値を判定

  Arguments:    driver        WXCDriverWork 構造体
                id            WM 関数の種類
                err           関数から返されたエラーコード

  Returns:      WM_ERRCODE_SUCCESS なら TRUE,
                そうでなければエラー出力して FALSE.
 *---------------------------------------------------------------------------*/
static BOOL WXCi_CheckWmApiResult(WXCDriverWork * driver, WMApiid id, WMErrCode err)
{
    BOOL    ret = WXC_CheckWmApiResult(id, err);
    /* エラーだった場合は API ごとの復帰処理 */
    if (!ret)
    {
        WXCi_RecoverWmApiFailure(driver, id, err);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CheckWmCallbackResult

  Description:  WM コールバックの返り値を判定

  Arguments:    driver        WXCDriverWork 構造体
                arg           関数から返された引数

  Returns:      WM_ERRCODE_SUCCESS なら TRUE,
                そうでなければエラー出力して FALSE.
 *---------------------------------------------------------------------------*/
static BOOL WXCi_CheckWmCallbackResult(WXCDriverWork * driver, void *arg)
{
    BOOL    ret = WXC_CheckWmCallbackResult(arg);
    /* エラーだった場合は API ごとの復帰処理 */
    if (!ret)
    {
        const WMCallback *cb = (const WMCallback *)arg;
        WXCi_RecoverWmApiFailure(driver, (WMApiid)cb->apiid, (WMErrCode)cb->errcode);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CallDriverEvent

  Description:  ワイヤレスドライバイベントを通知

  Arguments:    driver        WXCDriverWork 構造体
                event         通知するイベント
                arg           イベントごとに定義された引数

  Returns:      イベントごとに定義された u32 のイベント結果値.
 *---------------------------------------------------------------------------*/
static inline u32 WXCi_CallDriverEvent(WXCDriverWork * driver, WXCDriverEvent event, void *arg)
{
    u32     result = 0;
    if (driver->callback)
    {
        result = (*driver->callback) (event, arg);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CallSendEvent

  Description:  データ送信タイミングイベントのコールバック関数。

  Arguments:    driver        WXCDriverWork 構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_CallSendEvent(WXCDriverWork * driver)
{
    if (!driver->send_busy)
    {
        /*
         * 送信パケットバッファ情報の設定.
         * TODO:
         * これは MP 通信確立時に1回だけ保持したほうが効率的?
         * 実際 wxc_api.c に全く同じ処理がある.
         */
        const u16 max_length = (driver->own_aid == 0) ?
            driver->parent_param->parentMaxSize : driver->target_bss->gameInfo.childMaxSize;
        WXCPacketInfo packet;
        packet.bitmap = driver->peer_bitmap;
        packet.length = max_length;
        packet.buffer = driver->current_send_buf;
        /* 送信データをユーザに設定させる */
        (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_DATA_SEND, &packet);
        /* データ送信 */
        if (packet.length <= max_length)
        {
            WMErrCode ret;
            ret = WM_SetMPDataToPort(WXCi_MPSendCallback,
                                     (u16 *)packet.buffer, packet.length,
                                     packet.bitmap, WXC_DEFAULT_PORT, WXC_DEFAULT_PORT_PRIO);
            driver->send_busy = WXCi_CheckWmApiResult(driver, WM_APIID_SET_MP_DATA, ret);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_OnStateChanged

  Description:  指定のステートに安定した時の処理

  Arguments:    driver        WXCDriverWork 構造体
                state         遷移が確定したステート
                arg           イベントごとの引数

  Returns:      イベントごとに定義された u32 のイベント結果値.
 *---------------------------------------------------------------------------*/
static void WXCi_OnStateChanged(WXCDriverWork * driver, WXCDriverState state, void *arg)
{
    driver->state = state;

    /* 現在の状態が遷移対象であればユーザに通知 */
    if (driver->target == state)
    {
        switch (state)
        {
        case WXC_DRIVER_STATE_END:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_END, NULL);
            break;

        case WXC_DRIVER_STATE_STOP:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_STOP, NULL);
            break;

        case WXC_DRIVER_STATE_IDLE:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_IDLE, NULL);
            break;

        case WXC_DRIVER_STATE_PARENT:
            driver->send_busy = FALSE;
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_PARENT, NULL);
            break;

        case WXC_DRIVER_STATE_CHILD:
            driver->send_busy = FALSE;
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_CHILD, NULL);
            /* この場で親との接続を通知 */
            driver->peer_bitmap |= (u16)(1 << 0);
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_CONNECTED, arg);
            WXCi_CallSendEvent(driver);
            break;
        }
    }

    /*
     * そうでなければ継続遷移.
     * +-----+      +------+       +------+                     +--------+
     * |     > Init |      > Start >      > Measure/StartParent > PARENT |
     * |     |      |      |       |      <               Reset <        |
     * |     |      |      |       |      |                     +--------+
     * | END |      | STOP |       | IDLE > Scan/StartChild     > CHILD  |
     * |     |      |      |       |      <               Reset <        |
     * |     |      |      <  Stop <      |                     +--------+
     * |     <  End <      <   End <      |
     * +-----+      +------+       +------+
     */
    else
    {
        switch (state)
        {

        case WXC_DRIVER_STATE_END:
            WXCi_InitProc(NULL);
            break;

        case WXC_DRIVER_STATE_STOP:
            switch (driver->target)
            {
            case WXC_DRIVER_STATE_END:
                WXCi_EndProc(NULL);
                break;
            case WXC_DRIVER_STATE_IDLE:
            case WXC_DRIVER_STATE_PARENT:
            case WXC_DRIVER_STATE_CHILD:
                WXCi_StartProc(NULL);
                break;
            }
            break;

        case WXC_DRIVER_STATE_IDLE:
            switch (driver->target)
            {
            case WXC_DRIVER_STATE_END:
            case WXC_DRIVER_STATE_STOP:
                WXCi_StopProc(NULL);
                break;
            case WXC_DRIVER_STATE_PARENT:
                driver->need_measure_channel = TRUE;
                if (driver->need_measure_channel)
                {
                    WXCi_MeasureProc(NULL);
                }
                break;
            case WXC_DRIVER_STATE_CHILD:
                WXCi_ScanProc(NULL);
                break;
            }
            break;

        case WXC_DRIVER_STATE_PARENT:
        case WXC_DRIVER_STATE_CHILD:
            WXCi_ResetProc(NULL);
            break;

        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_MPSendCallback

  Description:  WM_SetMPDataに対するコールバック関数。

  Arguments:    arg - コールバック構造体へのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_MPSendCallback(void *arg)
{
    WXCDriverWork *const driver = work;

    (void)WXCi_CheckWmCallbackResult(driver, arg);
    driver->send_busy = FALSE;
    if (driver->peer_bitmap != 0)
    {
        WXCi_CallSendEvent(driver);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IndicateCallback

  Description:  Indicate発生時に呼び出されるコールバック関数。

  Arguments:    arg - コールバック構造体へのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_IndicateCallback(void *arg)
{
    /* TODO: これは CheckError に統合できないか? */
    WXCDriverWork *const driver = work;
    WMIndCallback *cb = (WMIndCallback *)arg;
    if (cb->errcode == WM_ERRCODE_FIFO_ERROR)
    {
        WXC_DRIVER_LOG("WM_ERRCODE_FIFO_ERROR Indication!\n");
        /* 復旧不能なエラー */
        driver->target = WXC_DRIVER_STATE_ERROR;
        driver->state = WXC_DRIVER_STATE_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PortCallback

  Description:  port に対する受信通知

  Arguments:    arg - コールバック構造体へのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_PortCallback(void *arg)
{
    WXCDriverWork *const driver = work;

    if (WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMPortRecvCallback *cb = (WMPortRecvCallback *)arg;
        switch (cb->state)
        {
        case WM_STATECODE_PORT_RECV:
            {
                WXCPacketInfo packet;
                packet.bitmap = (u16)(1 << cb->aid);
                packet.length = cb->length;
                packet.buffer = (u8 *)cb->data;
                (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_DATA_RECV, &packet);
            }
            break;
        case WM_STATECODE_CONNECTED:
            break;
        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        case WM_STATECODE_DISCONNECTED:
            WXC_DRIVER_LOG("disconnected(%02X-=%02X)\n", driver->peer_bitmap, (1 << cb->aid));
            driver->peer_bitmap &= (u16)~(1 << cb->aid);
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_DISCONNECTED,
                                       (void *)(1 << cb->aid));
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_InitProc

  Description:  READY -> STOP への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_InitProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* (outof) -> STOP */
        if (!arg)
        {
            /* 現状 MeasureChannel は起動時に1回のみ行う */
            driver->need_measure_channel = TRUE;
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_Init(driver->wm_work, driver->wm_dma);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_INITIALIZE, wmResult);
            wmResult = WM_Enable(WXCi_InitProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_ENABLE, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_ENABLE)
        {
            /* 各種通知を受信 */
            wmResult = WM_SetIndCallback(WXCi_IndicateCallback);
            if (WXCi_CheckWmApiResult(driver, WM_APIID_INDICATION, wmResult))
            {
                /* ポートコールバックを指定 */
                wmResult = WM_SetPortCallback(WXC_DEFAULT_PORT, WXCi_PortCallback, NULL);
                if (WXCi_CheckWmApiResult(driver, WM_APIID_PORT_SEND, wmResult))
                {
                    WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_STOP, NULL);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartProc

  Description:  STOP -> IDLE への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StartProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* STOP -> IDLE */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_PowerOn(WXCi_StartProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_POWER_ON, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_POWER_ON)
        {
            WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StopProc

  Description:  IDLE -> STOP への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StopProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* IDLE -> STOP */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_PowerOff(WXCi_StopProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_POWER_OFF, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_POWER_OFF)
        {
            WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_STOP, NULL);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_EndProc

  Description:  STOP -> READY への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_EndProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* STOP -> READY */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_Disable(WXCi_EndProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_DISABLE, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_DISABLE)
        {
            /* ワイヤレスの完全な解放完了通知 */
            wmResult = WM_Finish();
            if (WXCi_CheckWmApiResult(driver, WM_APIID_END, wmResult))
            {
                work = NULL;
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_END, NULL);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ResetProc

  Description:  (any) -> IDLE への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ResetProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* (any) -> IDLE */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_Reset(WXCi_ResetProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_RESET, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_RESET)
        {
            driver->own_aid = 0;
            WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ParentIndicate

  Description:  親機の StartParent インジケート

  Arguments:    arg           コールバック引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ParentIndicate(void *arg)
{
    WXCDriverWork *const driver = work;
    WMStartParentCallback *cb = (WMStartParentCallback *)arg;

    /* 単なる状態遷移の場合は無視してシーケンスを続ける */
    if (cb->state == WM_STATECODE_PARENT_START)
    {
        WXCi_StartParentProc(arg);
    }
    /* それ以外はインジケートなので wxc_api.c へフックさせる */
    else if (cb->errcode == WM_ERRCODE_SUCCESS)
    {
        switch (cb->state)
        {
        case WM_STATECODE_PARENT_START:
            break;
        case WM_STATECODE_CONNECTED:
            {
                BOOL    mp_start = (driver->peer_bitmap == 0);
                WXC_DRIVER_LOG("connected(%02X+=%02X)\n", driver->peer_bitmap, (1 << cb->aid));
                driver->peer_bitmap |= (u16)(1 << cb->aid);
                (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_CONNECTED, cb);
                /* 最初の子機なら送信タイミング通知 */
                if (mp_start)
                {
                    WXCi_CallSendEvent(driver);
                }
            }
            break;
        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        case WM_STATECODE_DISCONNECTED:
            /* 切断通知処理は PortCallback で一元化された */
            break;
        case WM_STATECODE_BEACON_SENT:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_BEACON_SEND, driver->parent_param);
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartParentProc

  Description:  IDLE -> PARENT -> MP_PARENT への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StartParentProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* IDLE -> IDLE (WM_SetParentParameter) */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            /* チャンネルや起動情報を更新 */
            driver->parent_param->channel = driver->current_channel;
            driver->parent_param->tgid = WM_GetNextTgid();
            WXC_DRIVER_LOG("start parent. (%2dch, TGID=%02X, GGID=%04X)\n",
                           driver->current_channel, driver->parent_param->tgid,
                           driver->parent_param->ggid);
            wmResult = WM_SetParentParameter(WXCi_StartParentProc, driver->parent_param);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_SET_P_PARAM, wmResult);
        }
        /* IDLE -> PARENT */
        else if (cb->apiid == WM_APIID_SET_P_PARAM)
        {
            /*
             * インジケートの都合上, コールバックを
             * WXCi_ParentIndicate() へ迂回する.
             */
            wmResult = WM_StartParent(WXCi_ParentIndicate);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_PARENT, wmResult);
        }
        /* PARENT -> MP_PARENT */
        else if (cb->apiid == WM_APIID_START_PARENT)
        {
            /*
             * ここでは常に WXCi_ParentIndicate() から
             * WM_STATECODE_PARENT_START のみ通知される.
             */
            driver->own_aid = 0;
            driver->peer_bitmap = 0;
            wmResult = WM_StartMP(WXCi_StartParentProc,
                                  (u16 *)driver->mp_recv_work, driver->recv_size_max,
                                  (u16 *)driver->mp_send_work, driver->send_size_max,
                                  (u16)(driver->parent_param->CS_Flag ? 0 : 1));
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_MP, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_START_MP)
        {
            WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_PARENT, NULL);
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ChildIndicate

  Description:  子機の StartConnect インジケート

  Arguments:    arg           コールバック引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ChildIndicate(void *arg)
{
    WXCDriverWork *const driver = work;

    if (WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;
        switch (cb->state)
        {
        case WM_STATECODE_CONNECT_START:
        case WM_STATECODE_BEACON_LOST:
            break;

        case WM_STATECODE_CONNECTED:
            /* 状態遷移中の場合は無視してシーケンスを続ける */
            if (driver->state != WXC_DRIVER_STATE_CHILD)
            {
                WXCi_StartChildProc(arg);
            }
            break;

        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        case WM_STATECODE_DISCONNECTED:
            /* 遷移中でなければここでリセット */
            if (driver->state != WXC_DRIVER_STATE_BUSY)
            {
                driver->target = WXC_DRIVER_STATE_PARENT;
                WXCi_ResetProc(NULL);
            }
            else
            {
                driver->target = WXC_DRIVER_STATE_IDLE;
            }
            break;

        default:
            WXCi_ErrorQuit(driver);
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartChildProc

  Description:  IDLE -> CHILD -> MP_CHILD への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StartChildProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* IDLE -> CHILD */
        if (!arg)
        {
            /*
             * 接続前にSSID情報などを設定するためのフック呼び出し.
             */
            u8      ssid_bak[WM_SIZE_BSSID];
            MI_CpuCopy8(driver->target_bss->ssid, ssid_bak, sizeof(ssid_bak));
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_CONNECTING, driver->target_bss);
            MI_CpuCopy8(driver->target_bss->ssid + 8, driver->ssid, WM_SIZE_CHILD_SSID);
            MI_CpuCopy8(ssid_bak, driver->target_bss->ssid, sizeof(ssid_bak));
            /*
             * インジケートの都合上, コールバックを
             * WXCi_ChildIndicate() へ迂回する.
             */
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_StartConnect(WXCi_ChildIndicate, driver->target_bss, driver->ssid);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_CONNECT, wmResult);
        }
        /* CHILD -> MP_CHILD */
        else if (cb->apiid == WM_APIID_START_CONNECT)
        {
            WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;
            /*
             * ここでは常に WXCi_ChildIndicate() から
             * WM_STATECODE_CONNECTED のみ通知される.
             */
            driver->own_aid = cb->aid;
            wmResult = WM_StartMP(WXCi_StartChildProc,
                                  (u16 *)driver->mp_recv_work, driver->recv_size_max,
                                  (u16 *)driver->mp_send_work, driver->send_size_max,
                                  (u16)(driver->parent_param->CS_Flag ? 0 : 1));
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_MP, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_START_MP)
        {
            WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_CHILD, cb);
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_MeasureProc

  Description:  IDLE -> (measure) -> IDLE への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_MeasureProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMMeasureChannelCallback *cb = (WMMeasureChannelCallback *)arg;
    u16     channel = 0;

    /* 計測値の初期化 */
    if (!arg)
    {
        driver->state = WXC_DRIVER_STATE_BUSY;
        driver->current_channel = 0;
        driver->measure_ratio_min = WXC_MAX_RATIO + 1;
    }
    else if (WXCi_CheckWmCallbackResult(driver, cb))
        /* 計測完了コールバック */
    {
        channel = cb->channel;
        /* より使用率の低いチャンネルを取得 (初期値 101% なので先頭は必ず選択) */
        if (driver->measure_ratio_min > cb->ccaBusyRatio)
        {
            driver->measure_ratio_min = cb->ccaBusyRatio;
            driver->current_channel = channel;
        }
        /* 全チャンネル計測完了 */
        if (channel == (32 - MATH_CountLeadingZeros(WM_GetAllowedChannel())))
        {
            driver->need_measure_channel = FALSE;
            /* 現在もなお PARENT が対象であれば接続へ移行 */
            //WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
            WXCi_StartParentProc(NULL);
            return;
        }
    }
    /* 計測に失敗 (エラー終了へ) */
    else
    {
        driver->need_measure_channel = FALSE;
    }

    /* 全てのチャンネルを下位から順に計測 */
    if (driver->need_measure_channel)
    {
        /* 1フレームに一回通信している電波を拾えるだけの間隔(ms) */
        const u16 WH_MEASURE_TIME = 30;
        /* キャリアセンスとED値の論理和 */
        const u16 WH_MEASURE_CS_OR_ED = 3;
        /* 実験データによる経験的に有効と思われるお勧めED閾値 */
        const u16 WH_MEASURE_ED_THRESHOLD = 17;
        WMErrCode ret;

        channel = WXC_GetNextAllowedChannel(channel);
        ret = WM_MeasureChannel(WXCi_MeasureProc,
                                WH_MEASURE_CS_OR_ED, WH_MEASURE_ED_THRESHOLD,
                                channel, WH_MEASURE_TIME);
        (void)WXCi_CheckWmApiResult(driver, WM_APIID_MEASURE_CHANNEL, ret);
    }

}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ScanProc

  Description:  IDLE -> SCAN -> IDLE への連続遷移

  Arguments:    arg           コールバック引数 (呼び出し時は NULL を指定)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ScanProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;
    
    static u16 scan_channelList;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;

        /* IDLE -> SCAN */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            driver->scan_found_num = 0;
            /* ブロードキャストアドレスで全チャンネルを探索 */
            driver->scan_param->scanBuf = (WMBssDesc *)driver->scan_buf;
            driver->scan_param->scanBufSize = WM_SIZE_SCAN_EX_BUF;
            
            /* 探索するチャンネルを 1 チャンネルずつにする */
            scan_channelList = WM_GetAllowedChannel();
            driver->scan_param->channelList = (u16)MATH_GetLeastSignificantBit(scan_channelList);
            scan_channelList = (u16)(scan_channelList ^ MATH_GetLeastSignificantBit(scan_channelList));
            
            driver->scan_param->maxChannelTime = WXC_SCAN_TIME_MAX;
            MI_CpuFill8(driver->scan_param->bssid, 0xFF, sizeof(driver->scan_param->bssid));
            driver->scan_param->scanType = WM_SCANTYPE_PASSIVE;
            driver->scan_param->ssidLength = 0;
            MI_CpuFill8(driver->scan_param->ssid, 0xFF, sizeof(driver->scan_param->ssid));
            wmResult = WM_StartScanEx(WXCi_ScanProc, driver->scan_param);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_SCAN_EX, wmResult);
        }
        /* SCAN -> IDLE */
        else if (cb->apiid == WM_APIID_START_SCAN_EX)
        {
            WMStartScanExCallback *cb = (WMStartScanExCallback *)arg;
            /* ビーコンが検出されれば保持 */
            if (cb->state == WM_STATECODE_PARENT_FOUND)
            {
                DC_InvalidateRange(driver->scan_buf, WM_SIZE_SCAN_EX_BUF);
                driver->scan_found_num = cb->bssDescCount;
            }
            wmResult = WM_EndScan(WXCi_ScanProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_END_SCAN, wmResult);
        }
        /* 完了 */
        else if (cb->apiid == WM_APIID_END_SCAN)
        {
            BOOL    ret = FALSE;
            /* 対象がまだ CHILD であればBssDesc の判定 */
            if (driver->target == WXC_DRIVER_STATE_CHILD)
            {
                int     i;
                const u8 *scan_buf = driver->scan_buf;

                WXC_DRIVER_LOG("found:%d beacons\n", driver->scan_found_num);
                for (i = 0; i < driver->scan_found_num; ++i)
                {
                    const WMBssDesc *p_desc = (const WMBssDesc *)scan_buf;
                    const int len = p_desc->length * 2;
                    BOOL    is_valid;
                    is_valid = WM_IsValidGameBeacon(p_desc);
                    WXC_DRIVER_LOG("   GGID=%08X(%2dch:%3dBYTE)\n",
                                   is_valid ? p_desc->gameInfo.ggid : 0xFFFFFFFF,
                                   p_desc->channel, len);
                    if (is_valid)
                    {
                        /* 各 BssDesc ごとにコールバック */
                        ret =
                            (BOOL)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_BEACON_RECV,
                                                       (void *)p_desc);
                        if (ret)
                        {
                            WXC_DRIVER_LOG("     -> matched!\n");
                            MI_CpuCopy8(p_desc, driver->target_bss, sizeof(WMBssDesc));
                            break;
                        }
                    }
                    scan_buf += MATH_ROUNDUP(len, 4);
                }
                /* まだ対象が決まっておらず、探索していないチャンネルがあるならば、探索する */
                if((ret == FALSE)&&(MATH_GetLeastSignificantBit(scan_channelList) != 0))
                {
                    driver->scan_found_num = 0;
                    driver->scan_param->channelList = (u16)MATH_GetLeastSignificantBit(scan_channelList);
                    scan_channelList = (u16)(scan_channelList ^ MATH_GetLeastSignificantBit(scan_channelList));
                    wmResult = WM_StartScanEx(WXCi_ScanProc, driver->scan_param);
                    (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_SCAN_EX, wmResult);
                    return;
                }
            }
            /* 対象がいれば接続開始 */
            if (ret)
            {
                WXCi_StartChildProc(NULL);
            }
            /* いなければ次の起動モードを選択 */
            else
            {
                if (driver->target == WXC_DRIVER_STATE_CHILD)
                {
                    driver->target = WXC_DRIVER_STATE_IDLE;
                }
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
            }
        }
    }
}

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
void WXC_InitDriver(WXCDriverWork * driver, WMParentParam *pp, WXCDriverEventFunc func, u32 dma)
{
    /* 割り込みハンドラの外で初回 TGID を取得 */
    {
        OSIntrMode bak_cpsr = OS_EnableInterrupts();
        (void)WM_GetNextTgid();
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    /* ワーク変数初期化 */
    work = driver;
    MI_CpuClear32(driver, sizeof(WXCDriverWork));
    driver->own_aid = 0;
    driver->send_busy = TRUE;
    driver->callback = func;
    driver->wm_dma = (u16)dma;
    driver->send_size_max = (u16)sizeof(driver->mp_send_work);
    driver->recv_size_max = (u16)sizeof(driver->mp_recv_work);
    driver->state = WXC_DRIVER_STATE_END;
    driver->parent_param = pp;
    driver->parent_param->entryFlag = 1;
    driver->parent_param->beaconPeriod = WXC_BEACON_PERIOD;
    driver->parent_param->channel = 1;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverTarget

  Description:  特定の状態を目標に遷移を開始する.

  Arguments:    driver        WXCDriverWork 構造体
                target        遷移先のステート

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetDriverTarget(WXCDriverWork * driver, WXCDriverState target)
{
    driver->target = target;
    /* 違う状態に安定していればここで遷移を開始 */
    if ((driver->state != WXC_DRIVER_STATE_BUSY) && (driver->state != driver->target))
    {
        WXCi_OnStateChanged(driver, driver->state, NULL);
    }
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
void WXC_SetDriverSsid(WXCDriverWork * driver, const void *buffer, u32 length)
{
    length = (u32)MATH_MIN(length, WM_SIZE_CHILD_SSID);
    MI_CpuCopy8(buffer, driver->ssid, length);
    MI_CpuFill8(driver->ssid + length, 0x00, (u32)(WM_SIZE_CHILD_SSID - length));
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
