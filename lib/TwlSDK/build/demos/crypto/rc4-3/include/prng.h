/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     prng.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: prng.h,v $
  Revision 1.1  2006/03/08 08:53:41  seiki_masashi
  rc4-3 デモの追加

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef PRNG_H_
#define PRNG_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/types.h>

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
void AddRandomSeed(u8* random_seed, u32 len);

/*---------------------------------------------------------------------------*
  Name:         ChurnRandomPool

  Description:  PRNG に現在のシステム状態を乱数の種として追加する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChurnRandomPool(void);

/*---------------------------------------------------------------------------*
  Name:         GetRandomBytes

  Description:  PRNG から乱数を入手する

  Arguments:    buffer - 乱数を格納するバッファへのポインタ
                len - 取得したい乱数のバイト数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GetRandomBytes(u8* buffer, u32 len);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif  /* PRNG_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
