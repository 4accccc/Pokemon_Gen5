/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_ERROR_H_
#define DWC_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif


    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    /**
     * エラー列挙子
     */
    typedef enum {
        DWC_ERROR_NONE = 0,            ///< エラーなし
        DWC_ERROR_DS_MEMORY_ANY,       ///< 本体セーブデータに関するエラー
        DWC_ERROR_AUTH_ANY,            ///< 認証に関するエラー
        DWC_ERROR_AUTH_OUT_OF_SERVICE, ///< サービス終了
        DWC_ERROR_AUTH_STOP_SERVICE,   ///< サービス一時中断
        DWC_ERROR_AC_ANY,              ///< 自動接続に関するエラー
        DWC_ERROR_NETWORK,             ///< その他のネットワークエラー
        DWC_ERROR_GHTTP_ANY,           ///< HTTP通信(GameSpy HTTP)のエラー
        DWC_ERROR_DISCONNECTED,        ///< 切断された
        DWC_ERROR_FATAL,               ///< 致命的なエラー
    
        // シーケンス上の軽度のエラー（DWCErrorType = DWC_ETYPE_LIGHT）
        DWC_ERROR_FRIENDS_SHORTAGE,    ///< 指定の人数の有効な友達がいないのに友達指定ピアマッチメイクを始めようとした
        DWC_ERROR_NOT_FRIEND_SERVER,   ///< サーバクライアントマッチメイクで、サーバに指定した友達が相互に友達ではない、もしくはサーバを立てていない
        DWC_ERROR_SERVER_FULL,         ///< サーバクライアントマッチメイクのサーバが定員オーバー
    	
    	// Ndライブラリ用のエラー種別追加
    	DWC_ERROR_ND_ANY,             ///< ダウンロードライブラリのエラー
    	DWC_ERROR_ND_HTTP,            ///< ダウンロードライブラリのHTTPエラー
    	
    	// SVLライブラリ用のエラー種別追加
    	DWC_ERROR_SVL_ANY,            ///< サービスロケータのエラー
    	DWC_ERROR_SVL_HTTP,           ///< サービスロケータ情報取得中のHTTP通信エラー
        
        // 不正文字列チェック用のエラー種別追加
        DWC_ERROR_PROF_ANY,           ///< 不正文字列チェックでサーバの応答がおかしいときに発生するエラー
        DWC_ERROR_PROF_HTTP,          ///< 不正文字列チェックのHTTPエラー
    	
    	// gdbライブラリのエラー
    	DWC_ERROR_GDB_ANY,            ///< GDBライブラリのエラー。通常のDWCでは発生しません。
        DWC_ERROR_SCL_ANY,            ///< SCライブラリのエラー。通常のDWCでは発生しません。
    
        DWC_ERROR_SC_CONNECT_BLOCK,   ///< サーバクライアントマッチメイクで、クライアントがサーバから接続を拒否された
        DWC_ERROR_NETWORK_LIGHT,	  ///< 軽度として扱うネットワークエラー
    	
        DWC_ERROR_NUM
    } DWCError;
    
    
    /**
     * エラー検出時に必要な処理を示すエラータイプ
     */
    typedef enum {
        DWC_ETYPE_NO_ERROR = 0,   ///< エラーなし
        DWC_ETYPE_LIGHT,          ///< ゲーム固有の表示のみで、エラーコード表示は必要ありません。
                                  ///< DWC_ClearError関数を呼び出せば、復帰可能です。
        
        DWC_ETYPE_SHOW_ERROR,     ///< エラーコードを表示してください。
                                  ///< DWC_ClearError関数を呼び出せば、復帰可能です。 
        
        DWC_ETYPE_SHUTDOWN_FM,    ///< DWC_ShutdownFriendsMatch関数を呼び出して、FriendsMatchライブラリを終了する必要があります。エラーコードを表示してください。
        
        DWC_ETYPE_SHUTDOWN_GHTTP, ///< 以下の各ライブラリ解放関数を必要に応じて呼び出してください。
                                  ///< DWC_RnkShutdown関数(汎用ランキングライブラリ)
                                  ///< エラーコードを表示してください。
        
        DWC_ETYPE_SHUTDOWN_ND,    ///< DWC_NdCleanupAsync関数を呼び出して、ダウンロードライブラリを終了する必要があります。エラーコードを表示してください。
        DWC_ETYPE_DISCONNECT,     ///< 以下の各ライブラリ解放関数を必要に応じて呼び出してください。
                                  ///< DWC_ShutdownFriendsMatch関数(FriendsMatchライブラリ)
                                  ///< DWC_NdCleanupAsync関数(ダウンロードライブラリ)
                                  ///< DWC_RnkShutdown関数(汎用ランキングライブラリ)
                                  ///< その後、DWC_CleanupInetあるいはDWC_CleanupInetAsync関数で通信の切断も行う必要があります。 エラーコードを表示してください。

        DWC_ETYPE_FATAL,          ///< FatalError相当なので、Nitroでは、電源OFFを促す必要があります。Twlではリセット(ランチャーへの戻り)を促す必要があります。エラーコードを表示してください。
        DWC_ETYPE_NUM
    } DWCErrorType;
    
    
    // デバッグ用dwc_baseエラーコード各要素
    enum {
        // どのシーケンス中に起こったエラーか
        DWC_ECODE_SEQ_LOGIN          = (-60000),  // ログイン処理でのエラー
        DWC_ECODE_SEQ_FRIEND         = (-70000),  // 友達管理処理でのエラー
        DWC_ECODE_SEQ_MATCH          = (-80000),  // マッチメイク処理でのエラー
        DWC_ECODE_SEQ_ETC            = (-90000),  // 上記以外の処理でのエラー
    
        // GameSpyのエラーの場合、どのSDKが出したエラーか
        DWC_ECODE_GS_GP              =  (-1000),  // GameSpy GP のエラー
        DWC_ECODE_GS_PERS            =  (-2000),  // GameSpy Persistent のエラー
        DWC_ECODE_GS_STATS           =  (-3000),  // GameSpy Stats のエラー
        DWC_ECODE_GS_QR2             =  (-4000),  // GameSpy QR2 のエラー
        DWC_ECODE_GS_SB              =  (-5000),  // GameSpy Server Browsing のエラー
        DWC_ECODE_GS_NN              =  (-6000),  // GameSpy Nat Negotiation のエラー
        DWC_ECODE_GS_GT2             =  (-7000),  // GameSpy gt2 のエラー
        DWC_ECODE_GS_HTTP            =  (-8000),  // GameSpy HTTP のエラー
        DWC_ECODE_GS_ETC             =  (-9000),  // GameSpy その他のSDKのエラー
    
        // 通常エラー種別
        DWC_ECODE_TYPE_NETWORK       =   ( -10),  // ネットワーク障害
        DWC_ECODE_TYPE_SERVER        =   ( -20),  // GameSpyサーバ障害
        DWC_ECODE_TYPE_DNS           =   ( -30),  // DNS障害
        DWC_ECODE_TYPE_DATA          =   ( -40),  // 不正なデータを受信
        DWC_ECODE_TYPE_SOCKET        =   ( -50),  // ソケット通信エラー
        DWC_ECODE_TYPE_BIND          =   ( -60),  // ソケットのバインドエラー
        DWC_ECODE_TYPE_TIMEOUT       =   ( -70),  // タイムアウト発生
        DWC_ECODE_TYPE_PEER          =   ( -80),  // １対１の通信で不具合
        DWC_ECODE_TYPE_CONN_OVER     =   (-100),  // コネクション数オーバ
        DWC_ECODE_TYPE_STATS_AUTH    =   (-200),  // STATSサーバログインエラー
        DWC_ECODE_TYPE_STATS_LOAD    =   (-210),  // STATSサーバデータロードエラー
        DWC_ECODE_TYPE_STATS_SAVE    =   (-220),  // STATSサーバデータセーブエラー
        DWC_ECODE_TYPE_NOT_FRIEND    =   (-400),  // 指定された相手が相互に登録された友達ではない
        DWC_ECODE_TYPE_OTHER         =   (-410),  // 相手が通信切断した（主にサーバクライアントマッチメイク時）
        DWC_ECODE_TYPE_MUCH_FAILURE  =   (-420),  // NATネゴシエーションに規定回数以上失敗した
        DWC_ECODE_TYPE_SC_CL_FAIL    =   (-430),  ///< サーバクライアントマッチメイクにおけるクライアントの接続失敗
        DWC_ECODE_TYPE_FAILED_SERVER_MIGRATION =   (-431),  ///< サーバホストが切断後、サーバ交代に失敗したかサーバ交代ができない条件だったので、新しいサーバホストに接続できなかった
        DWC_ECODE_TYPE_FAILED_CONNECT_GROUPID  =   (-432),  ///< GroupIDを指定しての接続に失敗
        DWC_ECODE_TYPE_MATCH_NOTICE  =   (-440),  // クライアント同士の接続通知が失敗した可能性あり(新標準では現在定義のみ)
        DWC_ECODE_TYPE_CLOSE         =   (-600),  // コネクションクローズ時のエラー
        DWC_ECODE_TYPE_TRANS_HEADER  =   (-610),  // ありえない受信ステートでReliableデータを受信した
        DWC_ECODE_TYPE_TRANS_BODY    =   (-620),  // 受信バッファオーバーフロー
        DWC_ECODE_TYPE_TRANS_SEND    =   (-630),  // Reliable送信に失敗
        DWC_ECODE_TYPE_TRANS_UNKNOWN_PEER =   (-640), ///< 自分が認識していないgt2コネクションからの受信
        DWC_ECODE_TYPE_AC_FATAL      =   (-700),  // AC処理中のACでのエラーグループ。この場合下位桁は専用のstate値。
        DWC_ECODE_TYPE_OPEN_FILE     =   (-800),  // GHTTPファイルオープン失敗
        DWC_ECODE_TYPE_INVALID_POST  =   (-810),  // GHTTP無効な送信
        DWC_ECODE_TYPE_REQ_INVALID   =   (-820),  // GHTTPファイル名等無効
        DWC_ECODE_TYPE_UNSPECIFIED   =   (-830),  // GHTTP詳細不明のエラー
        DWC_ECODE_TYPE_BUFF_OVER     =   (-840),  // GHTTPバッファオーバーフロー
        DWC_ECODE_TYPE_PARSE_URL     =   (-850),  // GHTTP URLの解析エラー
        DWC_ECODE_TYPE_BAD_RESPONSE  =   (-860),  // GHTTPサーバからのレスポンス解析エラー
        DWC_ECODE_TYPE_REJECTED      =   (-870),  // GHTTPサーバからのリクエスト等拒否
        DWC_ECODE_TYPE_FILE_RW       =   (-880),  // GHTTPローカルファイルRead/Writeエラー
        DWC_ECODE_TYPE_INCOMPLETE    =   (-890),  // GHTTPダウンロード中断
        DWC_ECODE_TYPE_TO_BIG        =   (-900),  // GHTTPファイルサイズが大きすぎてダウンロード不可
        DWC_ECODE_TYPE_ENCRYPTION    =   (-910),  // GHTTPエンクリプションエラー
    	
        // Fatal Error専用種別
        DWC_ECODE_TYPE_ALLOC         =     (-1),  // メモリ確保失敗
        DWC_ECODE_TYPE_PARAM         =     (-2),  // パラメータエラー
        DWC_ECODE_TYPE_SO_SOCKET     =     (-3),  // SOのエラーが原因のGameSpy gt2のソケットエラー
        DWC_ECODE_TYPE_NOT_INIT      =     (-4),  // ライブラリが初期化されていない。
        DWC_ECODE_TYPE_DUP_INIT      =     (-5),  // ライブラリが二度初期化された。
        DWC_ECODE_TYPE_WM_INIT       =     (-6),  // WMの初期化に失敗した。
        DWC_ECODE_TYPE_UNEXPECTED    =     (-9),  // 予期しない状態、もしくはUnkwonなGameSpyエラー発生
    
    	// 30000台エラー番号の定義(拡張機能用)
    	DWC_ECODE_SEQ_ADDINS        =   (-30000), // 拡張機能エラー
    	
    	DWC_ECODE_FUNC_ND            =   (-1000), // DWC_Nd用領域
        DWC_ECODE_FUNC_PROF          =   (-3000), // 不正文字列チェック用領域
    	
    	DWC_ECODE_TYPE_ND_SERVER     =   (  0),   // サーバがエラーコードを返してきた
        DWC_ECODE_TYPE_ND_ALLOC      =   ( -1),   ///< メモリ確保失敗
        DWC_ECODE_TYPE_ND_FATAL      =   ( -9),   ///< その他致命的エラー
        DWC_ECODE_TYPE_ND_BUSY       =   (-10),   ///< その関数を呼び出せないステートで関数を呼んだ
    	//DWC_ECODE_TYPE_ND_STATE      =   (-10), // その関数を呼び出せないステートで関数を呼んだ(RVLDWCのマージ)    
        DWC_ECODE_TYPE_ND_HTTP       =   (-20),   ///< HTTP通信に失敗
        DWC_ECODE_TYPE_ND_BUFFULL    =   (-30),   ///< HTTP通信バッファが不十分
        //DWC_ECODE_TYPE_ND_CANCEL     =   (-40),   ///< 非同期処理をキャンセルした
        DWC_ECODE_TYPE_ND_PARAM      =   (-40),   ///< 関数に渡したパラメータが不正
    
    	DWC_ECODE_TYPE_PROF_CONN     =   ( -10),// 不正文字列チェックでサーバとの接続に問題があった
    	DWC_ECODE_TYPE_PROF_PARSE    =   ( -20),// 不正文字列チェックでサーバの応答が正しくなかった
    	DWC_ECODE_TYPE_PROF_OTHER    =   ( -30) // 不正文字列チェックでその他のエラーが発生した
    };


    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    DWCError    DWC_GetLastError(int* errorCode);
    DWCError    DWC_GetLastErrorEx(int* errorCode, DWCErrorType* errorType);
    void        DWC_ClearError(void);



#ifdef __cplusplus
}
#endif


#endif // DWC_ERROR_H_
