/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     prng.c

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

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/
static u8 RandomPool[MATH_SHA1_DIGEST_SIZE];
static u32 RandomCount = 0;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         AddRandomSeed

  Description:  PRNG に乱数の種を追加する

  Arguments:    random_seed - 追加する乱数の種へのポインタ
                len - 追加する乱数の種の長さ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AddRandomSeed(u8* random_seed, u32 len)
{
    MATHSHA1Context context;
    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, RandomPool, sizeof(RandomPool));
    MATH_SHA1Update(&context, random_seed, len);
    MATH_SHA1GetHash(&context, RandomPool);
}

/*---------------------------------------------------------------------------*
  Name:         ChurnRandomPool

  Description:  PRNG に現在のシステム状態を乱数の種として追加する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChurnRandomPool(void)
{
    u32 data[OS_LOW_ENTROPY_DATA_SIZE/sizeof(u32)];
    OS_GetLowEntropyData(data);
    AddRandomSeed((u8*)data, sizeof(data));
}

/*---------------------------------------------------------------------------*
  Name:         GetRandomBytes

  Description:  PRNG から乱数を入手する

  Arguments:    buffer - 乱数を格納するバッファへのポインタ
                len - 取得したい乱数のバイト数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GetRandomBytes(u8* buffer, u32 len)
{
    MATHSHA1Context context;
    u8 tmp[MATH_SHA1_DIGEST_SIZE];
    u32 n;
    
    while (len > 0)
    {
        MATH_SHA1Init(&context);
        MATH_SHA1Update(&context, RandomPool, sizeof(RandomPool));
        MATH_SHA1Update(&context, &RandomCount, sizeof(RandomCount));
        MATH_SHA1GetHash(&context, tmp);
        RandomCount++;
        
        n = (len < MATH_SHA1_DIGEST_SIZE) ? len : MATH_SHA1_DIGEST_SIZE;
        MI_CpuCopy8(tmp, buffer, n);
        buffer += n;
        len -= n;
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
