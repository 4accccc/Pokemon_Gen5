/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_utility.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_UTILITY_H__
#define NITRO_LIBRARIES_CARD_UTILITY_H__


#include <nitro/types.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

// 新旧のDMAを切り替えるためのコマンドインタフェース
typedef struct CARDDmaInterface
{
    void  (*Recv)(u32 channel, const void *src, void *dst, u32 len);
    void  (*Stop)(u32 channel);
}
CARDDmaInterface;


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
const CARDDmaInterface* CARDi_GetDmaInterface(u32 channel);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ICInvalidateSmart

  Description:  閾値をもとにIC_InvalidateAll関数かIC_InvalidateRange関数を選択。

  Arguments:    buffer     invalidateすべきバッファ
                length     invalidateすべきサイズ
                threshold  処理を切り替える閾値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ICInvalidateSmart(void *buffer, u32 length, u32 threshold);

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
void CARDi_DCInvalidateSmart(void *buffer, u32 length, u32 threshold);


#ifdef __cplusplus
} // extern "C"
#endif



#endif // NITRO_LIBRARIES_CARD_UTILITY_H__
