/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - libraries
  File:     dgt.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro/mi/memory.h>
#include <nitro/math/dgt.h>
#include "hmac.h"

#ifdef  SDK_WIN32
#include <string.h>
#define MI_CpuCopy8(_x_, _y_, _z_)  memcpy(_y_, _x_, _z_)
#define MI_CpuFill8                 memset
#endif

//
// SHA1 ダイジェスト計算
//


/*
 * 実行速度より省スタックを優先する場合はこのオプションを有効にすること.
 * 具体的には以下の箇所に影響を与える.
 *   - MATHi_SHA1ProcessBlock 関数が使用するローカル変数256バイト分の増減.
 */
#define MATH_SHA1_SMALL_STACK

/*
 * BSAFEのSHA1を試しに使いたい場合はこのオプションを有効にすること
 */
//#define MATH_SHA1_BSAFE_TEST

// 内部関数宣言
#if !defined(MATH_SHA1_ASM)
static void MATHi_SHA1ProcessBlock(MATHSHA1Context *context);
#else
extern void MATHi_SHA1ProcessBlock(MATHSHA1Context *context);
#endif
static void MATHi_SHA1ProcessBlockForOverlay(MATHSHA1Context *context);
static void MATHi_SHA1Fill(MATHSHA1Context* context, u8 input, u32 length);


// static 変数
static int MATHi_OverlayTableMode = 0;
static void (*MATHi_SHA1ProcessMessageBlockFunc)(MATHSHA1Context*) = MATHi_SHA1ProcessBlock;



// 内部関数定義

#if defined(PLATFORM_ENDIAN_LITTLE)
#define NETConvert32HToBE        NETSwapBytes32
#else
#define NETConvert32HToBE(val)   (val)
#endif

inline static u32 NETSwapBytes32( u32 val )
{
    return (u32)( (((val) >> 24UL) & 0x000000FFUL) | 
                  (((val) >>  8UL) & 0x0000FF00UL) | 
                  (((val) <<  8UL) & 0x00FF0000UL) | 
                  (((val) << 24UL) & 0xFF000000UL) );
}

inline static u32 NETRotateLeft32(int shift, u32 value)
{
    return (u32)((value << shift) | (value >> (u32)(32 - shift)));
}

#if !defined(MATH_SHA1_ASM)
/*---------------------------------------------------------------------------*
  Name:         MATHi_SHA1ProcessBlock

  Description:  SHA-1コンテキストに満了した1ブロックを使用してハッシュ計算する.
  
  Arguments:    context MATHSHA1Context 構造体
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MATHi_SHA1ProcessBlock(MATHSHA1Context *context)
{
    u32     a = context->h[0];
    u32     b = context->h[1];
    u32     c = context->h[2];
    u32     d = context->h[3];
    u32     e = context->h[4];
#if defined(MATH_SHA1_SMALL_STACK)
    u32     w[16];
#define w_alias(t)  w[(t) & 15]
#define w_update(t)                         \
        if (t >= 16)                        \
        {                                   \
            w_alias(t) = NETRotateLeft32(1, \
                w_alias(t - 16 +  0) ^      \
                w_alias(t - 16 +  2) ^      \
                w_alias(t - 16 +  8) ^      \
                w_alias(t - 16 + 13));      \
        }(void)0

#else
    u32     w[80];
#define w_alias(t)  w[t]
#define w_update(t) (void)0
#endif /* defined(MATH_SHA1_SMALL_STACK) */

    int     t;
	u32     tmp;
    for (t = 0; t < 16; ++t)
    {
        w[t] = NETConvert32HToBE(((u32*)context->block)[t]);
    }
#if !defined(MATH_SHA1_SMALL_STACK)
    for (; t < 80; ++t)
    {
        u32    *prev = &w[t - 16];
        w[t] = NETRotateLeft32(1, prev[ 0] ^ prev[ 2] ^ prev[ 8] ^ prev[13]);
    }
#endif /* !defined(MATH_SHA1_SMALL_STACK) */
    for (t = 0; t < 20; ++t)
    {
        tmp = 0x5A827999UL + ((b & c) | (~b & d));
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    for (; t < 40; ++t)
    {
        tmp = 0x6ED9EBA1UL + (b ^ c ^ d);
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    for (; t < 60; ++t)
    {
        tmp = 0x8F1BBCDCUL + ((b & c) | (b & d) | (c & d));
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    for (; t < 80; ++t)
    {
        tmp = 0xCA62C1D6UL + (b ^ c ^ d);
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    context->h[0] += a;
    context->h[1] += b;
    context->h[2] += c;
    context->h[3] += d;
    context->h[4] += e;
}
#endif /* !defined(MATH_SHA1_ASM) */


// OverlayTable で Hash 値を求める際には後から計算される部分を 0 としておく
// OverlayTable の各エントリのサイズは 64 の倍数なので、ここで処理しても OK
static void MATHi_SHA1ProcessBlockForOverlay(MATHSHA1Context *context)
{
    u32 s0, s1;
    u32 *block = (u32*)context->block;

    // Save and clear file_id in OverlayTable
    s0 = block[6];      // 6   = location of file_id
    s1 = block[6+8];    // 6+8 = location of next file_id
    block[6]   = 0;
    block[6+8] = 0;

    MATHi_SHA1ProcessBlock(context);

    // Restore file_id
    block[6]   = s0;
    block[6+8] = s1;
}

static void MATHi_SHA1Fill(MATHSHA1Context* context, u8 input, u32 length)
{
    while (length > 0)
    {
        /* ブロックの余白にデータを充填する */
        u32     rest = MATH_SHA1_BLOCK_SIZE - context->pool;
        if (rest > length)
        {
            rest = length;
        }
        MI_CpuFill8(&context->block[context->pool], input, rest);
        length -= rest;
        context->pool += rest;
        /* ブロックが満了したらハッシュ計算を実行 */
        if (context->pool >= MATH_SHA1_BLOCK_SIZE)
        {
            MATHi_SHA1ProcessMessageBlockFunc(context);
            context->pool = 0;
            ++context->blocks_low;
            if (!context->blocks_low)
            {
                ++context->blocks_high;
            }
        }
    }
}

int MATHi_SetOverlayTableMode( int flag )
{
    int prev = MATHi_OverlayTableMode;

    MATHi_OverlayTableMode = flag;

    if (MATHi_OverlayTableMode)
    {
        MATHi_SHA1ProcessMessageBlockFunc = MATHi_SHA1ProcessBlockForOverlay;
    }
    else
    {
        MATHi_SHA1ProcessMessageBlockFunc = MATHi_SHA1ProcessBlock;
    }

    return prev;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Init

  Description:  SHA1 値を求めるために使う MATHSHA1Context 構造体を初期化する。
  
  Arguments:    context MATHSHA1Context 構造体
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Init(MATHSHA1Context* context)
{
    context->blocks_low = 0;
    context->blocks_high = 0;
    context->pool = 0;
    context->h[0] = 0x67452301;
    context->h[1] = 0xEFCDAB89;
    context->h[2] = 0x98BADCFE;
    context->h[3] = 0x10325476;
    context->h[4] = 0xC3D2E1F0;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Update

  Description:  SHA1 値を与えたデータで更新する。
  
  Arguments:    context MATHSHA1Context 構造体
                input   入力データへのポインタ
                length  入力データ長
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Update(MATHSHA1Context* context, const void* input, u32 length)
{
    while (length > 0)
    {
        /* ブロックの余白にデータを充填する */
        u32     rest = MATH_SHA1_BLOCK_SIZE - context->pool;
        if (rest > length)
        {
            rest = length;
        }
        MI_CpuCopy8(input, &context->block[context->pool], rest);
        input = (const u8 *)input + rest;
        length -= rest;
        context->pool += rest;
        /* ブロックが満了したらハッシュ計算を実行 */
        if (context->pool >= MATH_SHA1_BLOCK_SIZE)
        {
            MATHi_SHA1ProcessMessageBlockFunc(context);
            context->pool = 0;
            ++context->blocks_low;
            if (!context->blocks_low)
            {
                ++context->blocks_high;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1GetHash

  Description:  最終的な SHA1 値を得る。
  
  Arguments:    context MATHSHA1Context 構造体
                digest  SHA1 値を格納する場所へのポインタ
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1GetHash(MATHSHA1Context *context, void *digest)
{
    u32    footer[2];
    static const u8 padlead[1] = { 0x80 };
    static const u8 padalign[sizeof(footer)] = { 0x00, };
    /* setup footer field (in bits, big endian) */
    footer[1] = NETConvert32HToBE((u32)
        (context->blocks_low << (6 + 3)) + (context->pool << (0 + 3)));
    footer[0] = NETConvert32HToBE((u32)
        (context->blocks_high << (6 + 3)) + (context->blocks_low >> (u32)(32 - (6 + 3))));
    /* add leading padbyte '0x80' */
    MATH_SHA1Update(context, padlead, sizeof(padlead));
    /* if necessary, add 2 padblocks */
    if (MATH_SHA1_BLOCK_SIZE - context->pool < sizeof(footer))
    {
        MATH_SHA1Update(context, padalign, MATH_SHA1_BLOCK_SIZE - context->pool);
    }
    /* add trailing padbytes '0x00' */
    MATHi_SHA1Fill(context, 0x00, MATH_SHA1_BLOCK_SIZE - context->pool - sizeof(footer));
    /* add footer length */
    MATH_SHA1Update(context, footer, sizeof(footer));
    /* copy registers to the dst */
    context->h[0] = NETConvert32HToBE((u32)context->h[0]);
    context->h[1] = NETConvert32HToBE((u32)context->h[1]);
    context->h[2] = NETConvert32HToBE((u32)context->h[2]);
    context->h[3] = NETConvert32HToBE((u32)context->h[3]);
    context->h[4] = NETConvert32HToBE((u32)context->h[4]);
    MI_CpuCopy8(context->h, digest, sizeof(context->h));
}

#if defined(MATH_SHA1_BSAFE_TEST)
extern unsigned char *SHA1(const unsigned char *d, unsigned long n, unsigned char *md);
#endif
/*---------------------------------------------------------------------------*
  Name:         MATH_CalcSHA1

  Description:  SHA-1 を計算する。
  
  Arguments:    digest  SHA-1 値を格納する場所へのポインタ
                data    入力データのポインタ
                dataLength  入力データ長
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_CalcSHA1(void *digest, const void *data, u32 dataLength)
{
#if !defined(MATH_SHA1_BSAFE_TEST)
    MATHSHA1Context context;
    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, data, dataLength);
    MATH_SHA1GetHash(&context, digest);
#else
	SHA1((unsigned char*)data, dataLength, (unsigned char*)digest);
#endif
}



// HMAC

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcHMACSHA1

  Description:  HMAC-SHA-1 を計算する。
  
  Arguments:    digest  HMAC-SHA-1 値を格納する場所へのポインタ
                data    入力データのポインタ
                dataLength  入力データ長
                key     鍵のポインタ
                keyLength   鍵の長さ
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_CalcHMACSHA1(void *digest, const void *bin_ptr, u32 bin_len, const void *key_ptr, u32 key_len) 
{
    MATHSHA1Context context;
    unsigned char   hash_buf[ MATH_SHA1_DIGEST_SIZE ]; /* ハッシュ関数から得るハッシュ値 */
    
    MATHiHMACFuncs hash2funcs = {
        MATH_SHA1_DIGEST_SIZE,
        (512/8),
    };
    
    hash2funcs.context       = &context;
    hash2funcs.hash_buf      = hash_buf;
    hash2funcs.HashReset     = (void (*)(void*))                   MATH_SHA1Init;
    hash2funcs.HashSetSource = (void (*)(void*, const void*, u32)) MATH_SHA1Update;
    hash2funcs.HashGetDigest = (void (*)(void*, void*))            MATH_SHA1GetHash;
    
    MATHi_CalcHMAC(digest, bin_ptr, bin_len, key_ptr, key_len, &hash2funcs);
}

