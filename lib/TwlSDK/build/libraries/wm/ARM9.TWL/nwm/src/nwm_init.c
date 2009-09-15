/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_init.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-25#$
  $Rev: 8631 $
  $Author: sato_masaki $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "nwm_common_private.h"
#ifdef SDK_TWL
#include "wm_common.h"
#else
#include "nwm_arm9_private.h"
#include <nitro/wm.h>
#include <nitro/os/ARM9/protectionRegion.h>
#endif

u8 nwmInitialized = 0;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_InitCore(void* sysBuf, u32 bufSize, u8 dmaNo);
static BOOL NwmCheckEnableFlag(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_Init

  Description:  NWMライブラリの初期化処理を行う。
                ARM9側の初期化のみを行う同期関数。

  Arguments:    sysBuf     -   呼び出し元によって確保されたバッファへのポインタ。

                bufSize    -   呼び出し元によって確保されたバッファのサイズ。

                dmaNo      -   NWMが使用するDMA番号。

  Returns:      NWMRetCode -   処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Init(void* sysBuf, u32 bufSize, u8 dmaNo)
{
    if( NWMi_CheckEnableFlag() == FALSE ) // TWL 本体設定で無線を使用しないように設定した場合エラーを返す
    {
        return NWM_RETCODE_NWM_DISABLE;
    }

    return NWMi_InitCore(sysBuf, bufSize, dmaNo);
}

NWMRetCode NWMi_InitCore(void* sysBuf, u32 bufSize, u8 dmaNo)
{
    OSIntrMode e;
    extern NWMArm9Buf *nwm9buf;

    SDK_COMPILER_ASSERT(sizeof(NWMArm9Buf) <= NWM_ARM9NWM_BUF_SIZE);
    SDK_COMPILER_ASSERT(sizeof(NWMArm7Buf) <= NWM_ARM7NWM_BUF_SIZE);
    SDK_COMPILER_ASSERT(sizeof(NWMStatus) <= NWM_STATUS_BUF_SIZE);
    SDK_ASSERT(bufSize >= NWM_SYSTEM_BUF_SIZE);
    
    e = OS_DisableInterrupts();

#ifdef SDK_TWL
    /* WMライブラリの初期化済みを確認 */
    if (WM_CheckInitialized() == WM_ERRCODE_SUCCESS)
    {
        NWM_WARNING("WM has already been initialized.\n");
        (void)OS_RestoreInterrupts(e);
        return NWM_RETCODE_ILLEGAL_STATE;
    }
#endif

    /* NWMライブラリの初期化済みを確認 */
    if (nwmInitialized)
    {
        NWM_WARNING("NWM  has already been initialized.\n");
        (void)OS_RestoreInterrupts(e);
        return NWM_RETCODE_ILLEGAL_STATE;        // 初期化済み
    }

    if (sysBuf == NULL)
    {
        NWM_WARNING("Parameter \"sysBuf\" must not be NULL.\n");
        (void)OS_RestoreInterrupts(e);
        return NWM_RETCODE_INVALID_PARAM;
    }

    if (dmaNo > MI_DMA_MAX_NUM)
    {
        NWM_WARNING("Parameter \"dmaNo\" is over %d.\n", MI_DMA_MAX_NUM);
        (void)OS_RestoreInterrupts(e);
        return NWM_RETCODE_INVALID_PARAM;
    }

    if ((u32)sysBuf & 0x01f)
    {
        NWM_WARNING("Parameter \"sysBuf\" must be 32-byte aligned.\n");
        (void)OS_RestoreInterrupts(e);
        return NWM_RETCODE_INVALID_PARAM;
    }

    // ARM7側でWMライブラリが開始されているか確認
    PXI_Init();
    if (!PXI_IsCallbackReady(PXI_FIFO_TAG_WMW, PXI_PROC_ARM7))
    {
        NWM_WARNING("NWM library on ARM7 side is not ready yet.\n");
        (void)OS_RestoreInterrupts(e);
        return NWM_RETCODE_NWM_DISABLE;
    }

    /* Invalidate all buffer cache */
    DC_InvalidateRange(sysBuf, bufSize);
    MI_DmaClear32(dmaNo, sysBuf, bufSize);
    nwm9buf = (NWMArm9Buf *)sysBuf;
    DC_StoreRange(&nwm9buf, sizeof(nwm9buf));
    nwm9buf->NWM7 = (NWMArm7Buf *)((u32)nwm9buf + NWM_ARM9NWM_BUF_SIZE);
    nwm9buf->status = (NWMStatus *)((u32)(nwm9buf->NWM7) + NWM_ARM7NWM_BUF_SIZE);
    nwm9buf->fifo7to9 = (u32 *)((u32)(nwm9buf->status) + NWM_STATUS_BUF_SIZE);
    
    // FIFOバッファ書き込み許可フラグをクリア
    NWMi_ClearFifoRecvFlag();
    
    nwm9buf->dmaNo = dmaNo;
    
    // エントリー登録用キューを初期化
    {
        s32     i;

        OS_InitMessageQueue(&nwm9buf->apibufQ.q, nwm9buf->apibufQ.fifo, NWM_APIBUF_NUM);
        for (i = 0; i < NWM_APIBUF_NUM; i++)
        {
            // リングバッファを使用可能な状態にクリア
            *((u16 *)(nwm9buf->apibuf[i])) = NWM_API_REQUEST_ACCEPTED;
            DC_StoreRange(nwm9buf->apibuf[i], sizeof(u16));
            (void)OS_SendMessage(&nwm9buf->apibufQ.q, &nwm9buf->apibuf[i][0], OS_MESSAGE_BLOCK);
        }
    }
    
    // FIFO受信関数を設定
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_WMW, NWMi_ReceiveFifo9);

    // clear firmware binary information.
    MI_CpuClear8(&nwm9buf->binData, sizeof(NWMBinaryData));
    
    nwmInitialized = 1;
    nwm9buf->status->state = NWM_STATE_INITIALIZED;  // 例外的にここでだけ初期Stateをセット(本来はARM7の仕事)
    // store cache to ARM9 using part of main memory
    DC_StoreRange(nwm9buf, sizeof(NWMArm9Buf));
    DC_StoreRange(&nwmInitialized, sizeof(u8));
    (void)OS_RestoreInterrupts(e);

    return NWM_RETCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_CheckInitialized

  Description:  NWM ライブラリの初期化済みを確認する。

  Arguments:    None.

  Returns:      int -   NWM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_CheckInitialized(void)
{
    // 初期化済みを確認
    if (!nwmInitialized)
    {
//        NWM_WARNING("NWM library is not initialized yet.\n");
        return NWM_RETCODE_ILLEGAL_STATE;
    }
    return NWM_RETCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_CheckEnableFlag

  Description:  TWL の本体設定にて設定可能な無線使用許可フラグをチェックする。
                現状はフラグのチェックが出来ないため、強制的に TRUE を返す

  Arguments:    None.

  Returns:      無線機能を使用可能なら TRUE、不可能なら FALSE
 *---------------------------------------------------------------------------*/
BOOL NWMi_CheckEnableFlag(void)
{
    if( OS_IsAvailableWireless() == TRUE && OS_IsForceDisableWireless() == FALSE )
    {
        return TRUE;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
