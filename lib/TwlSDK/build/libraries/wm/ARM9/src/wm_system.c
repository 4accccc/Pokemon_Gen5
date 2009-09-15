/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_system.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"

#ifdef SDK_TWL /* TWL では初期化の際のチェックを強化 */
#include "wm_common.h"
#endif //SDK_TWL

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     WM_BUF_MSG_NUM                  10


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u16 wmInitialized = 0;          // WM初期化フラグ
static WMArm9Buf *wm9buf;
static u32 fifoBuf[WM_BUF_MSG_NUM][WM_FIFO_BUF_SIZE / sizeof(u32)] ATTRIBUTE_ALIGN(32);
static OSMessageQueue bufMsgQ;         // WM7へのリクエストキュー
static OSMessage bufMsg[WM_BUF_MSG_NUM];        // WM7へのリクエストキュー用バッファ
static PMSleepCallbackInfo sleepCbInfo;  //PM に登録するスリープコールバック情報


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void WmReceiveFifo(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err);
static void WmClearFifoRecvFlag(void);
static WMErrCode WmInitCore(void *wmSysBuf, u16 dmaNo, u32 bufSize);
static u32 *WmGetCommandBuffer4Arm7(void);
static void WmSleepCallback(void *);

/*---------------------------------------------------------------------------*
  Name:         WM_Init

  Description:  WMライブラリの初期化処理を行う。
                ARM9側の初期化のみを行う同期関数。

  Arguments:    wmSysBuf    -   呼び出し元によって確保されたバッファへのポインタ。
                                バッファのサイズはWM_SYSTEM_BUF_SIZEだけ必要。
                dmaNo       -   WMが使用するDMA番号。

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Init(void *wmSysBuf, u16 dmaNo)
{
    WMErrCode result;

    result = WmInitCore(wmSysBuf, dmaNo, WM_SYSTEM_BUF_SIZE);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return result;
    }
    wm9buf->scanOnlyFlag = FALSE;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_InitForScan

  Description:  スキャンのみを使用する場合のWMライブラリの初期化処理を行う。
                ARM9側の初期化のみを行う同期関数。

  Arguments:    wmSysBuf    -   呼び出し元によって確保されたバッファへのポインタ。
                                バッファのサイズはWM_SYSTEM_BUF_SIZE_FOR_SCANだけ必要。
                dmaNo       -   WMが使用するDMA番号。

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_InitForScan(void *wmSysBuf, u16 dmaNo);
WMErrCode WMi_InitForScan(void *wmSysBuf, u16 dmaNo)
{
#define WM_STATUS_BUF_SIZE_FOR_SCAN      768
#define WM_ARM9WM_BUF_SIZE_FOR_SCAN      320
#define WM_SYSTEM_BUF_SIZE_FOR_SCAN      (WM_ARM9WM_BUF_SIZE_FOR_SCAN + WM_ARM7WM_BUF_SIZE + WM_STATUS_BUF_SIZE_FOR_SCAN + WM_FIFO_BUF_SIZE + WM_FIFO_BUF_SIZE)

    WMErrCode result;
    result = WmInitCore(wmSysBuf, dmaNo, WM_SYSTEM_BUF_SIZE_FOR_SCAN);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return result;
    }
    wm9buf = (WMArm9Buf *)wmSysBuf;
    wm9buf->WM7 = (WMArm7Buf *)((u32)wm9buf + WM_ARM9WM_BUF_SIZE_FOR_SCAN);
    wm9buf->status = (WMStatus *)((u32)(wm9buf->WM7) + WM_ARM7WM_BUF_SIZE);
    wm9buf->fifo9to7 = (u32 *)((u32)(wm9buf->status) + WM_STATUS_BUF_SIZE_FOR_SCAN);
    wm9buf->fifo7to9 = (u32 *)((u32)(wm9buf->fifo9to7) + WM_FIFO_BUF_SIZE);
    wm9buf->dmaNo = dmaNo;
    wm9buf->scanOnlyFlag = TRUE;

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WmInitCore

  Description:  WMライブラリの初期化処理を行う、ライブラリ内部使用関数。
                ARM9側の初期化のみを行う同期関数。

  Arguments:    wmSysBuf    -   呼び出し元によって確保されたバッファへのポインタ。
                dmaNo       -   WMが使用するDMA番号。
                bufSize     -   WMライブラリに割り当てられるバッファサイズ

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static WMErrCode WmInitCore(void *wmSysBuf, u16 dmaNo, u32 bufSize)
{
    OSIntrMode e;

    // 構造体のサイズが想定値をオーバーしていないかチェック
    SDK_COMPILER_ASSERT(sizeof(WMArm9Buf) <= WM_ARM9WM_BUF_SIZE);
    SDK_COMPILER_ASSERT(sizeof(WMArm7Buf) <= WM_ARM7WM_BUF_SIZE);
    SDK_COMPILER_ASSERT(sizeof(WMStatus) <= WM_STATUS_BUF_SIZE);

    e = OS_DisableInterrupts();

#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        /* NWMライブラリが初期化前であることを確認 */
        if (NWM_CheckInitialized() != NWM_RETCODE_ILLEGAL_STATE)
        {
            WM_WARNING("NWM has already been initialized.\n");
            (void)OS_RestoreInterrupts(e);
            return WM_ERRCODE_ILLEGAL_STATE;
        }
    }
#endif

    /* WMライブラリの初期化済みを確認 */
    if (wmInitialized)
    {
        WM_WARNING("WM has already been initialized.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_ILLEGAL_STATE;        // 初期化済み
    }

    // パラメータチェック
    if (wmSysBuf == NULL)
    {
        WM_WARNING("Parameter \"wmSysBuf\" must not be NULL.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }
        // WM の PXI の利用法の都合で、wmSysBuf が 0x04000000 以上のアドレスにはできない
#ifdef SDK_TWL
    if( OS_IsOnVram(wmSysBuf) || ( OS_IsRunOnTwl() && OS_IsOnExtendedMainMemory(wmSysBuf) ) )
#else
    if( OS_IsOnVram(wmSysBuf) )
#endif
    {
        WM_WARNING("Parameter \"wmSysBuf\" must be alocated less than 0x04000000.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }
    
    if (dmaNo > MI_DMA_MAX_NUM)
    {
        WM_WARNING("Parameter \"dmaNo\" is over %d.\n", MI_DMA_MAX_NUM);
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)wmSysBuf & 0x01f)
    {
        WM_WARNING("Parameter \"wmSysBuf\" must be 32-byte aligned.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }

    // ARM7側でWMライブラリが開始されているか確認
    PXI_Init();
    if (!PXI_IsCallbackReady(PXI_FIFO_TAG_WM, PXI_PROC_ARM7))
    {
        WM_WARNING("WM library on ARM7 side is not ready yet.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_WM_DISABLE;
    }

    /*  キャッシュに余計なデータが残っていると想定外のタイミングで
       ライトバックが発生し、ARM7側の状態管理に支障を来たす可能性がある。
       その対策としてここで全ワークバッファのキャッシュを破棄しておく。 */
    DC_InvalidateRange(wmSysBuf, bufSize);
    // 各種バッファ初期化
    MI_DmaClear32(dmaNo, wmSysBuf, bufSize);
    wm9buf = (WMArm9Buf *)wmSysBuf;
    wm9buf->WM7 = (WMArm7Buf *)((u32)wm9buf + WM_ARM9WM_BUF_SIZE);
    wm9buf->status = (WMStatus *)((u32)(wm9buf->WM7) + WM_ARM7WM_BUF_SIZE);
    wm9buf->fifo9to7 = (u32 *)((u32)(wm9buf->status) + WM_STATUS_BUF_SIZE);
    wm9buf->fifo7to9 = (u32 *)((u32)(wm9buf->fifo9to7) + WM_FIFO_BUF_SIZE);

    // FIFOバッファ書き込み許可フラグをクリア
    WmClearFifoRecvFlag();

    // 各種変数を初期化
    wm9buf->dmaNo = dmaNo;
    wm9buf->connectedAidBitmap = 0x0000;
    wm9buf->myAid = 0;

    // 各ポート用コールバックテーブルを初期化
    {
        s32     i;

        for (i = 0; i < WM_NUM_OF_PORT; i++)
        {
            wm9buf->portCallbackTable[i] = NULL;
            wm9buf->portCallbackArgument[i] = NULL;
        }
    }

    // エントリー登録用キューを初期化
    {
        s32     i;

        OS_InitMessageQueue(&bufMsgQ, bufMsg, WM_BUF_MSG_NUM);
        for (i = 0; i < WM_BUF_MSG_NUM; i++)
        {
            // リングバッファを使用可能な状態にクリア
            *((u16 *)(fifoBuf[i])) = WM_API_REQUEST_ACCEPTED;
            DC_StoreRange(fifoBuf[i], 2);
            (void)OS_SendMessage(&bufMsgQ, fifoBuf[i], OS_MESSAGE_BLOCK);
        }
    }

    // FIFO受信関数を設定
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_WM, WmReceiveFifo);

    wmInitialized = 1;
    (void)OS_RestoreInterrupts(e);
    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Finish

  Description:  WMライブラリの終了処理を行う。同期関数。
                WM_Init関数を呼ぶ前の状態に戻る。

  Arguments:    None.

  Returns:      WMErrCode   -   処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_Finish(void)
{
    OSIntrMode e;
    WMErrCode result;

    e = OS_DisableInterrupts();
    // 初期化済みでない場合は実行不可
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    // ステートがREADYでない場合は実行不可
    result = WMi_CheckState(WM_STATE_READY);
    WM_CHECK_RESULT(result);

    // WMライブラリを終了
    WmClearFifoRecvFlag();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_WM, NULL);
    wm9buf = 0;

    wmInitialized = 0;
    (void)OS_RestoreInterrupts(e);
    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SetCallbackTable

  Description:  各非同期関数に対応したコールバック関数を登録する。

  Arguments:    id          -   非同期関数のAPI ID。
                callback    -   登録するコールバック関数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_SetCallbackTable(WMApiid id, WMCallbackFunc callback)
{
    SDK_NULL_ASSERT(wm9buf);

    wm9buf->CallbackTable[id] = callback;
}

/*---------------------------------------------------------------------------*
  Name:         WmGetCommandBuffer4Arm7

  Description:  ARM7 向けのコマンド用バッファをプールから確保する

  Arguments:    None.

  Returns:      確保できればその値, 確保できなければ NULL
 *---------------------------------------------------------------------------*/
u32    *WmGetCommandBuffer4Arm7(void)
{
    u32    *tmpAddr;

    if (FALSE == OS_ReceiveMessage(&bufMsgQ, (OSMessage *)&tmpAddr, OS_MESSAGE_NOBLOCK))
    {
        return NULL;
    }

    // リングバッファが使用可能な状態(キューが一杯でない)かを確認
    DC_InvalidateRange(tmpAddr, 2);
    if ((*((u16 *)tmpAddr) & WM_API_REQUEST_ACCEPTED) == 0)
    {
        // キューの先頭に再度予約( 一番初めに使用可能状態になるはずなので )
        (void)OS_JamMessage(&bufMsgQ, tmpAddr, OS_MESSAGE_BLOCK);
        // エラーとして終了
        return NULL;
    }

    return tmpAddr;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SendCommand

  Description:  FIFO経由でリクエストをARM7に送信する。
                u32型のパラメータをいくつか伴うコマンドの場合は、
                パラメータを列挙して指定する。

  Arguments:    id          -   リクエストに対応したAPI ID。
                paramNum    -   仮想引数の数。
                ...         -   仮想引数。

  Returns:      int -   WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SendCommand(WMApiid id, u16 paramNum, ...)
{
    va_list vlist;
    s32     i;
    int     result;
    u32    *tmpAddr;

    SDK_NULL_ASSERT(wm9buf);

    // コマンド送信用のバッファを確保
    tmpAddr = WmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        return WM_ERRCODE_FIFO_ERROR;
    }

    // API ID
    *(u16 *)tmpAddr = (u16)id;

    // 指定数個の引数を追加
    va_start(vlist, paramNum);
    for (i = 0; i < paramNum; i++)
    {
        tmpAddr[i + 1] = va_arg(vlist, u32);
    }
    va_end(vlist);

    DC_StoreRange(tmpAddr, WM_FIFO_BUF_SIZE);

    // FIFOで通知
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WM, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&bufMsgQ, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        WM_WARNING("Failed to send command through FIFO.\n");
        return WM_ERRCODE_FIFO_ERROR;
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SendCommandDirect

  Description:  FIFO経由でリクエストをARM7に送信する。
                ARM7 に送るコマンドを直接指定する。

  Arguments:    data        -   ARM7 に送るコマンド。
                length      -   ARM7 に送るコマンドのサイズ。

  Returns:      int -   WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SendCommandDirect(void *data, u32 length)
{
    int     result;
    u32    *tmpAddr;

    SDK_NULL_ASSERT(wm9buf);
    SDK_ASSERT(length <= WM_FIFO_BUF_SIZE);

    // コマンド送信用のバッファを確保
    tmpAddr = WmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        return WM_ERRCODE_FIFO_ERROR;
    }

    // ARM7 に送るコマンドを専用バッファにコピーする
    MI_CpuCopy8(data, tmpAddr, length);

    DC_StoreRange(tmpAddr, length);

    // FIFOで通知
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WM, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&bufMsgQ, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        WM_WARNING("Failed to send command through FIFO.\n");
        return WM_ERRCODE_FIFO_ERROR;
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_GetSystemWork

  Description:  WMライブラリ内部で使用するバッファの先頭へのポインタを取得する。

  Arguments:    None.

  Returns:      WMArm9Buf*  -   内部ワークバッファへのポインタを返す。
 *---------------------------------------------------------------------------*/
WMArm9Buf *WMi_GetSystemWork(void)
{
//    SDK_NULL_ASSERT(wm9buf);
    return wm9buf;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckInitialized

  Description:  WMライブラリの初期化済みを確認する。

  Arguments:    None.

  Returns:      int -   WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_CheckInitialized(void)
{
    // 初期化済みを確認
    if (!wmInitialized)
    {
        return WM_ERRCODE_ILLEGAL_STATE;
    }
    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckIdle

  Description:  WMライブラリの内部状態を確認し、無線ハードウェアが起動済みで
                あることを確認する。

  Arguments:    None.

  Returns:      int -   WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_CheckIdle(void)
{
    WMErrCode result;

    // 初期化済みを確認
    result = WMi_CheckInitialized();
    WM_CHECK_RESULT(result);

    // 現ステート確認
    DC_InvalidateRange(&(wm9buf->status->state), 2);
    if ((wm9buf->status->state == WM_STATE_READY) || (wm9buf->status->state == WM_STATE_STOP))
    {
        WM_WARNING("WM state is \"%d\" now. So can't execute request.\n", wm9buf->status->state);
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckStateEx

  Description:  WMライブラリの内部状態を確認する。
                許可される状態を示すWMState型のパラメータを列挙して指定する。

  Arguments:    paramNum    -   仮想引数の数。
                ...         -   仮想引数。

  Returns:      int -   WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WMi_CheckStateEx(s32 paramNum, ...)
{
    WMErrCode result;
    u16     now;
    u32     temp;
    va_list vlist;

    // 初期化済みを確認
    result = WMi_CheckInitialized();
    WM_CHECK_RESULT(result);

    // 現ステート取得
    DC_InvalidateRange(&(wm9buf->status->state), 2);
    now = wm9buf->status->state;

    // 一致確認
    result = WM_ERRCODE_ILLEGAL_STATE;
    va_start(vlist, paramNum);
    for (; paramNum; paramNum--)
    {
        temp = va_arg(vlist, u32);
        if (temp == now)
        {
            result = WM_ERRCODE_SUCCESS;
        }
    }
    va_end(vlist);

    if (result == WM_ERRCODE_ILLEGAL_STATE)
    {
        WM_WARNING("WM state is \"%d\" now. So can't execute request.\n", now);
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WmReceiveFifo

  Description:  FIFOを介したWM7からのコールバックを受信する。

  Arguments:    tag          - 未使用
                fifo_buf_adr - コールバックのパラメータ群へのポインタ
                err          - 未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WmReceiveFifo(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err)
{
#pragma unused( tag )

    WMCallback *callback_p = (WMCallback *)fifo_buf_adr;
    WMArm9Buf *w9b = wm9buf;           // volatile ではないことをコンパイラに伝える
#ifdef WM_DEBUG_CALLBACK
    int     beginVcount = GX_GetVCount();
#endif

    if (err)
        return;                        // WM_sp_init の PXI ハンドラの準備が整っていない

    // FIFOコマンドバッファ(9←7)のキャッシュを無効化
    DC_InvalidateRange(w9b->fifo7to9, WM_FIFO_BUF_SIZE);
    if (!w9b->scanOnlyFlag)
    {
        DC_InvalidateRange(w9b->status, WM_STATUS_BUF_SIZE);
    }

    // 応答バッファが fifo7to9 とは別バッファでないか確認
    if ((u32)callback_p != (u32)(w9b->fifo7to9))
    {
        DC_InvalidateRange(callback_p, WM_FIFO_BUF_SIZE);
    }


    if (callback_p->apiid >= WM_NUM_OF_CALLBACK)
    {
        if (callback_p->apiid == WM_APIID_INDICATION)
        {
            if (callback_p->errcode == WM_ERRCODE_FLASH_ERROR)
            {
                // ARM9フリーズ
                OS_Terminate();        // 返ってこない。
            }
            if (w9b->indCallback != NULL)
            {
                w9b->indCallback((void *)callback_p);
            }
        }
        else if (callback_p->apiid == WM_APIID_PORT_RECV)
        {
            // port 番号に応じたコールバック処理を行う
            WMPortRecvCallback *cb_Port = (WMPortRecvCallback *)callback_p;
            SDK_ASSERT(cb_Port->port < 16);
            // callback が登録されていたら通知する
            if (w9b->portCallbackTable[cb_Port->port] != NULL)
            {
                cb_Port->arg = w9b->portCallbackArgument[cb_Port->port];
                cb_Port->connectedAidBitmap = (u16)w9b->connectedAidBitmap;
                DC_InvalidateRange(cb_Port->recvBuf, w9b->status->mp_recvBufSize);      // ToDo: Invalidate しすぎている
                (w9b->portCallbackTable[cb_Port->port]) ((void *)cb_Port);
            }
#ifdef WM_DEBUG
            else
            {
                WM_DPRINTF("Warning: no callback function on port %d { %04x %04x ... }\n",
                           cb_Port->port, cb_Port->data[0], cb_Port->data[1]);
            }
#endif
        }
        else if (callback_p->apiid == WM_APIID_PORT_SEND)
        {
            // データをセットした際に指定したコールバック処理を行う
            WMPortSendCallback *cb_Port = (WMPortSendCallback *)callback_p;
            callback_p->apiid = WM_APIID_SET_MP_DATA;   // 偽装する
            if (cb_Port->callback != NULL)
            {
                (cb_Port->callback) ((void *)cb_Port);
            }
        }
        else
        {
            OS_Printf("ARM9: no callback function\n");

        }
    }
    else
    {
        // 受信バッファのキャッシュを無効化しておく
        // MPEND_IND, MP_IND の後に続く、PORT_RECV の呼び出しの際は、ここですでに
        // InvalidateRange されているため、改めてキャッシュを気にする必要はない
        if (callback_p->apiid == WM_APIID_START_MP)
        {
            WMStartMPCallback *cb_StartMP = (WMStartMPCallback *)callback_p;
            if (cb_StartMP->state == WM_STATECODE_MPEND_IND
                || cb_StartMP->state == WM_STATECODE_MP_IND)
            {
                if (cb_StartMP->errcode == WM_ERRCODE_SUCCESS)
                {
                    DC_InvalidateRange(cb_StartMP->recvBuf, w9b->status->mp_recvBufSize);
                }
            }
        }

        // WM_Enable が失敗した場合、WM_Disable が成功した場合にはスリープコールバックを削除
        if( ( (callback_p->apiid == WM_APIID_DISABLE  || callback_p->apiid == WM_APIID_END)      && callback_p->errcode == WM_ERRCODE_SUCCESS) ||
            ( (callback_p->apiid == WM_APIID_ENABLE || callback_p->apiid == WM_APIID_INITIALIZE) && callback_p->errcode != WM_ERRCODE_SUCCESS) )
        {
            WMi_DeleteSleepCallback();
        }
        
        // WM_Endの後処理
        if (callback_p->apiid == WM_APIID_END)
        {
            if (callback_p->errcode == WM_ERRCODE_SUCCESS)
            {
                WMCallbackFunc cb = w9b->CallbackTable[callback_p->apiid];
                // WMライブラリを終了
                (void)WM_Finish();
                if (cb != NULL)
                {
                    cb((void *)callback_p);
                }

                WM_DLOGF_CALLBACK("Cb[%x]", callback_p->apiid);

                return;
            }
        }

#if 0
        // WM_Reset, WM_Disconnect, WM_EndParent の後処理
        if (callback_p->apiid == WM_APIID_RESET
            || callback_p->apiid == WM_APIID_DISCONNECT || callback_p->apiid == WM_APIID_END_PARENT)
        {
            if (w9b->connectedAidBitmap != 0)
            {
                WM_WARNING("connectedAidBitmap should be 0, but %04x", w9b->connectedAidBitmap);
            }
            w9b->myAid = 0;
            w9b->connectedAidBitmap = 0;
        }
#endif

        // apiidに応じたコールバック処理 (コールバックが未設定(NULL)なら何もしない)
        if (NULL != w9b->CallbackTable[callback_p->apiid])
        {
            (w9b->CallbackTable[callback_p->apiid]) ((void *)callback_p);
            /* ユーザーコールバック内でWM_Finishが呼び出され、ワーク用メモリがクリアされた場合はここで終了 */
            if (!wmInitialized)
            {
                return;
            }
        }
        // 後処理
        // 接続・切断の通知を port の callback にも伝えるために、メッセージを加工する
        if (callback_p->apiid == WM_APIID_START_PARENT
            || callback_p->apiid == WM_APIID_START_CONNECT)
        {
            u16     state, aid, myAid, reason;
            u8     *macAddress;
            u8     *ssid;
            u16     parentSize, childSize;

            if (callback_p->apiid == WM_APIID_START_PARENT)
            {
                WMStartParentCallback *cb = (WMStartParentCallback *)callback_p;
                state = cb->state;
                aid = cb->aid;
                myAid = 0;
                macAddress = cb->macAddress;
                ssid = cb->ssid;
                reason = cb->reason;
                parentSize = cb->parentSize;
                childSize = cb->childSize;
            }
            else if (callback_p->apiid == WM_APIID_START_CONNECT)
            {
                WMStartConnectCallback *cb = (WMStartConnectCallback *)callback_p;
                state = cb->state;
                aid = 0;
                myAid = cb->aid;
                macAddress = cb->macAddress;
                ssid = NULL;
                reason = cb->reason;
                parentSize = cb->parentSize;
                childSize = cb->childSize;
            }
            if (state == WM_STATECODE_CONNECTED ||
                state == WM_STATECODE_DISCONNECTED
                || state == WM_STATECODE_DISCONNECTED_FROM_MYSELF)
            {
                // 割り込みハンドラ内のため、static でも問題はない
                static WMPortRecvCallback cb_Port;
                u16     iPort;

                // WM9 側で管理している接続状態を変更
                if (state == WM_STATECODE_CONNECTED)
                {
#ifdef WM_DEBUG
                    if (w9b->connectedAidBitmap & (1 << aid))
                    {
                        WM_DPRINTF("Warning: someone is connecting to connected aid: %d (%04x)",
                                   aid, w9b->connectedAidBitmap);
                    }
#endif
                    WM_DLOGF_AIDBITMAP("aid(%d) connected: %04x -> %04x", aid,
                                       w9b->connectedAidBitmap,
                                       w9b->connectedAidBitmap | (1 << aid));
                    w9b->connectedAidBitmap |= (1 << aid);
                }
                else                   // WM_STATECODE_DISCONNECTED || WM_STATECODE_DISCONNECTED_FROM_MYSELF
                {
#ifdef WM_DEBUG
                    if (!(w9b->connectedAidBitmap & (1 << aid)))
                    {
                        WM_DPRINTF
                            ("Warning: someone is disconnecting to disconnected aid: %d (%04x)",
                             aid, w9b->connectedAidBitmap);
                    }
#endif
                    WM_DLOGF_AIDBITMAP("aid(%d) disconnected: %04x -> %04x", aid,
                                       w9b->connectedAidBitmap,
                                       w9b->connectedAidBitmap & ~(1 << aid));
                    w9b->connectedAidBitmap &= ~(1 << aid);
                }
                w9b->myAid = myAid;

                MI_CpuClear8(&cb_Port, sizeof(WMPortRecvCallback));
                cb_Port.apiid = WM_APIID_PORT_RECV;
                cb_Port.errcode = WM_ERRCODE_SUCCESS;
                cb_Port.state = state;
                cb_Port.recvBuf = NULL;
                cb_Port.data = NULL;
                cb_Port.length = 0;
                cb_Port.aid = aid;
                cb_Port.myAid = myAid;
                cb_Port.connectedAidBitmap = (u16)w9b->connectedAidBitmap;
                cb_Port.seqNo = 0xffff;
                cb_Port.reason = reason;
                MI_CpuCopy8(macAddress, cb_Port.macAddress, WM_SIZE_MACADDR);
                if (ssid != NULL)
                {
                    MI_CpuCopy16(ssid, cb_Port.ssid, WM_SIZE_CHILD_SSID);
                }
                else
                {
                    MI_CpuClear16(cb_Port.ssid, WM_SIZE_CHILD_SSID);
                }
                cb_Port.maxSendDataSize = (u16)((myAid == 0) ? parentSize : childSize);
                cb_Port.maxRecvDataSize = (u16)((myAid == 0) ? childSize : parentSize);

                // 全ての port に対して、接続・切断の通知を行う
                for (iPort = 0; iPort < WM_NUM_OF_PORT; iPort++)
                {
                    cb_Port.port = iPort;
                    if (w9b->portCallbackTable[iPort] != NULL)
                    {
                        cb_Port.arg = w9b->portCallbackArgument[iPort];
                        (w9b->portCallbackTable[iPort]) ((void *)&cb_Port);
                    }
                }
            }
        }
    }
    // PORT_SEND, PORT_RECV で fifo7to9 領域に書き込みを行ってしまっているため
    // ライトバックされないようキャッシュを無効にする
    DC_InvalidateRange(w9b->fifo7to9, WM_FIFO_BUF_SIZE);
    WmClearFifoRecvFlag();

    // 応答バッファが fifo7to9 とは別バッファの場合、リクエスト受付完了フラグを偽装
    if ((u32)callback_p != (u32)(w9b->fifo7to9))
    {
        callback_p->apiid |= WM_API_REQUEST_ACCEPTED;
        DC_StoreRange(callback_p, WM_FIFO_BUF_SIZE);
    }

    WM_DLOGF2_CALLBACK(beginVcount, "[CB](%x)", callback_p->apiid);

    return;
}

/*---------------------------------------------------------------------------*
  Name:         WmClearFifoRecvFlag

  Description:  WM7からのコールバックに用いられたFIFOデータを参照し終えたことを
                WM7に通知する。
                WM7においてコールバックにFIFOを用いる際、このフラグがアンロック
                されるのを待ってから次のコールバックを編集する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WmClearFifoRecvFlag(void)
{
    if (OS_GetSystemWork()->wm_callback_control & WM_EXCEPTION_CB_MASK)
    {
        // CB排他フラグを下げる
        OS_GetSystemWork()->wm_callback_control &= ~WM_EXCEPTION_CB_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WMi_DebugPrintSendQueue

  Description:  ポート送信キューの内容をプリント出力する。

  Arguments:    queue - ポート送信キューへのポインタを指定。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_DebugPrintSendQueue(WMPortSendQueue *queue)
{
    WMstatus *status = wm9buf->status;
    WMPortSendQueueData *queueData;
    u16     index;

    DC_InvalidateRange(wm9buf->status, WM_STATUS_BUF_SIZE);     // ARM7ステータス領域のキャッシュを無効化
    queueData = status->sendQueueData;

    OS_Printf("head = %d, tail = %d, ", queue->head, queue->tail);
    if (queue->tail != WM_SEND_QUEUE_END)
    {
        OS_Printf("%s", (queueData[queue->tail].next == WM_SEND_QUEUE_END) ? "valid" : "invalid");
    }
    OS_Printf("\n");
    for (index = queue->head; index != WM_SEND_QUEUE_END; index = queueData[index].next)
    {
        WMPortSendQueueData *data = &(queueData[index]);

        OS_Printf("queueData[%d] -> %d { port=%d, destBitmap=%x, size=%d } \n", index, data->next,
                  data->port, data->destBitmap, data->size);
    }

}

/*---------------------------------------------------------------------------*
  Name:         WMi_DebugPrintAllSendQueue

  Description:  全てのポート送信キューの内容をプリント出力する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_DebugPrintAllSendQueue(void)
{
    WMstatus *status = wm9buf->status;
#if 0
    int     iPrio;

    DC_InvalidateRange(wm9buf->status, WM_STATUS_BUF_SIZE);     // ARM7ステータス領域のキャッシュを無効化
    for (iPrio = 0; iPrio < WM_PRIORITY_LEVEL; iPrio++)
    {
        OS_Printf("== send queue [%d]\n", iPrio);
        WMi_DebugPrintSendQueue(&status->sendQueue[iPrio]);
    }
    for (iPrio = 0; iPrio < WM_PRIORITY_LEVEL; iPrio++)
    {
        OS_Printf("== ready queue [%d]\n", iPrio);
        WMi_DebugPrintSendQueue(&status->readyQueue[iPrio]);
    }
    OS_Printf("== free queue\n");
    OS_Printf(" head: %d, tail: %d\n", status->sendQueueFreeList.head,
              status->sendQueueFreeList.tail);
//    WMi_DebugPrintSendQueue( &status->sendQueueFreeList );
#else
    DC_InvalidateRange(wm9buf->status, WM_STATUS_BUF_SIZE);     // ARM7ステータス領域のキャッシュを無効化
    OS_Printf("== ready queue [2]\n");
    OS_Printf(" head: %d, tail: %d\n", status->readyQueue[2].head, status->readyQueue[2].tail);
#endif

}

/*---------------------------------------------------------------------------*
  Name:         WMi_GetStatusAddress

  Description:  WM内で管理されているステータス構造体へのポインタを取得する。
                この構造体はARM7が直接操作するので、ARM9からの書き込みは禁止。
                また、内容参照の前に参照する部分を含むキャッシュラインを破棄
                する必要がある点に注意。

  Arguments:    None.

  Returns:      const WMStatus* - ステータス構造体へのポインタを返す。
 *---------------------------------------------------------------------------*/
const WMStatus *WMi_GetStatusAddress(void)
{
    // 初期化チェック
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return NULL;
    }

    return wm9buf->status;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckMpPacketTimeRequired

  Description:  MP通信の１パケットの転送所要時間が 5600μs 以下であることを
                確認する。

  Arguments:    parentSize  -   親機転送データサイズ。
                childSize   -   子機転送データサイズ。
                childs      -   通信する子機の台数。

  Returns:      BOOL        -   許容範囲である場合にTRUEを返す。
                                5600μs を越える場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL WMi_CheckMpPacketTimeRequired(u16 parentSize, u16 childSize, u8 childs)
{
    s32     mp_time;

    // 一回のMP通信にかかる時間をus単位で計算
    mp_time = ((                       // --- 親機送信部分 ---
                   96                  // Preamble
                   + (24               // 802.11 Header
                      + 4              // TXOP + PollBitmap
                      + 2              // wmHeader
                      + parentSize + 4 // wmFooter( parent )
                      + 4              // FCS
                   ) * 4               // 4us per 1 byte
               ) + (                   // --- 子機送信部分 ---
                       (10             // SIFS
                        + 96           // Preamble
                        + (24          // 802.11 Header
                           + 2         // wmHeader
                           + childSize + 2      // wmFooter( child )
                           + 4         // FCS
                        ) * 4          // 4us per 1 byte
                        + 6            // time to spare
                       ) * childs) + ( // --- MP ACK 送信部分 ---
                                         10     // SIFS
                                         + 96   // Preamble
                                         + (24  // 802.11 Header
                                            + 4 // ACK
                                            + 4 // FCS
                                         ) * 4  // 4us per 1 byte
               ));

    if (mp_time > WM_MAX_MP_PACKET_TIME)
    {
        OS_TWarning
            ("It is required %dus to transfer each MP packet.\nThat should not exceed %dus.\n",
             mp_time, WM_MAX_MP_PACKET_TIME);
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_IsMP

  Description:  現在の MP 通信状態を取得する。

  Arguments:    None.

  Returns:      MP 通信状態であれば TRUE
 *---------------------------------------------------------------------------*/
BOOL WMi_IsMP(void)
{
    WMStatus *status;
    BOOL    isMP;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // 初期化チェック
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return FALSE;
    }
#endif

    // 割込み禁止
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        status = wm9buf->status;
        DC_InvalidateRange(&(status->mp_flag), 4);
        isMP = status->mp_flag;
    }
    else
    {
        isMP = FALSE;
    }

    // 割込み禁止戻し
    (void)OS_RestoreInterrupts(e);

    return isMP;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetAID

  Description:  現在の AID を取得する。
                状態が PARENT, MP_PARENT, CHILD, MP_CHILD のいずれかの時のみ
                有効な値を返す。

  Arguments:    None.

  Returns:      AID
 *---------------------------------------------------------------------------*/
u16 WM_GetAID(void)
{
    u16     myAid;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // 初期化チェック
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }
#endif

    // 割込み禁止
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        myAid = wm9buf->myAid;
    }
    else
    {
        myAid = 0;
    }

    // 割込み禁止戻し
    (void)OS_RestoreInterrupts(e);

    return myAid;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetConnectedAIDs

  Description:  現在の接続相手をビットマップの形で取得する。
                状態が PARENT, MP_PARENT, CHILD, MP_CHILD のいずれかの時のみ
                有効な値を返す。
                子機の場合は親機と接続中は 0x0001 を返す。

  Arguments:    None.

  Returns:      接続している相手の AID のビットマップ
 *---------------------------------------------------------------------------*/
u16 WM_GetConnectedAIDs(void)
{
    u32     connectedAidBitmap;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // 初期化チェック
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }
#endif

    // 割込み禁止
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        connectedAidBitmap = wm9buf->connectedAidBitmap;
    }
    else
    {
        connectedAidBitmap = 0;
    }

    // 割込み禁止戻し
    (void)OS_RestoreInterrupts(e);

#ifdef WM_DEBUG
/*
    if (WMi_CheckStateEx(4, WM_STATE_PARENT, WM_STATE_CHILD, WM_STATE_MP_PARENT, WM_STATE_MP_CHILD)
        != WM_ERRCODE_SUCCESS && connectedAidBitmap != 0)
    {
        WM_WARNING("connectedAidBitmap should be 0, but %04x", connectedAidBitmap);
    }
*/
#endif

    return (u16)connectedAidBitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_GetMPReadyAIDs

  Description:  現在の接続相手のうち、MP を受信できる相手の AID の一覧を
                ビットマップの形で取得する。
                状態が PARENT, MP_PARENT, CHILD, MP_CHILD のいずれかの時のみ
                有効な値を返す。
                子機の場合は親機と接続中は 0x0001 を返す。

  Arguments:    None.

  Returns:      MP を開始している相手の AID のビットマップ
 *---------------------------------------------------------------------------*/
u16 WMi_GetMPReadyAIDs(void)
{
    WMStatus *status;
    u16     mpReadyAidBitmap;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // 初期化チェック
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return FALSE;
    }
#endif

    // 割込み禁止
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        status = wm9buf->status;
        DC_InvalidateRange(&(status->mp_readyBitmap), 2);
        mpReadyAidBitmap = status->mp_readyBitmap;
    }
    else
    {
        mpReadyAidBitmap = FALSE;
    }

    // 割込み禁止戻し
    (void)OS_RestoreInterrupts(e);

    return mpReadyAidBitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_RegisterSleepCallback

  Description:  スリープモードへ移行時に実行されるコールバック関数を登録する
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_RegisterSleepCallback(void)
{
    PM_SetSleepCallbackInfo(&sleepCbInfo, WmSleepCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&sleepCbInfo, PM_CALLBACK_PRIORITY_WM );
}

/*---------------------------------------------------------------------------*
  Name:         WMi_DeleteSleepCallback

  Description:  スリープモードへ移行時に実行されるコールバック関数を削除する
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_DeleteSleepCallback(void)
{
    PM_DeletePreSleepCallback( &sleepCbInfo );
}

/*---------------------------------------------------------------------------*
  Name:         WmSleepCallback

  Description:  無線通信中にスリープモードに入らないようにする
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WmSleepCallback(void *)
{
    /* ---------------------------------------------- *
     * プログラミングガイドラインの 6.5 項の通り、    *
     * 無線通信中に OS_GoSleepMode() を実行することは *
     * 禁止されています。                             *
     * ---------------------------------------------- */
    OS_Panic("Could not sleep during wireless communications.");
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         WM_GetWirelessCommFlag

  Description:  現在 無線通信が可能な状態であるかをチェックする

  Arguments:    None.

  Returns:      WM_WIRELESS_COMM_FLAG_OFF     … 無線通信が許可されていない
                WM_WIRELESS_COMM_FLAG_ON      … 無線通信が許可されている
                WM_WIRELESS_COMM_FLAG_UNKNOWN … DS 上で実行しているため判別不能 
 *---------------------------------------------------------------------------*/
u8 WM_GetWirelessCommFlag( void )
{
    u8 result = WM_WIRELESS_COMM_FLAG_UNKNOWN;
    
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() )
        {
            result = WM_WIRELESS_COMM_FLAG_ON;
        }
        else
        {
            result = WM_WIRELESS_COMM_FLAG_OFF;
        }
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckEnableFlag

  Description:  TWL の本体設定にて設定可能な無線使用許可フラグをチェックする。

  Arguments:    None.

  Returns:      無線機能を使用可能なら TRUE、不可能なら FALSE
 *---------------------------------------------------------------------------*/
BOOL WMi_CheckEnableFlag(void)
{

    if( OS_IsAvailableWireless() == TRUE && OS_IsForceDisableWireless() == FALSE )
    {
        return TRUE;
    }
    return FALSE;
}
#endif

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
