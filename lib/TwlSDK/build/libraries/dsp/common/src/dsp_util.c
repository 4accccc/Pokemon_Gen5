/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_util.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-16#$
  $Rev: 10913 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/pipe.h>
#include <twl/dsp/common/audio.h>

#include "dsp_process.h"

/*---------------------------------------------------------------------------*/
/* constants */

// サウンド再生優先度。
#define DSP_SOUND_PRIORITY_SHUTTER          0
#define DSP_SOUND_PRIORITY_NORMAL           16
#define DSP_SOUND_PRIORITY_NONE             32

// WAVE ファイルの解析用
#define MAKE_FOURCC(cc1, cc2, cc3, cc4) (u32)((cc1) | (cc2 << 8) | (cc3 << 16) | (cc4 << 24))
#define MAKE_TAG_DATA(ca) (u32)((*(ca)) | (*(ca+1) << 8) | (*(ca+2) << 16) | (*(ca+3) << 24))
#define FOURCC_RIFF MAKE_FOURCC('R', 'I', 'F', 'F')
#define FOURCC_WAVE MAKE_FOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt  MAKE_FOURCC('f', 'm', 't', ' ')
#define FOURCC_data MAKE_FOURCC('d', 'a', 't', 'a')

// シャッター音の再生前に入れるウェイト(msec)
#define DSP_WAIT_SHUTTER                    60

/*---------------------------------------------------------------------------*/
/* variables */

// DSPサウンド再生中フラグ。
static BOOL DSPiSoundPlaying = FALSE;
static OSAlarm DSPiSoundAlarm;

// 現在のDSPサウンド再生優先度。
static int DSPiSoundPriority = DSP_SOUND_PRIORITY_NONE;

// DSPのサンプリング制御情報。
typedef struct DSPAudioCaptureInfo
{
    DSPAddr bufferAddress;
    DSPWord bufferLength;
    DSPWord currentPosition;
}
DSPAudioCaptureInfo;
static DSPAudioCaptureInfo  DSPiAudioCapture;
extern DSPAddr              DSPiAudioCaptureAddress;
static DSPAddr              DSPiReadPosition = 0;

// ARM9側のローカルバッファ。
static u32  DSPiLocalRingLength = 0;
static u8  *DSPiLocalRingBuffer = NULL;
static int  DSPiLocalRingOffset = 0;
static DSPAddr DSPiAudioCaptureAddress = 0;

static BOOL DSPiPlayingShutter = FALSE;

// シャッター音再生完了時にサウンド設定を戻す関数
static void DSPiShutterPostProcessCallback( SNDEXResult result, void* arg )
{
#pragma unused(arg)
    if(result == SNDEX_RESULT_EXCLUSIVE)
    {
        // シャッターのサウンド設定を戻す関数は必ず成功する
        OS_TPanic("SNDEXi_PostProcessForShutterSound SNDEX_RESULT_EXCLUSIVE ... DSP_PlayShutterSound\n");
    }
    if(result != SNDEX_RESULT_SUCCESS)
    {
        // シャッターのサウンド設定を戻す関数は必ず成功する
        OS_TPanic("SNDEXi_PostProcessForShutterSound Error ... DSP_PlayShutterSound\n");
    }
    DSPiPlayingShutter = FALSE;
}

/*---------------------------------------------------------------------------*/
/* functions */

// DSPからのサウンド再生後のリングバッファの空再生待ち後に発生する関数
static void sound_handler(void* arg)
{
#pragma unused(arg)
    DSPiSoundPlaying = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PipeCallbackForSound

  Description:  サウンド再生完了コールバック。

  Arguments:    userdata : 任意のユーザ定義引数
                port     : ポート番号
                peer     : 送受信方向

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PipeCallbackForSound(void *userdata, int port, int peer)
{
    (void)userdata;
    if (peer == DSP_PIPE_INPUT)
    {
        // DSPからのコマンドレスポンスを受信。
        DSPAudioDriverResponse  response;
        DSPPipe                 pipe[1];
        (void)DSP_LoadPipe(pipe, port, peer);
        if (DSP_GetPipeReadableSize(pipe) >= sizeof(response))
        {
            DSP_ReadPipe(pipe, &response, sizeof(response));
            response.ctrl = DSP_32BIT_TO_DSP(response.ctrl);
            response.result = DSP_32BIT_TO_DSP(response.result);
            // オーディオ出力コマンド。
            if ((response.ctrl & DSP_AUDIO_DRIVER_TARGET_MASK) == DSP_AUDIO_DRIVER_TARGET_OUTPUT)
            {
                // 停止コマンド。
                if ((response.ctrl & DSP_AUDIO_DRIVER_CONTROL_MASK) == DSP_AUDIO_DRIVER_CONTROL_STOP)
                {
                    //DSPiSoundPlaying = FALSE;
                    OS_CreateAlarm(&DSPiSoundAlarm);
                    OS_SetAlarm(&DSPiSoundAlarm, OS_MilliSecondsToTicks(30), sound_handler, NULL);
                    if(DSPiSoundPriority == DSP_SOUND_PRIORITY_SHUTTER)
                    {
                          // シャッターのサウンド設定を戻す関数は必ず成功する
                          (void)SNDEXi_PostProcessForShutterSound(DSPiShutterPostProcessCallback, 0);
                    }
                    DSPiSoundPriority = DSP_SOUND_PRIORITY_NONE;
                }
            }
            // オーディオ入力コマンド。
            if ((response.ctrl & DSP_AUDIO_DRIVER_TARGET_MASK) == DSP_AUDIO_DRIVER_TARGET_INPUT)
            {
                // 開始コマンド。
                if ((response.ctrl & DSP_AUDIO_DRIVER_CONTROL_MASK) == DSP_AUDIO_DRIVER_CONTROL_START)
                {
                    DSPiAudioCaptureAddress = (DSPAddr)response.result;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PlaySoundEx

  Description:  DSPサウンド出力から直接サウンドを再生。
  
  Arguments:    src      : 元データとなるサンプリングデータ。
                           PCM16bit32768kHzで4バイト境界整合している必要がある。
                len      : サンプリングデータのバイトサイズ。
                ctrl     : DSP_AUDIO_DRIVER_MODE_* の組み合わせ。
                priority : 再生優先度。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PlaySoundEx(const void *src, u32 len, u32 ctrl, int priority)
{
    // コンポーネントが何も起動していなければ無視。
    DSPProcessContext  *context = DSP_FindProcess(NULL);
    if (context)
    {
        // 現在再生中のサウンドより低い優先度なら再生しない。
        if (DSPiSoundPriority < priority)
        {
            OS_TWarning("still now playing high-priority sound.\n");
        }
        else
        {
            DSPiSoundPriority = priority;
            ctrl |= DSP_AUDIO_DRIVER_TARGET_OUTPUT;
            ctrl |= DSP_AUDIO_DRIVER_CONTROL_START;
            // バイトサイズからハーフワードサイズに変更。
            len >>= 1;
            // 完了通知用にコールバックを設定。
            DSP_SetPipeCallback(DSP_PIPE_AUDIO, DSPi_PipeCallbackForSound, NULL);
            DSPiSoundPlaying = TRUE;
            {
                DSPAudioDriverCommand   command;
                command.ctrl = DSP_32BIT_TO_DSP(ctrl);
                command.buf = DSP_32BIT_TO_DSP(src);
                command.len = DSP_32BIT_TO_DSP(len);
                command.opt = DSP_32BIT_TO_DSP(0);
                DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PlaySound

  Description:  DSPサウンド出力から直接サウンドを再生。
  
  Arguments:    src    : 元データとなるサンプリングデータ。
                         PCM16bit32768kHzで4バイト境界整合している必要がある。
                len    : サンプリングデータのバイトサイズ。
                stereo : ステレオならTRUE、モノラルならFALSE。

  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_PlaySoundCore(const void *src, u32 len, BOOL stereo)
{
    u32     ctrl = (stereo != FALSE) ? DSP_AUDIO_DRIVER_MODE_STEREO : DSP_AUDIO_DRIVER_MODE_MONAURAL;
    DSPi_PlaySoundEx(src, len, ctrl, DSP_SOUND_PRIORITY_NORMAL);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PlayShutterSound

  Description:  DSPサウンド出力から直接サウンドを再生。
  
  Arguments:    src    : 元データとなるサンプリングデータ。
                         PCM16bit32768kHzで4バイト境界整合している必要がある。
                len    : サンプリングデータのバイトサイズ。

  
  Returns:      SDNEX関数が成功したならばTRUEを返す.
 *---------------------------------------------------------------------------*/
#if 0 // ここまできっちりした WAVE のチェックは行わない
BOOL DSPi_PlayShutterSoundCore(const void *src, u32 len)
{
    u8* wave_data = (u8*)src;
    u32 cur = 0;
    u32 tag;
    u32 wave_len;
    u32 raw_len;
    BOOL    fFmt = FALSE, fData = FALSE;
    
    static SNDEXFrequency freq;
    u32 sampling;
    u32 chunkSize;

    // 与えられたデータが WAVE ファイルか確認する
    if(len < cur+12)
        return FALSE;
    tag = MAKE_TAG_DATA(wave_data+cur);
    if(tag != FOURCC_RIFF)
        return FALSE;
    cur+=4;

    wave_len = MAKE_TAG_DATA(wave_data+cur);
    cur+=4;

    tag = MAKE_TAG_DATA(wave_data+cur);
    if(tag != FOURCC_WAVE)
        return FALSE;
    cur+=4;

    while (wave_len > 0)
    {
        if(len < cur+8)
            return FALSE;
        tag = MAKE_TAG_DATA(wave_data+cur);
        cur+=4;
        chunkSize = MAKE_TAG_DATA(wave_data+cur);
        cur+=4;
        
        if(len < cur+chunkSize)
            return FALSE;
        
        switch (tag)
        {
        case FOURCC_fmt:
            // フォーマット情報からサンプリングレートをチェックする
            // シャッター音再生中でなければ、周波数を取得
            if(!DSPi_IsShutterSoundPlayingCore)
            {
                if(SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
                    return FALSE;
            }
            sampling = MAKE_TAG_DATA(wave_data+cur+4);
            cur+=chunkSize;
            if( ((freq == SNDEX_FREQUENCY_32730)&&(sampling != 32730))||((freq == SNDEX_FREQUENCY_47610)&&(sampling != 47610)) )
                return FALSE;
            fFmt = TRUE;
            break;
        case FOURCC_data:
            raw_len = chunkSize;
            fData = TRUE;
            break;
        default:
            cur+=chunkSize;
            break;
        }
        if(fFmt && fData)
            break;
        wave_len -= chunkSize;
    }
    if(!(fFmt && fData))
        return FALSE;

    // ヘッドフォン接続時、本体スピーカーは切られている。
    // 出力を安定させるためにシャッター音再生前に 約60msec のウェイトを入れる
    // このウェイトは、操作感を統一するためにいかなる場合も入れることとする
    OS_SpinWait(67 * DSP_WAIT_SHUTTER * 1000);	// 約 60msec
    
    // 関数内部でサウンド出力モードを変更
    while(SNDEXi_PreProcessForShutterSound() != SNDEX_RESULT_SUCCESS)
    {
        OS_Sleep(1); // 成功するまでリトライ
    }

    {
        u32     ctrl = DSP_AUDIO_DRIVER_MODE_MONAURAL;
        // シャッター音は左右に50%ずつ減音。
        ctrl |= DSP_AUDIO_DRIVER_MODE_HALFVOL;
        DSPi_PlaySoundEx((wave_data+cur), raw_len, ctrl, DSP_SOUND_PRIORITY_SHUTTER);
        DSPiPlayingShutter = TRUE;
    }

    return TRUE;
}
#else // チェックの目的はあくまで周波数違いのシャッター音を鳴らさないという点のみ
BOOL DSPi_PlayShutterSoundCore(const void *src, u32 len)
{
#pragma unused(len)
    u32 cur;
    u32 sampling;
    u32 raw_len;
    u8* wave_data = (u8*)src;
    static SNDEXFrequency freq;

    if(len < 44)
        return FALSE;
    
    if(MAKE_TAG_DATA(wave_data) != FOURCC_RIFF)
        return FALSE;

    if(MAKE_TAG_DATA(wave_data+8) != FOURCC_WAVE)
        return FALSE;

    cur = 24;
    sampling = MAKE_TAG_DATA(wave_data+cur);
    
    // フォーマット情報からサンプリングレートをチェックする
    // シャッター音再生中でなければ、周波数を取得
    if(!DSPi_IsShutterSoundPlayingCore)
    {
        if(SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
            return FALSE;
    }
    if( ((freq == SNDEX_FREQUENCY_32730)&&(sampling != 32730))||((freq == SNDEX_FREQUENCY_47610)&&(sampling != 47610)) )
        return FALSE;
    
    cur += 16;
    raw_len = MAKE_TAG_DATA(wave_data+cur);
    cur += 4;

    if(len < cur+raw_len)
        return FALSE;
       
    // ヘッドフォン接続時、本体スピーカーは切られている。
    // 出力を安定させるためにシャッター音再生前に 約60msec のウェイトを入れる
    // このウェイトは、操作感を統一するためにいかなる場合も入れることとする
    OS_SpinWait(67 * DSP_WAIT_SHUTTER * 1000);	// 約 60msec

    // 関数内部でサウンド出力モードを変更
    while(SNDEXi_PreProcessForShutterSound() != SNDEX_RESULT_SUCCESS)
    {
        OS_Sleep(1); // 成功するまでリトライ
    }

    {
        u32     ctrl = DSP_AUDIO_DRIVER_MODE_MONAURAL;
        // シャッター音は左右に50%ずつ減音。
        ctrl |= DSP_AUDIO_DRIVER_MODE_HALFVOL;
        DSPi_PlaySoundEx((wave_data+cur), raw_len, ctrl, DSP_SOUND_PRIORITY_SHUTTER);
        DSPiPlayingShutter = TRUE;
    }

    return TRUE;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSound

  Description:  DSPサウンド出力からの再生を停止。
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_StopSoundCore(void)
{
    // コンポーネントが何も起動していなければ無視。
    DSPProcessContext  *context = DSP_FindProcess(NULL);
    if (context && DSPiSoundPlaying)
    {
        int     ctrl = 0;
        ctrl |= DSP_AUDIO_DRIVER_TARGET_OUTPUT;
        ctrl |= DSP_AUDIO_DRIVER_CONTROL_STOP;
        {
            DSPAudioDriverCommand   command;
            command.ctrl = DSP_32BIT_TO_DSP(ctrl);
            command.buf = DSP_32BIT_TO_DSP(0);
            command.len = DSP_32BIT_TO_DSP(0);
            command.opt = DSP_32BIT_TO_DSP(0);
            DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsSoundPlaying

  Description:  DSPサウンド出力が現在再生中か判定。
  
  Arguments:    None.
  
  Returns:      DSPサウンド出力が現在再生中ならTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSPi_IsSoundPlayingCore(void)
{
    return DSPiSoundPlaying;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsShutterSoundPlaying

  Description:  DSPサウンド出力が現在シャッター音を再生中か判定。
  
  Arguments:    None.
  
  Returns:      DSPサウンド出力が現在シャッター音を再生中ならTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSPi_IsShutterSoundPlayingCore(void)
{
    return (DSPiSoundPlaying | DSPiPlayingShutter);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StartSampling

  Description:  DSPサウンド入力から直接マイク音声をサンプリング。
  
  Arguments:    buffer : サンプリングに使用するリングバッファ。
                         16bitの整数倍に境界整合している必要がある。
                length : リングバッファのサイズ。
                         16bitの整数倍に境界整合している必要がある。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_StartSamplingCore(void *buffer, u32 length)
{
    SDK_ALIGN2_ASSERT(buffer);
    SDK_ALIGN2_ASSERT(length);
    {
        // コンポーネントが何も起動していなければ無視。
        DSPProcessContext  *context = DSP_FindProcess(NULL);
        if (context)
        {
            int     ctrl = 0;
            ctrl |= DSP_AUDIO_DRIVER_TARGET_INPUT;
            ctrl |= DSP_AUDIO_DRIVER_CONTROL_START;
            DSPiLocalRingLength = length;
            DSPiLocalRingBuffer = (u8 *)buffer;
            DSPiLocalRingOffset = 0;
            DSPiAudioCaptureAddress = 0;
            // 完了通知用にコールバックを設定。
            DSP_SetPipeCallback(DSP_PIPE_AUDIO, DSPi_PipeCallbackForSound, NULL);
            {
                DSPAudioDriverCommand   command;
                command.ctrl = DSP_32BIT_TO_DSP(ctrl);
                command.buf = DSP_32BIT_TO_DSP(0);
                command.len = DSP_32BIT_TO_DSP(0);
                command.opt = DSP_32BIT_TO_DSP(0);
                DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSampling

  Description:  DSPサウンド入力からのサンプリングを停止。
  
  Arguments:    None.

  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_StopSamplingCore(void)
{
    // コンポーネントが何も起動していなければ無視。
    DSPProcessContext  *context = DSP_FindProcess(NULL);
    if (context)
    {
        int     ctrl = 0;
        ctrl |= DSP_AUDIO_DRIVER_TARGET_INPUT;
        ctrl |= DSP_AUDIO_DRIVER_CONTROL_STOP;
        // 完了通知用にコールバックを設定。
        DSP_SetPipeCallback(DSP_PIPE_AUDIO, DSPi_PipeCallbackForSound, NULL);
        {
            DSPAudioDriverCommand   command;
            command.ctrl = DSP_32BIT_TO_DSP(ctrl);
            command.buf = DSP_32BIT_TO_DSP(0);
            command.len = DSP_32BIT_TO_DSP(0);
            command.opt = DSP_32BIT_TO_DSP(0);
            DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncSamplingBuffer

  Description:  DSP内部のリングバッファから更新部分だけをARM9側へ読み込み。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_SyncSamplingBufferCore(void)
{
    // DSP側のモニタリングアドレスを未受信なら何もしない。
    if (DSPiAudioCaptureAddress != 0)
    {
        // DSP内部のキャプチャ情報を取得し、未読があれば読み出す。
        DSP_LoadData(DSP_ADDR_TO_ARM(DSPiAudioCaptureAddress), &DSPiAudioCapture, sizeof(DSPiAudioCapture));
        if (DSPiAudioCapture.currentPosition != DSPiReadPosition)
        {
            // DSP側とARM9側でリングバッファのサイズが異なるため
            // 終端に注意しつつコピーしていく。
            int     cur = DSPiAudioCapture.currentPosition;
            int     end = (DSPiReadPosition > cur) ? DSPiAudioCapture.bufferLength : cur;
            int     len = end - DSPiReadPosition;
            while (len > 0)
            {
                int     segment = (int)MATH_MIN(len, DSP_WORD_TO_DSP32(DSPiLocalRingLength - DSPiLocalRingOffset));
                DSP_LoadData(DSP_ADDR_TO_ARM(DSPiAudioCapture.bufferAddress + DSPiReadPosition),
                             &DSPiLocalRingBuffer[DSPiLocalRingOffset], DSP_WORD_TO_ARM(segment));
                len -= segment;
                DSPiReadPosition += segment;
                if (DSPiReadPosition >= DSPiAudioCapture.bufferLength)
                {
                    DSPiReadPosition -= DSPiAudioCapture.bufferLength;
                }
                DSPiLocalRingOffset += (int)DSP_WORD_TO_ARM32(segment);
                if (DSPiLocalRingOffset >= DSPiLocalRingLength)
                {
                    DSPiLocalRingOffset -= DSPiLocalRingLength;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetLastSamplingAddress

  Description:  ARM9側にロードされているローカルリングバッファの
                最新のサンプル位置を取得。

  Arguments:    None.

  Returns:      最新のサンプル位置。
 *---------------------------------------------------------------------------*/
const u8* DSPi_GetLastSamplingAddressCore(void)
{
    int     offset = DSPiLocalRingOffset - (int)sizeof(u16);
    if (offset < 0)
    {
        offset += DSPiLocalRingLength;
    }
    return &DSPiLocalRingBuffer[offset];
}

