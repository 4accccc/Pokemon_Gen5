/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/prng.h

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CRYPTO_PRNG_H_
#define NITRO_CRYPTO_PRNG_H_

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    定値構造体宣言
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_PRNG_GatherEntropy

  Description:  乱数生成のためのエントロピー作成

  Arguments:    なし

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_PRNG_GatherEntropy( void );

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_PRNG_GenerateRandom

  Description:  乱数を生成する

  Arguments:    randomBytes:  乱数出力バッファ
                size:         出力サイズ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_PRNG_GenerateRandom( u8 *randomBytes, u32 size );


/* for internal use */



#ifdef __cplusplus
}
#endif

#endif //NITRO_CRYPTO_PRNG_H_
