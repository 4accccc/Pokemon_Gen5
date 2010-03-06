/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_NASLOGIN_H_
#define DWC_NASLOGIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    /**
     * ingamesnチェック結果列挙子
     */
    typedef enum
    {
        DWC_INGAMESN_NOT_CHECKED	= 0,    ///< 未チェック
        DWC_INGAMESN_VALID			= 1,    ///< 有効
        DWC_INGAMESN_INVALID		= 2     ///< 無効
    } DWCIngamesnCheckResult;

    /**
     *  任天堂認証サーバへの認証処理の状況を示す列挙型です。
     */
    typedef enum
    {
        DWC_NASLOGIN_STATE_DIRTY	= 0,	///< 認証処理が初期化されていません。
        DWC_NASLOGIN_STATE_IDLE,			///< 認証処理が開始されていません。待機状態です。
        DWC_NASLOGIN_STATE_HTTP,			///< 認証処理のためのHTTP通信を実行中です。
        DWC_NASLOGIN_STATE_SUCCESS,		    ///< 認証処理が成功しました。
        DWC_NASLOGIN_STATE_ERROR,		    ///< 認証処理が失敗しました。
        DWC_NASLOGIN_STATE_CANCELED,	    ///< 認証処理がキャンセルされました。
        DWC_NASLOGIN_STATE_MAX
    } DWCNasLoginState;



    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    DWCIngamesnCheckResult DWC_GetIngamesnCheckResult(void);

    BOOL DWC_GetDateTime(RTCDate *date, RTCTime *time);

    BOOL                DWC_NASLoginAsync(void);
    DWCNasLoginState    DWC_NASLoginProcess(void);
    void                DWC_NASLoginAbort(void);



#ifdef __cplusplus
}
#endif

#endif // DWC_NASLOGIN_H_
