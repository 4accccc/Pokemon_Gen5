/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_MATCH_H_
#define DWC_MATCH_H_


#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------
// preprocessor switch
//----------------------------------------------------------------------------

// サーバクライアントマッチメイク以外ではマッチメイクが終わったらQR2を終了する仕様
#define DWC_QR2_ALIVE_DURING_MATCHING

// 友達リストに友達がいなくても場合によってはマッチメイク開始をOKする
#define DWC_MATCH_ACCEPT_NO_FRIEND

// 友達限定の友達指定ピアマッチメイクを有効にする
#define DWC_LIMIT_FRIENDS_MATCH_VALID

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

/// 最大同時接続数
#define DWC_MAX_CONNECTIONS 32

    /**
     * ゲーム定義キーの開始値
     *
     */
#define DWC_QR2_GAME_KEY_START 100

    /**
     * DWCの予約キー数
     *
     */
#define DWC_QR2_RESERVED_KEYS  (100-50)     // 50 is originaly from NUM_RESERVED_KEYS in qr2regkeys.h
#define DWC_MAX_REGISTERED_KEYS 254         // 254 is originaly from NUM_RESERVED_KEYS in qr2regkeys.h

    /**
     * ゲームで最大使用できるキーの数（154）
     *
     */
#define DWC_QR2_GAME_RESERVED_KEYS (DWC_MAX_REGISTERED_KEYS-DWC_QR2_RESERVED_KEYS-DWC_QR2_RESERVED_KEYS)

//----------------------------------------------------------------------------
// struct
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------

    /// マッチメイク状態列挙子
    typedef enum
    {
        DWC_MATCH_STATE_INIT = 0,           ///< 初期状態

        // クライアント側の状態
        DWC_MATCH_STATE_CL_WAITING,         ///< 待ち状態
        DWC_MATCH_STATE_CL_SEARCH_HOST,     ///< 空きホスト検索状態（友達無指定の時のみ）
        DWC_MATCH_STATE_CL_WAIT_RESV,       ///< 予約に対するサーバからの返答待ち
        DWC_MATCH_STATE_CL_SEARCH_EVAL_HOST,///< 評価する相手ホストを検索中
        DWC_MATCH_STATE_CL_NN,              ///< NATネゴシエーション中
        DWC_MATCH_STATE_CL_GT2,             ///< GT2コネクション確立中
        DWC_MATCH_STATE_CL_CANCEL_SYN,      ///< サーバクライアントでマッチメイクキャンセル同期調整中
        DWC_MATCH_STATE_CL_SYN,             ///< マッチメイク完了同期調整中
        DWC_MATCH_STATE_CL_SVDOWN_1,        ///< サーバダウン検出中
        DWC_MATCH_STATE_CL_SVDOWN_2,        ///< サーバダウン検出中 2
        DWC_MATCH_STATE_CL_SVDOWN_3,        ///< サーバダウン検出中 3
        DWC_MATCH_STATE_CL_SEARCH_GROUPID_HOST, ///< グループID からホストを検索

        // サーバ側の状態
        DWC_MATCH_STATE_SV_WAITING,         ///< 待ち状態
        DWC_MATCH_STATE_SV_OWN_NN,          ///< クライアントとのNATネゴシエーション中
        DWC_MATCH_STATE_SV_OWN_GT2,         ///< クライアントとのGT2コネクション確立中
        DWC_MATCH_STATE_SV_WAIT_CL_LINK,    ///< クライアント同士の接続完了待ち
        DWC_MATCH_STATE_SV_CANCEL_SYN,      ///< サーバクライアントでマッチメイクキャンセル同期調整SYN-ACK待ち
        DWC_MATCH_STATE_SV_CANCEL_SYN_WAIT, ///< サーバクライアントでマッチメイクキャンセル同期調整終了待ち
        DWC_MATCH_STATE_SV_SYN,             ///< マッチメイク終了同期調整SYN-ACK待ち
        DWC_MATCH_STATE_SV_SYN_WAIT,        ///< マッチメイク終了同期調整終了待ち

        // 共通状態
        DWC_MATCH_STATE_WAIT_CLOSE,         ///< コネクションクローズ完了待ち
        DWC_MATCH_STATE_SEARCH_OWN,         ///< 自ホスト情報検索中（以前の名称はDWC_MATCH_STATE_CL_SEARCH_OWN)
        DWC_MATCH_STATE_SV_SYN_CLOSE_WAIT,	///< CLOSE コマンド送信後の待ち時間

        DWC_MATCH_STATE_NUM
    } DWCMatchState;

    /**
     * マッチメイク接続条件に使われるバッファ長
     *
     */
#define DWC_CONNECTION_USERDATA_LEN 4

    /// マッチメイクタイプ列挙子
    enum
    {
        DWC_MATCH_TYPE_ANYBODY = 0,  ///< 友達無指定ピアマッチメイク
        DWC_MATCH_TYPE_FRIEND,		 ///< 友達指定ピアマッチメイク
        DWC_MATCH_TYPE_SC_SV,        ///< サーバクライアントマッチメイクのサーバ側
        DWC_MATCH_TYPE_SC_CL,        ///< サーバクライアントマッチメイクのクライアント側
        DWC_MATCH_TYPE_NUM
    };

    /// 締め切りコールバック列挙子
    typedef enum
    {
        DWC_SUSPEND_SUCCESS = 0,	///< 正常終了
        DWC_SUSPEND_TIMEOUT,		///< タイムアウトした(現在、使用されません)
        DWC_SUSPEND_NUM
    }DWCSuspendResult;

    /// 接続形態列挙子
    typedef enum
    {
        DWC_TOPOLOGY_TYPE_HYBRID = 0,	///< ハイブリッド型
        DWC_TOPOLOGY_TYPE_STAR,			///< スター型
        DWC_TOPOLOGY_TYPE_FULLMESH,		///< フルメッシュ型
        DWC_TOPOLOGY_TYPE_NUM
    }DWCTopologyType;

//----------------------------------------------------------------------------
// typedef - function
//----------------------------------------------------------------------------
    /**
     * マッチメイク完了コールバック型
     *
     * DWC_ConnectToAnybodyAsync / DWC_ConnectToFriendsAsync / DWC_SetupGameServer /
     * DWC_ConnectToGameServerAsync / DWC_ConnectToGameServerByGroupID
     * 関数呼出し後、１つの
     * コネクションが確立された時、またはエラー、キャンセルが発生した時に呼ばれます。
     *
     * エラー発生時は、error, param以外の各引数の値は不定です。
     *
     * 本コールバック関数は
     * DWC_ConnectToAnybodyAsync / DWC_ConnectToFriendsAsync / DWC_SetupGameServer /
     * DWC_ConnectToGameServerAsync / DWC_ConnectToGameServerByGroupID
     * 関数によって設定することができます。
     *
     * 引数の取り得る値の組み合わせ
     *
     * ・error == DWC_ERROR_NONE && cancel == FALSE の場合
     * 	
     *			
     *				self    
     *				isServer
     *				内容    
     *			
     *			
     *				TRUE    
     *				TRUE    
     *				
     *				    なし
     *				
     *            
     *            
     *				TRUE    
     *				FALSE   
     *				
     *				    自分がクライアントで、サーバとそのネットワークへの接続に成功した。indexは -1。
     *				
     *            
     *            
     *				FALSE   
     *				TRUE    
     *				なし    
     *            
     *            
     *				FALSE   
     *				FALSE   
     *				
     *				    自分はサーバかクライアントで、他のクライアントがサーバとそのネットワークへの接続に成功した。
     *				    indexはそのクライアントの友達リストインデックスで、友達でなければ -1。
     *				
     *           
     *   
     *   
     *
     *   ・error == DWC_ERROR_NONE && cancel == TRUE の場合
     *   
     *   		
     *   			self    
     *   			isServer
     *   			内容    
     *			
     *			
     *				TRUE    
     *				TRUE    
     *				
     *				    自分がサーバで、自分でマッチメイクをキャンセルした。indexは -1。
     *				
     *            
     *            
     *				TRUE    
     *				FALSE   
     *				
     *				    自分がクライアントで、自分でマッチメイクをキャンセルした。indexは -1。
     *				
     *            
     *            
     *				FALSE   
     *				TRUE    
     *				
     *				    自分がクライアントで、サーバがマッチメイクをキャンセルした。indexはサーバの友達リストインデックス。
     *				
     *            
     *            
     *				FALSE   
     *				FALSE   
     *				
     *				    自分はサーバかクライアントで、他のクライアントがマッチメイクをキャンセルした。
     *				    indexはそのクライアントの友達リストインデックスで、友達でなければ -1。
     *            
     *	
     *
     * ※文章中のサーバ/クライアントは、GameSpyサーバではなくWii/DS端末のことを指します。
     *
     * Param:    error       DWCエラー種別。内容の詳細は、DWC_GetLastErrorExを参照してください。
     * Param:    cancel      TRUE:キャンセルでマッチメイク終了
     *                      FALSE:キャンセルではない
     * Param:    self        TRUE: 自分がサーバへの接続に成功、もしくは接続をキャンセルした。
     *                      FALSE:他のホストがサーバへの接続に成功、もしくは接続をキャンセルした。
     * Param:    isServer    TRUE: self = FALSE の時に上記の動作を完了したホストがサーバである
     *                      FALSE:self = FALSE の時に上記の動作を完了したホストがクライアントであるか、self = TRUE
     * Param:    index       self = FALSE の時に、上記の動作を完了したホストの友達リストインデックス。
     *                      ホストが友達でないか、self = TRUE の時は-1となる。
     * Param:    param       コールバック用パラメータ
     *
     * See also:   DWC_ConnectToAnybodyAsync
     * See also:   DWC_ConnectToFriendsAsync
     * See also:   DWC_SetupGameServer
     * See also:   DWC_ConnectToGameServerAsync
     * See also:   DWC_ConnectToGameServerByGroupID
     *
     */
    typedef void (*DWCMatchedSCCallback)(DWCError error, BOOL cancel, BOOL self, BOOL isServer, int index, void* param);

    /**
     * 新規接続クライアント接続開始通知コールバック型
     *
     * マッチメイクで、今できているグループに新たに別のクラ
     * イアントが接続を開始した時に呼ばれる
     *
     * Param:    index   新規接続クライアントの友達リストインデックス。
     *                  新規接続クライアントが友達でなければ-1となる。
     * Param:    param   コールバック用パラメータ
     *
     * See also:   DWC_ConnectToAnybodyAsync
     * See also:   DWC_ConnectToFriendsAsync
     * See also:   DWC_SetupGameServer
     * See also:   DWC_ConnectToGameServerAsync
     * See also:   DWC_ConnectToGameServerByGroupID
     *
     */
    typedef void (*DWCNewClientCallback)(int index, void* param);

    /**
     * プレイヤー評価コールバック型
     *
     * 友達指定・友達無指定ピアマッチメイク時に、マッチメイク対象プレイヤーを見つける度に呼ばれるプレイヤー評価コールバック関数です。
     * 本関数で設定した返り値の大きいプレイヤーほど接続先として選ばれる可能性が高くなり、
     * 0以下の値を返した場合は、そのプレイヤーはマッチメイクの対象から外されます。
     * ただし、評価値の絶対値は重要ではなく、見つかったプレイヤーの評価値順により選ばれやすさが決まります。
     * 例えば、評価値がそれぞれ1, 2, 3, 10万となっていた場合でも、10万のホストが圧倒的に選ばれやすくなるというわけではありません。
     *
     * また、ライブラリ内部で、返り値が8,388,607（0x007fffff）以下の正の数であった場合は、8ビット左シフトし、
     * 下位8ビットに乱数を付加することで、同じ評価値となる相手でもある程度分散して接続を試みる仕様となっています。
     * なお、返り値が8,388,607より大きい場合は、 8,388,607として扱われます。
     *
     * プレイヤーの評価に使うマッチメイク指標キーを取得するには、DWC_GetMatchIntValue / DWC_GetMatchStringValue関数を使用します。
     * これらの関数は、本関数内でしか使用できません。
     * 追加した指標キーがサーバに反映されるのに時間がかかるので、アプリケーション側で指標キーをセットしていても、
     * 指標キーがないと判断されてデフォルト値（デフォルト文字列）が返されることがあります。
     *
     * 本コールバック関数はDWC_ConnectToAnybodyAsync / DWC_ConnectToFriendsAsync関数によって設定することができます。 
     *
     * Param:    index   ライブラリが使用する評価対象プレイヤーのインデックス。
     *	                マッチメイク指標キーを取得するDWC_GetMatchIntValue / DWC_GetMatchStringValue関数に渡してください。
     * Param:    param   DWC_ConnectToAnybodyAsync / DWC_ConnectToFriendsAsync関数で指定したコールバック用パラメータ
     *
     * Return value:   評価値。大きい値ほどそのプレイヤーに接続を試みる確率が上がります。
     *	        とりうる値の範囲はintですが、0x007fffffより大きな正の数は、全て0x007fffffと同じに扱われます。
     *	        0以下の値を返した場合は、そのプレイヤーへは接続しません。 
     *
     */
    typedef int (*DWCEvalPlayerCallback)(int index, void* param);

    /**
     * マッチメイク条件判定コールバック型
     *
     * 新規接続クライアントの参加を決定する最終段階で、
     * アプリケーション側に判断を求めるために呼び出されるコールバックです。
     * (人数が一杯である場合や、受付拒否状態などの場合はこのコールバックは呼び出されません)
     * 新規接続クライアントを受け入れるかどうかの判断は、
     * newClientUserData引数から取得できる新規接続クライアントの接続ユーザデータ、
     * 及びDWC_GetConnectionUserDataで取得できる、
     * 接続済みクライアントの接続ユーザデータを考慮して判断してください。
     * クライアント側が DWC_ConnectToGameServerAsync か DWC_ConnectToGameServerByGroupID
     * で接続しようとして、サーバ側で呼ばれたこの関数がFALSEを返した場合、
     * クライアント側ではエラーとしてDWC_ERROR_SC_CONNECT_BLOCKがセットされます。
     *
     * Param:    newClientUserData  新規接続クライアントが、
     *                             DWC_ConnectToAnybodyAsync / DWC_ConnectToFriendsAsync /
     *                             DWC_SetupGameServer / DWC_ConnectToGameServerAsync /
     *                             DWC_ConnectToGameServerByGroupID関数の
     *                             connectionUserDataに設定した値を格納しているバッファへのポインタ。
     *                             u8[DWC_CONNECTION_USERDATA_LEN]分のサイズ。
     * Param:    param              コールバック用パラメータ
     *
     * Return value:  TRUE  新規接続クライアントを受け入れる。
     * Return value:  FALSE 新規接続クライアントの受け入れを拒否する。
     *
     */
    typedef BOOL (*DWCConnectAttemptCallback)(u8* newClientUserData, void* param);

    /**
     * 締め切り完了コールバック型
     *
     * DWC_RequestSuspendMatchAsync で指定され、締め切り処理が終了すると呼ばれます。
     * result が DWC_SUSPEND_SUCCESS の時は締め切り状態は suspend になっています。
     * result が DWC_SUSPEND_SUCCESS 以外の場合は締め切り処理に失敗していますが、
     * suspend には参考のために DWC_RequestSuspendMatchAsync で渡された値がそのまま渡されます。
     *
     *
     * Param:    result  締め切り処理の結果。
     *                  DWC_SUSPEND_SUCCESS:締め切り状態がsuspendになって完了した。
     *                  DWC_SUSPEND_SUCCESS以外:締め切り処理に失敗した。
     * Param:    suspend TRUE:受付拒否状態
     *                  FALSE:受付可能状態
     *                  DWC_RequestSuspendMatchAsync で指定された値がそのまま渡される。
     * Param:    data    コールバック用パラメータ
     *
     */
    typedef void (*DWCSuspendCallback)(DWCSuspendResult result, BOOL suspend, void* data);

//----------------------------------------------------------------------------
// function - external
//----------------------------------------------------------------------------

#ifdef DWC_QR2_ALIVE_DURING_MATCHING
    BOOL    DWC_RegisterMatchStatus  ( void );
#endif
    BOOL    DWC_CancelMatch          ( void );
    BOOL    DWC_CancelMatchAsync     ( void );
    BOOL    DWC_IsValidCancelMatch   ( void );
    u8      DWC_AddMatchKeyInt          ( u8 keyID, const char* keyString, const int* valueSrc );
    u8      DWC_AddMatchKeyString       ( u8 keyID, const char* keyString, const char* valueSrc );
    int     DWC_GetMatchIntValue        ( int index, const char* keyString, int idefault );
    const char* DWC_GetMatchStringValue ( int index, const char* keyString, const char* sdefault );
    int     DWC_GetLastMatchType     ( void );
    DWCMatchState DWC_GetMatchState  ( void );

    u32 DWC_GetGroupID(void);

    // 締め切り処理
    BOOL DWC_RequestSuspendMatchAsync(BOOL suspend, DWCSuspendCallback callback, void* data);
    BOOL DWC_GetSuspendMatch(void);

    // ConnUserData 関連
    BOOL DWC_GetConnectionUserData(u8 aid, u8* userData);

    // デバッグ用
    void DWC_SetDebugMeshNNRetryMax(int retryMax);
    int DWC_GetDebugMeshNNRetryMax(void);

    BOOL DWC_SetServerSearchRatio(int percentage);
    int DWC_GetServerSearchRatio();
    BOOL DWC_SetServerLockEnabled(BOOL enabled);
    BOOL DWC_GetServerLockEnabled(void);

#ifdef __cplusplus
}
#endif


#endif // DWC_MATCH_H_
