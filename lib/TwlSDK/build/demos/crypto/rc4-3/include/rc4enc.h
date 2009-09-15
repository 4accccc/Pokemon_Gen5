/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     rc4enc.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: rc4enc.h,v $
  Revision 1.3  2006/03/08 09:15:35  seiki_masashi
  コメントの修正

  Revision 1.2  2006/03/08 09:14:44  seiki_masashi
  コメントの修正

  Revision 1.1  2006/03/08 08:53:41  seiki_masashi
  rc4-3 デモの追加

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef RC4ENC_H_
#define RC4ENC_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/types.h>

#define RC4ENC_USER_KEY_LENGTH  12 // ユーザが指定する鍵長
#define RC4ENC_TOTAL_KEY_LENGTH 16 // 最終的な鍵長

// エンコード時に追加で必要になるサイズ (24バイト)
#define RC4ENC_ADDITIONAL_SIZE (sizeof(u32) /* iv */ + MATH_SHA1_DIGEST_SIZE /* message digest */)

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
typedef struct RC4EncoderContext
{
    CRYPTORC4Context rc4_context;
    u8 key[RC4ENC_TOTAL_KEY_LENGTH];
} RC4EncoderContext;

typedef struct RC4DecoderContext
{
    CRYPTORC4Context rc4_context;
    u8 key[RC4ENC_TOTAL_KEY_LENGTH];
} RC4DecoderContext;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         InitRC4Encoder

  Description:  RC4 アルゴリズムで暗号化を行うための初期化を行う。

  Arguments:    context - RC4 の鍵情報などを収めるコンテキスト構造体
                key     - 12バイトの鍵データ

  Returns:      なし
 *---------------------------------------------------------------------------*/
void InitRC4Encoder(
    RC4EncoderContext* context,
    const void* key
);

/*---------------------------------------------------------------------------*
  Name:         EncodeRC4

  Description:  RC4 アルゴリズムで暗号化を行う。

  Arguments:    context - RC4 の鍵情報などを収めたコンテキスト構造体
                in      - 入力データ
                in_len  - データ長
                out     - 出力データ
                out_len - 出力バッファとして確保している長さ

  Returns:      成功したら出力長, 失敗したら 0
 *---------------------------------------------------------------------------*/
u32 EncodeRC4(
    RC4EncoderContext* context,
    const void* in,
    u32 in_len,
    void* out,
    u32 out_len
);

/*---------------------------------------------------------------------------*
  Name:         InitRC4Decoder

  Description:  RC4 アルゴリズムで復号を行うための初期化を行う。

  Arguments:    context - RC4 の鍵情報などを収めるコンテキスト構造体
                key     - 12バイトの鍵データ

  Returns:      なし
 *---------------------------------------------------------------------------*/
void InitRC4Decoder(
    RC4DecoderContext* context,
    const void* key
);

/*---------------------------------------------------------------------------*
  Name:         DecodeRC4

  Description:  RC4 アルゴリズムで復号を行う。
                その際、データの改変チェックも行い、改変されていたら失敗する。

  Arguments:    context - RC4 の鍵情報などを収めたコンテキスト構造体
                in      - 入力データ
                in_len  - データ長
                out     - 出力データ
                out_len - 出力バッファとして確保している長さ

  Returns:      成功したら出力長, 失敗したら 0
 *---------------------------------------------------------------------------*/
u32 DecodeRC4(
    RC4DecoderContext* context,
    const void* in,
    u32 in_len,
    void* out,
    u32 out_len
);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif  /* RC4ENC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
