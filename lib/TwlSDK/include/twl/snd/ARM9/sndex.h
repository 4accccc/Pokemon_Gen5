/*---------------------------------------------------------------------------*
  Project:  TwlSDK - snd - include
  File:     sndex.h

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
#ifndef TWL_SND_ARM9_SNDEX_H_
#define TWL_SND_ARM9_SNDEX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include <twl/snd/common/sndex_common.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
/* 処理結果 */
typedef enum SNDEXResult
{
    SNDEX_RESULT_SUCCESS        =   0,
    SNDEX_RESULT_BEFORE_INIT    =   1,
    SNDEX_RESULT_INVALID_PARAM  =   2,
    SNDEX_RESULT_EXCLUSIVE      =   3,
    SNDEX_RESULT_ILLEGAL_STATE  =   4,
    SNDEX_RESULT_PXI_SEND_ERROR =   5,
    SNDEX_RESULT_DEVICE_ERROR   =   6,
    SNDEX_RESULT_FATAL_ERROR    =   7,
    SNDEX_RESULT_MAX

} SNDEXResult;

/* 消音設定 */
typedef enum SNDEXMute
{
    SNDEX_MUTE_OFF  =   0,
    SNDEX_MUTE_ON   =   1,
    SNDEX_MUTE_MAX

} SNDEXMute;

/* I2S 周波数設定 */
typedef enum SNDEXFrequency
{
    SNDEX_FREQUENCY_32730   =   0,
    SNDEX_FREQUENCY_47610   =   1,
    SNDEX_FREQUENCY_MAX

} SNDEXFrequency;

/* IIRフィルタパラメータ構造体 */
typedef struct _SNDEXIirFilterParam
{
    u16 n0;
    u16 n1;
    u16 n2;
    u16 d1;
    u16 d2;
} SNDEXIirFilterParam;

/* IIR フィルタのターゲット列挙型 */
typedef enum _SNDEXIirFilterTarget
{
    SNDEX_IIR_FILTER_ADC_1 = 0,
    SNDEX_IIR_FILTER_ADC_2,
    SNDEX_IIR_FILTER_ADC_3,
    SNDEX_IIR_FILTER_ADC_4,
    SNDEX_IIR_FILTER_ADC_5,
    SNDEX_IIR_FILTER_DAC_LEFT_1,
    SNDEX_IIR_FILTER_DAC_LEFT_2,
    SNDEX_IIR_FILTER_DAC_LEFT_3,
    SNDEX_IIR_FILTER_DAC_LEFT_4,
    SNDEX_IIR_FILTER_DAC_LEFT_5,
    SNDEX_IIR_FILTER_DAC_RIGHT_1,
    SNDEX_IIR_FILTER_DAC_RIGHT_2,
    SNDEX_IIR_FILTER_DAC_RIGHT_3,
    SNDEX_IIR_FILTER_DAC_RIGHT_4,
    SNDEX_IIR_FILTER_DAC_RIGHT_5,
    SNDEX_IIR_FILTER_DAC_BOTH_1,
    SNDEX_IIR_FILTER_DAC_BOTH_2,
    SNDEX_IIR_FILTER_DAC_BOTH_3,
    SNDEX_IIR_FILTER_DAC_BOTH_4,
    SNDEX_IIR_FILTER_DAC_BOTH_5,
    SNDEX_IIR_FILTER_TARGET_MAX
} SNDEXIirFilterTarget;

/* ヘッドフォン接続有無 */
typedef enum SNDEXHeadphone
{
    SNDEX_HEADPHONE_UNCONNECT   =   0,
    SNDEX_HEADPHONE_CONNECT     =   1,
    SNDEX_HEADPHONE_MAX
} SNDEXHeadphone;

/* DSP 合成比設定 */
#define SNDEX_DSP_MIX_RATE_MIN  0
#define SNDEX_DSP_MIX_RATE_MAX  8

/* 音量設定 */
#define SNDEX_VOLUME_MIN        0
#define SNDEX_VOLUME_MAX        7
#define SNDEX_VOLUME_MAX_EX     31

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
/* コールバック関数型定義 */
typedef void    (*SNDEXCallback)    (SNDEXResult result, void* arg);

#define SNDEXVolumeSwitchCallbackInfo SNDEXWork

/*---------------------------------------------------------------------------*
    非公開関数定義
 *---------------------------------------------------------------------------*/
void    SNDEXi_Init (void);

SNDEXResult SNDEXi_GetMuteAsync            (SNDEXMute* mute, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetMute                 (SNDEXMute* mute);
SNDEXResult SNDEXi_GetI2SFrequencyAsync    (SNDEXFrequency* freq, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetI2SFrequency         (SNDEXFrequency* freq);
SNDEXResult SNDEXi_GetDSPMixRateAsync      (u8* rate, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetDSPMixRate           (u8* rate);
SNDEXResult SNDEXi_GetVolumeAsync          (u8* volume, SNDEXCallback callback, void* arg, BOOL eightlv, BOOL keep);
SNDEXResult SNDEXi_GetVolume               (u8* volume, BOOL eightlv, BOOL keep);
SNDEXResult SNDEXi_GetVolumeExAsync        (u8* volume, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetVolumeEx             (u8* volume);
SNDEXResult SNDEXi_GetCurrentVolumeExAsync (u8* volume, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetCurrentVolumeEx      (u8* volume);

SNDEXResult SNDEXi_SetMuteAsync         (SNDEXMute mute, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetMute              (SNDEXMute mute);
SNDEXResult SNDEXi_SetI2SFrequencyAsync (SNDEXFrequency freq, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetI2SFrequency      (SNDEXFrequency freq);
SNDEXResult SNDEXi_SetDSPMixRateAsync   (u8 rate, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetDSPMixRate        (u8 rate);
SNDEXResult SNDEXi_SetVolumeAsync       (u8 volume, SNDEXCallback callback, void* arg, BOOL eightlv);
SNDEXResult SNDEXi_SetVolume            (u8 volume, BOOL eightlv);
SNDEXResult SNDEXi_SetVolumeExAsync     (u8 volume, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetVolumeEx          (u8 volume);

SNDEXResult SNDEXi_SetIirFilterAsync    (SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetIirFilter         (SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam);

SNDEXResult SNDEXi_IsConnectedHeadphoneAsync (SNDEXHeadphone *hp, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_IsConnectedHeadphone      (SNDEXHeadphone *hp);

void        SNDEXi_SetVolumeSwitchCallback   (SNDEXCallback callback, void* arg);

SNDEXResult SNDEXi_SetIgnoreHWVolume       (u8 volume, BOOL eightlv);
SNDEXResult SNDEXi_ResetIgnoreHWVolume     (void);

SNDEXResult SNDEXi_PreProcessForShutterSound  (void);
SNDEXResult SNDEXi_PostProcessForShutterSound (SNDEXCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         SNDEX_Init

  Description:  サウンド拡張機能ライブラリを初期化する。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void
SNDEX_Init (void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        SNDEXi_Init();
    }
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetMuteAsync

  Description:  音声出力の消音設定状態を非同期に取得する。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    mute        -   消音設定状態を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetMuteAsync (SNDEXMute* mute, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetMuteAsync(mute, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetMute

  Description:  音声出力の消音設定状態を取得する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    mute    -   消音設定状態を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetMute (SNDEXMute* mute)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetMute(mute) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetI2SFrequencyAsync

  Description:  I2S 周波数情報を非同期に取得する。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    freq        -   周波数情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetI2SFrequencyAsync (SNDEXFrequency* freq, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetI2SFrequencyAsync(freq, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetI2SFrequency

  Description:  I2S 周波数情報を取得する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    freq    -   周波数情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetI2SFrequency (SNDEXFrequency* freq)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetI2SFrequency(freq) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetDSPMixRateAsync

  Description:  CPU と DSP の音声出力合成比情報を取得する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    rate        -   合成比情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetDSPMixRateAsync (u8* rate, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetDSPMixRateAsync(rate, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetDSPMixRate

  Description:  CPU と DSP の音声出力合成比情報を取得する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    rate    -   合成比情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetDSPMixRate (u8* rate)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetDSPMixRate(rate) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetVolumeAsync

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    volume      -   音量情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetVolumeAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolumeAsync(volume, callback, arg, TRUE, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetCurrentVolumeAsync

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                結果取得用バッファには関数呼び出しとは非同期に結果が書き込まれ
                るため、非同期処理が完了するまで他の用途に使用すべきでない。

  Arguments:    volume      -   音量情報を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
                                ただし、この関数で取得できる音量は実際の設定値であり、
                                SNDEX_SetVolume[Async] によりいずれ更新される保留値ではない。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetCurrentVolumeAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolumeAsync(volume, callback, arg, TRUE, FALSE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetVolume

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetVolume (u8* volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolume(volume, TRUE, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetCurrentVolume

  Description:  音声出力音量情報を取得する。音量は最小: 0, 最大: 31 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量情報を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。
                                ただし、この関数で取得できる音量は実際の設定値であり、
                                SNDEX_SetVolume[Async] によりいずれ更新される保留値ではない。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetCurrentVolume (u8* volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolume(volume, TRUE, FALSE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetMuteAsync

  Description:  音声出力の消音設定状態を変更する。

  Arguments:    mute        -   消音設定状態を指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetMuteAsync (SNDEXMute mute, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetMuteAsync(mute, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetMute

  Description:  音声出力の消音設定状態を変更する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    mute    -   消音設定状態を指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetMute (SNDEXMute mute)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetMute(mute) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetI2SFrequencyAsync

  Description:  I2S 周波数を変更する。

  Arguments:    freq        -   周波数を指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetI2SFrequencyAsync (SNDEXFrequency freq, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetI2SFrequencyAsync(freq, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetI2SFrequency

  Description:  I2S 周波数を変更する。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    freq    -   周波数を指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetI2SFrequency (SNDEXFrequency freq)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetI2SFrequency(freq) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetDSPMixRateAsync

  Description:  CPU と DSP の音声出力合成比を変更する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。

  Arguments:    rate        -   合成比を 0 から 8 の数値で指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetDSPMixRateAsync (u8 rate, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetDSPMixRateAsync(rate, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetDSPMixRate

  Description:  CPU と DSP の音声出力合成比を変更する。
                合成比は最小: 0 (DSP 100%), 最大: 8 (CPU 100%)。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    rate    -   合成比を 0 から 8 の数値で指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetDSPMixRate (u8 rate)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetDSPMixRate(rate) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetVolumeAsync

  Description:  音声出力音量を変更する。音量は最小: 0, 最大: 31 。

  Arguments:    volume      -   音量を 0 から 31 の数値で指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetVolumeAsync (u8 volume, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetVolumeAsync(volume, callback, arg, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetVolume

  Description:  音声出力音量を変更する。音量は最小: 0, 最大: 31 。
                同期関数なので、割込みハンドラ内からの呼び出しは禁止。

  Arguments:    volume  -   音量を 0 から 31 の数値で指定する。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetVolume (u8 volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetVolume(volume, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIirFilterAsync

  Description:  IIRフィルター(Biquad)のパラメータ設定を非同期で行います。

  Arguments:    target   : IIRフィルタをかける対象 SNDEXIirFilterTarget列挙体
                pParam   : IIRフィルタの係数 SNDEXIirFilterParam構造体
                callback : IIRフィルタ設定後に呼ばれるコールバック関数
                arg      : コールバック関数へ渡す引数

  Returns:      正常終了時に SNDEX_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetIirFilterAsync(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetIirFilterAsync(target, pParam, callback, arg) : SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIirFilter

  Description:  IIRフィルター(Biquad)のパラメータ設定を行います。

  Arguments:    target   : IIRフィルタをかける対象 SNDEXIirFilterTarget列挙体
                pParam   : IIRフィルタの係数 SNDEXIirFilterParam構造体

  Returns:      正常終了時に SNDEX_RESULT_SUCCESS
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetIirFilter(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetIirFilter(target, pParam) : SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphoneAsync

  Description:  ヘッドフォンの接続の有無を取得する。

  Arguments:    hp          -   ヘッドフォンの接続状況を取得するバッファを指定する。
                callback    -   非同期処理完了時に結果を通知するコールバック
                                関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_IsConnectedHeadphoneAsync (SNDEXHeadphone *hp, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_IsConnectedHeadphoneAsync(hp, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphone

  Description:  ヘッドフォンの接続の有無を取得する。

  Arguments:    hp          -   ヘッドフォンの接続状況を取得するバッファを指定する。

  Returns:      SNDEXResult -   処理結果を返す。SNDEX_RESULT_SUCCESS を返す
                                場合は、非同期処理完了後に実際の処理結果が
                                コールバックに登録した関数に通知される。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_IsConnectedHeadphone (SNDEXHeadphone *hp)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_IsConnectedHeadphone(hp) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetVolumeSwitchCallback

  Description:  音量調整ボタン押下時に呼ばれるコールバック関数を設定する。

  Arguments:    callback    -   音量調整ボタン押下時に呼ばれるコールバック関数を指定する。
                arg         -   コールバック関数に渡すパラメータを指定する。

  Returns:      なし
 *---------------------------------------------------------------------------*/
static inline void
SNDEX_SetVolumeSwitchCallback (SNDEXCallback callback, void* arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        SNDEXi_SetVolumeSwitchCallback(callback, arg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIgnoreHWVolume

  Description:  時計のアラーム音など、そのときの本体音量の値にとらわれず
                指定した音量で音を鳴らす必要がある場合、ユーザの操作によらず
                音量変更を行わねばならない。
                本関数では、音量変更を行うだけでなく、変更前の音量を保存する。
                また、アプリの意図しないタイミングで本体リセット・シャットダウンが
                行われた際に元の音量に戻す終了コールバック関数を登録する。
                SNDEX_ResetIgnoreHWVolume を呼ぶことで、元の音量に戻すと共に
                登録した終了コールバック関数を削除する。

  Arguments:    volume      -   変更したい音量値。

  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetIgnoreHWVolume (u8 volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetIgnoreHWVolume(volume, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_ResetIgnoreHWVolume

  Description:  SNDEX_SetIgnoreHWVolume で変更した音量を、変更前の音量へ戻す。
                同時に登録していた終了コールバック関数を削除する。

  Arguments:    なし
  
  Returns:      SNDEXResult -   処理結果を返す。
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_ResetIgnoreHWVolume (void)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_ResetIgnoreHWVolume() :
            SNDEX_RESULT_ILLEGAL_STATE);
}
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   // extern "C"
#endif
#endif  // TWL_SND_ARM9_SNDEX_H_
