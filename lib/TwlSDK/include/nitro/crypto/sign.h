/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/sign.h

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

#ifndef NITRO_CRYPTO_SIGN_H_
#define NITRO_CRYPTO_SIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_VerifySignatureWithHash

  Description:  ハッシュ値から電子署名を検証する

  Arguments:    hash_ptr - ハッシュ値へのポインタ
                sign_ptr - 電子署名へのポインタ
                mod_ptr - 公開鍵へのポインタ
  Returns:      認証できたら TRUE
                認証できなかったら FALSE
 *---------------------------------------------------------------------------*/
int CRYPTO_VerifySignatureWithHash(
    const void* hash_ptr,
    const void* sign_ptr,
    const void* mod_ptr
);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_VerifySignature

  Description:  データから電子署名を検証する

  Arguments:    data_ptr - データへのポインタ
                data_len - データのサイズ
                sign_ptr - 電子署名へのポインタ
                mod_ptr - 公開鍵へのポインタ

  Returns:      認証できたら TRUE
                認証できなかったら FALSE
 *---------------------------------------------------------------------------*/
int CRYPTO_VerifySignature(
    const void* data_ptr,
    int   data_len,
    const void* sign_ptr,
    const void* mod_ptr
);


/*---------------------------------------------------------------------------*
  Name:         CRYPTO_SIGN_GetModulus

  Description:  公開鍵のmodulusへのポインタを返す
                (返ってきたポインタへのアドレスを以下のAPIの引数 mod_ptr に渡す
                   CRYPTO_VerifySignature, CRYPTO_VerifySignatureWithHash )

  Arguments:    pub_ptr - 公開鍵へのポインタ
  Returns:      成功した場合は modulus へのアドレスが返る
                失敗した場合はNULLが返る
 *---------------------------------------------------------------------------*/
void *CRYPTO_SIGN_GetModulus(
	const void* pub_ptr
);


#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_SIGN_H_
