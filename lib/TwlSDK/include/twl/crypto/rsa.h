/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/rsa.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CRYPTO_RSA_H_
#define NITRO_CRYPTO_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif


#define CRYPTO_RSA_VERIFY    // RSA署名検証機能を有効にしたい場合はここを定義する


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define CRYPTO_RSA_CONTEXT_SIZE         (4 * 1)
#define CRYPTO_RSA_SIGN_CONTEXT_SIZE    (4 * 1)



/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

// RSA 処理コンテキスト
//   CRYPTORSAContext_localとサイズが一致する必要がある
//   メンバを追加・削除したときは対応すること(CRYPTO_RSA_CONTEXT_SIZE)
typedef struct CRYPTORSAContext
{
/* private: */
    u8      mem[CRYPTO_RSA_CONTEXT_SIZE];
}
CRYPTORSAContext;

// CRYPTO_RSA_EncryptInit I/Oストリームパラメータ
typedef struct CRYPTORSAEncryptInitParam
{
    void    *key;       // [in]  公開鍵文字列
    u32     key_len;    // [in]  公開鍵文字列長
}
CRYPTORSAEncryptInitParam;

// CRYPTO_RSA_EncryptInit_PrivateKey I/Oストリームパラメータ
typedef struct CRYPTORSAEncryptInitPKParam
{
    void    *privkey;       // [in]  秘密鍵文字列
    u32     privkey_len;    // [in]  秘密鍵文字列長
}
CRYPTORSAEncryptInitPKParam;

// CRYPTO_RSA_Encrypt I/Oストリームパラメータ
typedef struct CRYPTORSAEncryptParam
{
    void    *in;        // [in]  暗号化文字列
    u32     in_len;     // [in]  暗号化文字列長
    void    *out;       // [out] 出力文字列バッファ
    u32     out_size;   // [in]  出力文字列バッファサイズ
}
CRYPTORSAEncryptParam;

// CRYPTO_RSA_DecryptInit I/Oストリームパラメータ
typedef struct CRYPTORSADecryptInitParam
{
    void    *key;       // [in]  公開鍵文字列
    u32     key_len;    // [in]  公開鍵文字列長
}
CRYPTORSADecryptInitParam;

// CRYPTO_RSA_Decrypt I/Oストリームパラメータ
typedef struct CRYPTORSADecryptParam
{
    void    *in;        // [in]  復号化文字列
    u32     in_len;     // [in]  復号化文字列長
    void    *out;       // [out] 出力文字列バッファ
    u32     out_size;   // [in]  出力文字列バッファサイズ
}
CRYPTORSADecryptParam;

// RSA 電子署名系処理用コンテキスト
//   CRYPTORSASignContext_localとサイズが一致する必要がある
//   メンバを追加・削除したときは対応すること(CRYPTO_RSA_SIGN_CONTEXT_SIZE)
typedef struct CRYPTORSASignContext
{
/* private: */
    u8      mem[CRYPTO_RSA_SIGN_CONTEXT_SIZE];
}
CRYPTORSASignContext;

// CRYPTO_RSA_SignInit I/Oストリームパラメータ
typedef struct CRYPTORSASignInitParam
{
    void    *key;       // [in]  秘密鍵文字列
    u32     key_len;    // [in]  秘密鍵文字列長
}
CRYPTORSASignInitParam;

// CRYPTO_RSA_Sign I/Oストリームパラメータ
typedef struct CRYPTORSASignParam
{
    void    *in;        // [in]  署名対象文字列
    u32     in_len;     // [in]  署名対象文字列長
    void    *out;       // [out] 出力署名文字列バッファ
    u32     out_size;   // [in]  出力署名文字列バッファサイズ
}
CRYPTORSASignParam;

#if defined(CRYPTO_RSA_VERIFY)
// CRYPTO_RSA_VerifyInit I/Oストリームパラメータ
typedef struct CRYPTORSAVerifyInitParam
{
    void    *key;       // [in]  公開鍵文字列
    u32     key_len;    // [in]  公開鍵文字列長
}
CRYPTORSAVerifyInitParam;

// CRYPTO_RSA_Verify I/Oストリームパラメータ
typedef struct CRYPTORSAVerifyParam
{
    void    *in;        // [in]  検証する文字列
    u32     in_len;     // [in]  検証する文字列長
    void    *sign;      // [in]  署名文字列
    u32     sign_len;   // [in]  署名文字列長
}
CRYPTORSAVerifyParam;
#endif


/*---------------------------------------------------------------------------*
    定値構造体宣言
 *---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_EncryptInit

  Description:  RSA 暗号化のための初期化処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_EncryptInit(CRYPTORSAContext *context, CRYPTORSAEncryptInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_EncryptInit_PrivateKey

  Description:  RSA 暗号化のための初期化処理 (秘密鍵を使用する)

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_EncryptInit_PrivateKey(CRYPTORSAContext *context, CRYPTORSAEncryptInitPKParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Encrypt

  Description:  RSA 暗号化処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      正 - 暗号化した文字列長, -1 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Encrypt(CRYPTORSAContext *context, CRYPTORSAEncryptParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_EncryptTerminate

  Description:  RSA 暗号化のための終了処理

  Arguments:    context:    ライブラリコンテキスト

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_EncryptTerminate(CRYPTORSAContext *context);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_DecryptInit

  Description:  RSA 復号化のための初期化処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_DecryptInit(CRYPTORSAContext *context, CRYPTORSADecryptInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Decrypt

  Description:  RSA 復号化処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      正 - 暗号化した文字列長, -1 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Decrypt(CRYPTORSAContext *context, CRYPTORSADecryptParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_DecryptTerminate

  Description:  RSA 復号化のための終了処理

  Arguments:    context:    ライブラリコンテキスト

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_DecryptTerminate(CRYPTORSAContext *context);


/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_SignInit

  Description:  RSA 署名のための初期化処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_SignInit(CRYPTORSASignContext *context, CRYPTORSASignInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Sign

  Description:  RSA 署名作成処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      正 - 作成した署名文字列長, -1 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Sign(CRYPTORSASignContext *context, CRYPTORSASignParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_SignTerminate

  Description:  RSA 署名のための終了処理

  Arguments:    context:    ライブラリコンテキスト

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_SignTerminate(CRYPTORSASignContext *context);


#if defined(CRYPTO_RSA_VERIFY)
/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_VerifyInit

  Description:  RSA 署名検証のための初期化処理

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_VerifyInit(CRYPTORSASignContext *context, CRYPTORSAVerifyInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Verify

  Description:  RSA 署名検証

  Arguments:    context:    ライブラリコンテキスト
                param:      I/Oストリームパラメータ

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Verify(CRYPTORSASignContext *context, CRYPTORSAVerifyParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_VerifyTerminate

  Description:  RSA 署名検証のための終了処理

  Arguments:    context:    ライブラリコンテキスト

  Returns:      0 - 成功, それ以外 - 失敗
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_VerifyTerminate(CRYPTORSASignContext *context);
#endif


/* for internal use */



#ifdef __cplusplus
}
#endif

#endif //NITRO_CRYPTO_RSA_H_
