/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera_api.c

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
#include <twl.h>
#include <twl/camera.h>

#include "camera_intr.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 詰めてコピーする
#define CAMERA_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define CAMERA_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))


#define CAMERA_VSYNC_TIMEOUT    OS_SecondsToTicks( 1 )  // 1秒
#define CAMERA_RETRY_COUNT 2

typedef enum
{
    I2C_Init,
    I2C_PhotoMode_IN,
    I2C_PhotoMode_OUT,
    I2C_SizeA_IN,
    I2C_SizeB_IN,
    I2C_SizeA_OUT,
    I2C_SizeB_OUT,
    I2C_FrameRate_IN,
    I2C_FrameRate_OUT,
    I2C_EffectA_IN,
    I2C_EffectB_IN,
    I2C_EffectA_OUT,
    I2C_EffectB_OUT,
    I2C_FlipA_IN,
    I2C_FlipB_IN,
    I2C_FlipA_OUT,
    I2C_FlipB_OUT,
    I2C_WhiteBalance_IN,
    I2C_WhiteBalance_OUT,
    I2C_Exposure_IN,
    I2C_Exposure_OUT,
    I2C_Sharpness_IN,
    I2C_Sharpness_OUT,
    I2C_AutoExposure_IN,
    I2C_AutoExposure_OUT,
    I2C_AutoWhiteBalance_IN,
    I2C_AutoWhiteBalance_OUT,
    I2C_Context_IN,
    I2C_Context_OUT,
    I2C_Activate,
    Set_LED,
    I2C_MAX
}
CAMERAI2CList;

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct CAMERAState
{
    CAMERASize size_A;
    CAMERASize size_B;
    CAMERAEffect effect_A;
    CAMERAEffect effect_B;
    CAMERAFlip flip_A;
    CAMERAFlip flip_B;
    CAMERAPhotoMode photo;
    CAMERAWhiteBalance wb;
    CAMERAFrameRate rate;
    int exposure;
    int sharpness;
    BOOL ae;
    BOOL awb;
    CAMERAContext context;
    BOOL blink;
}
CAMERAState;

typedef struct CAMERAStateTmp
{
    CAMERASelect set_camera; // この一時設定情報がどちらのカメラのものなのか
    CAMERAContext set_context; // この一時設定情報がどちらコンテキストのものなのか
    union
    {
        CAMERASize size;
        CAMERAEffect effect;
        CAMERAFlip flip;
        CAMERAPhotoMode photo;
        CAMERAWhiteBalance wb;
        CAMERAFrameRate rate;
        int exposure;
        int sharpness;
        BOOL ae;
        BOOL awb;
        CAMERAContext context;
        BOOL blink;
    };
}
CAMERAStateTmp;

typedef struct CAMERAWork
{
    BOOL            lock;
    CAMERACallback  callback;
    CAMERAResult    result;             // 先頭データだけ別枠
    void            *callbackArg;
    CAMERAPxiCommand    command;        // コマンド種別
    CAMERAPxiResult     pxiResult;      // 先頭データだけ別枠
    u8      current;                    // 受信済みデータ個数 (バイト単位) (先頭を除く!!)
    u8      total;                      // 最終データ個数 (1 + 後続コマンド*3)
    u8 padding[2];
    u8      *data;                      // save API arg if any
    size_t  size;                       // save API arg if any

    BOOL    last_state;                 // last state before pxi
    BOOL    force_activate;             // keep clock request
    BOOL    force_deactivate;           // stop clock request

    PMSleepCallbackInfo sleepCbInfo;    // スリープ復帰時コールバック情報
    PMExitCallbackInfo  exitCbInfo;     // リセット/電源OFF時コールバック情報

    OSTick CAMERAiLastCameraVSync;      // カメラの誤作動を検知するための時間
    CAMERAIntrCallback CAMERAiVsyncCallback;       // カメラVSYNCで呼ばれるコールバック
    CAMERAIntrCallback CAMERAiBufferErrorCallback; // カメラバッファエラーで呼ばれるコールバック
    CAMERAIntrCallback CAMERAiRebootCallback;      // カメラ再起動完了後に呼ばれるコールバック
    OSThread  CAMERAiMonitorThread[1];                 // カメラの誤作動検知用スレッド構造体
    u32 CAMERAiMonitorThreadStack[1024];               // カメラの誤作動検知用スレッドのスタック

    CAMERASelect CAMERAiCurrentCamera;  // ユーザが直前に設定したカレントのカメラ

    CAMERAState CAMERAiStateIn;         // IN側のカメラの設定値
    CAMERAState CAMERAiStateOut;        // OUT側のカメラの設定値
    CAMERAStateTmp CAMERAiStateTmp;     // I2Cコマンドが成功するまで設定を保持

    u32 CAMERAiRetryCount;              // カメラ再起動のリトライ回数

    int             CAMERAiMonitorCounter;
    OSMessageQueue  CAMERAiMonitorMessageQueue[1];  // MonitorThread宛
    OSMessage       CAMERAiMonitorMessage[1];       // 1種類しかないしダブっても意味ないので1つだけ
}
CAMERAWork;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL cameraInitialized;
static CAMERAWork cameraWork;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL CameraSendPxiCommand(CAMERAPxiCommand command, u8 size, u8 data);
static void CameraSendPxiData(u8 *pData);
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CameraSyncCallback(CAMERAResult result, void *arg);
static void CameraCallCallbackAndUnlock(CAMERAResult result);
static void CameraWaitBusy(void);
static void CameraStandbyCallback(void* args);


//--------------------------------------------------------------------------------
//    カメラ割り込み処理 (エラー時とVsync時に発生)
//
/*---------------------------------------------------------------------------*
  Name:         CAMERA_CameraIntr

  Description:  カメラの割り込み処理
                バッファエラー時とVSYNC時に呼び出される

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CAMERA_CameraIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_CAMERA);

    if (CAMERA_GetErrorStatus())
    {
        // ユーザ登録のバッファエラー割り込みコールバックを呼び出す
        if(cameraWork.CAMERAiBufferErrorCallback != 0)
            cameraWork.CAMERAiBufferErrorCallback(CAMERA_RESULT_SUCCESS);
    }
    else
    {
        cameraWork.CAMERAiLastCameraVSync = OS_GetTick();
        // ユーザ登録のVSYNC割り込みコールバックを呼び出す
        if(cameraWork.CAMERAiVsyncCallback != 0)
            cameraWork.CAMERAiVsyncCallback(CAMERA_RESULT_SUCCESS);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetVsynCallback

  Description:  カメラのVSYNC割り込み発生時に呼び出されるコールバック関数を登録する

  Arguments:    callback    - 登録するコールバック関数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_SetVsyncCallbackCore(CAMERAIntrCallback callback)
{
    cameraWork.CAMERAiVsyncCallback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetBufferErrorCallback

  Description:  カメラのバッファエラー割り込み発生時に呼び出されるコールバック関数を登録する

  Arguments:    callback    - 登録するコールバック関数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_SetBufferErrorCallbackCore(CAMERAIntrCallback callback)
{
    cameraWork.CAMERAiBufferErrorCallback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetRebootCallback

  Description:  カメラの誤作動からの復帰処理が完了した際に呼び出されるコールバック関数を登録する

  Arguments:    callback    - 登録するコールバック関数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_SetRebootCallbackCore(CAMERAIntrCallback callback)
{
    cameraWork.CAMERAiRebootCallback = callback;
}


static CAMERAWhiteBalance CAMERA_GetWhiteBalanceFromPhotoMode(CAMERAPhotoMode photo)
{
    switch (photo)
    {
    case CAMERA_PHOTO_MODE_NORMAL:
    case CAMERA_PHOTO_MODE_PORTRAIT:
        return CAMERA_WHITE_BALANCE_NORMAL;
    case CAMERA_PHOTO_MODE_LANDSCAPE:
        return CAMERA_WHITE_BALANCE_DAYLIGHT;
    case CAMERA_PHOTO_MODE_NIGHTVIEW:
    case CAMERA_PHOTO_MODE_LETTER:
        return CAMERA_WHITE_BALANCE_NORMAL;
    }
    return CAMERA_WHITE_BALANCE_NORMAL; // ここにはこないはず
}

static int CAMERA_GetSharpnessFromPhotoMode(CAMERAPhotoMode photo)
{
    switch (photo)
    {
    case CAMERA_PHOTO_MODE_NORMAL:
        return 0;
    case CAMERA_PHOTO_MODE_PORTRAIT:
        return -2;
    case CAMERA_PHOTO_MODE_LANDSCAPE:
        return 1;
    case CAMERA_PHOTO_MODE_NIGHTVIEW:
        return -1;
    case CAMERA_PHOTO_MODE_LETTER:
        return 2;
    }
    return 0; // ここにはこないはず
}

static int CAMERA_GetExposureFromPhotoMode(CAMERAPhotoMode photo)
{
    switch (photo)
    {
    case CAMERA_PHOTO_MODE_NORMAL:
    case CAMERA_PHOTO_MODE_PORTRAIT:
    case CAMERA_PHOTO_MODE_LANDSCAPE:
        return 0;
    case CAMERA_PHOTO_MODE_NIGHTVIEW:
    case CAMERA_PHOTO_MODE_LETTER:
        return 2;
    }
    return 0; // ここにはこないはず
}

static void CAMERA_GoReboot(void)
{
    cameraWork.lock = TRUE;
    cameraWork.CAMERAiRetryCount++;
    /*
        CAMERA_MonitorThreadに再起動処理を開始するように指示する。
            これは、シーケンス的に多重呼び出しされることはない。
            また、多重呼び出しがあっても1回通知できれば問題ない。
    */
    (void)OS_SendMessage(cameraWork.CAMERAiMonitorMessageQueue, NULL, OS_MESSAGE_NOBLOCK);    // ロック中に呼び出すこと
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Reboot

  Description:  カメラの再起動ルーチン

  Arguments:    result - 前回の I2C 処理の結果
                arg    - 次に呼び出す I2C 処理の種類

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CAMERA_Reboot(CAMERAResult result, void* arg)
{
    CAMERAI2CList state = (CAMERAI2CList)arg;

//OS_TPrintf("callback was called (%d).\n", state);
    if(result != CAMERA_RESULT_SUCCESS) // 失敗していればやり直し
    {
//OS_TPrintf("Failed to reboot (%d).\n", result);
        return;
    }

    switch(state)
    {
    case I2C_Init:
//OS_TPrintf("CAMERA_I2CInitAsyncCore(CAMERA_SELECT_BOTH)\n");
        (void)CAMERA_I2CInitAsyncCore(CAMERA_SELECT_BOTH, CAMERA_Reboot, (void*)I2C_PhotoMode_IN);
        return;
    case I2C_PhotoMode_IN:
        if(cameraWork.CAMERAiStateIn.photo != CAMERA_PHOTO_MODE_PORTRAIT)
        {
//OS_TPrintf("CAMERA_I2CPhotoModeAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.photo)\n");
            (void)CAMERA_I2CPhotoModeAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.photo, CAMERA_Reboot, (void*)I2C_PhotoMode_OUT);
            return;
        }
    case I2C_PhotoMode_OUT:
        if(cameraWork.CAMERAiStateOut.photo != CAMERA_PHOTO_MODE_NORMAL)
        {
//OS_TPrintf("CAMERA_I2CPhotoModeAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.photo)\n");
            (void)CAMERA_I2CPhotoModeAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.photo, CAMERA_Reboot, (void*)I2C_SizeA_IN);
            return;
        }
    case I2C_SizeA_IN:
        if(cameraWork.CAMERAiStateIn.size_A != CAMERA_SIZE_DS_LCD)
        {
//OS_TPrintf("CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateIn.size_A)\n");
            (void)CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateIn.size_A, CAMERA_Reboot, (void*)I2C_SizeB_IN);
            return;
        }
    case I2C_SizeB_IN:
        if(cameraWork.CAMERAiStateIn.size_B != CAMERA_SIZE_VGA)
        {
//OS_TPrintf("CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateIn.size_B)\n");
            (void)CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateIn.size_B, CAMERA_Reboot, (void*)I2C_SizeA_OUT);
            return;
        }
    case I2C_SizeA_OUT:
        if(cameraWork.CAMERAiStateOut.size_A != CAMERA_SIZE_DS_LCD)
        {
//OS_TPrintf("CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateOut.size_A)\n");
            (void)CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateOut.size_A, CAMERA_Reboot, (void*)I2C_SizeB_OUT);
            return;
        }
    case I2C_SizeB_OUT:
        if(cameraWork.CAMERAiStateOut.size_B != CAMERA_SIZE_VGA)
        {
//OS_TPrintf("CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateOut.size_B)\n");
            (void)CAMERA_I2CSizeExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateOut.size_B, CAMERA_Reboot, (void*)I2C_FrameRate_IN);
            return;
        }
    case I2C_FrameRate_IN:
        if(cameraWork.CAMERAiStateIn.rate != CAMERA_FRAME_RATE_15)
        {
//OS_TPrintf("CAMERA_I2CFrameRateAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.rate)\n");
            (void)CAMERA_I2CFrameRateAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.rate, CAMERA_Reboot, (void*)I2C_FrameRate_OUT);
            return;
        }
    case I2C_FrameRate_OUT:
        if(cameraWork.CAMERAiStateOut.rate != CAMERA_FRAME_RATE_15)
        {
//OS_TPrintf("CAMERA_I2CFrameRateAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.rate)\n");
            (void)CAMERA_I2CFrameRateAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.rate, CAMERA_Reboot, (void*)I2C_EffectA_IN);
            return;
        }
    case I2C_EffectA_IN:
        if(cameraWork.CAMERAiStateIn.effect_A != CAMERA_EFFECT_NONE)
        {
//OS_TPrintf("CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateIn.effect_A)\n");
            (void)CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateIn.effect_A, CAMERA_Reboot, (void*)I2C_EffectB_IN);
            return;
        }
    case I2C_EffectB_IN:
        if(cameraWork.CAMERAiStateIn.effect_B != CAMERA_EFFECT_NONE)
        {
//OS_TPrintf("CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateIn.effect_B)\n");
            (void)CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateIn.effect_B, CAMERA_Reboot, (void*)I2C_EffectA_OUT);
            return;
        }
    case I2C_EffectA_OUT:
        if(cameraWork.CAMERAiStateOut.effect_A != CAMERA_EFFECT_NONE)
        {
//OS_TPrintf("CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateOut.effect_A)\n");
            (void)CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateOut.effect_A, CAMERA_Reboot, (void*)I2C_EffectB_OUT);
            return;
        }
    case I2C_EffectB_OUT:
        if(cameraWork.CAMERAiStateOut.effect_B != CAMERA_EFFECT_NONE)
        {
//OS_TPrintf("CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateOut.effect_B)\n");
            (void)CAMERA_I2CEffectExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateOut.effect_B, CAMERA_Reboot, (void*)I2C_FlipA_IN);
            return;
        }
    case I2C_FlipA_IN:
        if(cameraWork.CAMERAiStateIn.flip_A != CAMERA_FLIP_HORIZONTAL)
        {
//OS_TPrintf("CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateIn.flip_A)\n");
            (void)CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateIn.flip_A, CAMERA_Reboot, (void*)I2C_FlipB_IN);
            return;
        }
    case I2C_FlipB_IN:
        if(cameraWork.CAMERAiStateIn.flip_B != CAMERA_FLIP_HORIZONTAL)
        {
//OS_TPrintf("CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateIn.flip_B)\n");
            (void)CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_IN, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateIn.flip_B, CAMERA_Reboot, (void*)I2C_FlipA_OUT);
            return;
        }
    case I2C_FlipA_OUT:
        if(cameraWork.CAMERAiStateOut.flip_A != CAMERA_FLIP_NONE)
        {
//OS_TPrintf("CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateOut.flip_A)\n");
            (void)CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_A, cameraWork.CAMERAiStateOut.flip_A, CAMERA_Reboot, (void*)I2C_FlipB_OUT);
            return;
        }
    case I2C_FlipB_OUT:
        if(cameraWork.CAMERAiStateOut.flip_B != CAMERA_FLIP_NONE)
        {
//OS_TPrintf("CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateOut.flip_B)\n");
            (void)CAMERA_I2CFlipExAsyncCore(CAMERA_SELECT_OUT, CAMERA_CONTEXT_B, cameraWork.CAMERAiStateOut.flip_B, CAMERA_Reboot, (void*)I2C_WhiteBalance_IN);
            return;
        }
    case I2C_WhiteBalance_IN:
        if(cameraWork.CAMERAiStateIn.wb != CAMERA_GetWhiteBalanceFromPhotoMode(cameraWork.CAMERAiStateIn.photo))
        {
//OS_TPrintf("CAMERA_I2CWhiteBalanceAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.wb)\n");
            (void)CAMERA_I2CWhiteBalanceAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.wb, CAMERA_Reboot, (void*)I2C_WhiteBalance_OUT);
            return;
        }
    case I2C_WhiteBalance_OUT:
        if(cameraWork.CAMERAiStateOut.wb != CAMERA_GetWhiteBalanceFromPhotoMode(cameraWork.CAMERAiStateOut.photo))
        {
//OS_TPrintf("CAMERA_I2CWhiteBalanceAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.wb)\n");
            (void)CAMERA_I2CWhiteBalanceAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.wb, CAMERA_Reboot, (void*)I2C_Exposure_IN);
            return;
        }
    case I2C_Exposure_IN:
        if (cameraWork.CAMERAiStateIn.exposure != CAMERA_GetExposureFromPhotoMode(cameraWork.CAMERAiStateIn.photo))
        {
//OS_TPrintf("CAMERA_I2CExposureAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.exposure)\n");
            (void)CAMERA_I2CExposureAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.exposure, CAMERA_Reboot, (void*)I2C_Exposure_OUT);
            return;
        }
    case I2C_Exposure_OUT:
        if (cameraWork.CAMERAiStateOut.exposure != CAMERA_GetExposureFromPhotoMode(cameraWork.CAMERAiStateOut.photo))
        {
//OS_TPrintf("CAMERA_I2CExposureAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.exposure)\n");
            (void)CAMERA_I2CExposureAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.exposure, CAMERA_Reboot, (void*)I2C_Sharpness_IN);
            return;
        }
    case I2C_Sharpness_IN:
        if (cameraWork.CAMERAiStateIn.sharpness != CAMERA_GetSharpnessFromPhotoMode(cameraWork.CAMERAiStateIn.photo))
        {
//OS_TPrintf("CAMERA_I2CSharpnessAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.sharpness)\n");
            (void)CAMERA_I2CSharpnessAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.sharpness, CAMERA_Reboot, (void*)I2C_Sharpness_OUT);
            return;
        }
    case I2C_Sharpness_OUT:
        if (cameraWork.CAMERAiStateOut.sharpness != CAMERA_GetSharpnessFromPhotoMode(cameraWork.CAMERAiStateOut.photo))
        {
//OS_TPrintf("CAMERA_I2CSharpnessAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.sharpness)\n");
            (void)CAMERA_I2CSharpnessAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.sharpness, CAMERA_Reboot, (void*)I2C_AutoExposure_IN);
            return;
        }
    case I2C_AutoExposure_IN:
        if (cameraWork.CAMERAiStateIn.ae != TRUE)
        {
//OS_TPrintf("CAMERA_I2CAutoExposureAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.ae)\n");
            (void)CAMERA_I2CAutoExposureAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.ae, CAMERA_Reboot, (void*)I2C_AutoExposure_OUT);
            return;
        }
    case I2C_AutoExposure_OUT:
        if (cameraWork.CAMERAiStateOut.ae != TRUE)
        {
//OS_TPrintf("CAMERA_I2CAutoExposureAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.ae)\n");
            (void)CAMERA_I2CAutoExposureAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.ae, CAMERA_Reboot, (void*)I2C_AutoWhiteBalance_IN);
            return;
        }
    case I2C_AutoWhiteBalance_IN:
        if (cameraWork.CAMERAiStateIn.awb != TRUE)
        {
//OS_TPrintf("CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.awb)\n");
            (void)CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.awb, CAMERA_Reboot, (void*)I2C_AutoWhiteBalance_OUT);
            return;
        }
    case I2C_AutoWhiteBalance_OUT:
        if (cameraWork.CAMERAiStateOut.awb != TRUE)
        {
//OS_TPrintf("CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.awb)\n");
            (void)CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.awb, CAMERA_Reboot, (void*)I2C_Context_IN);
            return;
        }
    case I2C_Context_IN:
        if (cameraWork.CAMERAiStateIn.context != CAMERA_CONTEXT_A)
        {
//OS_TPrintf("CAMERA_I2CContextSwitchAsync(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.context)\n");
            (void)CAMERA_I2CContextSwitchAsync(CAMERA_SELECT_IN, cameraWork.CAMERAiStateIn.context, CAMERA_Reboot, (void*)I2C_Context_OUT);
            return;
        }
    case I2C_Context_OUT:
        if (cameraWork.CAMERAiStateOut.context != CAMERA_CONTEXT_A)
        {
//OS_TPrintf("CAMERA_I2CContextSwitchAsync(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.context)\n");
            (void)CAMERA_I2CContextSwitchAsync(CAMERA_SELECT_OUT, cameraWork.CAMERAiStateOut.context, CAMERA_Reboot, (void*)I2C_Activate);
            return;
        }
    case I2C_Activate:
        // 元々のカメラ側で起動
        if (cameraWork.CAMERAiCurrentCamera != CAMERA_SELECT_NONE)
        {
//OS_TPrintf("CAMERA_I2CActivateAsyncCore(cameraWork.CAMERAiCurrentCamera)\n");
            (void)CAMERA_I2CActivateAsyncCore(cameraWork.CAMERAiCurrentCamera, CAMERA_Reboot, (void*)Set_LED);
            return;
        }
    case Set_LED:
        // OUT側のカメラが有効となっている場合のみ、LEDを変更する必要があるならばする
        if ((cameraWork.CAMERAiCurrentCamera == CAMERA_SELECT_OUT)&&(cameraWork.CAMERAiStateOut.blink == TRUE))
        {
            (void)CAMERA_SetLEDAsync(TRUE, CAMERA_Reboot, (void*)I2C_MAX);
            return;
        }
    }
//OS_TPrintf("Reboot was done.\n");
    cameraWork.CAMERAiRetryCount = 0; // リトライカウンタの初期化
    if(cameraWork.CAMERAiRebootCallback != 0)    // 再起動処理が完了したことをアプリに通知する
    {
        cameraWork.CAMERAiRebootCallback(CAMERA_RESULT_SUCCESS);
    }
}

static CAMERAResult CAMERA_CheckRetryCount(void)
{
    // I2C関数が一定回数以上失敗しているなら、FATALとする
    if(cameraWork.CAMERAiRetryCount > CAMERA_RETRY_COUNT)
    {
        // カメラが無効でなかった場合は、エラーを返す前にカメラを無効にしておく
        if(cameraWork.CAMERAiCurrentCamera != CAMERA_SELECT_NONE)
        {
            // マスタークロック指定
            cameraWork.force_deactivate = TRUE;

            cameraWork.CAMERAiCurrentCamera = CAMERA_SELECT_NONE;

            (void)CameraSendPxiCommand(CAMERA_PXI_COMMAND_ACTIVATE, CAMERA_PXI_SIZE_ACTIVATE, (u8)CAMERA_SELECT_NONE);
        }
        return CAMERA_RESULT_FATAL_ERROR;
    }
    // 再起動中であれば、ILLEGAL_STATUS とする
    if(cameraWork.CAMERAiRetryCount > 0)
    {
        return CAMERA_RESULT_ILLEGAL_STATUS;
    }
    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_MonitorThread

  Description:  カメラ監視兼再起動開始スレッド

  Arguments:    arg   - 未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define CAMERA_MONITOR_INTERVAL 100
#define CAMERA_MONITOR_COUNT    (1000/CAMERA_MONITOR_INTERVAL)
static void CAMERA_MonitorThread(void* arg)
{
#pragma unused(arg)
    (void)OS_DisableInterrupts();
    while (1)
    {
        OSMessage msg;
        OS_Sleep(CAMERA_MONITOR_INTERVAL);
        if ( !OS_ReceiveMessage(cameraWork.CAMERAiMonitorMessageQueue, &msg, OS_MESSAGE_NOBLOCK) )
        {   // 定期呼び出し処理
            if (cameraWork.CAMERAiRetryCount != 0                               // すでに発動中ならなにもしない
                || cameraWork.CAMERAiCurrentCamera == CAMERA_SELECT_NONE        // スタンバイ中なら何もしない
                || cameraWork.lock                                              // 別のI2C呼び出し中なら何もしない
                || ++cameraWork.CAMERAiMonitorCounter < CAMERA_MONITOR_COUNT)   // 時間が十分経っていなければ何もしない
            {
                continue;
            }
            cameraWork.CAMERAiMonitorCounter = 0;
            if ( (int)(OS_GetTick() - cameraWork.CAMERAiLastCameraVSync) < CAMERA_VSYNC_TIMEOUT )
            {
                continue;
            }
            // 再起動確定 (GoReboot();return;でも良いが・・)
            cameraWork.lock = TRUE;
            cameraWork.CAMERAiRetryCount++;
        }
//OS_TPrintf("Reboot....\n");
        // 再起動開始処理
        if ( CAMERA_CheckRetryCount() == CAMERA_RESULT_FATAL_ERROR )    // 一定以上の失敗でFATALエラーになる
        {
            cameraWork.lock = FALSE; // 失敗したのでロック解除
            if(cameraWork.CAMERAiRebootCallback != 0)    // 再起動処理が失敗したことをアプリに通知する
            {
                cameraWork.CAMERAiRebootCallback(CAMERA_RESULT_FATAL_ERROR);
            }
            return;
        }
        (void)OS_EnableInterrupts();    // 時間がかかるので・・・
        CAMERA_ResetCore(); // ハードウェアリセット
        (void)OS_DisableInterrupts();
        cameraWork.lock = FALSE;
        CAMERA_Reboot(CAMERA_RESULT_SUCCESS, (void*)I2C_Init); // 再起動ルーチンは、lock == FALSE, 割り込み禁止で呼び出す
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERAライブラリを初期化する。

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_InitCore(void)
{
    //---- check if tick system is available
    SDK_ASSERTMSG(OS_IsTickAvailable(), "CAMERA_Init: camera system needs of tick system.");

    // 初期化済みを確認
    if (cameraInitialized)
    {
        return CAMERA_RESULT_BUSY;
    }
    cameraInitialized = 1;

    // 各種コールバック関数のクリア
    cameraWork.CAMERAiVsyncCallback = 0;
    cameraWork.CAMERAiBufferErrorCallback = 0;
    cameraWork.CAMERAiRebootCallback = 0;

    // 変数初期化
    cameraWork.lock = FALSE;
    cameraWork.callback = NULL;
    cameraWork.force_deactivate = cameraWork.force_activate = FALSE;

    cameraWork.CAMERAiCurrentCamera = CAMERA_SELECT_NONE;
    cameraWork.CAMERAiRetryCount = 0; // リトライカウンタの初期化
    cameraWork.CAMERAiMonitorCounter = 0;

    // PXI関連を初期化
    PXI_Init();
    if (!PXI_IsCallbackReady(PXI_FIFO_TAG_CAMERA, PXI_PROC_ARM7))
    {
        return CAMERA_RESULT_FATAL_ERROR;
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CAMERA, CameraPxiCallback);
#if 0
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, 0, 0))
    {
        OS_TPrintf("===failed PXI_Send...ARM7 Start\n");
        return CAMERA_RESULT_SEND_ERROR;
    }
#endif
    // ハードウェアリセット
    CAMERA_ResetCore();
    // キャプチャの停止とエラー無しを保証
    CAMERA_StopCaptureCore();
    CAMERA_ClearBufferCore();

    // スリープ復帰時コールバックを登録する。
    PM_SetSleepCallbackInfo(&cameraWork.sleepCbInfo, CameraStandbyCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&cameraWork.sleepCbInfo, PM_CALLBACK_PRIORITY_CAMERA);
    // リセット/電源OFF時コールバックを登録する。
    PM_SetExitCallbackInfo(&cameraWork.exitCbInfo, CameraStandbyCallback, NULL);
    PMi_InsertPostExitCallbackEx(&cameraWork.exitCbInfo, PM_CALLBACK_PRIORITY_CAMERA);

    // カメラ割り込み設定
    CAMERA_SetVsyncInterruptCore(CAMERA_INTR_VSYNC_NEGATIVE_EDGE);
    CAMERA_SetBufferErrorInterruptCore(TRUE);
    CAMERA_SetMasterInterruptCore(TRUE);
    OS_SetIrqFunction(OS_IE_CAMERA, CAMERA_CameraIntr);
    (void)OS_EnableIrqMask(OS_IE_CAMERA);

    // カメラIN/OUTの設定値を初期化
    {
        cameraWork.CAMERAiStateIn.size_A = CAMERA_SIZE_DS_LCD;
        cameraWork.CAMERAiStateIn.size_B = CAMERA_SIZE_VGA;
        cameraWork.CAMERAiStateIn.effect_A = CAMERA_EFFECT_NONE;
        cameraWork.CAMERAiStateIn.effect_B = CAMERA_EFFECT_NONE;
        cameraWork.CAMERAiStateIn.flip_A = CAMERA_FLIP_HORIZONTAL;
        cameraWork.CAMERAiStateIn.flip_B = CAMERA_FLIP_HORIZONTAL;
        cameraWork.CAMERAiStateIn.photo = CAMERA_PHOTO_MODE_PORTRAIT;
        cameraWork.CAMERAiStateIn.wb = CAMERA_WHITE_BALANCE_NORMAL;
        cameraWork.CAMERAiStateIn.rate = CAMERA_FRAME_RATE_15;
        cameraWork.CAMERAiStateIn.exposure = 0;
        cameraWork.CAMERAiStateIn.sharpness = -2;
        cameraWork.CAMERAiStateIn.ae = TRUE;
        cameraWork.CAMERAiStateIn.awb = TRUE;
        cameraWork.CAMERAiStateIn.context = CAMERA_CONTEXT_A;
        cameraWork.CAMERAiStateIn.blink = FALSE; // IN側にLEDの点灯点滅オプションはないが、一応初期化だけしておく

        cameraWork.CAMERAiStateOut.size_A = CAMERA_SIZE_DS_LCD;
        cameraWork.CAMERAiStateOut.size_B = CAMERA_SIZE_VGA;
        cameraWork.CAMERAiStateOut.effect_A = CAMERA_EFFECT_NONE;
        cameraWork.CAMERAiStateOut.effect_B = CAMERA_EFFECT_NONE;
        cameraWork.CAMERAiStateOut.flip_A = CAMERA_FLIP_NONE;
        cameraWork.CAMERAiStateOut.flip_B = CAMERA_FLIP_NONE;
        cameraWork.CAMERAiStateOut.photo = CAMERA_PHOTO_MODE_NORMAL;
        cameraWork.CAMERAiStateOut.wb = CAMERA_WHITE_BALANCE_NORMAL;
        cameraWork.CAMERAiStateOut.rate = CAMERA_FRAME_RATE_15;
        cameraWork.CAMERAiStateOut.exposure = 0;
        cameraWork.CAMERAiStateOut.sharpness = 0;
        cameraWork.CAMERAiStateOut.ae = TRUE;
        cameraWork.CAMERAiStateOut.awb = TRUE;
        cameraWork.CAMERAiStateOut.context = CAMERA_CONTEXT_A;
        cameraWork.CAMERAiStateOut.blink = FALSE;
    }

    // 再起動のためのメッセージキュー作成
    OS_InitMessageQueue(cameraWork.CAMERAiMonitorMessageQueue, cameraWork.CAMERAiMonitorMessage, sizeof(cameraWork.CAMERAiMonitorMessage)/sizeof(OSMessage));
    // 誤動作検知用のスレッドを作成
    OS_CreateThread(cameraWork.CAMERAiMonitorThread, CAMERA_MonitorThread, 0, &cameraWork.CAMERAiMonitorThreadStack[1024], 4096, 4);
    OS_WakeupThreadDirect(cameraWork.CAMERAiMonitorThread);
    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_End

  Description:  CAMERAライブラリを終了する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_EndCore(void)
{
    // 初期化済みを確認
    if (cameraInitialized == 0)
    {
        return;
    }
    cameraInitialized = 0;

    // スリープ復帰時コールバックを削除する。
    PM_DeletePreSleepCallback(&cameraWork.sleepCbInfo);
    // リセット/電源OFF時コールバックを削除する。
    PM_DeletePostExitCallback(&cameraWork.exitCbInfo);

    // カメラモジュール停止
    (void)CAMERA_I2CActivateCore(CAMERA_SELECT_NONE);

    // ハードウェアリセット
    CAMERA_ResetCore();
    // キャプチャの停止とエラー無しを保証
    CAMERA_StopCaptureCore();
    CAMERA_ClearBufferCore();

    // PXI関連停止
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CAMERA, NULL);

    // カメラ割り込み設定解除
    CAMERA_SetVsyncInterruptCore(CAMERA_INTR_VSYNC_NONE);
    CAMERA_SetBufferErrorInterruptCore(FALSE);
    CAMERA_SetMasterInterruptCore(FALSE);
    OS_SetIrqFunction(OS_IE_CAMERA, 0);
    (void)OS_DisableIrqMask(OS_IE_CAMERA);

    // 誤動作検知用のスレッドを削除
    OS_KillThread(cameraWork.CAMERAiMonitorThread, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Start

  Description:  キャプチャを開始する上位API。切り替えにも使える。
                sync version only

  Arguments:    camera      - one of CAMERASelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_StartCore(CAMERASelect camera)
{
    CAMERAResult result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    //case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    result = CAMERA_I2CActivateCore(camera);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        return result;
    }
    CAMERA_StopCaptureCore();
    while ( CAMERA_IsBusyCore() != FALSE)
    {
    }
    CAMERA_ClearBufferCore();
    CAMERA_StartCaptureCore();
    return CAMERA_RESULT_SUCCESS_TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Stop

  Description:  キャプチャを停止する上位API。
                sync version only

  Arguments:    None

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_StopCore(void)
{
    int limit = OS_MilliSecondsToTicks( 200 ) * 64 / 4;
    CAMERA_StopCaptureCore();
    while ( limit-- > 0 && CAMERA_IsBusyCore() != FALSE) // タイムアウト付き
    {
        OS_SpinWaitSysCycles( 4 );
    }
    CAMERA_ClearBufferCore();
    return CAMERA_I2CActivateCore(CAMERA_SELECT_NONE);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  initialize camera registers via I2C
                async version.

  Arguments:    camera      - one of CAMERASelect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInitAsyncCore(CAMERASelect camera, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_INIT;
    const u8                _size   = CAMERA_PXI_SIZE_INIT;
    OSIntrMode enabled;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;
    // マスタークロック指定
    cameraWork.force_deactivate = TRUE;
    return CameraSendPxiCommand(command, _size, (u8)camera) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize camera registers via I2C
                sync version.

  Arguments:    camera      - one of CAMERASelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInitCore(CAMERASelect camera)
{
    cameraWork.result = CAMERA_I2CInitAsyncCore(camera, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivateAsync

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                async version

  Arguments:    camera      - one of CAMERASelect (BOTH is not valid)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivateAsyncCore(CAMERASelect camera, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_ACTIVATE;
    const u8                _size   = CAMERA_PXI_SIZE_ACTIVATE;
    OSIntrMode enabled;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    //case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;
    // マスタークロック指定
    if (camera == CAMERA_SELECT_NONE)
    {
        cameraWork.force_deactivate = TRUE;
    }
    else
    {
        cameraWork.force_activate = TRUE;
    }

    cameraWork.CAMERAiCurrentCamera = camera;
    if(cameraWork.CAMERAiRetryCount == 0)
    {
        cameraWork.CAMERAiStateTmp.blink = FALSE;
    }

    cameraWork.CAMERAiLastCameraVSync = OS_GetTick();

    return CameraSendPxiCommand(command, _size, (u8)camera) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivate

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                sync version.

  Arguments:    camera      - one of CAMERASelect (BOTH is not valid)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivateCore(CAMERASelect camera)
{
    cameraWork.result = CAMERA_I2CActivateAsyncCore(camera, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
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
CAMERAResult CAMERA_I2CContextSwitchAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_CONTEXT_SWITCH;
    const u8                _size   = CAMERA_PXI_SIZE_CONTEXT_SWITCH;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    // case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (context)
    {
    case CAMERA_CONTEXT_A:
    case CAMERA_CONTEXT_B:
    // case CAMERA_CONTEXT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)context;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.context = context;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CContextSwitch

  Description:  resize CAMERA output image
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CContextSwitchCore(CAMERASelect camera, CAMERAContext context)
{
    cameraWork.result = CAMERA_I2CContextSwitchAsyncCore(camera, context, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CSizeExAsync

  Description:  set CAMERA frame size
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B or BOTH)
                size        - one of CAMERASize
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CSizeExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERASize size, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_SIZE;
    const u8                _size   = CAMERA_PXI_SIZE_SIZE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (context)
    {
    case CAMERA_CONTEXT_A:
    case CAMERA_CONTEXT_B:
    case CAMERA_CONTEXT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (size)
    {
    case CAMERA_SIZE_VGA:
    case CAMERA_SIZE_QVGA:
    case CAMERA_SIZE_QQVGA:
    case CAMERA_SIZE_CIF:
    case CAMERA_SIZE_QCIF:
    case CAMERA_SIZE_DS_LCD:
    case CAMERA_SIZE_DS_LCDx4:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)context;
    data[2] = (u8)size;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.set_context = context;
    cameraWork.CAMERAiStateTmp.size = size;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CSizeEx

  Description:  set CAMERA frame size
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                size        - one of CAMERASize

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CSizeExCore(CAMERASelect camera, CAMERAContext context, CAMERASize size)
{
    cameraWork.result = CAMERA_I2CSizeExAsyncCore(camera, context, size, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRateAsync

  Description:  set CAMERA frame rate
                async version

  Arguments:    camera      - one of CAMERASelect
                rate        - one of CAMERAFrameRate
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRateAsyncCore(CAMERASelect camera, CAMERAFrameRate rate, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_FRAME_RATE;
    const u8                _size   = CAMERA_PXI_SIZE_FRAME_RATE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (rate)
    {
    case CAMERA_FRAME_RATE_15:
    case CAMERA_FRAME_RATE_15_TO_5:
    case CAMERA_FRAME_RATE_15_TO_2:
    case CAMERA_FRAME_RATE_8_5:
    case CAMERA_FRAME_RATE_5:
    case CAMERA_FRAME_RATE_20:
    case CAMERA_FRAME_RATE_20_TO_5:
    case CAMERA_FRAME_RATE_30:
    case CAMERA_FRAME_RATE_30_TO_5:
    case CAMERA_FRAME_RATE_15_TO_10:
    case CAMERA_FRAME_RATE_20_TO_10:
    case CAMERA_FRAME_RATE_30_TO_10:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)rate;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.rate = rate;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate
                sync version.

  Arguments:    camera      - one of CAMERASelect
                rate        - one of CAMERAFrameRate

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRateCore(CAMERASelect camera, CAMERAFrameRate rate)
{
    cameraWork.result = CAMERA_I2CFrameRateAsyncCore(camera, rate, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectExAsync

  Description:  set CAMERA effect
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                effect      - one of CAMERAEffect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffectExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_EFFECT;
    const u8                _size   = CAMERA_PXI_SIZE_EFFECT;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (context)
    {
    case CAMERA_CONTEXT_A:
    case CAMERA_CONTEXT_B:
    case CAMERA_CONTEXT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (effect)
    {
    case CAMERA_EFFECT_NONE:
    case CAMERA_EFFECT_MONO:
    case CAMERA_EFFECT_SEPIA:
    case CAMERA_EFFECT_NEGATIVE:
    case CAMERA_EFFECT_NEGAFILM:
    case CAMERA_EFFECT_SEPIA01:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)context;
    data[2] = (u8)effect;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.set_context = context;
    cameraWork.CAMERAiStateTmp.effect = effect;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectEx

  Description:  set CAMERA effect
                sync version.

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                effect      - one of CAMERAEffect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffectExCore(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect)
{
    cameraWork.result = CAMERA_I2CEffectExAsyncCore(camera, context, effect, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipExAsync

  Description:  set CAMERA flip/mirror
                async version

  Arguments:    camera      - one of CAMERASelect
                context     - one of CAMERAContext (A or B)
                flip        - one of CAMERAFlip
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlipExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_FLIP;
    const u8                _size   = CAMERA_PXI_SIZE_FLIP;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (context)
    {
    case CAMERA_CONTEXT_A:
    case CAMERA_CONTEXT_B:
    case CAMERA_CONTEXT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (flip)
    {
    case CAMERA_FLIP_NONE:
    case CAMERA_FLIP_VERTICAL:
    case CAMERA_FLIP_HORIZONTAL:
    case CAMERA_FLIP_REVERSE:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)context;
    data[2] = (u8)flip;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.set_context = context;
    cameraWork.CAMERAiStateTmp.flip = flip;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
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
CAMERAResult CAMERA_I2CFlipExCore(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip)
{
    cameraWork.result = CAMERA_I2CFlipExAsyncCore(camera, context, flip, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPhotoModeAsync

  Description:  set CAMERA photo mode
                async version

  Arguments:    camera      - one of CAMERASelect
                mode        - one of CAMERAPhotoMode
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPhotoModeAsyncCore(CAMERASelect camera, CAMERAPhotoMode mode, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_PHOTO_MODE;
    const u8                _size   = CAMERA_PXI_SIZE_PHOTO_MODE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (mode)
    {
    case CAMERA_PHOTO_MODE_NORMAL:
    case CAMERA_PHOTO_MODE_PORTRAIT:
    case CAMERA_PHOTO_MODE_LANDSCAPE:
    case CAMERA_PHOTO_MODE_NIGHTVIEW:
    case CAMERA_PHOTO_MODE_LETTER:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)mode;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    // PhotoModeについては、同時に変更される3つのパラメータも記憶する必要がある
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.photo = mode;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPhotoMode

  Description:  set CAMERA photo mode
                sync version.

  Arguments:    camera      - one of CAMERASelect
                mode        - one of CAMERAPhotoMode

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPhotoModeCore(CAMERASelect camera, CAMERAPhotoMode mode)
{
    cameraWork.result = CAMERA_I2CPhotoModeAsyncCore(camera, mode, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CWhiteBalanceAsync

  Description:  set CAMERA white balance
                async version

  Arguments:    camera      - one of CAMERASelect
                wb          - one of CAMERAWhiteBalance
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CWhiteBalanceAsyncCore(CAMERASelect camera, CAMERAWhiteBalance wb, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_WHITE_BALANCE;
    const u8                _size   = CAMERA_PXI_SIZE_WHITE_BALANCE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (wb)
    {
    case CAMERA_WHITE_BALANCE_AUTO:
    case CAMERA_WHITE_BALANCE_3200K:
    case CAMERA_WHITE_BALANCE_4150K:
    case CAMERA_WHITE_BALANCE_5200K:
    case CAMERA_WHITE_BALANCE_6000K:
    case CAMERA_WHITE_BALANCE_7000K:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)wb;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.wb = wb;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CWhiteBalance

  Description:  set CAMERA white balance
                sync version.

  Arguments:    camera      - one of CAMERASelect
                wb          - one of CAMERAWhiteBalance

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CWhiteBalanceCore(CAMERASelect camera, CAMERAWhiteBalance wb)
{
    cameraWork.result = CAMERA_I2CWhiteBalanceAsyncCore(camera, wb, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CExposureAsync

  Description:  set CAMERA exposure
                async version

  Arguments:    camera      - one of CAMERASelect
                exposure    - -5 to +5
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CExposureAsyncCore(CAMERASelect camera, int exposure, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_EXPOSURE;
    const u8                _size   = CAMERA_PXI_SIZE_EXPOSURE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    if (exposure < -5 || exposure > 5)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)exposure;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.exposure = exposure;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CExposure

  Description:  set CAMERA exposure
                sync version.

  Arguments:    camera      - one of CAMERASelect
                exposure    - -5 to +5
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CExposureCore(CAMERASelect camera, int exposure)
{
    cameraWork.result = CAMERA_I2CExposureAsyncCore(camera, exposure, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSharpnessAsync

  Description:  set CAMERA sharpness
                async version

  Arguments:    camera      - one of CAMERASelect
                sharpness   - -3 to +5
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CSharpnessAsyncCore(CAMERASelect camera, int sharpness, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_SHARPNESS;
    const u8                _size   = CAMERA_PXI_SIZE_SHARPNESS;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    if (sharpness < -3 || sharpness > 5)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)sharpness;

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.sharpness = sharpness;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSharpness

  Description:  set CAMERA sharpness
                sync version.

  Arguments:    camera      - one of CAMERASelect
                sharpness   - -3 to +5
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CSharpnessCore(CAMERASelect camera, int sharpness)
{
    cameraWork.result = CAMERA_I2CSharpnessAsyncCore(camera, sharpness, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CTestPatternAsync

  Description:  set CAMERA test pattern
                async version

  Arguments:    camera      - one of CAMERASelect
                pattern     - one of CAMERATestPattern
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERAi_I2CTestPatternAsyncCore(CAMERASelect camera, CAMERATestPattern pattern, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_TEST_PATTERN;
    const u8                _size   = CAMERA_PXI_SIZE_TEST_PATTERN;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    switch (pattern)
    {
    case CAMERA_TEST_PATTERN_DISABLED:
    case CAMERA_TEST_PATTERN_COLOR_BAR:
    case CAMERA_TEST_PATTERN_NOISE:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)pattern;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CTestPattern

  Description:  set CAMERA test pattern
                sync version.

  Arguments:    camera      - one of CAMERASelect
                pattern     - one of CAMERATestPattern

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERAi_I2CTestPatternCore(CAMERASelect camera, CAMERATestPattern pattern)
{
    cameraWork.result = CAMERAi_I2CTestPatternAsyncCore(camera, pattern, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoExposureAsync

  Description:  enable/disable CAMERA auto-exposure
                async version

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AE will enable
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CAutoExposureAsyncCore(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_AUTO_EXPOSURE;
    const u8                _size   = CAMERA_PXI_SIZE_AUTO_EXPOSURE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)(on ? TRUE : FALSE);

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.ae = on;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoExposure

  Description:  enable/disable CAMERA auto-exposure
                sync version.

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AE will enable
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CAutoExposureCore(CAMERASelect camera, BOOL on)
{
    cameraWork.result = CAMERA_I2CAutoExposureAsyncCore(camera, on, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoWhiteBalanceAsync

  Description:  enable/disable CAMERA auto-white-balance
                async version

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AWB will enable
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_AUTO_WHITE_BALANCE;
    const u8                _size   = CAMERA_PXI_SIZE_AUTO_WHITE_BALANCE;
    OSIntrMode enabled;
    u8  data[_size+2];
    int i;
    CAMERAResult result;

    SDK_NULL_ASSERT(callback);

    // I2C関数のリトライ回数及び再起動中か否かによる状態チェック
    result = CAMERA_CheckRetryCount();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        return result;

    switch (camera)
    {
    // case CAMERA_SELECT_NONE:
    case CAMERA_SELECT_IN:
    case CAMERA_SELECT_OUT:
    case CAMERA_SELECT_BOTH:
        break;
    default:
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    if ((camera & CAMERA_SELECT_IN) && cameraWork.CAMERAiStateIn.wb != CAMERA_WHITE_BALANCE_NORMAL)
    {
        return CAMERA_RESULT_INVALID_COMMAND;
    }
    if ((camera & CAMERA_SELECT_OUT) && cameraWork.CAMERAiStateOut.wb != CAMERA_WHITE_BALANCE_NORMAL)
    {
        return CAMERA_RESULT_INVALID_COMMAND;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return result == CAMERA_RESULT_ILLEGAL_STATUS ? CAMERA_RESULT_ILLEGAL_STATUS : CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)(on ? TRUE : FALSE);

    // 再起動ルーチン用のI2Cコマンド保存構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.set_camera = camera;
    cameraWork.CAMERAiStateTmp.awb = on;

    // コマンド送信
    if (CameraSendPxiCommand(command, _size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < _size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoWhiteBalance

  Description:  enable/disable CAMERA auto-white-balance
                sync version.

  Arguments:    camera      - one of CAMERASelect
                on          - TRUE if AE will enable
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CAutoWhiteBalanceCore(CAMERASelect camera, BOOL on)
{
    cameraWork.result = CAMERA_I2CAutoWhiteBalanceAsyncCore(camera, on, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetLEDAsync

  Description:  set CAMERA LED (outside) to blink or not
                it works only OUT camera is active.
                default state when calling I2CActivate(OUT/BOTH) does not blink.
                async version

  Arguments:    isBlink     - 点滅させたいならTRUE、点灯ならFALSE
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetLEDAsyncCore(BOOL isBlink, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_SET_LED;
    const u8                _size   = CAMERA_PXI_SIZE_SET_LED;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // 再起動ルーチン用の構造体に設定値をセット
    cameraWork.CAMERAiStateTmp.blink = isBlink;

    return CameraSendPxiCommand(command, _size, (u8)isBlink) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
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
CAMERAResult CAMERA_SetLEDCore(BOOL isBlink)
{
    cameraWork.result = CAMERA_SetLEDAsyncCore(isBlink, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

static CAMERACallback cameraSwitchOffLEDCallback;

static void CAMERAi_SwitchOffLEDAsyncCallback(CAMERAResult result, void* arg)
{

    if(result != CAMERA_RESULT_SUCCESS)
        cameraSwitchOffLEDCallback(result, arg);

    (void)CAMERA_SetLEDAsyncCore(FALSE, cameraSwitchOffLEDCallback, arg);
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
CAMERAResult CAMERA_SwitchOffLEDAsyncCore(CAMERACallback callback, void *arg)
{
    cameraSwitchOffLEDCallback = callback;

    return CAMERA_SetLEDAsyncCore(TRUE, CAMERAi_SwitchOffLEDAsyncCallback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SwitchOffLED

  Description:  一度だけカメラのLEDを消灯させます。
                CAMERA_SetLED(TRUE)、CAMERA_SetLED(FALSE) を連続で呼び出した場合と
                同じ動作となります。

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SwitchOffLEDCore(void)
{
    CAMERAResult result;

    result = CAMERA_SetLEDCore(TRUE);
    if(result != CAMERA_RESULT_SUCCESS)
        return result;
    return CAMERA_SetLEDCore(FALSE);
}

static inline void CAMERAi_Wait(u32 clocks)
{
    OS_SpinWaitSysCycles(clocks << 1);
}
static inline BOOL CAMERAi_StopMasterClock(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    u16  reg = reg_SCFG_CLK;
    reg_SCFG_CLK = (u16)(reg & ~REG_SCFG_CLK_CAMCKI_MASK);
    (void)OS_RestoreInterrupts(enabled);
    return (BOOL)((reg & REG_SCFG_CLK_CAMCKI_MASK) >> REG_SCFG_CLK_CAMCKI_SHIFT);
}
static inline BOOL CAMERAi_StartMasterClock(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    u16  reg = reg_SCFG_CLK;
    reg_SCFG_CLK = (u16)(reg | REG_SCFG_CLK_CAMCKI_MASK);
    (void)OS_RestoreInterrupts(enabled);
    if ( (reg & REG_SCFG_CLK_CAMCKI_MASK) == 0 )
    {
        CAMERAi_Wait( 10 );
    }
    return (BOOL)((reg & REG_SCFG_CLK_CAMCKI_MASK) >> REG_SCFG_CLK_CAMCKI_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         CameraSendPxiCommand

  Description:  指定先頭コマンドをPXI経由でARM7に送信する。

  Arguments:    command     - 対象コマンド
                size        - 送信データサイズ (バイト単位)
                data        - 先頭データ (1バイトのみ)

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL CameraSendPxiCommand(CAMERAPxiCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            ((size << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((data << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    cameraWork.last_state = CAMERAi_StartMasterClock();
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
        if ( cameraWork.last_state == FALSE )
        {
            (void)CAMERAi_StopMasterClock();
        }
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CameraSendPxiData

  Description:  指定後続データをPXI経由でARM7に送信する。

  Arguments:    pData   - 3バイトデータの先頭へのポインタ

  Returns:      None
 *---------------------------------------------------------------------------*/
static void CameraSendPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraPxiCallback

  Description:  非同期RTC関数用の共通コールバック関数。

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    CAMERAResult   result;

    // PXI通信エラーを確認
    if (err)
    {
        // シーケンスを強制終了
        CameraCallCallbackAndUnlock(CAMERA_RESULT_FATAL_ERROR);
        return;
    }
    // 先頭データ
    if (data & CAMERA_PXI_START_BIT)
    {
        // 受信データを解析
        SDK_ASSERT((data & CAMERA_PXI_RESULT_BIT) == CAMERA_PXI_RESULT_BIT);
        cameraWork.total = (u8)((data & CAMERA_PXI_DATA_NUMS_MASK) >> CAMERA_PXI_DATA_NUMS_SHIFT);
        cameraWork.current = 0;
        cameraWork.command = (CAMERAPxiCommand)((data & CAMERA_PXI_COMMAND_MASK) >> CAMERA_PXI_COMMAND_SHIFT);
        cameraWork.pxiResult = (CAMERAPxiResult)((data & CAMERA_PXI_1ST_DATA_MASK) >> CAMERA_PXI_1ST_DATA_SHIFT);
    }
    // 後続データ
    else
    {
        if (cameraWork.data == NULL)
        {
            // シーケンスを強制終了
            CameraCallCallbackAndUnlock(CAMERA_RESULT_FATAL_ERROR);
            return;
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0xFF0000) >> 16);
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0x00FF00) >> 8);
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0x0000FF) >> 0);
        }
    }

    if (cameraWork.current >= cameraWork.total-1)   // > は無いはず
    {
        if (cameraWork.force_deactivate != FALSE || (cameraWork.force_activate == FALSE && cameraWork.last_state == FALSE))
        {
            (void)CAMERAi_StopMasterClock();
        }
        cameraWork.force_deactivate = cameraWork.force_activate = FALSE;

        // 処理結果を確認
        switch (cameraWork.pxiResult)
        {
        case CAMERA_PXI_RESULT_SUCCESS:     // alias CAMERA_PXI_RESULT_SUCCESS_TRUE
            result = CAMERA_RESULT_SUCCESS; // alias CAMERA_RESULT_SUCCESS_TRUE
            // 処理が成功したので、再起動用の構造体に変更点を修正する
            switch (cameraWork.command)
            {
            case CAMERA_PXI_COMMAND_ACTIVATE:
                cameraWork.CAMERAiStateOut.blink = cameraWork.CAMERAiStateTmp.blink;
                break;
            case CAMERA_PXI_COMMAND_CONTEXT_SWITCH:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    cameraWork.CAMERAiStateIn.context = cameraWork.CAMERAiStateTmp.context;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    cameraWork.CAMERAiStateOut.context = cameraWork.CAMERAiStateTmp.context;
                break;
            case CAMERA_PXI_COMMAND_SIZE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_A)
                        cameraWork.CAMERAiStateIn.size_A = cameraWork.CAMERAiStateTmp.size;
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_B)
                        cameraWork.CAMERAiStateIn.size_B = cameraWork.CAMERAiStateTmp.size;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_A)
                        cameraWork.CAMERAiStateOut.size_A = cameraWork.CAMERAiStateTmp.size;
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_B)
                        cameraWork.CAMERAiStateOut.size_B = cameraWork.CAMERAiStateTmp.size;
                break;
            case CAMERA_PXI_COMMAND_FRAME_RATE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    cameraWork.CAMERAiStateIn.rate = cameraWork.CAMERAiStateTmp.rate;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    cameraWork.CAMERAiStateOut.rate = cameraWork.CAMERAiStateTmp.rate;
                break;
            case CAMERA_PXI_COMMAND_EFFECT:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_A)
                        cameraWork.CAMERAiStateIn.effect_A = cameraWork.CAMERAiStateTmp.effect;
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_B)
                        cameraWork.CAMERAiStateIn.effect_B = cameraWork.CAMERAiStateTmp.effect;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_A)
                        cameraWork.CAMERAiStateOut.effect_A = cameraWork.CAMERAiStateTmp.effect;
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_B)
                        cameraWork.CAMERAiStateOut.effect_B = cameraWork.CAMERAiStateTmp.effect;
                break;
            case CAMERA_PXI_COMMAND_FLIP:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_A)
                        cameraWork.CAMERAiStateIn.flip_A = cameraWork.CAMERAiStateTmp.flip;
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_B)
                        cameraWork.CAMERAiStateIn.flip_B = cameraWork.CAMERAiStateTmp.flip;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_A)
                        cameraWork.CAMERAiStateOut.flip_A = cameraWork.CAMERAiStateTmp.flip;
                    if(cameraWork.CAMERAiStateTmp.set_context & CAMERA_CONTEXT_B)
                        cameraWork.CAMERAiStateOut.flip_B = cameraWork.CAMERAiStateTmp.flip;
                break;
            case CAMERA_PXI_COMMAND_PHOTO_MODE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                {
                    cameraWork.CAMERAiStateIn.photo = cameraWork.CAMERAiStateTmp.photo;
                    switch (cameraWork.CAMERAiStateIn.photo)
                    {
                    case CAMERA_PHOTO_MODE_NORMAL:
                        cameraWork.CAMERAiStateIn.sharpness = 0;
                        cameraWork.CAMERAiStateIn.exposure = 0;
                        cameraWork.CAMERAiStateIn.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    case CAMERA_PHOTO_MODE_PORTRAIT:
                        cameraWork.CAMERAiStateIn.sharpness = -2;
                        cameraWork.CAMERAiStateIn.exposure = 0;
                        cameraWork.CAMERAiStateIn.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    case CAMERA_PHOTO_MODE_LANDSCAPE:
                        cameraWork.CAMERAiStateIn.sharpness = 1;
                        cameraWork.CAMERAiStateIn.exposure = 0;
                        cameraWork.CAMERAiStateIn.wb = CAMERA_WHITE_BALANCE_DAYLIGHT;
                        break;
                    case CAMERA_PHOTO_MODE_NIGHTVIEW:
                        cameraWork.CAMERAiStateIn.sharpness = -1;
                        cameraWork.CAMERAiStateIn.exposure = 2;
                        cameraWork.CAMERAiStateIn.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    case CAMERA_PHOTO_MODE_LETTER:
                        cameraWork.CAMERAiStateIn.sharpness = 2;
                        cameraWork.CAMERAiStateIn.exposure = 2;
                        cameraWork.CAMERAiStateIn.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    }
                }
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                {
                    cameraWork.CAMERAiStateOut.photo = cameraWork.CAMERAiStateTmp.photo;
                    switch (cameraWork.CAMERAiStateOut.photo)
                    {
                    case CAMERA_PHOTO_MODE_NORMAL:
                        cameraWork.CAMERAiStateOut.sharpness = 0;
                        cameraWork.CAMERAiStateOut.exposure = 0;
                        cameraWork.CAMERAiStateOut.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    case CAMERA_PHOTO_MODE_PORTRAIT:
                        cameraWork.CAMERAiStateOut.sharpness = -2;
                        cameraWork.CAMERAiStateOut.exposure = 0;
                        cameraWork.CAMERAiStateOut.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    case CAMERA_PHOTO_MODE_LANDSCAPE:
                        cameraWork.CAMERAiStateOut.sharpness = 1;
                        cameraWork.CAMERAiStateOut.exposure = 0;
                        cameraWork.CAMERAiStateOut.wb = CAMERA_WHITE_BALANCE_DAYLIGHT;
                        break;
                    case CAMERA_PHOTO_MODE_NIGHTVIEW:
                        cameraWork.CAMERAiStateOut.sharpness = -1;
                        cameraWork.CAMERAiStateOut.exposure = 2;
                        cameraWork.CAMERAiStateOut.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    case CAMERA_PHOTO_MODE_LETTER:
                        cameraWork.CAMERAiStateOut.sharpness = 2;
                        cameraWork.CAMERAiStateOut.exposure = 2;
                        cameraWork.CAMERAiStateOut.wb = CAMERA_WHITE_BALANCE_NORMAL;
                        break;
                    }
                }
                break;
            case CAMERA_PXI_COMMAND_WHITE_BALANCE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                {
                    cameraWork.CAMERAiStateIn.wb = cameraWork.CAMERAiStateTmp.wb;
                    cameraWork.CAMERAiStateIn.awb = TRUE;
                }
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                {
                    cameraWork.CAMERAiStateOut.wb = cameraWork.CAMERAiStateTmp.wb;
                    cameraWork.CAMERAiStateOut.awb = TRUE;
                }
                break;
            case CAMERA_PXI_COMMAND_EXPOSURE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    cameraWork.CAMERAiStateIn.exposure = cameraWork.CAMERAiStateTmp.exposure;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    cameraWork.CAMERAiStateOut.exposure = cameraWork.CAMERAiStateTmp.exposure;
                break;
            case CAMERA_PXI_COMMAND_SHARPNESS:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    cameraWork.CAMERAiStateIn.sharpness = cameraWork.CAMERAiStateTmp.sharpness;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    cameraWork.CAMERAiStateOut.sharpness = cameraWork.CAMERAiStateTmp.sharpness;
                break;
            case CAMERA_PXI_COMMAND_AUTO_EXPOSURE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    cameraWork.CAMERAiStateIn.ae = cameraWork.CAMERAiStateTmp.ae;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    cameraWork.CAMERAiStateOut.ae = cameraWork.CAMERAiStateTmp.ae;
                break;
            case CAMERA_PXI_COMMAND_AUTO_WHITE_BALANCE:
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_IN)
                    cameraWork.CAMERAiStateIn.awb = cameraWork.CAMERAiStateTmp.awb;
                if(cameraWork.CAMERAiStateTmp.set_camera & CAMERA_SELECT_OUT)
                    cameraWork.CAMERAiStateOut.awb = cameraWork.CAMERAiStateTmp.awb;
                break;
            case CAMERA_PXI_COMMAND_SET_LED:
                cameraWork.CAMERAiStateOut.blink = cameraWork.CAMERAiStateTmp.blink;
                break;
            default:
                break;
            }
            break;
        case CAMERA_PXI_RESULT_SUCCESS_FALSE:
            result = CAMERA_RESULT_SUCCESS_FALSE;
            break;
        case CAMERA_PXI_RESULT_ILLEGAL_STATUS:
            result = CAMERA_RESULT_ILLEGAL_STATUS;
            break;
        case CAMERA_PXI_RESULT_INVALID_COMMAND:
        case CAMERA_PXI_RESULT_INVALID_PARAMETER:
        case CAMERA_PXI_RESULT_BUSY:
        default:
            result = CAMERA_RESULT_FATAL_ERROR;
        }

        // コールバックの呼び出し
        CameraCallCallbackAndUnlock(result);

        // CAMERA_RESULT_ILLEGAL_STATUS だった場合は再起動ルーチンを起動する
        if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        {
//OS_TPrintf("Send message to reboot from PXI.\n");
            CAMERA_GoReboot();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraSyncCallback

  Description:  同期API用のコールバック

  Arguments:    result  - ARM7から送られた結果
                arg     - 未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraSyncCallback(CAMERAResult result, void *arg)
{
#pragma unused(arg)
    cameraWork.result = result;
}

/*---------------------------------------------------------------------------*
  Name:         CameraCallCallbackAndUnlock

  Description:  コールバックの呼び出しとロックの解除を行う

  Arguments:    result  - ARM7から送られた結果

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraCallCallbackAndUnlock(CAMERAResult result)
{
    CAMERACallback cb;

    if (cameraWork.lock)
    {
        cameraWork.lock = FALSE;
    }
    if (cameraWork.callback)
    {
        cb = cameraWork.callback;
        cameraWork.callback = NULL;
        cb(result, cameraWork.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraWaitBusy

  Description:  CAMERAの非同期処理がロックされている間待つ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if 0
#include    <nitro/code32.h>
static asm void CameraWaitBusy(void)
{
    ldr     r12,    =cameraWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #TRUE
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>
#else
extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void CameraWaitBusy(void)
{
    volatile BOOL *p = &cameraWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE || OS_GetIrq() == OS_IME_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}
#endif

/*---------------------------------------------------------------------------*
  Name:         CameraStandbyCallback

  Description:  CAMERAをスタンバイ状態にする

  Arguments:    args        未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraStandbyCallback(void* args)
{
#pragma unused(args)
    (void)CAMERA_I2CActivateCore(CAMERA_SELECT_NONE);
}
