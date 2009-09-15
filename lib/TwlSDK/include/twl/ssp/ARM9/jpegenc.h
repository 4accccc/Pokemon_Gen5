/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     jpegenc.h

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

#ifndef TWL_SSP_JPEGENC_H_
#define TWL_SSP_JPEGENC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>
#include <twl/ssp/ARM9/exifenc.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegEncoderBufferSize

  Description:  エンコードに必要なワークサイズを返す

  Arguments:    width    : 画像の横幅
                height   : 画像の縦幅
                sampling : サンプリング(1または2)
                option   : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。

  Returns:      必要なメモリサイズ
 *---------------------------------------------------------------------------*/
u32 SSP_GetJpegEncoderBufferSize(u32 width, u32 height, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoder

  Description:  JPEGエンコードする
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                src, dst, wrkには4バイトアラインメントが必要です。

  Arguments:    src    :     画像データ(左上から右下へのRGB555/YUV422データ)
                dst   :      エンコードされたJPEGデータを入れる場所
                limit :      dstの限界サイズ(これを超えるとエンコード失敗)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                quality :    エンコードのクオリティ(1-100)
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。

  Returns:      エンコードされたJPEGのサイズ(0の場合はエンコード失敗)
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoder(const void* src, u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderEx

  Description:  署名機能の有無を指定してJPEGエンコードする
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                src, dst, wrkには4バイトアラインメントが必要です。

  Arguments:    src    :     画像データ(左上から右下へのRGB555/YUV422データ)
                dst   :      エンコードされたJPEGデータを入れる場所
                limit :      dstの限界サイズ(これを超えるとエンコード失敗)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                quality :    エンコードのクオリティ(1-100)
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。
                sign :       署名付加を行う場合は TRUE を指定する。

  Returns:      エンコードされたJPEGのサイズ(0の場合はエンコード失敗)
 *---------------------------------------------------------------------------*/
static inline u32 SSP_StartJpegEncoderEx(const void* src, u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option, BOOL sign)
{
    u32 result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegEncoderSignMode(sign);
    result = SSP_StartJpegEncoder(src, dst, limit, wrk, width, height, quality, sampling, option);
    (void)SSP_SetJpegEncoderSignMode(old_sign);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_ConvertJpegEncodeData

  Description:  JPEGエンコードする形式に入力データをコンバートする。
                本関数の終了後に入力元データのクリアが可能になります。
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                src, dst, wrkには4バイトアラインメントが必要です。

  Arguments:    src    :     画像データ(左上から右下へのRGB555/YUV422データ)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。

  Returns:      成功ならばTRUE
 *---------------------------------------------------------------------------*/
BOOL SSP_ConvertJpegEncodeData(const void* src, u8 *wrk,
                         u32 width, u32 height, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderWithEncodeData

  Description:  事前にコンバートしたエンコード用データからJPEGエンコードする
                本関数の呼び出し前にSSP_ConvertJpegEncodeData関数を呼び出す必要があります。
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                dst, wrkには4バイトアラインメントが必要です。

  Arguments:    dst   :      エンコードされたJPEGデータを入れる場所
                limit :      dstの限界サイズ(これを超えるとエンコード失敗)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                quality :    エンコードのクオリティ(1-100)
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。

  Returns:      エンコードされたJPEGのサイズ(0の場合はエンコード失敗)
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoderWithEncodeData(u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderWithEncodeDataEx

  Description:  事前にコンバートしたエンコード用データから署名機能の有無を指定してJPEGエンコードする
                本関数の呼び出し前にSSP_ConvertJpegEncodeData関数を呼び出す必要があります。
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                dst, wrkには4バイトアラインメントが必要です。

  Arguments:    dst   :      エンコードされたJPEGデータを入れる場所
                limit :      dstの限界サイズ(これを超えるとエンコード失敗)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                quality :    エンコードのクオリティ(1-100)
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。
                sign :       署名付加を行う場合は TRUE を指定する。

  Returns:      エンコードされたJPEGのサイズ(0の場合はエンコード失敗)
 *---------------------------------------------------------------------------*/
static inline u32 SSP_StartJpegEncoderWithEncodeDataEx(u8 *dst, u32 limit, u8 *wrk,
                         u32 width, u32 height,
                         u32 quality, u32 sampling, u32 option, BOOL sign)
{
    u32 result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegEncoderSignMode(sign);
    result = SSP_StartJpegEncoderWithEncodeData(dst, limit, wrk, width, height, quality, sampling, option);
    (void)SSP_SetJpegEncoderSignMode(old_sign);
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SSP_GetJpegEncoderLiteBufferSize

  Description:  エンコードに必要なワークサイズを返す (省メモリ版)
                SSP_StartJpegEncoderLite用のワークサイズです。

  Arguments:    option   : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はRGB555形式とみなします。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。

  Returns:      必要なメモリサイズ
 *---------------------------------------------------------------------------*/
u32 SSP_GetJpegEncoderLiteBufferSize(u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderLite

  Description:  JPEGエンコードする (省メモリ版)
                ※SSP_StartJpegEncoderExとは無関係。
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                サンプリングが３の場合、widthは16の倍数,heightは８の倍数でないといけない
                src, dst, wrkには4バイトアラインメントが必要です。

  Arguments:    src    :     画像データ(左上から右下へのRGB555/YUV422データ)
                dst   :      エンコードされたJPEGデータを入れる場所
                limit :      dstの限界サイズ(これを超えるとエンコード失敗)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                quality :    エンコードのクオリティ(1-100)
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はエラーになります。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。

  Returns:      エンコードされたJPEGのサイズ(0の場合はエンコード失敗)
 *---------------------------------------------------------------------------*/
u32 SSP_StartJpegEncoderLite(const void* src, u8 *dst, u32 limit, u8 *wrk,
                             u32 width, u32 height,
                             u32 quality, u32 sampling, u32 option);

/*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegEncoderLiteEx

  Description:  JPEGエンコードする (省メモリ版)
                ※SSP_StartJpegEncoderExとは無関係。
                サンプリングが１の場合、width,heightは８の倍数
                サンプリングが２の場合、width,heightは16の倍数でないといけない
                サンプリングが３の場合、widthは16の倍数,heightは８の倍数でないといけない
                src, dst, wrkには4バイトアラインメントが必要です。

  Arguments:    src    :     画像データ(左上から右下へのRGB555/YUV422データ)
                dst   :      エンコードされたJPEGデータを入れる場所
                limit :      dstの限界サイズ(これを超えるとエンコード失敗)
                wrk :        ワークエリア
                width :      画像の横幅
                height :     画像の縦幅
                quality :    エンコードのクオリティ(1-100)
                sampling :   主画像のサンプリング(1=YUV444,2=YUV420,3=YUV422。サムネイルは内部的に3固定)
                option : オプション(ビット論理和)
                           SSP_JPEG_RGB555: RGB555形式からエンコード。
                           SSP_JPEG_YUV422: YUV422形式からエンコード。
                                  形式指定が無い場合はエラーになります。
                           SSP_JPEG_THUMBNAIL: サムネイル付きエンコード。
                sign :       署名付加を行う場合は TRUE を指定する。

  Returns:      エンコードされたJPEGのサイズ(0の場合はエンコード失敗)
 *---------------------------------------------------------------------------*/
static inline u32 SSP_StartJpegEncoderLiteEx(const void* src, u8 *dst, u32 limit, u8 *wrk,
                             u32 width, u32 height,
                             u32 quality, u32 sampling, u32 option, BOOL sign)
{
    u32 result;
    BOOL old_sign;
    
    old_sign = SSP_SetJpegEncoderSignMode(sign);
    result = SSP_StartJpegEncoderLite(src, dst, limit, wrk, width, height, quality, sampling, option);
    (void)SSP_SetJpegEncoderSignMode(old_sign);
    
    return result;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_JPEGENC_H_ */
#endif
