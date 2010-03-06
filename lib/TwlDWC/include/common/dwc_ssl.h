/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_SSL_H_
#define DWC_SSL_H_

#ifdef __cplusplus
extern "C"
{
#endif


    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */


    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */
    
    /// SSL通信の状態を示す列挙型です。
    typedef enum
    {
        DWC_SSL_TYPE_SERVER_AUTH,           ///< サーバ認証を行いSSLで通信します。
        DWC_SSL_TYPE_SERVER_CLIENT_AUTH,    ///< サーバ認証とクライアント認証を行いSSLで通信します。※Nitro,Twlでは使用できません。
        DWC_SSL_TYPE_NUM
    } DWCSSLType;


#ifdef __cplusplus
}
#endif

#endif // DWC_SSL_H_
