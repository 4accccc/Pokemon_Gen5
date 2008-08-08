#ifndef DWC_GHTTP_H_
#define DWC_GHTTP_H_

#define GHTTP_EXTENDEDERROR 
// gamespy modules
#include "ghttp/ghttp.h"

#ifdef __cplusplus
extern "C" {
#endif

// 通信状態
typedef enum
{
	// GameSpy HTTP SDK 1.07.07(NITRO-DWC 1.0 - 2.2)
	DWC_GHTTP_HOST_LOOKUP       = GHTTPHostLookup,            // Resolving hostname to IP (asynchronously if possible).
	DWC_GHTTP_CONNECTING        = GHTTPConnecting,            // Waiting for socket connect to complete.
	DWC_GHTTP_SECURING_SESSION  = GHTTPSecuringSession,       // Setup secure channel.
	DWC_GHTTP_SENDING_REQUEST   = GHTTPSendingRequest,        // Sending the request.
	DWC_GHTTP_POSTING           = GHTTPPosting,               // Positing data (skipped if not posting).
	DWC_GHTTP_WAITING           = GHTTPWaiting,               // Waiting for a response.
	DWC_GHTTP_RECEIVING_STATUS  = GHTTPReceivingStatus,       // Receiving the response status.
	DWC_GHTTP_RECEIVING_HEADERS = GHTTPReceivingHeaders,      // Receiving the headers.
	DWC_GHTTP_RECEIVING_FILE    = GHTTPReceivingFile,         // Receiving the file.

	// GameSpy HTTP SDK 1.10.00(NITRO-DWC 3.0 - )
	DWC_GHTTP_SOCKET_INIT       = GHTTPSocketInit,            // Socket creation and initialization.
	DWC_GHTTP_LOOKUP_PENDING    = GHTTPLookupPending,         // Asychronous DNS lookup pending.

	// NITRO-DWC original
	DWC_GHTTP_FALSE             = -1                          // Invalid processing
} DWCGHTTPState;    // Based on GHTTPState

// リクエストエラー値
typedef enum
{
	// GameSpy HTTP SDK 1.07.07(NITRO-DWC 1.0 - 2.2)
	DWC_GHTTP_ERROR_START         = GHTTPErrorStart,
	DWC_GHTTP_FAILED_TO_OPEN_FILE = GHTTPFailedToOpenFile,
	DWC_GHTTP_INVALID_POST        = GHTTPInvalidPost,
	DWC_GHTTP_INSUFFICIENT_MEMORY = GHTTPInsufficientMemory,
	DWC_GHTTP_INVALID_FILE_NAME   = GHTTPInvalidFileName,
	DWC_GHTTP_INVALID_BUFFER_SIZE = GHTTPInvalidBufferSize,
	DWC_GHTTP_INVALID_URL         = GHTTPInvalidURL,
	DWC_GHTTP_UNSPECIFIED_ERROR   = GHTTPUnspecifiedError,

	// GameSpy HTTP SDK 1.10.00(NITRO-DWC 3.0 - )
	// [No addition]

	// NITRO-DWC original
    DWC_GHTTP_IN_ERROR            = DWC_GHTTP_ERROR_START
} DWCGHTTPRequestError; // Based on GHTTPRequestError

// 通信結果
typedef enum
{
	// GameSpy HTTP SDK 1.07.07(NITRO-DWC 1.0 - 2.2)
	DWC_GHTTP_SUCCESS            = GHTTPSuccess,               // 0:  Successfully retrieved file.
	DWC_GHTTP_OUT_OF_MEMORY      = GHTTPOutOfMemory,           // 1:  A memory allocation failed.
	DWC_GHTTP_BUFFER_OVERFLOW    = GHTTPBufferOverflow,        // 2:  The user-supplied buffer was too small to hold the file.
	DWC_GHTTP_PARSE_URL_FAILED   = GHTTPParseURLFailed,        // 3:  There was an error parsing the URL.
	DWC_GHTTP_HOST_LOOKUP_FAILED = GHTTPHostLookupFailed,      // 4:  Failed looking up the hostname.
	DWC_GHTTP_SOCKET_FAILED      = GHTTPSocketFailed,          // 5:  Failed to create/initialize/read/write a socket.
	DWC_GHTTP_CONNECT_FAILED     = GHTTPConnectFailed,         // 6:  Failed connecting to the http server.
	DWC_GHTTP_BAD_RESPONSE       = GHTTPBadResponse,           // 7:  Error understanding a response from the server.
	DWC_GHTTP_REQUEST_REJECTED   = GHTTPRequestRejected,       // 8:  The request has been rejected by the server.
	DWC_GHTTP_UNAUTHORIZED       = GHTTPUnauthorized,          // 9:  Not authorized to get the file.
	DWC_GHTTP_FORBIDDEN          = GHTTPForbidden,             // 10: The server has refused to send the file.
	DWC_GHTTP_FILE_NOT_FOUND     = GHTTPFileNotFound,          // 11: Failed to find the file on the server.
	DWC_GHTTP_SERVER_ERROR       = GHTTPServerError,           // 12: The server has encountered an internal error.
	DWC_GHTTP_FILE_WRITE_FAILED  = GHTTPFileWriteFailed,       // 13: An error occured writing to the local file (for ghttpSaveFile[Ex]).
	DWC_GHTTP_FILE_READ_FAILED   = GHTTPFileReadFailed,        // 14: There was an error reading from a local file (for posting files from disk).
	DWC_GHTTP_FILE_INCOMPLETE    = GHTTPFileIncomplete,        // 15: Download started but was interrupted.  Only reported if file size is known.
	DWC_GHTTP_FILE_TOO_BIG       = GHTTPFileToBig,             // 16: The file is to big to be downloaded (size exceeds range of interal data types)
	DWC_GHTTP_ENCRYPTION_ERROR   = GHTTPEncryptionError,       // 17: Error with encryption engine.

	// GameSpy HTTP SDK 1.10.00(NITRO-DWC 3.0 - )
	DWC_GHTTP_REQUEST_CANCELLED  = GHTTPRequestCancelled,      // 18: User requested cancel and/or graceful close.

	// NITRO-DWC original[CAUTION!:互換性のために残しているが将来的に問題になるかもしれない定義]
	DWC_GHTTP_NUM                = DWC_GHTTP_ENCRYPTION_ERROR + 1,
	DWC_GHTTP_MEMORY_ERROR       = DWC_GHTTP_NUM + 1
} DWCGHTTPResult;   // Based on GHTTPResult

// 関数型宣言
//---------------------------------------------------------
typedef void (*DWC_GHTTPCompletedCALLBACK)(const char* buf, int len, DWCGHTTPResult result, void* param);
typedef void (*DWC_GHTTPProgressCALLBACK)(DWCGHTTPState state, const char* buf, int len, int bytesReceived, int totalSize, void* param);
typedef GHTTPRequest DWCGHTTPRequest;
typedef GHTTPPost    DWCGHTTPPost;

//命名規則ミスのための処置
typedef void (*DWCGHTTPCompletedCallback)(const char* buf, int len, DWCGHTTPResult result, void* param);
typedef void (*DWCGHTTPProgressCallback)(DWCGHTTPState state, const char* buf, int len, int bytesReceived, int totalSize, void* param);

// 構造体宣言
//---------------------------------------------------------
typedef struct
{
    void* param;
    DWCGHTTPCompletedCallback completedCallback;
    DWCGHTTPProgressCallback  progressCallback;
    BOOL buffer_clear;
}DWCGHTTPParam;

/**
 * ライブラリの初期化と終了
 */
BOOL	DWC_InitGHTTP(  const char* gamename  );
BOOL	DWC_ShutdownGHTTP( void );

/**
 * Think
 */
BOOL	DWC_ProcessGHTTP( void );

/**
 * postオブジェクトの作成
 */
void    DWC_GHTTPNewPost( DWCGHTTPPost* post );
BOOL    DWC_GHTTPPostAddString( DWCGHTTPPost* post, const char* key, const char* value );
BOOL    DWC_GHTTPPostAddFileFromMemory( DWCGHTTPPost *post, const char *key, const char *buffer, int bufferlen, const char *filename, const char *contentType );

/**
 * データの送受信
 */
int  	DWC_PostGHTTPData( const char* url, DWCGHTTPPost* post, DWCGHTTPCompletedCallback completedCallback, void* param );
int     DWC_GetGHTTPData( const char* url, DWCGHTTPCompletedCallback completedCallback, void* param);
int 	DWC_GetGHTTPDataEx( const char* url, int bufferlen, BOOL buffer_clear, DWCGHTTPProgressCallback progressCallback, DWCGHTTPCompletedCallback completedCallback, void* param );
int 	DWC_GetGHTTPDataEx2( const char* url, int bufferlen, BOOL buffer_clear, DWCGHTTPPost *post, DWCGHTTPProgressCallback progressCallback, DWCGHTTPCompletedCallback completedCallback, void* param );
void    DWC_CancelGHTTPRequest(int req);

/**
 * 通信状態の確認
 */
DWCGHTTPState DWC_GetGHTTPState( int req );

#ifdef __cplusplus
}
#endif


#endif
