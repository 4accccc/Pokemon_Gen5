/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     rc4enc.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Pseudo Random Number Generator
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>

#include "prng.h"
#include "rc4enc.h"

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/

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
)
{
    MI_CpuClear8(context, sizeof(RC4EncoderContext));

    // 鍵の設定
    MI_CpuCopy8(key, context->key, RC4ENC_USER_KEY_LENGTH);
}

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
)
{
    u8 digest[MATH_SHA1_DIGEST_SIZE];
    u32 iv;
    u8* out_ptr = (u8*)out;

    if ((out_len < in_len)
        ||
        (out_len - in_len < RC4ENC_ADDITIONAL_SIZE))
    {
        // 出力バッファが足りない
        return 0;
    }
    
    // ランダムな IV (Initialization Vector) の作成
    GetRandomBytes((u8*)(&iv), sizeof(iv));

    // TODO: ネットワークバイトオーダー対応のための Endian 変換
    MI_CpuCopy8(&iv, out_ptr, sizeof(iv));
    out_ptr += sizeof(iv);

    // 指定された鍵と IV を結合して、今回使用する鍵を作成
    MI_CpuCopy8(&iv, &context->key[RC4ENC_USER_KEY_LENGTH], sizeof(iv));
    CRYPTO_RC4Init(&context->rc4_context, context->key, sizeof(context->key));

    // 入力データの暗号化
    CRYPTO_RC4Encrypt(&context->rc4_context, in, in_len, out_ptr);
    out_ptr += in_len;

    // 入力データのメッセージダイジェスト値の計算
    MATH_CalcSHA1(digest, in, in_len);

    // ダイジェスト値の暗号化
    CRYPTO_RC4Encrypt(&context->rc4_context, digest, MATH_SHA1_DIGEST_SIZE, out_ptr);
    out_ptr += sizeof(digest);

    return (u32)(out_ptr - out);
}

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
)
{
    MI_CpuClear8(context, sizeof(RC4DecoderContext));

    // 鍵の設定
    MI_CpuCopy8(key, context->key, RC4ENC_USER_KEY_LENGTH);
}

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
)
{
    u8 digest[MATH_SHA1_DIGEST_SIZE];
    u8 decrypted_digest[MATH_SHA1_DIGEST_SIZE];
    u32 iv;
    u8* in_ptr = (u8*)in;
    u32 data_len = in_len - RC4ENC_ADDITIONAL_SIZE;

    if ((in_len < RC4ENC_ADDITIONAL_SIZE)
        ||
        (out_len < data_len))
    {
        // 出力バッファが足りない
        return 0;
    }
    
    // IV の取得
    // TODO: ネットワークバイトオーダー対応のための Endian 変換
    MI_CpuCopy8(in_ptr, &iv, sizeof(iv));
    in_ptr += sizeof(iv);

    // 今回使用する鍵の作成
    MI_CpuCopy8(&iv, &context->key[RC4ENC_USER_KEY_LENGTH], sizeof(iv));
    CRYPTO_RC4Init(&context->rc4_context, context->key, sizeof(context->key));

    // 入力データの復号
    CRYPTO_RC4Decrypt(&context->rc4_context, in_ptr, data_len, out);
    in_ptr += data_len;

    // 復号データのメッセージダイジェスト値の計算
    MATH_CalcSHA1(digest, out, data_len);

    // ダイジェスト値の復号
    CRYPTO_RC4Decrypt(&context->rc4_context, in_ptr, MATH_SHA1_DIGEST_SIZE, decrypted_digest);
    in_ptr += data_len;

    // ダイジェスト値が正しい値になっているかを検証
    {
        int i;
        for (i = 0; i < MATH_SHA1_DIGEST_SIZE; i++)
        {
            if (digest[i] != decrypted_digest[i])
            {
                // 検証失敗
                return 0;
            }
        }
    }

    return data_len;
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
