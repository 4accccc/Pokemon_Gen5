/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_FRIEND_H_
#define DWC_FRIEND_H_


#ifdef __cplusplus
extern "C"
{
#endif

/// DWC_GetOwnStatusString関数やDWC_SetOwnStatusString関数やDWC_GetFriendStatusSC関数で指定できるステータス文字列の長さ(NULL終端含む)
#define DWC_FRIEND_STATUS_STRING_LEN (256)    // GP_LOCATION_STRING_LEN

//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------

    /// 友達の通信状態（GPEnumのStatus＋αを再定義して使用）
    enum
    {
        DWC_STATUS_OFFLINE = 0,    ///< オフライン
        DWC_STATUS_ONLINE,         ///< オンライン（GPサーバにログイン済み）
        DWC_STATUS_PLAYING,        ///< ゲームプレイ中
        DWC_STATUS_MATCH_ANYBODY,  ///< 友達無指定ピアマッチメイク中
        DWC_STATUS_MATCH_FRIEND,   ///< 友達指定ピアマッチメイク中
        DWC_STATUS_MATCH_SC_CL,    ///< サーバクライアントマッチメイク中のクライアント
        DWC_STATUS_MATCH_SC_SV,    ///< サーバクライアントマッチメイク中のサーバ
        DWC_STATUS_NUM
    };

    /// 友達管理処理状態
    typedef enum
    {
        DWC_FRIEND_STATE_INIT = 0,      ///< 初期状態
        DWC_FRIEND_STATE_PERS_LOGIN,    ///< Persistentサーバログイン＆友達リスト同期処理中
        DWC_FRIEND_STATE_LOGON,         ///< Persistentサーバログイン後
        DWC_FRIEND_STATE_NUM
    } DWCFriendState;

    /// ローカル友達リストとGPバディリストとの同期処理状態
    enum
    {
        DWC_BUDDY_UPDATE_STATE_WAIT = 0,  ///< バディリストダウンロード完了待ち
        DWC_BUDDY_UPDATE_STATE_CHECK,     ///< バディリストチェック中
        DWC_BUDDY_UPDATE_STATE_PSEARCH,   ///< プロファイルサーチ中
        DWC_BUDDY_UPDATE_STATE_COMPLETE,  ///< バディリスト同期処理完了
        DWC_BUDDY_UPDATE_STATE_NUM
    };


//----------------------------------------------------------------------------
// typedef - function
//----------------------------------------------------------------------------

    /**
     * 友達リスト同期処理コールバック型
     *
     * 友達リスト同期処理が完了したときに呼び出されます。
     * 
     * 友達リストの同期処理の内容については、DWC_UpdateServersAsync関数を参照してください。
     * 
     * 本コールバック関数はDWC_UpdateServersAsync関数によって設定することができます。 
     * 
     * Param:    error       DWCエラー種別。内容の詳細は、DWC_GetLastErrorExを参照してください。
     * Param:    isChanged   TRUE:同期処理中に友達リストが書き換えられた
     *                      FALSE:友達リストの書き換えなし
     * Param:    param       DWC_UpdateServersAsync関数で指定したコールバック用パラメータ
     *
     */
    typedef void (*DWCUpdateServersCallback)(DWCError error, BOOL isChanged, void* param);

    /**
     * 友達状態変化通知コールバック型
     *
     * 友達の状態が変化したときに呼び出されます。
     *
     * プレイヤーの通信状態は、statusで表されるstatusフラグと、statusStringで表されるstatus文字列から成っており、
     * GameSpyサーバ上で管理されています。 statusフラグはDWCライブラリが自動的にセットしますが、
     * status文字列はDWC_SetOwnStatusString / DWC_SetOwnStatusData関数を用いてアプリケーションが設定することができます。
     *
     * 本コールバック関数はDWC_UpdateServersAsync関数/DWC_SetFriendStatusCallback関数によって設定することができます。 
     * 
     * 
     * statusの値は、以下のいずれかになります。
     * 			
     * 			    
     * 			      DWC_STATUS_OFFLINE
     * 			      オフライン自分がオフラインの時、または相手と友達関係が成立していないとき
     * 			    
     * 			    
     * 			      DWC_STATUS_ONLINE
     * 			      オンライン（Wi-Fiコネクションサーバにログイン済み）
     * 			    
     * 			    
     * 			      DWC_STATUS_PLAYING
     * 			      ・クライアントとしてゲームプレイ中（マッチメイクタイプの種類によらない）・友達指定マッチメイク又はサーバクライアントマッチメイクで、一度でもサーバ交代が起こった後のサーバとしてゲームプレイ中上記2状態は共通して、この相手に対して接続に行くことは出来ない事を示している。(PLAYING状態は受付可能な状態では無い)
     * 			    
     * 			    
     * 			      DWC_STATUS_MATCH_ANYBODY
     * 			      ・友達無指定ピアマッチメイク中で接続相手を検索中・友達無指定ピアマッチメイク中でサーバとしてゲームプレイ中
     * 			    
     * 			    
     * 			      DWC_STATUS_MATCH_FRIEND
     * 			      ・友達指定ピアマッチメイク中で接続相手を検索中・友達指定ピアマッチメイク中でサーバとしてゲームプレイ中
     * 			    
     * 			    
     * 			      DWC_STATUS_MATCH_SC_CL
     * 			      ・サーバクライアントマッチメイク中のクライアントホストとして、サーバと接続処理中・グループID接続のクライアントホストとして、サーバと接続処理中
     * 			    
     * 			    
     * 			      DWC_STATUS_MATCH_SC_SV
     * 			      ・サーバクライアントマッチメイク中のサーバホストとしてゲームプレイ中
     *	            
     *	        
     * 
     *	
     * Param:    index           状態が変化した友達の友達リストインデックス
     * Param:    status          友達の状態。DWC_STATUS_* の列挙子
     * Param:    statusString    ゲームで定義したGameSpy status文字列。
     *                          文字列は最大255文字＋NULL終端。
     * Param:    param           DWC_UpdateServersAsync関数/DWC_SetFriendStatusCallback関数で指定したコールバック用パラメータ
     * 
     */
    typedef void (*DWCFriendStatusCallback)(int index, u8 status, const char* statusString, void* param);

    /**
     * 友達リスト削除コールバック型
     *
     * DWC_UpdateServersAsync関数の処理中に、友達リスト中に同じ友達情報を見つけ、
     * ライブラリ側で消去したときに呼び出されます。
     * 
     * 本コールバック関数はDWC_UpdateServersAsync関数によって設定することができます。 
     *
     * Param:    deletedIndex    削除された友達の友達リストインデックス
     * Param:    srcIndex        同じ友達情報と判定された友達リストインデックス
     * Param:    param           DWC_UpdateServersAsync関数で指定したコールバック用パラメータ
     *
     */
    typedef void (*DWCDeleteFriendListCallback)(int deletedIndex, int srcIndex, void* param);

    /**
     * バディ成立コールバック型
     *
     * GameSpyサーバ上で友達関係が成立したときに呼び出されます。
     * ただし、DWC_UpdateServersAsync関数呼出し後から、友達リスト同期処理が完了するまでの間は呼び出されません。
     *
     * 本コールバック関数はDWC_SetBuddyFriendCallback関数によって設定することができます。 
     *
     * Param:    index   友達関係が成立した友達リストのインデックス
     * Param:    param   DWC_SetBuddyFriendCallback関数で指定したコールバック用パラメータ
     *
     */
    typedef void (*DWCBuddyFriendCallback)(int index, void* param);

//----------------------------------------------------------------------------
// function - external
//----------------------------------------------------------------------------
    u8      DWC_GetFriendStatus         ( const DWCFriendData* friendData, char* statusString );
    u8      DWC_GetFriendStatusSC       ( const DWCFriendData* friendData, u8* maxEntry, u8* numEntry, char* statusString );
    u8      DWC_GetFriendStatusData     ( const DWCFriendData* friendData, char* statusData, int* size );
    u8      DWC_GetFriendStatusDataSC   ( const DWCFriendData* friendData, u8* maxEntry, u8* numEntry, char* statusData, int* size );
    int     DWC_GetNumFriend            ( const DWCFriendData friendList[], int friendListLen );
    BOOL    DWC_SetOwnStatusString      ( const char* statusString );
    BOOL    DWC_GetOwnStatusString      ( char* statusString );
    BOOL    DWC_SetOwnStatusData        ( const char* statusData, u32 size );
    int     DWC_GetOwnStatusData        ( char* statusData );
    BOOL    DWC_CanChangeFriendList     ( void );
    void    DWC_DeleteBuddyFriendData   ( DWCFriendData* friendData );
    BOOL    DWC_SetBuddyFriendCallback  ( DWCBuddyFriendCallback callback, void* param );
    BOOL    DWC_SetFriendStatusCallback ( DWCFriendStatusCallback callback, void* param );

#ifdef __cplusplus
}
#endif


#endif // DWC_FRIEND_H_
