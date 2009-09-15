/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_byteaccess.c

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

#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/byteaccess.h>


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_Load16

  Description:  DSPデータメモリ空間から16ビット読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)

  Returns:      読み出した16ビットデータ。
 *---------------------------------------------------------------------------*/
u16 DSP_Load16(DSPAddrInARM offset)
{
    OSIntrMode  cpsr = OS_DisableInterrupts();
    u16     result = DSP_LoadWord(DSP_WORD_TO_DSP(offset));
    if ((offset & 1) != 0)
    {
        result = (u16)((DSP_LoadWord(DSP_WORD_TO_DSP(offset + 2)) << (16 - 8)) | (result >> 8));
    }
    (void)OS_RestoreInterrupts(cpsr);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Load32

  Description:  DSPデータメモリ空間から32ビット読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)

  Returns:      読み出した32ビットデータ。
 *---------------------------------------------------------------------------*/
u32 DSP_Load32(DSPAddrInARM offset)
{
    u32     result = (u32)((DSP_LoadWord(DSP_WORD_TO_DSP(offset + 0)) << 0) |
                           (DSP_LoadWord(DSP_WORD_TO_DSP(offset + 2)) << 16));
    if ((offset & 1) != 0)
    {
        result = (u32)((DSP_LoadWord(DSP_WORD_TO_DSP(offset + 4)) << (32 - 8)) | (result >> 8));
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadData

  Description:  DSPデータメモリ空間から任意長のデータを読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)
                buffer : 転送先バッファ
                length : 転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_LoadData(DSPAddrInARM offset, void *buffer, u32 length)
{
    if (length > 0)
    {
        // まず転送元の先端を揃える。
        if ((offset & 1) != 0)
        {
            MI_StoreLE8(buffer, DSP_Load8(offset));
            buffer = (u8 *)buffer + 1;
            offset += 1;
            length -= 1;
        }
        // bufferのアラインメントが揃っていればそのままワード転送し、
        // そうでなければ非効率的だがワード転送の繰り返しで処理する。
        // (この対応はDSP_RecvFifo関数内でおこなってもよいのではないか?)
        if (((u32)buffer & 1) == 0)
        {
            u32     aligned = (u32)(length & ~1);
            if (aligned > 0)
            {
                OSIntrMode  cpsr = OS_DisableInterrupts();
                DSP_RecvFifo(DSP_FIFO_MEMSEL_DATA,
                             (u16*)buffer,
                             DSP_WORD_TO_DSP(offset),
                             DSP_WORD_TO_DSP(aligned));
                reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
                (void)OS_RestoreInterrupts(cpsr);
                buffer = (u8 *)buffer + aligned;
                offset += aligned; 
                length -= aligned;
            }
        }
        else
        {
            while (length >= 2)
            {
                MI_StoreLE16(buffer, DSP_LoadWord(DSP_WORD_TO_DSP(offset)));
                buffer = (u8 *)buffer + 2;
                offset += 2;
                length -= 2;
            }
        }
        // 必要なら終端を読み込む。
        if (length > 0)
        {
            MI_StoreLE8(buffer, DSP_Load8(offset));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Store8

  Description:  DSPデータメモリ空間へ8ビット書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                value  : 書き込む8ビット値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Store8(DSPAddrInARM offset, u8 value)
{
    if ((offset & 1) == 0)
    {
        u16     tmp = DSP_LoadWord(DSP_WORD_TO_DSP(offset));
        tmp = (u16)((tmp & 0xFF00) | ((value << 0) & 0x00FF));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset), tmp);
    }
    else
    {
        u16     tmp = DSP_LoadWord(DSP_WORD_TO_DSP(offset));
        tmp = (u16)((tmp & 0x00FF) | ((value << 8) & 0xFF00));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset), tmp);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Store16

  Description:  DSPデータメモリ空間へ16ビット書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                value  : 書き込む16ビット値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Store16(DSPAddrInARM offset, u16 value)
{
    if ((offset & 1) == 0)
    {
        DSP_StoreWord(DSP_WORD_TO_DSP(offset), value);
    }
    else
    {
        DSP_Store8(offset + 0, (u8)(value >> 0));
        DSP_Store8(offset + 1, (u8)(value >> 8));
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Store32

  Description:  DSPデータメモリ空間へ32ビット書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                value  : 書き込む32ビット値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Store32(DSPAddrInARM offset, u32 value)
{
    if ((offset & 1) == 0)
    {
        DSP_StoreWord(DSP_WORD_TO_DSP(offset + 0), (u16)(value >> 0));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset + 2), (u16)(value >> 16));
    }
    else
    {
        DSP_Store8(offset + 0, (u8)(value >> 0));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset + 1), (u16)(value >> 8));
        DSP_Store8(offset + 3, (u8)(value >> 24));
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StoreData

  Description:  DSPデータメモリ空間へ任意長のデータを書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                buffer : 転送元バッファ
                length : 転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_StoreData(DSPAddrInARM offset, const void *buffer, u32 length)
{
    if (length > 0)
    {
        // まず転送先の先端を揃える。
        if ((offset & 1) != 0)
        {
            DSP_Store8(offset, MI_LoadLE8(buffer));
            buffer = (const u8 *)buffer + 1;
            offset += 1;
            length -= 1;
        }
        // bufferのアラインメントが揃っていればそのままワード転送し、
        // そうでなければ非効率的だがワード転送の繰り返しで処理する。
        // (この対応はDSP_SendFifo関数内でおこなってもよいのではないか?)
        if (((u32)buffer & 1) == 0)
        {
            u32     aligned = (u32)(length & ~1);
            if (aligned > 0)
            {
                OSIntrMode  cpsr = OS_DisableInterrupts();
                DSP_SendFifo(DSP_FIFO_MEMSEL_DATA,
                             DSP_WORD_TO_DSP(offset),
                             (const u16 *)buffer,
                             DSP_WORD_TO_DSP(aligned));
            //    reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
                (void)OS_RestoreInterrupts(cpsr);
                buffer = (const u8 *)buffer + aligned;
                offset += aligned; 
                length -= aligned;
            }
        }
        else
        {
            while (length >= 2)
            {
                DSP_StoreWord(DSP_WORD_TO_DSP(offset), MI_LoadLE16(buffer));
                buffer = (const u8 *)buffer + 2;
                offset += 2;
                length -= 2;
            }
        }
        // 必要なら終端を書き込む。
        if (length > 0)
        {
            DSP_Store8(offset, MI_LoadLE8(buffer));
        }
    }
}
