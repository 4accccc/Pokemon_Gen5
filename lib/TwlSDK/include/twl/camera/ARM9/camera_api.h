/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     camera_api.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-20#$
  $Rev: 9373 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_CAMERA_API_H_
#define TWL_CAMERA_CAMERA_API_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// 処理結果定義
typedef enum CAMERAResult
{
    CAMERA_RESULT_SUCCESS = 0,
    CAMERA_RESULT_SUCCESS_TRUE = 0,
    CAMERA_RESULT_SUCCESS_FALSE,
    CAMERA_RESULT_BUSY,
    CAMERA_RESULT_ILLEGAL_PARAMETER,
    CAMERA_RESULT_SEND_ERROR,
    CAMERA_RESULT_INVALID_COMMAND,
    CAMERA_RESULT_ILLEGAL_STATUS,
    CAMERA_RESULT_FATAL_ERROR,
    CAMERA_RESULT_MAX
}
CAMERAResult;

// コールバック
typedef void (*CAMERACallback)(CAMERAResult result, void *arg);
typedef void (*CAMERAIntrCallback)(CAMERAResult result);

// core APIs
CAMERAResult CAMERA_InitCore(void);
void CAMERA_EndCore(void);
CAMERAResult CAMERA_StartCore(CAMERASelect camera);
CAMERAResult CAMERA_StopCore(void);
CAMERAResult CAMERA_I2CInitAsyncCore(CAMERASelect camera, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CInitCore(CAMERASelect camera);
CAMERAResult CAMERA_I2CActivateAsyncCore(CAMERASelect camera, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CActivateCore(CAMERASelect camera);
CAMERAResult CAMERA_I2CContextSwitchAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CContextSwitchCore(CAMERASelect camera, CAMERAContext context);
CAMERAResult CAMERA_I2CSizeExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERASize size, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CSizeExCore(CAMERASelect camera, CAMERAContext context, CAMERASize size);
CAMERAResult CAMERA_I2CFrameRateAsyncCore(CAMERASelect camera, CAMERAFrameRate rate, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CFrameRateCore(CAMERASelect camera, CAMERAFrameRate rate);
CAMERAResult CAMERA_I2CEffectExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CEffectExCore(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect);
CAMERAResult CAMERA_I2CFlipExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CFlipExCore(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip);
CAMERAResult CAMERA_I2CPhotoModeAsyncCore(CAMERASelect camera, CAMERAPhotoMode mode, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CPhotoModeCore(CAMERASelect camera, CAMERAPhotoMode mode);
CAMERAResult CAMERA_I2CWhiteBalanceAsyncCore(CAMERASelect camera, CAMERAWhiteBalance wb, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CWhiteBalanceCore(CAMERASelect camera, CAMERAWhiteBalance wb);
CAMERAResult CAMERA_I2CExposureAsyncCore(CAMERASelect camera, int exposure, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CExposureCore(CAMERASelect camera, int exposure);
CAMERAResult CAMERA_I2CSharpnessAsyncCore(CAMERASelect camera, int sharpness, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CSharpnessCore(CAMERASelect camera, int sharpness);
CAMERAResult CAMERAi_I2CTestPatternAsyncCore(CAMERASelect camera, CAMERATestPattern pattern, CAMERACallback callback, void *arg);
CAMERAResult CAMERAi_I2CTestPatternCore(CAMERASelect camera, CAMERATestPattern pattern);
CAMERAResult CAMERA_I2CAutoExposureAsyncCore(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CAutoExposureCore(CAMERASelect camera, BOOL on);
CAMERAResult CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CAutoWhiteBalanceCore(CAMERASelect camera, BOOL on);
CAMERAResult CAMERA_SetLEDAsyncCore(BOOL isBlink, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_SetLEDCore(BOOL isBlink);
CAMERAResult CAMERA_SwitchOffLEDAsyncCore(CAMERACallback callback, void *arg);
CAMERAResult CAMERA_SwitchOffLEDCore(void);

void CAMERA_SetVsyncCallbackCore(CAMERAIntrCallback callback);
void CAMERA_SetBufferErrorCallbackCore(CAMERAIntrCallback callback);
void CAMERA_SetRebootCallbackCore(CAMERAIntrCallback callback);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERAライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_Init(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERAResult result;
        result = CAMERA_InitCore();
        if (result == CAMERA_RESULT_SUCCESS)
        {
            return CAMERA_I2CInitCore(CAMERA_SELECT_BOTH);
        }
        else
        {
            return result;
        }
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_End

  Description:  CAMERAライブラリを終了する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_End(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_EndCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Start

  Description:  キャプチャを開始する上位API。切り替えにも使える。
                sync version only

  Arguments:    camera      - one of CAMERASelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_Start(CAMERASelect camera)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_StartCore(camera);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Stop

  Description:  キャプチャを停止する上位API。
                sync version only

  Arguments:    None

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_Stop(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_StopCore();
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  initialize camera registers via I2C
                async version.

  Arguments:    camera      - one of CAMERASelect
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CInitAsync(CAMERASelect camera, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CInitAsyncCore(camera, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize camera registers via I2C
                sync version.

  Arguments:    camera      - one of CAMERASelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CInit(CAMERASelect camera)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CInitCore(camera);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivateAsync

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                async version

  Arguments:    camera      - one of CAMERASelect (BOTH is not valid)
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CActivateAsync(CAMERASelect camera, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CActivateAsyncCore(camera, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivate

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                sync version.

  Arguments:    camera      - one of CAMERASelect (BOTH is not valid)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CActivate(CAMERASelect camera)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CActivateCore(camera);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CContextSwitchAsync

  Description:  resize CAMERA output image
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CContextSwitchAsync(CAMERASelect camera, CAMERAContext context, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CContextSwitchAsyncCore(camera, context, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CContextSwitch

  Description:  resize CAMERA output image
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CContextSwitch(CAMERASelect camera, CAMERAContext context)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CContextSwitchCore(camera, context);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSizeExAsync

  Description:  set CAMERA frame size
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                size        - one of CAMERASize
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSizeExAsync(CAMERASelect camera, CAMERAContext context, CAMERASize size, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExAsyncCore(camera, context, size, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSizeAsync

  Description:  set CAMERA frame size for all contexts
                async version

  Arguments:    camera      - one of CAMERASelect
                size        - one of CAMERASize
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSizeAsync(CAMERASelect camera, CAMERASize size, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExAsyncCore(camera, CAMERA_CONTEXT_BOTH, size, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSizeEx

  Description:  set CAMERA frame size
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                size        - one of CAMERASize

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSizeEx(CAMERASelect camera, CAMERAContext context, CAMERASize size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExCore(camera, context, size);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSize

  Description:  set CAMERA frame size for all contexts
                sync version.

  Arguments:    camera      - one of CAMERASelect
                size        - one of CAMERASize

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSize(CAMERASelect camera, CAMERASize size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExCore(camera, CAMERA_CONTEXT_BOTH, size);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRateAsync

  Description:  set CAMERA frame rate
                async version

  Arguments:    camera      - one of CAMERASelect
                rate        - one of CAMERAFrameRate
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFrameRateAsync(CAMERASelect camera, CAMERAFrameRate rate, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFrameRateAsyncCore(camera, rate, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate
                sync version.

  Arguments:    camera      - one of CAMERASelect
                rate        - one of CAMERAFrameRate

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFrameRate(CAMERASelect camera, CAMERAFrameRate rate)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFrameRateCore(camera, rate);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectExAsync

  Description:  set CAMERA effect
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                effect      - one of CAMERAEffect
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffectExAsync(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExAsyncCore(camera, context, effect, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectAsync

  Description:  set CAMERA effect for all contexts
                async version

  Arguments:    camera      - one of CAMERASelect
                effect      - one of CAMERAEffect
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffectAsync(CAMERASelect camera, CAMERAEffect effect, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExAsyncCore(camera, CAMERA_CONTEXT_BOTH, effect, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA effect
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                effect      - one of CAMERAEffect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffectEx(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExCore(camera, context, effect);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA effect for all contexts
                sync version.

  Arguments:    camera      - one of CAMERASelect
                effect      - one of CAMERAEffect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffect(CAMERASelect camera, CAMERAEffect effect)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExCore(camera, CAMERA_CONTEXT_BOTH, effect);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipExAsync

  Description:  set CAMERA flip/mirror
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                flip        - one of CAMERAFlip
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlipExAsync(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExAsyncCore(camera, context, flip, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipAsync

  Description:  set CAMERA flip/mirror for all contexts
                async version

  Arguments:    camera      - one of CAMERASelect
                flip        - one of CAMERAFlip
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlipAsync(CAMERASelect camera, CAMERAFlip flip, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExAsyncCore(camera, CAMERA_CONTEXT_BOTH, flip, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipEx

  Description:  set CAMERA flip/mirror
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                flip        - one of CAMERAFlip

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlipEx(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExCore(camera, context, flip);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  set CAMERA flip/mirror for all contexts
                sync version.

  Arguments:    camera      - one of CAMERASelect
                flip        - one of CAMERAFlip

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlip(CAMERASelect camera, CAMERAFlip flip)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExCore(camera, CAMERA_CONTEXT_BOTH, flip);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPhotoModeAsync

  Description:  set CAMERA photo mode
                async version

  Arguments:    camera      - one of CAMERASelect
                mode        - one of CAMERAPhotoMode
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CPhotoModeAsync(CAMERASelect camera, CAMERAPhotoMode mode, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CPhotoModeAsyncCore(camera, mode, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPhotoMode

  Description:  set CAMERA photo mode
                sync version.

  Arguments:    camera      - one of CAMERASelect
                mode        - one of CAMERAPhotoMode

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CPhotoMode(CAMERASelect camera, CAMERAPhotoMode mode)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CPhotoModeCore(camera, mode);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CWhiteBalanceAsync

  Description:  set CAMERA white balance
                async version

  Arguments:    camera      - one of CAMERASelect
                wb          - one of CAMERAWhiteBalance
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CWhiteBalanceAsync(CAMERASelect camera, CAMERAWhiteBalance wb, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CWhiteBalanceAsyncCore(camera, wb, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CWhiteBalance

  Description:  set CAMERA white balance
                sync version.

  Arguments:    camera      - one of CAMERASelect
                wb          - one of CAMERAWhiteBalance

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CWhiteBalance(CAMERASelect camera, CAMERAWhiteBalance wb)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CWhiteBalanceCore(camera, wb);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CExposureAsync

  Description:  set CAMERA exposure
                async version

  Arguments:    camera      - one of CAMERASelect
                exposure    - -5 to +5
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CExposureAsync(CAMERASelect camera, int exposure, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CExposureAsyncCore(camera, exposure, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CExposure

  Description:  set CAMERA exposure
                sync version.

  Arguments:    camera      - one of CAMERASelect
                exposure    - -5 to +5
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CExposure(CAMERASelect camera, int exposure)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CExposureCore(camera, exposure);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSharpnessAsync

  Description:  set CAMERA sharpness
                async version

  Arguments:    camera      - one of CAMERASelect
                sharpness   - -3 to +5
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSharpnessAsync(CAMERASelect camera, int sharpness, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSharpnessAsyncCore(camera, sharpness, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSharpness

  Description:  set CAMERA sharpness
                sync version.

  Arguments:    camera      - one of CAMERASelect
                sharpness   - -3 to +5
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSharpness(CAMERASelect camera, int sharpness)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSharpnessCore(camera, sharpness);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CTestPatternAsync

  Description:  set CAMERA test pattern
                async version

  Arguments:    camera      - one of CAMERASelect
                pattern     - one of CAMERATestPattern
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERAi_I2CTestPatternAsync(CAMERASelect camera, CAMERATestPattern pattern, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERAi_I2CTestPatternAsyncCore(camera, pattern, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CTestPattern

  Description:  set CAMERA test pattern
                sync version.

  Arguments:    camera      - one of CAMERASelect
                pattern     - one of CAMERATestPattern

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERAi_I2CTestPattern(CAMERASelect camera, CAMERATestPattern pattern)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERAi_I2CTestPatternCore(camera, pattern);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoExposureAsync

  Description:  enable/disable CAMERA auto-exposure
                async version

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AE will enable
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoExposureAsync(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoExposureAsyncCore(camera, on, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoExposure

  Description:  enable/disable CAMERA auto-exposure
                sync version.

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AE will enable
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoExposure(CAMERASelect camera, BOOL on)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoExposureCore(camera, on);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoWhiteBalanceAsync

  Description:  enable/disable CAMERA auto-white-balance
                async version

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AWB will enable
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoWhiteBalanceAsync(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoWhiteBalanceAsyncCore(camera, on, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoWhiteBalance

  Description:  enable/disable CAMERA auto-white-balance
                sync version.

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AWB will enable
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoWhiteBalance(CAMERASelect camera, BOOL on)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoWhiteBalanceCore(camera, on);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetLEDAsync

  Description:  set CAMERA LED (outside) to blink or not
                it works only OUT camera is active.
                default state when calling I2CActivate(OUT/BOTH) does not blink.
                async version

  Arguments:    isBlink     - 点滅させたいならTRUE、点灯ならFALSE
                callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SetLEDAsync(BOOL isBlink, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SetLEDAsyncCore(isBlink, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetLED

  Description:  set CAMERA LED (outside) to blink or not
                it works only OUT camera is active.
                default state when calling I2CActivate(OUT/BOTH) does not blink.
                sync version.

  Arguments:    isBlink     - 点滅させたいならTRUE、点灯ならFALSE

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SetLED(BOOL isBlink)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SetLEDCore(isBlink);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SwitchOffLEDAsync

  Description:  一度だけカメラのLEDを消灯させます。
                CAMERA_SetLED(TRUE)、CAMERA_SetLED(FALSE) を連続で呼び出した場合と
                同じ動作となります。

  Arguments:    callback    - 非同期処理が完了した際に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SwitchOffLEDAsync(CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SwitchOffLEDAsyncCore(callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SwitchOffLED

  Description:  一度だけカメラのLEDを消灯させます。
                CAMERA_SetLED(TRUE)、CAMERA_SetLED(FALSE) を連続で呼び出した場合と
                同じ動作となります。

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SwitchOffLED()
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SwitchOffLEDCore();
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetVsynCallback

  Description:  カメラのVSYNC割り込み発生時に呼び出されるコールバック関数を登録する

  Arguments:    callback    - 登録するコールバック関数

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_SetVsyncCallback(CAMERAIntrCallback callback)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_SetVsyncCallbackCore(callback);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetBufferErrorCallback

  Description:  カメラのバッファエラー割り込み発生時に呼び出されるコールバック関数を登録する

  Arguments:    callback    - 登録するコールバック関数

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_SetBufferErrorCallback(CAMERAIntrCallback callback)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_SetBufferErrorCallbackCore(callback);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetRebootCallback

  Description:  カメラの誤作動からの復帰処理が完了した際に呼び出されるコールバック関数を登録する

  Arguments:    callback    - 登録するコールバック関数

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_SetRebootCallback(CAMERAIntrCallback callback)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_SetRebootCallbackCore(callback);
    }
}

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_CAMERA_API_H_ */
