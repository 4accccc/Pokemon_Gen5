/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp - common
  File:     byteaccess.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-03#$
  $Rev: 10855 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_BYTEACCESS_H_
#define TWL_DSP_BYTEACCESS_H_


#ifdef SDK_TWL
#include <twl/types.h>
#include <twl/os.h>
#else
#include <dsp/types.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

// プロセッサ間通信のデータ共有に使用する型定義。
typedef u16 DSPAddr;        // DSP内でアドレスを表現する型 (2バイト1ワード)
typedef u16 DSPWord;        // DSP内でサイズを表現する型 (2バイト1ワード)
typedef u16 DSPByte;        // DSP内でバイト単位を表現する型 (1バイト1ワード)
typedef u32 DSPWord32;      // DSP内でサイズを表現する型 (2バイト1ワード)
typedef u32 DSPByte32;      // DSP内でバイト単位を表現する型 (1バイト1ワード)
typedef u32 DSPAddrInARM;   // DSP内アドレスをバイト単位に変換した型

// 明示的な型変換マクロ。
#define DSP_ADDR_TO_ARM(address)    (u32)((address) << 1)
#define DSP_ADDR_TO_DSP(address)    (u16)((u32)(address) >> 1)
#define DSP_WORD_TO_ARM(word)       (u16)((word) << 1)
#define DSP_WORD_TO_DSP(word)       (u16)((word) >> 1)
#define DSP_WORD_TO_ARM32(word)     (u32)((word) << 1)
#define DSP_WORD_TO_DSP32(word)     (u32)((word) >> 1)
#define DSP_32BIT_TO_ARM(value)     (u32)(((u32)(value) >> 16) | ((u32)(value) << 16))
#define DSP_32BIT_TO_DSP(value)     (u32)(((u32)(value) >> 16) | ((u32)(value) << 16))
#ifdef SDK_TWL
#define DSP_BYTE_TO_UNIT(byte)      (u16)(byte)
#define DSP_UNIT_TO_BYTE(unit)      (u16)(unit)
#else
#define DSP_BYTE_TO_UNIT(byte)      (u16)((byte) >> 1)
#define DSP_UNIT_TO_BYTE(unit)      (u16)((unit) << 1)
#endif

// ネイティブなsizeof(char)のサイズ。 (DSP側では2, ARM側では1)
#define DSP_WORD_UNIT       (3 - sizeof(DSPWord))


/*---------------------------------------------------------------------------*/
/* functions */

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadWord

  Description:  DSPデータメモリ空間から1ワード(偶数単位の16ビット)読み出し。

  Arguments:    offset : 転送元DSPアドレス (ワード単位)

  Returns:      読み出した16ビットデータ。
 *---------------------------------------------------------------------------*/
SDK_INLINE u16 DSP_LoadWord(DSPAddr offset)
{
    u16     value;
    OSIntrMode  cpsr = OS_DisableInterrupts();
    DSP_RecvFifo(DSP_FIFO_MEMSEL_DATA, &value, offset, DSP_WORD_TO_DSP(sizeof(u16)));
    reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
    (void)OS_RestoreInterrupts(cpsr);
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StoreWord

  Description:  DSPデータメモリ空間へ1ワード(偶数単位の16ビット)読み出し書き込み。

  Arguments:    offset : 転送先DSPアドレス (ワード単位)
                value  : 書き込むワード値

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StoreWord(DSPAddr offset, u16 value)
{
    OSIntrMode  cpsr = OS_DisableInterrupts();
    DSP_SendFifo(DSP_FIFO_MEMSEL_DATA, offset, &value, DSP_WORD_TO_DSP(sizeof(u16)));
//    reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
    (void)OS_RestoreInterrupts(cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Load8

  Description:  DSPデータメモリ空間から8ビット読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)

  Returns:      読み出した8ビットデータ。
 *---------------------------------------------------------------------------*/
SDK_INLINE u8 DSP_Load8(DSPAddrInARM offset)
{
    return (u8)(DSP_LoadWord(DSP_WORD_TO_DSP(offset)) >> ((offset & 1) << 3));
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Load16

  Description:  DSPデータメモリ空間から16ビット読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)

  Returns:      読み出した16ビットデータ。
 *---------------------------------------------------------------------------*/
u16     DSP_Load16(DSPAddrInARM offset);

/*---------------------------------------------------------------------------*
  Name:         DSP_Load32

  Description:  DSPデータメモリ空間から32ビット読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)

  Returns:      読み出した32ビットデータ。
 *---------------------------------------------------------------------------*/
u32     DSP_Load32(DSPAddrInARM offset);

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadData

  Description:  DSPデータメモリ空間から任意長のデータを読み出し。

  Arguments:    offset : 転送元DSPアドレス (バイト単位)
                buffer : 転送先バッファ
                length : 転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_LoadData(DSPAddrInARM offset, void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_Store8

  Description:  DSPデータメモリ空間へ8ビット書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                value  : 書き込む8ビット値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_Store8(DSPAddrInARM offset, u8 value);

/*---------------------------------------------------------------------------*
  Name:         DSP_Store16

  Description:  DSPデータメモリ空間へ16ビット書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                value  : 書き込む16ビット値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_Store16(DSPAddrInARM offset, u16 value);

/*---------------------------------------------------------------------------*
  Name:         DSP_Store32

  Description:  DSPデータメモリ空間へ32ビット書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                value  : 書き込む32ビット値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_Store32(DSPAddrInARM offset, u32 value);

/*---------------------------------------------------------------------------*
  Name:         DSP_StoreData

  Description:  DSPデータメモリ空間へ任意長のデータを書き込み。

  Arguments:    offset : 転送先DSPアドレス (バイト単位)
                buffer : 転送元バッファ
                length : 転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_StoreData(DSPAddrInARM offset, const void *buffer, u32 length);


#endif // SDK_TWL


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_BYTEACCESS_H_ */
