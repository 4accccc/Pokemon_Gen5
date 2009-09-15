/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera.c

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
#define SYNC_TYPE   (0 << REG_CAM_MCNT_SYNC_SHIFT)  // 1 if low active
#define RCLK_TYPE   (0 << REG_CAM_MCNT_IRCLK_SHIFT) // 1 if negative edge

#define RESET_ON    ( SYNC_TYPE | RCLK_TYPE )
#define RESET_OFF   ( REG_CAM_MCNT_VIO_MASK | REG_CAM_MCNT_RST_MASK | SYNC_TYPE | RCLK_TYPE ) // RST is only for TS-X2

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static inline void CAMERAi_Wait(u32 clocks)
{
    OS_SpinWaitSysCycles(clocks << 1);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Reset

  Description:  hardware reset before I2C access

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_ResetCore( void )
{
    reg_SCFG_CLK |= REG_SCFG_CLK_CAMHCLK_MASK;  // 確実に電源をオンにする

    reg_CAM_MCNT = RESET_ON;                    // ハードウェアリセット
    CAMERAi_Wait( 15 );
    reg_SCFG_CLK |= REG_SCFG_CLK_CAMCKI_MASK;   // 内部ROMコードを読み出すためカメラモジュールにクロック供給
    CAMERAi_Wait( 15 );
    reg_CAM_MCNT = RESET_OFF;                   // リセット状態の解除
    CAMERAi_Wait( 4100 );                       // 内部ROMコードの読み出しを待つ

    reg_SCFG_CLK &= ~REG_SCFG_CLK_CAMCKI_MASK;  // CAM_CKIをディセーブルにする
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_IsBusy

  Description:  whether camera is busy

  Arguments:    None

  Returns:      TRUE if camera is busy
 *---------------------------------------------------------------------------*/
BOOL CAMERA_IsBusyCore( void )
{
    return (reg_CAM_CNT & REG_CAM_CNT_E_MASK) >> REG_CAM_CNT_E_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_StartCapture

  Description:  start to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_StartCaptureCore( void )
{
    OSIntrMode old = OS_DisableInterrupts();
    reg_CAM_CNT |= REG_CAM_CNT_E_MASK;
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_StopCapture

  Description:  stop to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_StopCaptureCore( void )
{
    OSIntrMode old = OS_DisableInterrupts();
    reg_CAM_CNT &= ~REG_CAM_CNT_E_MASK;
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimmingParamsCenter

  Description:  set camera trimming parameters by centering
                NOTE: should call CAMERA_SetTrimming to enable trimming

  Arguments:    destWidth       width of image to output
                destHeight      height of image to output
                srcWidth        original width of image
                srcHeight       original height of image

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimmingParamsCenterCore(u16 destWidth, u16 destHeight, u16 srcWidth, u16 srcHeight)
{
    if( (destWidth > srcWidth)||(destHeight > srcHeight) ) //トリミング範囲が元画像から溢れている
    {
        return;
    }

    destWidth -= 2;
    destHeight -= 1;
    reg_CAM_SOFS = REG_CAM_SOFS_FIELD( (srcHeight-destHeight) >> 1, (srcWidth-destWidth) >> 1);
    reg_CAM_EOFS = REG_CAM_EOFS_FIELD( (srcHeight+destHeight) >> 1, (srcWidth+destWidth) >> 1);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimmingParams

  Description:  set camera trimming parameters
                NOTE: width = x2 - x1;  height = y2 - y1;
                NOTE: should call CAMERA_SetTrimming to enable trimming

  Arguments:    x1      X of top-left trimming point (multiple of 2)
                y1      Y of top-left trimming point
                x2      X of bottom-right trimming point (multiple of 2)
                y2      Y of bottom-right trimming point

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimmingParamsCore(u16 x1, u16 y1, u16 x2, u16 y2)
{
    if( (x1 > x2)||(y1 > y2) ) // 終了位置オフセットが開始位置オフセットよりも小さい
    {
        return;
    }
    // 終了位置が元画像の範囲外にある場合の挙動はリファレンスに記述してあります
    // 開始位置が元画像の範囲外にある場合の挙動はリファレンスに記述してあります

    reg_CAM_SOFS = REG_CAM_SOFS_FIELD( y1, x1 );
    reg_CAM_EOFS = REG_CAM_EOFS_FIELD( y2 - 1, x2 -2 );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetTrimmingParams

  Description:  Get camera trimming parameters
                NOTE: width = x2 - x1;  height = y2 - y1;
                NOTE: should call CAMERA_SetTrimming to enable trimming

  Arguments:    x1      X of top-left trimming point (multiple of 2)
                y1      Y of top-left trimming point
                x2      X of bottom-right trimming point (multiple of 2)
                y2      Y of bottom-right trimming point

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_GetTrimmingParamsCore(u16* x1, u16* y1, u16* x2, u16* y2)
{
    *x1 = (u16)((reg_CAM_SOFS & REG_CAM_SOFS_HOFS_MASK) >> REG_CAM_SOFS_HOFS_SHIFT);
    *y1 = (u16)((reg_CAM_SOFS & REG_CAM_SOFS_VOFS_MASK) >> REG_CAM_SOFS_VOFS_SHIFT);
    *x2 = (u16)(((reg_CAM_EOFS & REG_CAM_EOFS_HOFS_MASK) >> REG_CAM_EOFS_HOFS_SHIFT) + 2);
    *y2 = (u16)(((reg_CAM_EOFS & REG_CAM_EOFS_VOFS_MASK) >> REG_CAM_EOFS_VOFS_SHIFT) + 1);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimming

  Description:  set trimming to be enabled/disabled

  Arguments:    enabled     TRUE if set trimming will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimmingCore( BOOL enabled )
{
    OSIntrMode old = OS_DisableInterrupts();
    u16 value = reg_CAM_CNT;
    reg_CAM_CNT = (u16)(enabled ? (value | REG_CAM_CNT_T_MASK)
                                : (value & ~REG_CAM_CNT_T_MASK));
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_IsTrimming

  Description:  get trimming to be enabled/disabled

  Arguments:    None

  Returns:      TRUE if set trimming will be enabled
 *---------------------------------------------------------------------------*/
BOOL CAMERA_IsTrimmingCore( void )
{
    return ((reg_CAM_CNT & REG_CAM_CNT_T_MASK) >> REG_CAM_CNT_T_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetOutputFormat

  Description:  set CAMERA output format.

  Arguments:    output      one of CAMERAOutput to set.

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetOutputFormatCore( CAMERAOutput output )
{
    OSIntrMode old = OS_DisableInterrupts();
    u16 value = reg_CAM_CNT;
    switch (output)
    {
    case CAMERA_OUTPUT_YUV:
        reg_CAM_CNT = (u16)(value & ~REG_CAM_CNT_F_MASK);
        break;
    case CAMERA_OUTPUT_RGB:
        reg_CAM_CNT = (u16)(value | REG_CAM_CNT_F_MASK);
        break;
    }
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetOutputFormat

  Description:  get CAMERA output format.

  Arguments:    None

  Returns:      one of CAMERAOutput
 *---------------------------------------------------------------------------*/
CAMERAOutput CAMERA_GetOutputFormatCore( void )
{
    return (reg_CAM_CNT & REG_CAM_CNT_F_MASK) ? CAMERA_OUTPUT_RGB : CAMERA_OUTPUT_YUV;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetErrorStatus

  Description:  whether line buffer has occurred some errors or not

  Arguments:    None

  Returns:      TRUE if error has occurred
 *---------------------------------------------------------------------------*/
BOOL CAMERA_GetErrorStatusCore( void )
{
    return (reg_CAM_CNT & REG_CAM_CNT_ERR_MASK) >> REG_CAM_CNT_ERR_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearBuffer

  Description:  clear line buffer and error status

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_ClearBufferCore( void )
{
    OSIntrMode old = OS_DisableInterrupts();
    reg_CAM_CNT |= REG_CAM_CNT_CL_MASK;
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetMasterInterrupt

  Description:  set interrupt mode

  Arguments:    enabled     TRUE if set master interrupt will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetMasterInterruptCore( BOOL enabled )
{
    OSIntrMode old = OS_DisableInterrupts();
    u16 value = reg_CAM_CNT;
    reg_CAM_CNT = (u16)(enabled ? (value | REG_CAM_CNT_IREQI_MASK)
                                : (value & ~REG_CAM_CNT_IREQI_MASK));
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetMasterInterrupt

  Description:  get interrupt mode

  Arguments:    None

  Returns:      TRUE if set master interrupt will be enabled
 *---------------------------------------------------------------------------*/
BOOL CAMERA_GetMasterInterruptCore( void )
{
    return ((reg_CAM_CNT & REG_CAM_CNT_IREQI_MASK) >> REG_CAM_CNT_IREQI_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetVsyncInterrupt

  Description:  set vsync interrupt mode

  Arguments:    type        one of CAMERAIntrVsync to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetVsyncInterruptCore( CAMERAIntrVsync type )
{
    OSIntrMode old = OS_DisableInterrupts();
    reg_CAM_CNT = (u16)((reg_CAM_CNT & ~REG_CAM_CNT_IREQVS_MASK) | type);
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetVsyncInterrupt

  Description:  get vsync interrupt mode

  Arguments:    None

  Returns:      one of CAMERAIntrVsync
 *---------------------------------------------------------------------------*/
CAMERAIntrVsync CAMERA_GetVsyncInterruptCore( void )
{
    return (CAMERAIntrVsync)(reg_CAM_CNT & REG_CAM_CNT_IREQVS_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetBufferErrorInterrupt

  Description:  set buffer error interrupt mode

  Arguments:    enabled     TRUE if set buffer error interrupt will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetBufferErrorInterruptCore( BOOL enabled )
{
    OSIntrMode old = OS_DisableInterrupts();
    u16 value = reg_CAM_CNT;
    reg_CAM_CNT = (u16)(enabled ? (value | REG_CAM_CNT_IREQBE_MASK)
                                : (value & ~REG_CAM_CNT_IREQBE_MASK));
    (void)OS_RestoreInterrupts(old);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetBufferErrorInterrupt

  Description:  get buffer error interrupt mode

  Arguments:    None

  Returns:      TRUE if set buffer error interrupt will be enabled
 *---------------------------------------------------------------------------*/
BOOL CAMERA_GetBufferErrorInterruptCore( void )
{
    return ((reg_CAM_CNT & REG_CAM_CNT_IREQBE_MASK) >> REG_CAM_CNT_IREQBE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTransferLines

  Description:  set number of lines to store the buffer at once.

  Arguments:    lines   number of lines

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTransferLinesCore( int lines )
{
    if (lines >= 1 && lines <= 16)
    {
        OSIntrMode old = OS_DisableInterrupts();
        u16 bits = (u16)((lines - 1) << REG_CAM_CNT_TL_SHIFT);
        reg_CAM_CNT = (u16)((reg_CAM_CNT & ~REG_CAM_CNT_TL_MASK) | bits);
    (void)OS_RestoreInterrupts(old);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetTransferLines

  Description:  get number of lines to load the buffer at once.

  Arguments:    None

  Returns:      number of lines
 *---------------------------------------------------------------------------*/
int CAMERA_GetTransferLinesCore( void )
{
    return ( ((reg_CAM_CNT & REG_CAM_CNT_TL_MASK) >> REG_CAM_CNT_TL_SHIFT) + 1);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetMaxLinesRound

  Description:  CAMERA_GET_MAX_LINES の値を CAMERA_SetTransferLines に適した値に丸める

  Arguments:    width    width of image
                height   height of image

  Returns:      max lines
 *---------------------------------------------------------------------------*/
int CAMERA_GetMaxLinesRoundCore(u16 width, u16 height)
{
    int lines;

    for(lines = CAMERA_GET_MAX_LINES(width) ;lines > 1; lines--)
    {
        if( height % lines == 0 )
        {
            return lines;
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetBytesAtOnce

  Description:  カメラバッファからフレームデータを受信する際の一回の転送サイズを求める

  Arguments:    width    width of image

  Returns:      一回の転送サイズ
 *---------------------------------------------------------------------------*/
u32 CAMERA_GetBytesAtOnceCore(u16 width)
{
    return (u32)( CAMERA_GET_LINE_BYTES(width) * CAMERA_GetTransferLinesCore() );
}
