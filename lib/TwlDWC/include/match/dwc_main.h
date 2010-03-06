/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_MAIN_H_
#define DWC_MAIN_H_

#include <match/dwc_transport.h>

#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

/// プレイヤー名（ゲーム内スクリーンネーム）の最大長（ワイド文字25文字＋"\0\0"）
#define DWC_MAX_PLAYER_NAME 26

// 過去に誤った関数宣言がなされていたのでその互換性を保つために定義
#define DWC_CloseConnectionsBitmapFromServer DWC_CloseConnectionHardBitmapFromServer

//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------
    /// DWCネットワーク状態列挙子
    typedef enum
    {
        DWC_STATE_INIT = 0,         ///< 初期状態
        DWC_STATE_AVAILABLE_CHECK,  ///< ゲームの利用可能状況をチェック中
        DWC_STATE_LOGIN,            ///< ログイン処理中
        DWC_STATE_ONLINE,           ///< ログイン完了後オンライン状態
        DWC_STATE_MATCHING,         ///< マッチメイク処理中
        DWC_STATE_CONNECTED,        ///< マッチメイク完了後、メッシュ型ネットワーク構築済み状態
        DWC_STATE_NUM
    } DWCState;


//----------------------------------------------------------------------------
// typedef - function
//----------------------------------------------------------------------------
    /**
     * コネクションクローズコールバック型
     *
     * 通信相手との接続が切断するたびに呼び出されます。
     *
     * DWC_SetConnectionClosedCallback関数で指定します。
     *
     * Param:    error   DWCエラー種別
     * Param:    isLocal TRUE:自分がクローズした。
     *                  FALSE:他の誰かがクローズした。
     *                  マッチメイク中に応答のないホストを切断する時もTRUEになる。
     * Param:    isServer    TRUE: サーバホストが切断したことを示す。
     *                            自分がサーバでisLoacal = TRUE の時もTRUEになる。
     *                            ハイブリッド型もしくはフルメッシュ型のときはサーバ交代が発生する。スター型のときは発生しない。
     *                      FALSE:サーバホスト以外のホストが切断したことを示す
     * Param:    aid     クローズしたプレイヤーのAID
     * Param:    index   クローズしたプレイヤーの友達リストインデックス。
     *                  クローズしたプレイヤーが友達でなければ-1となる。
     * Param:    param   コールバック用パラメータ
     *
     * See also:   DWC_SetConnectionClosedCallback
     *
     */
    typedef void (*DWCConnectionClosedCallback)(DWCError error, BOOL isLocal, BOOL isServer, u8 aid, int index, void* param);


//----------------------------------------------------------------------------
// struct
//----------------------------------------------------------------------------

    /// FriendsMatch制御構造体
    typedef void*   DWCFriendsMatchControl;


//----------------------------------------------------------------------------
// function - external
//----------------------------------------------------------------------------

    void    DWC_InitFriendsMatch        ( DWCUserData* userdata, int productID, const char* secretKey, int sendBufSize, int recvBufSize, DWCFriendData friendList[], int friendListLen );
    void    DWC_ShutdownFriendsMatch    ( void);
    void    DWC_ProcessFriendsMatch     ( void);
    BOOL    DWC_LoginAsync              ( const u16*  ingamesn, const char* reserved, DWCLoginCallback callback, void* param );
    BOOL    DWC_UpdateServersAsync      ( const char* playerName, DWCUpdateServersCallback updateCallback, void* updateParam, DWCFriendStatusCallback statusCallback, void* statusParam, DWCDeleteFriendListCallback deleteCallback, void* deleteParam );
    BOOL    DWC_ConnectToAnybodyAsync   ( DWCTopologyType topology, u8  maxEntry, const char* addFilter, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCEvalPlayerCallback evalCallback, void* evalParam, DWCConnectAttemptCallback attemptCallback, u8* connectionUserData, void* attemptParam );
    BOOL    DWC_ConnectToFriendsAsync   ( DWCTopologyType topology, const u8 friendIdxList[], int friendIdxListLen, u8  maxEntry, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCEvalPlayerCallback evalCallback, void* evalParam, DWCConnectAttemptCallback attemptCallback, u8* connectionUserData, void* attemptParam );
    BOOL    DWC_SetupGameServer         ( DWCTopologyType topology, u8  maxEntry, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCConnectAttemptCallback attemptCallback, u8* connectionUserData, void* attemptParam );
    BOOL    DWC_ConnectToGameServerAsync( DWCTopologyType topology, int serverIndex, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCConnectAttemptCallback attemptCallback, u8* connectionUserData, void* attemptParam );
    BOOL    DWC_ConnectToGameServerByGroupID( DWCTopologyType topology, u32 groupID, DWCMatchedSCCallback matchedCallback, void* matchedParam, DWCNewClientCallback newClientCallback, void* newClientParam, DWCConnectAttemptCallback attemptCallback, u8* connectionUserData, void* attemptParam);
    BOOL    DWC_SetConnectionClosedCallback( DWCConnectionClosedCallback callback, void* param );
    int		DWC_CloseConnectionHardFromServer( u8 aid );
    int		DWC_CloseConnectionHardBitmapFromServer( u32* bitmap );
    int     DWC_CloseAllConnectionsHard ( void );
    int     DWC_GetNumConnectionHost    ( void );
    u8      DWC_GetMyAID                ( void );
    int     DWC_GetAIDList              ( u8** aidList );
    u32     DWC_GetAIDBitmap            ( void );
    BOOL    DWC_IsValidAID              ( u8 aid );
    DWCState DWC_GetState               ( void );
    int     DWC_GetLastSocketError      ( void );

    u8      DWC_GetServerAID(void);
    BOOL    DWC_IsServerMyself(void);
    u32     DWC_GetDirectConnectedAIDBitmap(void);

    /**
     * 無効なAIDを示す値
     *
     */
#define DWC_INVALID_AID		(0xff)


#ifdef __cplusplus
}
#endif


#endif // DWC_MAIN_H_
