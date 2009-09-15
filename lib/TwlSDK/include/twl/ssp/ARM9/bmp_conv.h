/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     bmp_conv.h

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-01-09#$
  $Rev: 9810 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SSP_BMP_H_
#define TWL_SSP_BMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

typedef enum
{
    SSP_BMP_YUV422 = 1,
    SSP_BMP_RGB555
}
SSPConvertBmpType;

/*---------------------------------------------------------------------------*
  Name:         SSP_YUV422ToRGB888b

  Description:  YUV422 形式のデータを RGB888(Bottom up) 形式のデータにコンバートする

  Arguments:    src    : 入力データ(YUV422)
                dst    : 出力データ(RGB888)
                width  : 画像の幅
                height : 画像の高さ

  Returns:      サイズを返す
 *---------------------------------------------------------------------------*/
u32 SSP_YUV422ToRGB888b( const void* src, void* dst, u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_RGB555ToRGB888b

  Description:  RGB555 形式のデータを RGB888(Bottom up) 形式のデータにコンバートする

  Arguments:    src    : 入力データ(YUV422)
                dst    : 出力データ(RGB888)
                width  : 画像の幅
                height : 画像の高さ

  Returns:      サイズを返す
 *---------------------------------------------------------------------------*/
u32 SSP_RGB555ToRGB888b( const void* src, void* dst, u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_GetRGB888Size

  Description:  RGB888 形式のデータのサイズを取得する

  Arguments:    width  : 画像の幅
                height : 画像の高さ

  Returns:      サイズを返す
 *---------------------------------------------------------------------------*/
u32 SSP_GetRGB888Size( u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_CreateBmpHeader

  Description:  BMP ファイルのヘッダを作成する

  Arguments:    dst    : 出力データ
                width  : 画像の幅
                height : 画像の高さ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SSP_CreateBmpHeader( u8 *dst, u16 width, u16 height );

/*---------------------------------------------------------------------------*
  Name:         SSP_GetBmpHeaderSize

  Description:  BMP ファイルのヘッダのサイズを取得する

  Arguments:    None.

  Returns:      サイズを返す
 *---------------------------------------------------------------------------*/
u32 SSP_GetBmpHeaderSize();

/*---------------------------------------------------------------------------*
  Name:         SSP_GetBmpFileSize

  Description:  BMP ファイルのサイズを取得する

  Arguments:    width  : 画像の幅
                height : 画像の高さ

  Returns:      サイズを返す
 *---------------------------------------------------------------------------*/
u32 SSP_GetBmpFileSize(u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         SSP_CreateBmpFile

  Description:  BMP ファイルを作成する

  Arguments:    src    : 入力データ(YUV422 or RGB555)
                dst    : BMPファイルを格納するバッファ
                width  : 画像の幅
                height : 画像の高さ
                type   : 入力データの種類

  Returns:      作成したBMPファイルのサイズを返す
 *---------------------------------------------------------------------------*/
u32 SSP_CreateBmpFile(const void* src, void* dst, u16 width, u16 height, SSPConvertBmpType type);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_BMP_H_ */
#endif
