/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - include
  File:     math/dgt.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MATH_DGT_H_
#define NITRO_MATH_DGT_H_

#ifndef SDK_WIN32
#include <nitro/misc.h>
#endif
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Constant definition
//----------------------------------------------------------------------------

// ダイジェスト長
#define MATH_MD5_DIGEST_SIZE    (128/8)     // 128bit
#define MATH_SHA1_DIGEST_SIZE   (160/8)     // 160bit
#define MATH_SHA256_DIGEST_SIZE 32          // 256bit

// 最大のダイジェスト長
#define MATH_HASH_DIGEST_SIZE_MAX  MATH_SHA1_DIGEST_SIZE

// 処理ブロック長
#define MATH_HASH_BLOCK_SIZE    (512/8)
#define MATH_MD5_BLOCK_SIZE     MATH_HASH_BLOCK_SIZE     // 512bit
#define MATH_SHA1_BLOCK_SIZE    MATH_HASH_BLOCK_SIZE     // 512bit

//----------------------------------------------------------------------------
// Type definition
//----------------------------------------------------------------------------

typedef struct MATHMD5Context
{
    union
    {
        struct
        {
            unsigned long a, b, c, d;
        };
        unsigned long state[4];
    };
    unsigned long long length;
    union
    {
        unsigned long buffer32[16];
        unsigned char buffer8[64];
    };
} MATHMD5Context;

typedef struct MATHSHA1Context
{
    u32     h[5];                        /* H0,H1,H2,H3,H4 */
    u8      block[MATH_SHA1_BLOCK_SIZE]; /* current message block */
    u32     pool;                        /* message length in 'block' */
    u32     blocks_low;                  /* total blocks (in bytes) */
    u32     blocks_high;
}
MATHSHA1Context;

//----------------------------------------------------------------------------
// Declaration of function
//----------------------------------------------------------------------------

/*****************************************************************************/
/* MD5                                                                       */
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5Init

  Description:  MD5 値を求めるために使う MATHMD5Context 構造体を初期化する。
  
  Arguments:    context MATHMD5Context 構造体
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_MD5Init(MATHMD5Context * context);

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5Update

  Description:  MD5 値を与えたデータで更新する。
  
  Arguments:    context MATHMD5Context 構造体
                input   入力データへのポインタ
                length  入力データ長
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_MD5Update(MATHMD5Context * context, const void *input, u32 length);

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5GetHash

  Description:  最終的な MD5 値を得る。
  
  Arguments:    context MATHMD5Context 構造体
                digest  MD5 値を格納する場所へのポインタ
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_MD5GetHash(MATHMD5Context * context, void *digest);

// 下位互換のため
static inline void MATH_MD5GetDigest(MATHMD5Context * context, void *digest)
{
    MATH_MD5GetHash(context, digest);
}


/*****************************************************************************/
/* SHA-1                                                                     */
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Init

  Description:  SHA1 値を求めるために使う MATHSHA1Context 構造体を初期化する。
  
  Arguments:    context MATHSHA1Context 構造体
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Init(MATHSHA1Context * context);

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Update

  Description:  SHA1 値を与えたデータで更新する。
  
  Arguments:    context MATHSHA1Context 構造体
                input   入力データへのポインタ
                length  入力データ長
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Update(MATHSHA1Context * context, const void *input, u32 length);

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1GetHash

  Description:  最終的な SHA1 値を得る。
  
  Arguments:    context MATHSHA1Context 構造体
                digest  SHA1 値を格納する場所へのポインタ
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1GetHash(MATHSHA1Context * context, void *digest);

// 下位互換のため
static inline void MATH_SHA1GetDigest(MATHSHA1Context * context, void *digest)
{
    MATH_SHA1GetHash(context, digest);
}


/*****************************************************************************/
/* SHA256                                                                     */
/*****************************************************************************/
#define MATHSHA256_CBLOCK	64
#define MATHSHA256_LBLOCK	16
#define MATHSHA256_BLOCK	16
#define MATHSHA256_LAST_BLOCK  56
#define MATHSHA256_LENGTH_BLOCK 8
#define MATHSHA256_DIGEST_LENGTH 32

typedef struct MATHSHA256Context MATHSHA256Context;
typedef void (MATHSHA256_BLOCK_FUNC)(MATHSHA256Context *c, u32 *W, int num); 

struct MATHSHA256Context
{
    u32 h[8];
    u32 Nl,Nh;
    u8 data[MATHSHA256_CBLOCK];
    int num;
};

void MATH_SHA256Init(MATHSHA256Context *c);
void MATH_SHA256Update(MATHSHA256Context *c, const void* data, u32 len);
void MATH_SHA256GetHash(MATHSHA256Context *c, void* digest);
void MATH_CalcSHA256(void* digest, const void* data, u32 dataLength);


/*****************************************************************************/
/* ユーティリティ関数                                                        */
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcMD5

  Description:  MD5 を計算する。
  
  Arguments:    digest  MD5 値を格納する場所へのポインタ
                data    入力データのポインタ
                dataLength  入力データ長
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_CalcMD5(void *digest, const void *data, u32 dataLength);

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcSHA1

  Description:  SHA-1 を計算する。
  
  Arguments:    digest  SHA-1 値を格納する場所へのポインタ
                data    入力データのポインタ
                dataLength  入力データ長
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_CalcSHA1(void *digest, const void *data, u32 dataLength);


/*---------------------------------------------------------------------------*
  Name:         MATH_CalcHMACMD5

  Description:  HMAC-MD5 を計算する。
  
  Arguments:    digest  HMAC-MD5 値を格納する場所へのポインタ
                data    入力データのポインタ
                dataLength  入力データ長
                key     鍵のポインタ
                keyLength   鍵の長さ
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_CalcHMACMD5(void *digest, const void *data, u32 dataLength, const void *key, u32 keyLength);

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
void
MATH_CalcHMACSHA1(void *digest, const void *data, u32 dataLength, const void *key, u32 keyLength);

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcHMACSHA256

  Description:  HMAC-SHA-256 を計算する。
  
  Arguments:    digest  HMAC-SHA-256 値を格納する場所へのポインタ
                data    入力データのポインタ
                dataLength  入力データ長
                key     鍵のポインタ
                keyLength   鍵の長さ
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_CalcHMACSHA256(void *digest, const void *data, u32 dataLength, const void *key, u32 keyLength);

int MATHi_SetOverlayTableMode( int flag );

#ifdef __cplusplus
}/* extern "C" */
#endif

/* NITRO_MATH_DGT_H_ */
#endif
