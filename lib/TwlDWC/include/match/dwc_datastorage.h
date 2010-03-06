/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_DATASTORAGE_H_
#define DWC_DATASTORAGE_H_


#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------

    /// Persistentサーバログイン状態
    enum
    {
        DWC_PERS_STATE_INIT = 0,   ///< 初期状態
        DWC_PERS_STATE_LOGIN,      ///< ログイン中
        DWC_PERS_STATE_CONNECTED,  ///< 接続状態
        DWC_PERS_STATE_NUM
    };

//----------------------------------------------------------------------------
// typedef - function
//----------------------------------------------------------------------------

    /**
     * データストレージサーバ・ログインコールバック型
     *
     * データストレージサーバへのログイン処理が完了すると呼び出されます。
     * 
     * 本コールバック関数はDWC_LoginToStorageServerAsync関数によって設定することができます。 
     * 
     * DWC_LogoutFromStorageServer()をコールバック内で呼び出さないでください。
     *
     * Param:    error   DWCエラー種別。内容の詳細は、DWC_GetLastErrorExを参照してください。
     * Param:    param   DWC_LoginToStorageServerAsync関数で指定したコールバック用パラメータ
     *
     */
    typedef void (*DWCStorageLoginCallback)(DWCError error, void* param);

    /**
     * データストレージサーバ・セーブ完了コールバック型
     *
     * データストレージへのセーブが完了したときに呼び出されます。
     *
     * 本コールバック関数はDWC_SetStorageServerCallback関数によって設定することができます。 
     * 
     * DWC_LogoutFromStorageServer()をコールバック内で呼び出さないでください。
     *
     * Param:    success     TRUE:セーブ成功
     *                      FALSE:セーブ失敗
     * Param:    isPublic    TRUE:セーブしたデータがパブリックデータ
     *                      FALSE:プライベートデータ
     *	                    success = FALSE の場合も有効な値が入っています。
     * Param:    param       DWC_SavePublicDataAsync / DWC_SavePrivateDataAsync関数で指定したコールバック用パラメータ
     *
     */
    typedef void (*DWCSaveToServerCallback)(BOOL success, BOOL isPublic, void* param);

    /**
     * データストレージサーバ・ロード完了コールバック型
     *
     * ロード関数で指定したキーが存在しなかった場合や、他人が
     * DWC_SavePrivateDataAsync()でセーブしたキーを読もうとした場合は
     * successがTRUE、dataが""(空文字列)、lenが0になります。
     *	
     * 本コールバック関数はDWC_SetStorageServerCallback関数によって設定することができます。 
     *
     * DWC_LogoutFromStorageServer()をコールバック内で呼び出さないでください。
     *
     * Param:    success TRUE:通信成功
     *                  FALSE:通信エラー
     * Param:    index   ロード元データ保持者の友達リストインデックス
     *                  自分の場合と、ロード完了前に友達でなくなった場合は-1になる。
     * Param:    data    ロードしたデータ（key/value組の文字列）
     * Param:    len     ロードデータ長
     * Param:    param   DWC_LoadOwnPrivateDataAsync / DWC_LoadOthersDataAsync関数で指定したコールバック用パラメータ
     *
     */
    typedef void (*DWCLoadFromServerCallback)(BOOL success, int index, char* data, int len, void* param);

//----------------------------------------------------------------------------
// struct
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// function - external
//----------------------------------------------------------------------------

    BOOL    DWC_LoginToStorageServerAsync(DWCStorageLoginCallback callback, void* param );
    void    DWC_LogoutFromStorageServer ( void );
    BOOL    DWC_SetStorageServerCallback( DWCSaveToServerCallback saveCallback, DWCLoadFromServerCallback loadCallback );
    BOOL    DWC_SavePublicDataAsync     ( char* keyvalues, void* param );
    BOOL    DWC_SavePrivateDataAsync    ( char* keyvalues, void* param );
    BOOL    DWC_LoadOwnPublicDataAsync  ( char* keys, void* param );
    BOOL    DWC_LoadOwnPrivateDataAsync ( char* keys, void* param );
    BOOL    DWC_LoadOthersDataAsync     ( char* keys, int index, void* param );

#ifdef __cplusplus
}
#endif


#endif // DWC_DATASTORAGE_H_
