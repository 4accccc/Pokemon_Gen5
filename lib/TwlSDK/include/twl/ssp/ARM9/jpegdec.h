/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     jpegdec.h

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

#ifndef TWL_SSP_JPEGDEC_H_
#define TWL_SSP_JPEGDEC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>
#include <twl/ssp/ARM9/exifdec.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_CheckJpegDecoderSign

  Description:  JPEGファイルの署名チェックのみを行う

  Arguments:    data   : JPEGファイル
                size   : JPEGファイルのサイズ

  Returns:      成功ならばTRUEを返す
 *---------------------------------------------------------------------------*/
BOOL SSP_CheckJpegDecoderSign(u8* data, u32 size);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoder

  Description:  JPEGファイルをデコードする

  Arguments:    data   : JPEGファイル
                size   : JPEGファイルのサイズ
                dst    : デコード先バッファ (今はRGB555のみ)
                width  : 引数として画像横サイズの最大値を与えます。
                         デコード成功時にはデコードした画像の横サイズが返ります。
                height : 引数として画像縦サイズの最大値を与えます。
                         デコード成功時にはデコードした画像の縦サイズが返ります。
                option : オプション(ビット論理和)
                         SSP_JPEG_RGB555: RGB555形式でデコード。
                                今は指定しなくてもRGB555形式でデコードします。
                         SSP_JPEG_THUMBNAIL: サムネイルのデコード。無指定時は主画像のデコード。
                         SSP_JPEG_EXIF: EXIF情報のみ取得する

  Returns:      成功ならばTRUEを返す
 *---------------------------------------------------------------------------*/
BOOL SSP_StartJpegDecoder(u8* data, u32 size, void* dst, s16* width, s16* height, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoderEx

  Description:  署名機能の有無を指定してJPEGファイルをデコードする

  Arguments:    data   : JPEGファイル
                size   : JPEGファイルのサイズ
                dst    : デコード先バッファ (今はRGB555のみ)
                width  : 引数として画像横サイズの最大値を与えます。
                         デコード成功時にはデコードした画像の横サイズが返ります。
                height : 引数として画像縦サイズの最大値を与えます。
                         デコード成功時にはデコードした画像の縦サイズが返ります。
                option : オプション(ビット論理和)
                         SSP_JPEG_RGB555: RGB555形式でデコード。
                                今は指定しなくてもRGB555形式でデコードします。
                         SSP_JPEG_THUMBNAIL: サムネイルのデコード。無指定時は主画像のデコード。
                         SSP_JPEG_EXIF: EXIF情報のみ取得する
                sign   : 署名チェックを行う場合は TRUE を指定する。

  Returns:      成功ならばTRUEを返す
 *---------------------------------------------------------------------------*/
static inline BOOL SSP_StartJpegDecoderEx(u8* data, u32 size, void* dst, s16* width, s16* height, u32 option, BOOL sign)
{
    BOOL result;
    
    (void)SSP_SetJpegDecoderSignMode(sign);
    result = SSP_StartJpegDecoder(data, size, dst, width, height, option);
    (void)SSP_SetJpegDecoderSignMode(FALSE);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_ExtractJpegDecoderExif

  Description:  JPEGファイルからExif情報だけを抽出する
                実際に抽出された値は各種 Get 関数で取得すること

  Arguments:    data   : JPEGファイル
                size   : JPEGファイルのサイズ
                
  Returns:      成功ならばTRUEを返す
 *---------------------------------------------------------------------------*/
static inline BOOL SSP_ExtractJpegDecoderExif(u8* data, u32 size)
{
    return SSP_StartJpegDecoder(data, size, 0, 0, 0, SSP_JPEG_EXIF);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_JPEGDEC_H_ */
#endif
