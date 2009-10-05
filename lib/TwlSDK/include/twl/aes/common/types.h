/*---------------------------------------------------------------------------*
  Project:  TwlSDK - AES - include
  File:     types.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#ifndef	TWL_AES_COMMON_TYPE_H_
#define	TWL_AES_COMMON_TYPE_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<twl/misc.h>
#include	<twl/types.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define AES_BLOCK_SIZE          16  // 128 bit
#define AES_KEY_SIZE            16  // 128 bit
#define AES_NONCE_SIZE          12  //  96 bit
#define AES_MAC_MAX_SIZE        16  // 128 bit
#define AES_COUNTER_SIZE        16  // 128 bit

#define AES_ADATA_BLOCK_NUM_MAX 0xFFFF
#define AES_PDATA_BLOCK_NUM_MAX 0xFFFF

#define AES_ADATA_SIZE_MAX      (AES_BLOCK_SIZE * AES_ADATA_BLOCK_NUM_MAX)
#define AES_PDATA_SIZE_MAX      (AES_BLOCK_SIZE * AES_PDATA_BLOCK_NUM_MAX)


#define AESi_ASSERT_MAC_LENGTH(x)                               \
    SDK_TASSERTMSG( ((x) == AES_MAC_LENGTH_4)                    \
                || ((x) == AES_MAC_LENGTH_6)                    \
                || ((x) == AES_MAC_LENGTH_8)                    \
                || ((x) == AES_MAC_LENGTH_10)                   \
                || ((x) == AES_MAC_LENGTH_12)                   \
                || ((x) == AES_MAC_LENGTH_14)                   \
                || ((x) == AES_MAC_LENGTH_16),                  \
                "%s(=%d) is not valid AESMacLength.", #x, (x) )


// AES 処理の結果
typedef enum AESResult
{
    AES_RESULT_NONE,                    // 処理結果未取得
    AES_RESULT_SUCCESS,                 // 暗号化/復号化成功 または 認証成功
    AES_RESULT_VERIFICATION_FAILED,     // 認証失敗
    AES_RESULT_INVALID,                 // 引数がおかしい
    AES_RESULT_BUSY,                    // AES 処理中
    AES_RESULT_ON_DS,                   // DS 上で動作中のため使用不可
    AES_RESULT_UNKNOWN,                 // ライブラリ内部エラー
    AES_RESULT_MAX
}
AESResult;

// MAC のバイトサイズ
typedef enum AESMacLength
{
    AES_MAC_LENGTH_4  = 1,      // 4 byte
    AES_MAC_LENGTH_6  = 2,
    AES_MAC_LENGTH_8  = 3,
    AES_MAC_LENGTH_10 = 4,
    AES_MAC_LENGTH_12 = 5,
    AES_MAC_LENGTH_14 = 6,
    AES_MAC_LENGTH_16 = 7,      // 16 byte
    AES_MAC_LENGTH_MAX
}
AESMacLength;



/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

// AES の鍵 128 bit
typedef union AESKey
{
    u8  bytes[AES_KEY_SIZE];
    u32 words[AES_KEY_SIZE/sizeof(u32)];
}
AESKey;

// AES の nonce 96 bit
typedef union AESNonce
{
    u8  bytes[AES_NONCE_SIZE];
    u32 words[AES_NONCE_SIZE/sizeof(u32)];
}
AESNonce;

// AES の MAC 32 ～ 128 bit
typedef union AESMac
{
    u8  bytes[AES_MAC_MAX_SIZE];
    u32 words[AES_MAC_MAX_SIZE/sizeof(u32)];
}
AESMac;

// AES のカウンター初期値 128 bit
typedef union AESCounter
{
    u8  bytes[AES_COUNTER_SIZE];
    u32 words[AES_COUNTER_SIZE/sizeof(u32)];
}
AESCounter;

// AES 処理完了コールバック
typedef void (*AESCallback)(AESResult result, void* arg);



/*---------------------------------------------------------------------------*
	関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         AES_GetMacLengthValue

  Description:  AESMacLength 型からバイト単位の MAC サイズを取得します。

  Arguments:    macLength:  MAC のサイズを表す列挙子

  Returns:      バイト単位の MAC サイズを返します。
 *---------------------------------------------------------------------------*/
static inline u32 AES_GetMacLengthValue(AESMacLength macLength)
{
    AESi_ASSERT_MAC_LENGTH(macLength);

    return (u32)macLength * 2 + 2;
}



/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_AES_COMMON_TYPE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
