/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_dma.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_utility.h"


/*---------------------------------------------------------------------------*/
/* constants */

static const CARDDmaInterface CARDiDmaUsingFormer =
{
    MIi_CardDmaCopy32,
    MI_StopDma,
};
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static CARDDmaInterface CARDiDmaUsingNew =
{
    MI_Card_A_NDmaCopy,
    MI_StopNDma,
};
#include <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetDmaInterface

  Description:  DMAチャンネルから適切に判定して
                新旧いずれかのDMAコマンドインタフェースを取得。

  Arguments:    channel : DMAチャンネル。
                          0からMI_DMA_MAX_NUMまでの範囲であり、
                          MI_DMA_USING_NEWが有効なら新規DMAであることを示す。

  Returns:      該当する新旧いずれかのDMAインタフェース。
 *---------------------------------------------------------------------------*/
const CARDDmaInterface* CARDi_GetDmaInterface(u32 channel)
{
    const CARDDmaInterface *retval = NULL;
    // 有効なチャンネル範囲ならDMAを使用。
    BOOL    isNew = ((channel & MI_DMA_USING_NEW) != 0);
    channel &= ~MI_DMA_USING_NEW;
    if (channel <= MI_DMA_MAX_NUM)
    {
        // 従来のDMA。
        if (!isNew)
        {
            retval = &CARDiDmaUsingFormer;
        }
        // 新規DMAはTWL環境で動作している場合のみ使用可能。
        else if (!OS_IsRunOnTwl())
        {
            OS_TPanic("NDMA can use only TWL!");
        }
#ifdef SDK_TWL
        else
        {
            retval = &CARDiDmaUsingNew;
        }
#endif
    }
    return retval;
}

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         CARDi_ICInvalidateSmart

  Description:  閾値をもとにIC_InvalidateAll関数かIC_InvalidateRange関数を選択。

  Arguments:    buffer     invalidateすべきバッファ
                length     invalidateすべきサイズ
                threshold  処理を切り替える閾値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ICInvalidateSmart(void *buffer, u32 length, u32 threshold)
{
    if (length >= threshold)
    {
        IC_InvalidateAll();
    }
    else
    {
        IC_InvalidateRange((void *)buffer, length);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_DCInvalidateSmart

  Description:  閾値をもとにDC_FlushAll関数かDC_InvalidateRange関数を選択。
                バッファがキャッシュライン境界整合していない場合は
                先端および終端だけStore操作も自動で行う。

  Arguments:    buffer     invalidateすべきバッファ
                length     invalidateすべきサイズ
                threshold  処理を切り替える閾値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_DCInvalidateSmart(void *buffer, u32 length, u32 threshold)
{
    if (length >= threshold)
    {
        DC_FlushAll();
    }
    else
    {
        u32     len = length;
        u32     pos = (u32)buffer;
        u32     mod = (pos & (HW_CACHE_LINE_SIZE - 1));
        if (mod)
        {
            pos -= mod;
            DC_StoreRange((void *)(pos), HW_CACHE_LINE_SIZE);
            DC_StoreRange((void *)(pos + length), HW_CACHE_LINE_SIZE);
            length += HW_CACHE_LINE_SIZE;
        }
        DC_InvalidateRange((void *)pos, length);
        DC_WaitWriteBufferEmpty();
    }
}

#endif // SDK_ARM9
