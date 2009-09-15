/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dspcomponents - include - dsp - audio
  File:     g711.h

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
#ifndef DSP_AUDIO_G711_H__
#define DSP_AUDIO_G711_H__


#include <twl/dsp/common/pipe.h>
#include <twl/dsp/common/audio.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* functions */

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
 * internal core functions placed on LTDMAIN.
 *---------------------------------------------------------------------------*/

void DSPi_OpenStaticComponentG711Core(FSFile *file);
BOOL DSPi_LoadG711Core(FSFile *file, int slotB, int slotC);
void DSPi_UnloadG711Core(void);
void DSPi_EncodeG711Core(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);
void DSPi_DecodeG711Core(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);
BOOL DSPi_TryWaitForG711Core(void);
void DSPi_WaitForG711Core(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_OpenStaticComponentG711

  Description:  G.711コンポーネント用のメモリファイルを開く。
                ファイルシステムに事前に用意しておく必要がなくなるが
                静的メモリとしてリンクされるためプログラムサイズが増加する。
  
  Arguments:    file : メモリファイルを開くFSFile構造体。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_OpenStaticComponentG711(FSFile *file)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_OpenStaticComponentG711Core(file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadG711

  Description:  G.711符号化方式をサポートするコンポーネントをDSPへロード。
  
  Arguments:    file  : G.711コンポーネントのファイル。
                slotB : コードメモリのために使用を許可されたWRAM-B。
                slotC : データメモリのために使用を許可されたWRAM-C。
  
  Returns:      G.711コンポーネントが正しくDSPへロードされればTRUE。
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_LoadG711(FSFile *file, int slotB, int slotC)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_LoadG711Core(file, slotB, slotC);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadG711

  Description:  G.711符号化方式をサポートするコンポーネントをDSPからアンロード。
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_UnloadG711(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_UnloadG711Core();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EncodeG711

  Description:  G.711エンコード。
  
  Arguments:    dst  : 変換先バッファ。 (A-law/u-law 8bit)
                src  : 変換元バッファ。 (PCM 16bit)
                len  : 変換サンプル数。
                mode : コーデック種別。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_EncodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_EncodeG711Core(dst, src, len, mode);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DecodeG711

  Description:  G.711デコード。
  
  Arguments:    dst  : 変換先バッファ。 (PCM 16bit)
                src  : 変換元バッファ。 (A-law/u-law 8bit)
                len  : 変換サンプル数。
                mode : コーデック種別。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_DecodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_DecodeG711Core(dst, src, len, mode);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_TryWaitForG711

  Description:  最後に発行したコマンドが完了しているか判定。
  
  Arguments:    None.
  
  Returns:      最後に発行したコマンドが完了しているならTRUE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_TryWaitForG711(void)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_TryWaitForG711Core();
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_WaitForG711

  Description:  最後に発行したコマンドの完了を待つ。
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_WaitForG711(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_WaitForG711Core();
    }
}

#else

void DSP_EncodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);
void DSP_DecodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);

#endif // SDK_TWL


#ifdef __cplusplus
}
#endif


#endif // DSP_AUDIO_G711_H__
