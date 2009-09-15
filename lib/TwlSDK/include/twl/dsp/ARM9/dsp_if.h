/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp
  File:     dsp_if.h

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

#ifndef TWL_DSP_IF_H_
#define TWL_DSP_IF_H_

#include <twl/types.h>
#include <twl/hw/ARM9/ioreg_DSP.h>
#include <nitro/os/common/emulator.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
typedef enum
{
    DSP_FIFO_MEMSEL_DATA    = (0x0 << REG_DSP_PCFG_MEMSEL_SHIFT),
    DSP_FIFO_MEMSEL_MMIO    = (0x1 << REG_DSP_PCFG_MEMSEL_SHIFT),
    DSP_FIFO_MEMSEL_PROGRAM = (0x5 << REG_DSP_PCFG_MEMSEL_SHIFT)
}
DSPFifoMemSel;

typedef enum
{
    DSP_FIFO_RECV_2B            = (0x0 << REG_DSP_PCFG_DRS_SHIFT),
    DSP_FIFO_RECV_16B           = (0x1 << REG_DSP_PCFG_DRS_SHIFT),
    DSP_FIFO_RECV_32B           = (0x2 << REG_DSP_PCFG_DRS_SHIFT),
    DSP_FIFO_RECV_CONTINUOUS    = (0x3 << REG_DSP_PCFG_DRS_SHIFT)
}
DSPFifoRecvLength;

typedef enum
{
    DSP_FIFO_INTR_SEND_EMPTY        = REG_DSP_PCFG_WFEIE_MASK,
    DSP_FIFO_INTR_SEND_FULL         = REG_DSP_PCFG_WFFIE_MASK,
    DSP_FIFO_INTR_RECV_NOT_EMPTY    = REG_DSP_PCFG_RFNEIE_MASK,
    DSP_FIFO_INTR_RECV_FULL         = REG_DSP_PCFG_RFFIE_MASK
}
DSPFifoIntr;

// for complex API
typedef enum
{
    DSP_FIFO_FLAG_SRC_INC   = (0UL << 0),
    DSP_FIFO_FLAG_SRC_FIX   = (1UL << 0),

    DSP_FIFO_FLAG_DEST_INC  = (0UL << 1),
    DSP_FIFO_FLAG_DEST_FIX  = (1UL << 1),

    DSP_FIFO_FLAG_RECV_UNIT_CONTINUOUS  = (0UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_2B          = (1UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_16B         = (2UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_32B         = (3UL << 8),
    DSP_FIFO_FLAG_RECV_MASK             = (3UL << 8)
}
DSPFifoFlag;

// 旧仕様のロード処理をしばらくの間サポートする。
#define DSP_SUPPORT_OBSOLETE_LOADER

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOn

  Description:  DSPの電源をオンにして、リセット状態にします。
                DSPを起動させるには DSP_ResetOff() を呼び出す必要があります。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_PowerOnCore(void);
SDK_INLINE void DSP_PowerOn(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_PowerOnCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOff

  Description:  DSPの電源をオフにします。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_PowerOffCore(void);
SDK_INLINE void DSP_PowerOff(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_PowerOffCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOn

  Description:  DSPをリセットします。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetOnCore(void);
SDK_INLINE void DSP_ResetOn(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetOnCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOff

  Description:  リセット状態からDSPを起動します。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetOffCore(void);
SDK_INLINE void DSP_ResetOff(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetOffCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOffEx

  Description:  リセット状態からDSPを起動します。

  Arguments:    bitmap : DSPからARM9への割り込みを許可するセマフォ、リプライレジスタのビットマップ。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetOffExCore(u16 bitmap);
SDK_INLINE void DSP_ResetOffEx(u16 bitmap)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetOffExCore(bitmap);
    }
}


/*---------------------------------------------------------------------------*
  Name:         DSP_ResetInterface

  Description:  レジスタの初期化を行います。
                DSPをリセットした場合に呼び出す必要があります。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetInterfaceCore(void);
SDK_INLINE void DSP_ResetInterface(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetInterfaceCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableRecvDataInterrupt

  Description:  指定したリプライレジスタの割り込みを有効にします。

  Arguments:    dataNo : リプライレジスタ番号 (0-2)

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_EnableRecvDataInterruptCore(u32 dataNo);
SDK_INLINE void DSP_EnableRecvDataInterrupt(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_EnableRecvDataInterruptCore(dataNo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableRecvDataInterrupt

  Description:  指定したリプライレジスタの割り込みを無効にします。

  Arguments:    dataNo : リプライレジスタの番号 (0-2)

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_DisableRecvDataInterruptCore(u32 dataNo);
SDK_INLINE void DSP_DisableRecvDataInterrupt(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_DisableRecvDataInterruptCore(dataNo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendDataIsEmpty

  Description:  指定したコマンドレジスタのデータをDSPが受信済みかチェックします。

  Arguments:    dataNo : コマンドレジスタの番号 (0-2)

  Returns:      DSPが既にデータを受信していれば TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_SendDataIsEmptyCore(u32 dataNo);
SDK_INLINE BOOL DSP_SendDataIsEmpty(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_SendDataIsEmptyCore(dataNo);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvDataIsReady

  Description:  指定したリプライレジスタにDSPからデータが送信されたかチェックします。

  Arguments:    dataNo : リプライレジスタの番号 (0-2)

  Returns:      DSPがデータを送信していれば TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_RecvDataIsReadyCore(u32 dataNo);
SDK_INLINE BOOL DSP_RecvDataIsReady(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_RecvDataIsReadyCore(dataNo);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendData

  Description:  DSPにデータを送信します。

  Arguments:    dataNo : コマンドレジスタの番号 (0-2)
                data   : 送信するデータ

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SendDataCore(u32 dataNo, u16 data);
SDK_INLINE void DSP_SendData(u32 dataNo, u16 data)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SendDataCore(dataNo, data);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvData

  Description:  DSPからのデータを受信します。

  Arguments:    dataNo : リプライレジスタの番号 (0-2)

  Returns:      受信したデータ
 *---------------------------------------------------------------------------*/
u16 DSP_RecvDataCore(u32 dataNo);
SDK_INLINE u16 DSP_RecvData(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_RecvDataCore(dataNo);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableFifoInterrupt

  Description:  FIFOの割り込みを有効にします。

  Arguments:    type : FIFOの割り込み要因

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_EnableFifoInterruptCore(DSPFifoIntr type);
SDK_INLINE void DSP_EnableFifoInterrupt(DSPFifoIntr type)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_EnableFifoInterruptCore(type);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableFifoInterrupt

  Description:  FIFOの割り込みを無効にします。

  Arguments:    type : FIFOの割り込み要因

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_DisableFifoInterruptCore(DSPFifoIntr type);
SDK_INLINE void DSP_DisableFifoInterrupt(DSPFifoIntr type)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_DisableFifoInterruptCore(type);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendFifoEx

  Description:  DSPのメモリ空間内にデータを書き込みます。

  Arguments:    memsel : データを書き込むメモリ空間
                dest   : 書き込み先アドレス (ハーフワード).
                         上位 16 ビットを設定する場合は、事前に DMA レジスタに
                         値を設定する必要があります。
                src    : 書き込むデータ
                size   : 書き込むデータの長さ (ハーフワード)
                flags  : 読み出しモードを DSPFifoFlag の
                         DSP_FIFO_FLAG_RECV_UNIT_* 以外から選択します。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SendFifoExCore(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags);
SDK_INLINE void DSP_SendFifoEx(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SendFifoExCore(memsel, dest, src, size, flags);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendFifo

  Description:  DSPのメモリ空間内にデータを書き込みます。

  Arguments:    memsel : データを書き込むメモリ空間
                dest   : 書き込み先アドレス (ハーフワード).
                         上位 16 ビットを設定する場合は、事前に DMA レジスタに
                         値を設定する必要があります。
                src    : 書き込むデータ
                size   : 書き込むデータの長さ (ハーフワード)

  Returns:      なし
 *---------------------------------------------------------------------------*/
static inline void DSP_SendFifo(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SendFifoExCore(memsel, dest, src, size, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifoEx

  Description:  DSPのメモリ空間内からデータを読み込みます。

  Arguments:    memsel : データを読み込むメモリ空間 (プログラムメモリ以外)
                dest   : 受信先アドレス
                src    : 受信元アドレス (ハーフワード)
                         上位 16 ビットを設定する場合は、事前に DMA レジスタに
                         値を設定する必要があります。
                size   : 読み込むデータのサイズ (ハーフワード)
                flags  : 書き込みモードを DSPFifoFlag から選択します。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_RecvFifoExCore(DSPFifoMemSel memsel, u16 *dest, u16 src, int size, u16 flags);
SDK_INLINE void DSP_RecvFifoEx(DSPFifoMemSel memsel, u16 *dest, u16 src, int size, u16 flags)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_RecvFifoExCore(memsel, dest, src, size, flags);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifo

  Description:  DSPのメモリ空間内からデータを読み込みます。

  Arguments:    memsel : データを読み込むメモリ空間 (プログラムメモリ以外)
                dest   : 受信先アドレス
                src    : 受信元アドレス (ハーフワード)
                         上位 16 ビットを設定する場合は、事前に DMA レジスタに
                         値を設定する必要があります。
                size   : 読み込むデータのサイズ (ハーフワード)

  Returns:      なし
 *---------------------------------------------------------------------------*/
static inline void DSP_RecvFifo(DSPFifoMemSel memsel, u16* dest, u16 src, int size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_RecvFifoExCore(memsel, dest, src, size, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetCommandReg

  Description:  コマンドレジスタに値を書き込みます。

  Arguments:    regNo : コマンドレジスタ番号 (0-2)
                data  : データ

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SetCommandRegCore(u32 regNo, u16 data);
SDK_INLINE void DSP_SetCommandReg(u32 regNo, u16 data)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SetCommandRegCore(regNo, data);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetReplyReg

  Description:  リプライレジスタから値を読み込みます。

  Arguments:    regNo : リプライレジスタ番号 (0-2)

  Returns:      読み込んだデータ
 *---------------------------------------------------------------------------*/
u16 DSP_GetReplyRegCore(u32 regNo);
SDK_INLINE u16 DSP_GetReplyReg(u32 regNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_GetReplyRegCore(regNo);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetSemaphore

  Description:  ARMからDSPへの通知用セマフォレジスタに値を書き込みます。

  Arguments:    mask : セットする値

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SetSemaphoreCore(u16 mask);
SDK_INLINE void DSP_SetSemaphore(u16 mask)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SetSemaphoreCore(mask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetSemaphore

  Description:  DSPからARMへの通知用セマフォレジスタの値を読み込みます。

  Arguments:    なし

  Returns:      DSPがセマフォに書き込んだ値
 *---------------------------------------------------------------------------*/
u16 DSP_GetSemaphoreCore(void);
SDK_INLINE u16 DSP_GetSemaphore(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_GetSemaphoreCore();
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ClearSemaphore

  Description:  DSPからARMへの通知用セマフォレジスタの値をクリアします。

  Arguments:    mask : クリアするビット

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ClearSemaphoreCore(u16 mask);
SDK_INLINE void DSP_ClearSemaphore(u16 mask)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ClearSemaphoreCore(mask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MaskSemaphore

  Description:  割り込みを無効にするDSPからARMへの通知用セマフォレジスタのビットを設定します。

  Arguments:    mask : 割り込みを無効にするビット

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_MaskSemaphoreCore(u16 mask);
SDK_INLINE void DSP_MaskSemaphore(u16 mask)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_MaskSemaphoreCore(mask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CheckSemaphoreRequest

  Description:  セマフォレジスタによる割り込み要求があるかチェックします。

  Arguments:    なし

  Returns:      要求がある場合は TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_CheckSemaphoreRequestCore(void);
SDK_INLINE BOOL DSP_CheckSemaphoreRequest(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_CheckSemaphoreRequestCore();
    }
    return FALSE;
}

#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * 以下は現在使用されていないと思われる廃止候補インタフェース。
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadFileAuto

  Description:  COFF形式のDSPプログラムをロードして必要なWRAMをDSPに割り当てる

  Arguments:    image : COFFファイル

  Returns:      成功ならば TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadFileAutoCore(const void *image);
SDK_INLINE BOOL DSP_LoadFileAuto(const void *image)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_LoadFileAutoCore(image);
    }
    return FALSE;
}

#endif


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_IF_H_ */
