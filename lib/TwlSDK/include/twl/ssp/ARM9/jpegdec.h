/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - include
  File:     jpegdec.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-06#$
  $Rev: 10865 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SSP_JPEGDEC_H_
#define TWL_SSP_JPEGDEC_H_

#include <twl/ssp/common/ssp_jpeg_type.h>
#include <twl/ssp/ARM9/exifdec.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>


// errorCodeはs8の範囲で定義してください。
// 「データを処理できませんでした。」の原因は、
// データ形式が不正かサポートされていない、あるいはデータが途中で終わっています。
typedef enum SSPJpegDecoderErrorCode
{
    SSP_JPEG_DECODER_OK = 0,       // 成功。
    SSP_JPEG_DECODER_ERROR_ARGUMENT = (-1),   // option以外の引数に誤りがあります。
    SSP_JPEG_DECODER_ERROR_WORK_ALIGN = (-2),   // pCtxが4バイトアラインメントされていません。
    SSP_JPEG_DECODER_ERROR_OPTION = (-3),   // option引数に誤りがあります。
    SSP_JPEG_DECODER_ERROR_SIGN = (-10),

    SSP_JPEG_DECODER_ERROR_WIDTH_HEIGHT = (-20),  // 幅か高さが、指定された最大値を超えています。pCtx->widthおよびpCtx->heightに実サイズが入っています。

    SSP_JPEG_DECODER_ERROR_EXIF_0 = (-30),   // データを処理できませんでした。
// サムネイルが存在しない画像に対してサムネイルのデコードを指示した場合にも、
// このエラーコードが返る可能性があります。

    SSP_JPEG_DECODER_ERROR_MARKER_COMBINATION = (-50),   // データを処理できませんでした。
// マーカーの組み合わせが正しくありません。
// 例えば、SOFマーカーに対応するSOSマーカーが見つからなかった場合、
// このエラーコードが返る可能性があります。

    SSP_JPEG_DECODER_ERROR_SOI = (-60),   // データ(マーカー)を処理できませんでした。
    SSP_JPEG_DECODER_ERROR_SOF = (-61),
    SSP_JPEG_DECODER_ERROR_SOF_BLOCK_ID = (-62),
    SSP_JPEG_DECODER_ERROR_DHT = (-63),
    SSP_JPEG_DECODER_ERROR_SOS = (-64),
    SSP_JPEG_DECODER_ERROR_DQT = (-65),
    SSP_JPEG_DECODER_ERROR_DRI = (-66),

    SSP_JPEG_DECODER_ERROR_UNDERRUN_0 = (-90),   // データを処理できませんでした。
    SSP_JPEG_DECODER_ERROR_UNDERRUN_1 = (-91),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_2 = (-92),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_3 = (-93),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_4 = (-94),
    SSP_JPEG_DECODER_ERROR_UNDERRUN_5 = (-95),

    SSP_JPEG_DECODER_ERROR_RANGE_0 = (-110),  // データを処理できませんでした。
    SSP_JPEG_DECODER_ERROR_RANGE_1 = (-111),
    SSP_JPEG_DECODER_ERROR_RANGE_2 = (-112),
    SSP_JPEG_DECODER_ERROR_RANGE_3 = (-113),
    SSP_JPEG_DECODER_ERROR_RANGE_4 = (-114),
    SSP_JPEG_DECODER_ERROR_RANGE_5 = (-115),

    SSP_JPEG_DECODER_ERROR_HLB_0 = (-120),  // データを処理できませんでした。

    SSP_JPEG_DECODER_ERROR_INTERNAL_CTX_SIZE = (-128)  // 内部エラー。通常は発生しません。
} SSPJpegDecoderErrorCode;

// Fast版には、「コンテキスト構造体」と呼ぶ作業用メモリが必要です。
// 関数呼び出し前後の初期化は不要です。関数終了後にはライブラリから使用されません。
// コンテキスト構造体はサイズが8KBytesと大きいので、
// ローカル変数のようにスタックから割り当てないでください。

// コンテキスト構造体。
typedef struct SSPJpegDecoderFastContext {
    u8*     pSrc;
    u32     inputDataSize;
    void*   pDst;
    u32     option;
    u16     maxWidth;       // 許容最大画像幅(65536未満)。SSP_StartJpegDecoderFast()の引数maxWidthがコピーされます。
    u16     maxHeight;      // 許容最大画像高さ(65536未満)。SSP_StartJpegDecoderFast()の引数maxHeightがコピーされます。
    u16     width;          // 実際の画像幅。
    u16     height;         // 実際の画像高さ。
    SSPJpegDecoderErrorCode errorCode; // 0ならエラー無し。
    u8      reserved[4];
    u32     work[0x7f8];
} SSPJpegDecoderFastContext;

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
    BOOL old_sign;
    
    old_sign = SSP_SetJpegDecoderSignMode(sign);
    result = SSP_StartJpegDecoder(data, size, dst, width, height, option);
    (void)SSP_SetJpegDecoderSignMode(old_sign);
    
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

 /*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoderFast

  Description:  JPEGファイルをデコードする (Fast版)

                画像サイズが8ピクセルあるいは16ピクセルの倍数(データ形式による)
                である場合、最も高速にデコードできます。

  Arguments:    pCtx   : コンテキスト構造体へのポインタ。
                         関数呼び出し前後の初期化は不要です。関数終了後にはライブラリから使用されません。
                         コンテキスト構造体はサイズが大きいので、ローカル変数のようにスタックから割り当てないでください。
                         現時点では複数のスレッドから同時に呼び出すことはできません。
                data   : JPEGファイル
                size   : JPEGファイルのサイズ
                dst    : デコード先バッファ (今はRGB555のみ)
                         出力形式にかかわらず、4バイトアラインメントが必要です。
                maxWidth  : 引数として画像横サイズの最大値を与えます。(65535まで)
                            デコード成功時にはpCtx->widthに、デコードした画像の横サイズが返ります。
                maxHeight : 引数として画像縦サイズの最大値を与えます。(65535まで)
                            デコード成功時にはpCtx->heightに、デコードした画像の縦サイズが返ります。
                option : オプション(ビット論理和)
                         SSP_JPEG_RGB555: RGB555形式でデコード。
                                今はRGB555のみですが、指定は省略できません。
                         SSP_JPEG_THUMBNAIL: サムネイルのデコード。無指定時は主画像のデコード。

  Returns:      成功ならばTRUEを返す
                失敗した場合、pCtx->errorCodeに詳細なエラーコードが格納されます。
 *---------------------------------------------------------------------------*/
BOOL SSP_StartJpegDecoderFast(SSPJpegDecoderFastContext* pCtx, u8* data, u32 size, void* dst, u32 maxWidth, u32 maxHeight, u32 option);

 /*---------------------------------------------------------------------------*
  Name:         SSP_StartJpegDecoderFastEx

  Description:  署名機能の有無を指定してJPEGファイルをデコードする (Fast版)

                画像サイズが8ピクセルあるいは16ピクセルの倍数(データ形式による)
                である場合、最も高速にデコードできます。

  Arguments:    pCtx   : コンテキスト構造体へのポインタ。
                         関数呼び出し前後の初期化は不要です。関数終了後にはライブラリから使用されません。
                         コンテキスト構造体はサイズが大きいので、ローカル変数のようにスタックから割り当てないでください。
                         現時点では複数のスレッドから同時に呼び出すことはできません。
                data   : JPEGファイル
                size   : JPEGファイルのサイズ
                dst    : デコード先バッファ (今はRGB555のみ)
                         出力形式にかかわらず、4バイトアラインメントが必要です。
                maxWidth  : 引数として画像横サイズの最大値を与えます。(65535まで)
                            デコード成功時にはpCtx->widthに、デコードした画像の横サイズが返ります。
                maxHeight : 引数として画像縦サイズの最大値を与えます。(65535まで)
                            デコード成功時にはpCtx->heightに、デコードした画像の縦サイズが返ります。
                option : オプション(ビット論理和)
                         SSP_JPEG_RGB555: RGB555形式でデコード。
                                今はRGB555のみですが、指定は省略できません。
                         SSP_JPEG_THUMBNAIL: サムネイルのデコード。無指定時は主画像のデコード。
                sign   : 署名チェックを行う場合は TRUE を指定する。

  Returns:      成功ならばTRUEを返す
                失敗した場合、pCtx->errorCodeに詳細なエラーコードが格納されます。
 *---------------------------------------------------------------------------*/
BOOL SSP_StartJpegDecoderFastEx(SSPJpegDecoderFastContext* pCtx, u8* data, u32 size, void* dst, u32 maxWidth, u32 maxHeight, u32 option, BOOL sign);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SSP_JPEGDEC_H_ */
#endif
