/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/

#ifndef DWC_LOGIN_H_
#define DWC_LOGIN_H_


#ifdef __cplusplus
extern "C"
{
#endif


//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------

/// ログイン進行状況列挙子
    typedef enum {
        DWC_LOGIN_STATE_INIT = 0,         ///< 初期状態
        DWC_LOGIN_STATE_REMOTE_AUTH,      ///< リモート認証中
        DWC_LOGIN_STATE_CONNECTING,       ///< GPサーバ接続中
        DWC_LOGIN_STATE_GPGETINFO,        ///< GPにloginしたときのlastname取得時
        DWC_LOGIN_STATE_GPSETINFO,        ///< GPに初めてloginしたときのlastname設定
        DWC_LOGIN_STATE_CONNECTED,        ///< コネクト完了状態
        DWC_LOGIN_STATE_NUM
    } DWCLoginState;

//----------------------------------------------------------------------------
// typedef - function
//----------------------------------------------------------------------------

    /**
     * ログイン完了コールバック型
     *
     * DWC_LoginAsync関数で開始したログイン処理が完了すると呼び出されます。
     *
     * Param:    error       DWCエラー種別。内容の詳細は、DWC_GetLastErrorExを参照してください。
     * Param:    profileID   取得できた自分のGSプロファイルID
     * Param:    param       DWC_LoginAsync関数で指定したコールバック用パラメータ
     *
     * See also:   DWC_LoginAsync
     *
     */
    typedef void (*DWCLoginCallback)(DWCError error, int profileID, void *param);

    void    DWC_AllowMovedUserData(BOOL allowMovedUserData);

#ifdef __cplusplus
}
#endif


#endif // DWC_LOGIN_H_
