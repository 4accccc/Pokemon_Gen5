/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dsp - 
  File:     dsp_graphics.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-02#$
  $Rev: 8184 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#ifndef TWL_DSP_GRAPHICS_H_
#define TWL_DSP_GRAPHICS_H_

#include <twl/dsp/common/pipe.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* constants */
typedef DSPWord DSPGraphicsScalingMode;
#define DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR (DSPGraphicsScalingMode)0x0001
#define DSP_GRAPHICS_SCALING_MODE_BILINEAR   (DSPGraphicsScalingMode)0x0002
#define DSP_GRAPHICS_SCALING_MODE_BICUBIC    (DSPGraphicsScalingMode)0x0003

#define DSP_GRAPHICS_SCALING_MODE_NPARTSHRINK (DSPGraphicsScalingMode)0x000A

// 使用するレジスタ
#define DSP_GRAPHICS_COM_REGISTER 0    // 処理内容通知用レジスタ番号
#define DSP_GRAPHICS_REP_REGISTER 1    // 処理結果通知用レジスタ番号

/*---------------------------------------------------------------------------*/
/* DSP <-> ARM 間で使う構造体、列挙型 */

// セマフォレジスタでやりとりする値
typedef enum DspState
{
    DSP_STATE_FAIL        = 0x0000,
    DSP_STATE_SUCCESS     = 0x0001
//    DSP_STATE_END_ONEPROC = 0x0002
} DSPSTATE;

// ARM 側から指定される、DSPが実行する処理を表す値
typedef enum _GraphicsFuncID
{
    DSP_G_FUNCID_SCALING = 0x0001,
    DSP_G_FUNCID_YUV2RGB,
    
    DSP_FUNCID_NUM
} DSPGraphicsFuncID;

// YUV <-> RGB 変換に用いるデータ構造体
typedef struct _Yuv2RgbParam
{
    u32 size;
    u32 src;
    u32 dst;
} DSPYuv2RgbParam;

// Scaling に用いるデータ構造体
typedef struct _ScalingParam
{
    u32 src;
    u32 dst;
    u16 mode;
    u16 img_width;
    u16 img_height;
    u16 rate_w;
    u16 rate_h;
    u16 block_x;
    u16 block_y;
    u16 width;
    u16 height;
    u16 pad[1];    // _ScalingParam のサイズを 4の倍数に揃えるため
} DSPScalingParam;

typedef void (*DSP_GraphicsCallback)(void);  // 処理終了時に呼び出す、ユーザ指定のコールバック関数

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
    マクロ定義
*---------------------------------------------------------------------------*/
// DSP_Scaling* で出力されるサイズを計算するマクロ
#define DSP_CALC_SCALING_SIZE(value, ratio) ((u32)(value * (u32)(ratio * 1000) / 1000))

#define DSP_CALC_SCALING_SIZE_FX(value, ratio) ((u32)( value * (u32)(ratio * 1000 / 4096.0f) / 1000))

/*---------------------------------------------------------------------------*
    内部変数定義
*---------------------------------------------------------------------------*/
static DSP_GraphicsCallback callBackFunc;   // 非同期処理終了時に呼ばれるコールバック関数

static volatile BOOL isBusy;                         // DSP が何らかの処理を実行中ならば TRUE
static volatile BOOL isAsync;                        // DSP が何らかの非同期処理を実行中ならば TRUE

/*---------------------------------------------------------------------------*
    internal core functions placed on LTDMAIN.
*---------------------------------------------------------------------------*/
void DSPi_OpenStaticComponentGraphicsCore(FSFile *file);
BOOL DSPi_LoadGraphicsCore(FSFile *file, int slotB, int slotC);
void DSPi_UnloadGraphicsCore(void);
BOOL DSPi_ConvertYuvToRgbCore(const void* src, void* dst, u32 size, DSP_GraphicsCallback callback, BOOL async);
BOOL DSPi_ScalingCore(const void* src, void* dst, u16 img_width, u16 img_height, f32 rw, f32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async);
BOOL DSPi_ScalingFxCore(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rw, fx32 ry, DSPGraphicsScalingMode mode,
                      u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback, BOOL async);


/*---------------------------------------------------------------------------*
  Name:         DSP_OpenStaticComponentGraphics

  Description:  グラフィックスコンポーネント用のメモリファイルを開く。
                ファイルシステムに事前に用意しておく必要がなくなるが
                静的メモリとしてリンクされるためプログラムサイズが増加する。
  
  Arguments:    file : メモリファイルを開くFSFile構造体。
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DSP_OpenStaticComponentGraphics(FSFile *file)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_OpenStaticComponentGraphicsCore(file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadGraphics

  Description:  DSPのグラフィックスコンポーネントをロードする。

  Arguments:    file  : グラフィックスコンポーネントのファイル。
                slotB : コードメモリのために使用を許可されたWRAM-B。
                slotC : データメモリのために使用を許可されたWRAM-C。
  
  Returns:      グラフィックスコンポーネントが正しくDSPへロードされればTRUE。
 *---------------------------------------------------------------------------*/
static inline BOOL DSP_LoadGraphics(FSFile *file, int slotB, int slotC)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSPi_LoadGraphicsCore(file, slotB, slotC);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadGraphics

  Description:  DSPのグラフィックスコンポーネントを終了する。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
static inline void DSP_UnloadGraphics(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_UnloadGraphicsCore();
    }
}

//--------------------------------------------------------------------------------
//    YUV -> RGB convert
//
/*---------------------------------------------------------------------------*
  Name:         DSPi_YuvToRgbConvert[Async]

  Description:  YUV->RGB 変換を行います

  Arguments:    src       : 処理元データアドレス
                dest      : 処理後データ格納先アドレス
                size      : src データサイズ
     Async のみ[callback  : 変換終了後に実行されるコールバック関数のポインタ]

  Returns:      成功ならTRUE
 *---------------------------------------------------------------------------*/
static inline BOOL DSP_ConvertYuvToRgb(const void* src, void* dst, u32 size)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ConvertYuvToRgbCore(src, dst, size, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ConvertYuvToRgbAsync(const void* src, void* dst, u32 size, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ConvertYuvToRgbCore(src, dst, size, callback, TRUE);
    }
    return FALSE;
}

//--------------------------------------------------------------------------------
//    
//

//--------------------------------------------------------------------------------
//    Scaling
//

/*---------------------------------------------------------------------------*
  Name:         DSP_Scaling[Fx][Async][Ex]

  Description:  画像データの拡大（縮小）処理を実行
                *Async では、非同期で処理を実行することが可能。終了時には登録したコールバックが呼ばれる。
                *Ex では、画像の任意の領域を指定することが可能。

  Arguments:    src       : 処理対象の画像データへのポインタ（2 バイトアラインメント）
                dst       : 処理結果の格納先ポインタ（2 バイトアラインメント、処理後のデータサイズを確保していること）
                img_width : src の横サイズ
                img_height: src の縦サイズ
                rx        : 横方向の倍率(31～0.001 小数点第4位以下は切り捨て) : [ Fx版 も同様 ]
                ry        : 縦方向の倍率(31～0.001 小数点第4位以下は切り捨て) : [ Fx版 も同様 ]
                mode      : 補間方法 (nearest neighbor, bilinear, bicubic)
       Ex のみ[ x         : 処理対象領域の左上の x 座標  ]
       Ex のみ[ y         : 処理対象領域の左上の y 座標  ]
       Ex のみ[ width     : 処理対象領域の横サイズ       ]
       Ex のみ[ height    : 処理対象領域の縦サイズ       ]
    Async のみ[ callback  : 処理終了時のコールバック関数 ]

  Returns:      成功ならば TRUE を返す
 *---------------------------------------------------------------------------*/
static inline BOOL DSP_Scaling(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFx(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingAsync(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry,
                             DSPGraphicsScalingMode mode, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, callback, TRUE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFxAsync(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry,
                             DSPGraphicsScalingMode mode, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, 0, 0, img_width, img_height, callback, TRUE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingEx(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode,
                                u16 x, u16 y, u16 width, u16 height)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFxEx(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode,
                                u16 x, u16 y, u16 width, u16 height)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, NULL, FALSE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingAsyncEx(const void* src, void* dst, u16 img_width, u16 img_height, f32 rx, f32 ry, DSPGraphicsScalingMode mode,
                                u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, callback, TRUE);
    }
    return FALSE;
}

static inline BOOL DSP_ScalingFxAsyncEx(const void* src, void* dst, u16 img_width, u16 img_height, fx32 rx, fx32 ry, DSPGraphicsScalingMode mode,
                                  u16 x, u16 y, u16 width, u16 height, DSP_GraphicsCallback callback)
{
    if (OS_IsRunOnTwl() && !isBusy)
    {
        return DSPi_ScalingFxCore(src, dst, img_width, img_height, rx, ry, mode, x, y, width, height, callback, TRUE);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CalcScalingFactor[Fx]

  Description:  入力値と出力値から、DSP_Scaling に指定する倍率を逆算する関数。
                0.001 以下で切り捨てられてしまう値についての誤差修正は入れているが、
                倍率の制約上、完全にどんな出力サイズでも算出できる倍率を導き出せるかは未検証です。

  Arguments:    src_size : 入力サイズ
                dst_size : 出力サイズ

  Returns:      dst_size を導く倍率（f32 or fx32)を返す。
 *---------------------------------------------------------------------------*/
static inline f32 DSP_CalcScalingFactorF32(const u16 src_size, const u16 dst_size)
{
    // DSP_Scaling の引数となる倍率の入力制限 ～0.001 による誤差を修正するために 0.0009f を加える
    return (dst_size / (f32)src_size + 0.0009f);
}

static inline fx32 DSP_CalcScalingFactorFx32(const u16 src_size, const u16 dst_size)
{
    return FX_F32_TO_FX32(dst_size / (f32)src_size + 0.0009f);
}

/*===========================================================================*/

#endif    // SDK_TWL

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_GRAPHICS_H_ */
