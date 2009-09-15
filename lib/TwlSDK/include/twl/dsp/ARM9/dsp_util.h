/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp
  File:     dsp_util.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-26#$
  $Rev: 9412 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_UTIL_H_
#define TWL_DSP_UTIL_H_


#include <twl/types.h>


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* constants */

#define DSP_SLOT_B_COMPONENT_G711           1
#define DSP_SLOT_C_COMPONENT_G711           2

#define DSP_SLOT_B_COMPONENT_JPEGENCODER    3
#define DSP_SLOT_C_COMPONENT_JPEGENCODER    4

#define DSP_SLOT_B_COMPONENT_JPEGDECODER    2
#define DSP_SLOT_C_COMPONENT_JPEGDECODER    4

#define DSP_SLOT_B_COMPONENT_GRAPHICS       1
#define DSP_SLOT_C_COMPONENT_GRAPHICS       4

#define DSP_SLOT_B_COMPONENT_AACDECODER     2
#define DSP_SLOT_C_COMPONENT_AACDECODER     4

/*---------------------------------------------------------------------------*/
/* functions */

void DSPi_PlaySoundCore(const void *src, u32 len, BOOL stereo);
BOOL DSPi_PlayShutterSoundCore(const void *src, u32 len);
void DSPi_StopSoundCore(void);
BOOL DSPi_IsSoundPlayingCore(void);
BOOL DSPi_IsShutterSoundPlayingCore(void);
void DSPi_StartSamplingCore(void *buffer, u32 length);
void DSPi_StopSamplingCore(void);
void DSPi_SyncSamplingBufferCore(void);
const u8* DSPi_GetLastSamplingAddressCore(void);


/*---------------------------------------------------------------------------*
  Name:         DSP_PlaySound

  Description:  DSPサウンド出力から直接サウンドを再生。
  
  Arguments:    src    : 元データとなるサンプリングデータ。
                         PCM16bit32768kHzで4バイト境界整合している必要がある。
                len    : サンプリングデータのバイトサイズ。
                stereo : ステレオならTRUE、モノラルならFALSE。

  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_PlaySound(const void *src, u32 len, BOOL stereo)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_PlaySoundCore(src, len, stereo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PlayShutterSound

  Description:  DSPサウンド出力からシャッター音を再生。
  
  Arguments:    src    : SDK付属のWAVE形式のシャッター音データ。
                len    : シャッター音データのバイトサイズ。

  
  Returns:      SDNEX関数が成功したならばTRUEを返す.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_PlayShutterSound(const void *src, u32 len)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_PlayShutterSoundCore(src, len);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsSoundPlaying

  Description:  DSPサウンド出力が現在再生中か判定。
  
  Arguments:    None.
  
  Returns:      DSPサウンド出力が現在再生中ならTRUE。
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_IsSoundPlaying(void)
{
    BOOL    retval = FALSE;
    if (OS_IsRunOnTwl())
    {
        retval = DSPi_IsSoundPlayingCore();
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsShutterSoundPlaying

  Description:  DSPサウンド出力が現在シャッター音を再生中か判定。
  
  Arguments:    None.
  
  Returns:      DSPサウンド出力が現在シャッター音を再生中ならTRUE。
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_IsShutterSoundPlaying(void)
{
    BOOL    retval = FALSE;
    if (OS_IsRunOnTwl())
    {
        retval = DSPi_IsShutterSoundPlayingCore();
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSound

  Description:  DSPサウンド出力から直接サウンドを再生。
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StopSound(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_StopSoundCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StartSampling

  Description:  DSPによるマイクサンプリングを開始。

  Arguments:    buffer : サンプリングに使用するリングバッファ。
                         16bitの整数倍に境界整合している必要がある。
                length : リングバッファのサイズ。
                         16bitの整数倍に境界整合している必要がある。

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StartSampling(void *buffer, u32 length)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_StartSamplingCore(buffer, length);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSampling

  Description:  DSPによるマイクサンプリングを停止。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StopSampling(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_StopSamplingCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncSamplingBuffer

  Description:  DSP内部のリングバッファから更新部分だけをARM9側へ読み込み。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_SyncSamplingBuffer(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_SyncSamplingBufferCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetLastSamplingAddress

  Description:  ARM9側にロードされているローカルリングバッファの
                最新のサンプル位置を取得。

  Arguments:    None.

  Returns:      最新のサンプル位置。
 *---------------------------------------------------------------------------*/
SDK_INLINE const u8* DSP_GetLastSamplingAddress(void)
{
    const u8   *retval = NULL;
    if (OS_IsRunOnTwl())
    {
        retval = DSPi_GetLastSamplingAddressCore();
    }
    return retval;
}


/*---------------------------------------------------------------------------*
 * 以下は内部関数。
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPostStartProcess

  Description:  DSPプロセスイメージをロードした直後のフック。
                DSPコンポーネント開発者がデバッガを起動するために必要。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_HookPostStartProcess(void);


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TWL_DSP_UTIL_H_ */
