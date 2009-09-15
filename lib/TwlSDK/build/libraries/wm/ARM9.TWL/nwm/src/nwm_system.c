/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_system.c

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

#include <twl.h>
#include "nwm_arm9_private.h"
#include "nwm_common_private.h"

NWMArm9Buf *nwm9buf = NULL;
static PMSleepCallbackInfo sleepCbInfo;  //PM に登録するスリープコールバック情報

#ifdef NWM_SUPPORT_HWRESET
PMExitCallbackInfo hwResetCbInfo;
#endif

static void NwmSleepCallback(void *);

/*---------------------------------------------------------------------------*
  Name:         NWMi_GetSystemWork

  Description:  New WMライブラリ内部で使用するバッファの先頭へのポインタを取得する。

  Arguments:    None.

  Returns:      NWMArm9Buf*  -   内部ワークバッファへのポインタを返す。
 *---------------------------------------------------------------------------*/
NWMArm9Buf *NWMi_GetSystemWork(void)
{
//    SDK_NULL_ASSERT(nwm9buf);
    return nwm9buf;
}


/*---------------------------------------------------------------------------*
  Name:         NWMi_ClearFifoRecvFlag

  Description:  New WM7 からのコールバックに用いられたFIFOデータを参照し終えたことを
                New WM7 に通知する。
                New WM7 においてコールバックにFIFOを用いる際、このフラグがアンロック
                されるのを待ってから次のコールバックを編集する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void NWMi_ClearFifoRecvFlag(void)
{
    NWMArm7Buf *w7b;

    if (nwm9buf == NULL) {
        return;
    }
    
    w7b = nwm9buf->NWM7;

    // ARM7が更新した値を読みに行くため、ARM9側のキャッシュを無効化する。
    DC_InvalidateRange(&w7b->callbackSyncFlag, 1);
    
    if (w7b->callbackSyncFlag & NWM_EXCEPTION_CB_MASK)
    {
        // CB排他フラグを下げる
        w7b->callbackSyncFlag &= ~NWM_EXCEPTION_CB_MASK;
        // store cache immediately
        DC_StoreRange(&w7b->callbackSyncFlag, 1);
    }
}


/*---------------------------------------------------------------------------*
  Name:         NWMi_ReceiveFifo9

  Description:  FIFOを介した New WM7 からのコールバックを受信する。

  Arguments:    tag          - 未使用
                fifo_buf_adr - コールバックのパラメータ群へのポインタ
                err          - 未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/

void NWMi_ReceiveFifo9(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err)
{
    #pragma unused( tag )
    NWMCallback *pCallback = (NWMCallback *)fifo_buf_adr;
    NWMCallbackFunc callback;
    NWMArm9Buf *w9b = nwm9buf;

    if (w9b == NULL) {
        return;
    }
    
    if (err) {
        NWM_WARNING("NWM9 FIFO receive error. :%d\n", err);
        return;
    }

    if (!fifo_buf_adr) {
        NWM_WARNING("NWM9 FIFO receive error.(NULL address) :%d\n", err);
        return;
    }

    DC_InvalidateRange(w9b->fifo7to9, NWM_APIFIFO_BUF_SIZE);
    DC_InvalidateRange(w9b->status, NWM_STATUS_BUF_SIZE);
    if ((u32)pCallback != (u32)(w9b->fifo7to9))
    {
        DC_InvalidateRange(pCallback, NWM_APIFIFO_BUF_SIZE);
    }

    NWM_DPRINTF("APIID%04x\n", pCallback->apiid);

    // NWM_LoadDevice が失敗した場合、WM_UnloadDevice が成功した場合にはスリープコールバックを削除
    if( (pCallback->apiid == NWM_APIID_LOAD_DEVICE   && pCallback->retcode != NWM_RETCODE_SUCCESS) ||
        (pCallback->apiid == NWM_APIID_UNLOAD_DEVICE && pCallback->retcode == NWM_RETCODE_SUCCESS) ||
         pCallback->apiid == NWM_APIID_INSTALL_FIRMWARE )
    {
        NWMi_DeleteSleepCallback();
    }

    // apiidに応じたコールバック処理 (コールバックが未設定(NULL)なら何もしない)
    {
        NWMSendFrameCallback *pSfcb = (NWMSendFrameCallback *)pCallback;
        NWMRetCode result = NWM_RETCODE_FAILED;
        
        if (pCallback->apiid == NWM_APIID_SEND_FRAME)
        {
            if (pCallback->retcode != NWM_RETCODE_INDICATION && NULL != pSfcb->callback)
            {
                NWM_DPRINTF("Execute CallbackFunc APIID 0x%04x\n", pCallback->apiid);
                (pSfcb->callback)((void *)pCallback);
            }

        }
    }

    // in the case of several special APIs
    if (pCallback->apiid == NWM_APIID_SEND_FRAME) {
        NWMSendFrameCallback *pSfcb = (NWMSendFrameCallback *)pCallback;

    } else {

#ifdef NWM_SUPPORT_HWRESET
        if (pCallback->apiid == NWM_APIID_UNLOAD_DEVICE
            || pCallback->apiid == NWM_APIID_INSTALL_FIRMWARE
            || (pCallback->apiid == NWM_APIID_LOAD_DEVICE
                && pCallback->retcode != NWM_RETCODE_SUCCESS)) {
            // delete HW reset callback
            PM_DeletePostExitCallback(&hwResetCbInfo);
        }
#endif
        
        // other APIs
        callback = w9b->callbackTable[pCallback->apiid];

        // in case START_SCAN callback, scan buffer cache must be invalidated.
        if (pCallback->apiid == NWM_APIID_START_SCAN) {
            NWMStartScanCallback *psscb = (NWMStartScanCallback *)pCallback;

            DC_InvalidateRange(psscb->bssDesc[0], psscb->allBssDescSize);
        }

        if (NULL != callback)
        {
            NWM_DPRINTF("Execute CallbackFunc APIID 0x%04x\n", pCallback->apiid);
            (callback) ((void *)pCallback);
        }

    }

    MI_CpuClear8(pCallback, NWM_APIFIFO_BUF_SIZE);
    DC_StoreRange(pCallback, NWM_APIFIFO_BUF_SIZE);
    if (w9b) { // nwm might be terminated after callback
        NWMi_ClearFifoRecvFlag();
    }

}

/*---------------------------------------------------------------------------*
  Name:         NWM_GetState

  Description:  NWMライブラリの内部状態を確認する。
                NWMi_CheckState と似た目的で使用されるが用途が大きく異なるので
                別々に用意している。

  Arguments:    None.

  Returns:      u16  -   NWM の内部状態を示す整数
 *---------------------------------------------------------------------------*/
u16 NWM_GetState(void)
{
    NWMStatus*  nwmStatus;
    NWMArm9Buf* sys = NWMi_GetSystemWork();
    u16 state = NWM_STATE_NONE;

    if (sys) {
        nwmStatus = sys->status;
        DC_InvalidateRange(nwmStatus, 2);
        state = nwmStatus->state;
    }

    return state;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_CheckState

  Description:  NWMライブラリの内部状態を確認する。
                許可される状態を示すWMState型のパラメータを列挙して指定する。

  Arguments:    paramNum    -   仮想引数の数。
                ...         -   仮想引数。

  Returns:      int -   NWM_RETCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_CheckState(s32 paramNum, ...)
{
    NWMRetCode result;
    u16     now;
    u32     temp;
    va_list vlist;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_NULL_ASSERT(sys);

    // 初期化済みを確認
    result = NWMi_CheckInitialized();
    NWM_CHECK_RESULT(result);

    // 現ステート取得
    DC_InvalidateRange(&(sys->status->state), 2);
    now = sys->status->state;

    // 一致確認
    result = NWM_RETCODE_ILLEGAL_STATE;
    va_start(vlist, paramNum);
    for (; paramNum; paramNum--)
    {
        temp = va_arg(vlist, u32);
        if (temp == now)
        {
            result = NWM_RETCODE_SUCCESS;
        }
    }
    va_end(vlist);

    if (result == NWM_RETCODE_ILLEGAL_STATE)
    {
        NWM_WARNING("New WM state is \"0x%04x\" now. So can't execute request.\n", now);
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NWM_GetAllowedChannel

  Description:  新無線で使用可能な IEEE 802.11b/g のチャンネルリストを取得する。

  Arguments:    None.

  Returns:      u16  -   WM と同一フォーマットのチャンネルビットマップを返す。
 *---------------------------------------------------------------------------*/
u16 NWM_GetAllowedChannel(void)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    return sys->status->allowedChannel;
}

/*---------------------------------------------------------------------------*
  Name:         NWM_CalcLinkLevel

  Description:  nwm_common_private.h にて定義されている閾値からリンクレベルを算出

  Arguments:    s16          -   Atheros ドライバから通知されるRSSI値

  Returns:      u16          -   WM と同様のリンクレベル
 *---------------------------------------------------------------------------*/
u16 NWM_CalcLinkLevel(s16 rssi)
{

/* [TODO] 動作モードによって切り替える必要がある? 必要なら条件文を改める必要あり */

    if(1) /* Infra Structure Mode */
    {
        if(rssi < NWM_RSSI_INFRA_LINK_LEVEL_1)
        {
            return WM_LINK_LEVEL_0;
        }
        if(rssi < NWM_RSSI_INFRA_LINK_LEVEL_2)
        {
            return WM_LINK_LEVEL_1;
        }
        if(rssi < NWM_RSSI_INFRA_LINK_LEVEL_3)
        {
            return WM_LINK_LEVEL_2;
        }

        return WM_LINK_LEVEL_3;
    }
    else if(0) /*Ad Hoc Mode*/
    {
        if(rssi < NWM_RSSI_ADHOC_LINK_LEVEL_1)
        {
            return WM_LINK_LEVEL_0;
        }
        if(rssi < NWM_RSSI_ADHOC_LINK_LEVEL_2)
        {
            return WM_LINK_LEVEL_1;
        }
        if(rssi < NWM_RSSI_ADHOC_LINK_LEVEL_3)
        {
            return WM_LINK_LEVEL_2;
        }
        return WM_LINK_LEVEL_3;
    }

}

/*---------------------------------------------------------------------------*
  Name:         NWM_GetDispersionScanPeriod

  Description:  STA として AP もしくは DS 親機を探索する際に設定すべき探索限度時間を取得する。

  Arguments:    u16 scanType - スキャンタイプ、NWM_SCANTYPE_PASSIVE or NWM_SCANTYPE_ACTIVE
  
  Returns:      u16 -   設定すべき探索限度時間(ms)。
 *---------------------------------------------------------------------------*/
u16 NWM_GetDispersionScanPeriod(u16 scanType)
{
    u8      mac[6];
    u16     ret;
    s32     i;

    OS_GetMacAddress(mac);
    for (i = 0, ret = 0; i < 6; i++)
    {
        ret += mac[i];
    }
    ret += OS_GetVBlankCount();
    ret *= 13;
    
    if( scanType == NWM_SCANTYPE_ACTIVE )
    {
        ret = (u16)(NWM_DEFAULT_ACTIVE_SCAN_PERIOD + (ret % 10));
    }
    else /* 未知のスキャンタイプはパッシブスキャンと同等に扱う */
    {
        ret = (u16)(NWM_DEFAULT_PASSIVE_SCAN_PERIOD + (ret % 10));
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_RegisterSleepCallback

  Description:  スリープモードへ移行時に実行されるコールバック関数を登録する
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_RegisterSleepCallback(void)
{
    PM_SetSleepCallbackInfo(&sleepCbInfo, NwmSleepCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&sleepCbInfo, PM_CALLBACK_PRIORITY_NWM );
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_DeleteSleepCallback

  Description:  スリープモードへ移行時に実行されるコールバック関数を削除する
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_DeleteSleepCallback(void)
{
    PM_DeletePreSleepCallback( &sleepCbInfo );
}

/*---------------------------------------------------------------------------*
  Name:         NwmSleepCallback

  Description:  無線通信中にスリープモードに入らないようにする。
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NwmSleepCallback(void *)
{
    /* ---------------------------------------------- *
     * プログラミングガイドラインの通り、             *
     * 無線通信中に OS_GoSleepMode() を実行することは *
     * 禁止されています。                             *
     * ---------------------------------------------- */
    OS_Panic("Could not sleep during wireless communications.");
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
