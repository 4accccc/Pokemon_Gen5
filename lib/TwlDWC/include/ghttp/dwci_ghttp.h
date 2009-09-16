/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright $(dwc_packed_year) Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#ifndef DWCi_GHTTP_H_
#define DWCi_GHTTP_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/// GHTTPの通信状態
    typedef enum
    {

        DWC_GHTTP_SOCKET_INIT,          ///< ソケット初期化中
        DWC_GHTTP_HOST_LOOKUP,			///< ホスト名をIPに変換中
        DWC_GHTTP_LOOKUP_PENDING,       ///< DNS解決待ち
        DWC_GHTTP_CONNECTING,			///< ソケット接続完了待ち
        DWC_GHTTP_SECURING_SESSION,     ///< 安全なチャンネルを確立中
        DWC_GHTTP_SENDING_REQUEST,	    ///< リクエスト送信中
        DWC_GHTTP_POSTING,				///< データ送信中（送信しない場合はスキップされます）
        DWC_GHTTP_WAITING,				///< レスポンス待ち
        DWC_GHTTP_RECEIVING_STATUS,     ///< レスポンスの状態を受信中
        DWC_GHTTP_RECEIVING_HEADERS,    ///< ヘッダを受信中
        DWC_GHTTP_RECEIVING_FILE,       ///< ファイルを受信中

        DWC_GHTTP_FALSE = 0             ///< 通信失敗

    } DWCGHTTPState;

    /**
     *  リクエストエラー値
     *
     *  @version 1.4.0 定数の値を1ずつ減らしてDWC_GHTTP_IN_ERRORも-1になるようにしました
     */
    typedef enum
    {

        DWC_GHTTP_ERROR_START           = -8,   ///< -8,
        DWC_GHTTP_FAILED_TO_OPEN_FILE,          ///< ファイルを開くのに失敗
        DWC_GHTTP_INVALID_POST,                 ///< 無効な送信
        DWC_GHTTP_INSUFFICIENT_MEMORY,          ///< メモリ不足
        DWC_GHTTP_INVALID_FILE_NAME,	        ///< 無効なファイル名
        DWC_GHTTP_INVALID_BUFFER_SIZE,          ///< 無効なバッファサイズ
        DWC_GHTTP_INVALID_URL,                  ///< 無効なURL
        DWC_GHTTP_UNSPECIFIED_ERROR,            ///< 詳細不明のエラー

        DWC_GHTTP_IN_ERROR              = -1    ///< エラー発生中

    } DWCGHTTPRequestError;

/// 通信結果
    typedef enum
    {

        DWC_GHTTP_SUCCESS,          ///< 成功
        DWC_GHTTP_OUT_OF_MEMORY,    ///< メモリ割り当て失敗
        DWC_GHTTP_BUFFER_OVERFLOW,  ///< 供給されたバッファが小さすぎるため、ファイルの取得失敗
        DWC_GHTTP_PARSE_URL_FAILED, ///< URL解析エラー
        DWC_GHTTP_HOST_LOOKUP_FAILED,///< ホスト名検索失敗
        DWC_GHTTP_SOCKET_FAILED,    ///< ソケットの作成、初期化、読み出し、書き込み失敗
        DWC_GHTTP_CONNECT_FAILED,   ///< HTTPサーバへの接続失敗
        DWC_GHTTP_BAD_RESPONSE,     ///< HTTPサーバからのレスポンスの解析エラー
        DWC_GHTTP_REQUEST_REJECTED, ///< HTTPサーバのリクエスト拒否
        DWC_GHTTP_UNAUTHORIZED,     ///< ファイル取得未許可
        DWC_GHTTP_FORBIDDEN,        ///< HTTPサーバのファイル送信拒否
        DWC_GHTTP_FILE_NOT_FOUND,   ///< HTTPサーバ上のファイル検索失敗
        DWC_GHTTP_SERVER_ERROR,     ///< HTTPサーバ内部エラー
        DWC_GHTTP_FILE_WRITE_FAILED,///< ローカルファイルへの書き込みエラー
        DWC_GHTTP_FILE_READ_FAILED, ///< ローカルファイルからの読み出しエラー
        DWC_GHTTP_FILE_INCOMPLETE,  ///< ダウンロードの中断
        DWC_GHTTP_FILE_TOO_BIG,     ///< ファイルが大きすぎるためダウンロード不可能
        DWC_GHTTP_ENCRYPTION_ERROR, ///< 暗号化エラー
        DWC_GHTTP_REQUEST_CANCELLED,///< リクエストがキャンセルされた
        DWC_GHTTP_NUM,

        DWC_GHTTP_MEMORY_ERROR = DWC_GHTTP_NUM + 1           ///< メモリ割り当て失敗


    } DWCGHTTPResult;

// 関数型宣言
//---------------------------------------------------------

    /**
     * DWC_PostGHTTPData / DWC_GetGHTTPData / DWC_GetGHTTPDataEx関数によるデータの
     * アップロード、ダウンロードが完了した際に呼び出されます。
     *
     * 本コールバック関数はDWC_PostGHTTPData / DWC_GetGHTTPData /
     * DWC_GetGHTTPDataEx関数によって設定することができます。
     *
     * @param   buf     受信データ格納バッファ。アップロードの時はNULL
     * @param   len     受信データのサイズ。アップロードの時は0
     * @param   result  アップロード、ダウンロードの結果。エラーが発生した場合は、
     *                  DWCのエラーがライブラリによってセットされています。
     *                  DWCのエラーはDWC_GetLastErrorEx関数で取得できます。
     * @param   param   DWC_PostGHTTPData / DWC_GetGHTTPData / DWC_GetGHTTPDataEx
     *                  で指定したコールバック用パラメータ
     *
     * @sa      DWC_PostGHTTPData
     * @sa      DWC_GetGHTTPData
     * @sa      DWC_GetGHTTPDataEx
     */
    typedef void (*DWCGHTTPCompletedCallback)(const char* buf, int len, DWCGHTTPResult result, void* param);

    /**
     * DWC_GetGHTTPDataEx関数を呼び出した後のGHTTPライブラリの現在の状態を通知します。
     *
     * リクエスト送信中、データ受信中など、ダウンロードシーケンスの状態が変化する
     * 度に呼び出されます。本コールバック関数はDWC_GetGHTTPDataEx関数によって設定
     * することができます。
     *
     * @param   state           GHTTPの通信状態
     *                          DWC_GetGHTTPState関数の返り値と同じ値を返す
     * @param   buf             受信データ格納バッファ
     * @param   len             受信データのサイズ
     * @param   bytesReceived   受信された総バイト数
     * @param   totalSize       ファイルのトータルサイズ。不明な場合は-1
     * @param   param           DWC_GetGHTTPDataEx関数で指定したコールバック用パラメータ
     *
     * @sa  DWC_GetGHTTPDataEx
     * @sa  DWC_GetGHTTPState
     */
    typedef void (*DWCGHTTPProgressCallback)(DWCGHTTPState state, const char* buf, int len, int bytesReceived, int totalSize, void* param);


// 型宣言
//---------------------------------------------------------

    typedef void*   DWCGHTTPPost;


// 関数宣言
//---------------------------------------------------------

    BOOL	DWC_InitGHTTP           ( const char* gamename  );
    BOOL	DWC_ShutdownGHTTP       ( void );
    BOOL	DWC_ProcessGHTTP        ( void );
    void    DWC_GHTTPNewPost        ( DWCGHTTPPost* post );
    BOOL    DWC_GHTTPPostAddString  ( DWCGHTTPPost* post, const char* key, const char* value );
    BOOL    DWC_GHTTPPostAddFileFromMemory( DWCGHTTPPost *post, const char *key, const char *buffer, int bufferlen, const char *filename, const char *contentType );
    void    DWC_CancelGHTTPRequest  ( int req );
    DWCGHTTPState DWC_GetGHTTPState ( int req );
    int  	DWC_PostGHTTPData       ( const char* url, DWCGHTTPPost* post, DWCGHTTPCompletedCallback completedCallback, void* param );
    int     DWC_GetGHTTPData        ( const char* url, DWCGHTTPCompletedCallback completedCallback, void* param );
    int 	DWC_GetGHTTPDataEx      ( const char* url, int bufferlen, BOOL buffer_clear, DWCGHTTPProgressCallback progressCallback, DWCGHTTPCompletedCallback completedCallback, void* param );
    int 	DWC_GetGHTTPDataEx2     ( const char* url, int bufferlen, BOOL buffer_clear, DWCGHTTPPost *post, DWCGHTTPProgressCallback progressCallback, DWCGHTTPCompletedCallback completedCallback, void* param );

// 型宣言
//---------------------------------------------------------

    typedef struct
    {

        void* param;
        DWCGHTTPCompletedCallback completedCallback;
        DWCGHTTPProgressCallback  progressCallback;
        BOOL buffer_clear;

    }
    DWCGHTTPParam;

	// windows用デバッグ情報
#ifdef _WIN32
	typedef enum DWC_GHTTP_ERROR_TYPE
	{
		DWC_GHTTP_ERROR_TYPE_CONNECTION,
		DWC_GHTTP_ERROR_TYPE_SERVER
	}
	DWC_GHTTP_ERROR_TYPE;

	typedef struct DWC_GhttpDebugInfo
	{
		DWC_GHTTP_ERROR_TYPE errorType;
		int debugBufLen;
		char debugBuf[4096];
	}
	DWC_GhttpDebugInfo;
#endif // _WIN32

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DWCi_GHTTP_H
