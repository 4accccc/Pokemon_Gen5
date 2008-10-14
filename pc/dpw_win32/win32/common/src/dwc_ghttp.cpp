#include "dpw_typedef.h"
#include "dpw_pcstub.h"

// the original header
//---------------------------------------------------------
#include "dwc_ghttp.h"

// プロトタイプ宣言
//---------------------------------------------------------
static GHTTPBool GHTTPCompletedCallback( GHTTPRequest   request,
										 GHTTPResult	result,
										 char *			buffer,
										 GHTTPByteCount	bufferLen,
										 void *			param);

static void GHTTPProgressCallback(GHTTPRequest   request, 
                                  GHTTPState     state, 
                                  const char *   buffer, 
                                  GHTTPByteCount bufferLen, 
                                  GHTTPByteCount bytesReceived, 
                                  GHTTPByteCount totalSize, 
                                  void *         param );

static DWCGHTTPResult DWCi_HandleGHTTPError(DWCGHTTPResult result);

//=============================================================================
/*!
 *	@brief	ライブラリの初期化
 *
 *	@param	gamename	[in] ゲーム名
 *
 *	@retval	TRUE		成功
 *	@retval FALSE   	失敗
 */
//=============================================================================
BOOL	DWC_InitGHTTP( const char* gamename ){
#pragma unused(gamename)

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_InitGHTTP\n");

    //
	// GHTTP の開始
	//
	ghttpStartup();

	return TRUE;

}

//=============================================================================
/*!
 *	@brief	ライブラリのシャットダウン
 *
 *	@retval	TRUE		成功
 *	@retval	FALSE   	失敗
 */
//=============================================================================
BOOL	DWC_ShutdownGHTTP( void ){

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_ShutdownGHTTP\n");

	//
	// GHTTP の終了
	//
	ghttpCleanup();

	return TRUE;

}


//=============================================================================
/*!
 *	@brief	データ通信
 *
 *	@retval	TRUE		成功
 *	@retval	FALSE   	失敗
 */
//=============================================================================
BOOL	DWC_ProcessGHTTP( void ){

    if (DWCi_IsError()) return FALSE;

	//
	// GHTTP のレスポンス待ち
	//
	ghttpThink();

	return TRUE;

}

//=============================================================================
/*!
 *	@brief	HTTPリクエストのコールバック
 *
 *	@retval	GHTTPTrue		バッファ開放
 *	@retval	GHTTPFalse   	バッファ開放せず
 */
//=============================================================================
static GHTTPBool GHTTPCompletedCallback( GHTTPRequest   request,
										 GHTTPResult	result,
										 char *			buffer,
										 GHTTPByteCount	bufferLen,
										 void *			param)
{
#pragma unused(request)

    DWCGHTTPParam* parameter = (DWCGHTTPParam*)param;

	// コールバック関数のポインタを取得
	DWCGHTTPCompletedCallback callback = (DWCGHTTPCompletedCallback)parameter->completedCallback;

    BOOL buffer_clear = parameter->buffer_clear;

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "GHTTPCompleteCallback result : %d\n", result);

    if(callback){
        if(result == GHTTPSuccess){
            callback( buffer, (int)bufferLen, (DWCGHTTPResult)result, parameter->param );
        }else{
            DWCi_HandleGHTTPError((DWCGHTTPResult)result);
            callback( NULL, 0, (DWCGHTTPResult)result, parameter->param );
        }
    }
    else{
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "Callback is NULL\n");
    }

	DWC_Free(DWC_ALLOCTYPE_BASE, (void*)parameter, 0);
    
    if(!buffer_clear){
        return GHTTPFalse;
    }

	return GHTTPTrue;
}

//=============================================================================
/*!
 *	@brief	HTTPリクエスト中のコールバック
 */
//=============================================================================
static void GHTTPProgressCallback(GHTTPRequest   request, 
                                  GHTTPState     state, 
                                  const char *   buffer, 
                                  GHTTPByteCount bufferLen, 
                                  GHTTPByteCount bytesReceived, 
                                  GHTTPByteCount totalSize, 
                                  void *         param )
{
#pragma unused(request)

    DWCGHTTPParam* parameter = (DWCGHTTPParam*)param;

	// コールバック関数のポインタを取得
	DWCGHTTPProgressCallback callback = (DWCGHTTPProgressCallback)parameter->progressCallback;

    if(callback){
        callback( (DWCGHTTPState)state, buffer, (int)bufferLen, (int)bytesReceived, (int)totalSize, parameter->param );
    }

}

//=============================================================================
/*!
 *	@brief	postするオブジェクトの作成
 *
 *	@param	post	  [in] 作成されるオブジェクト
 *
 */
//=============================================================================
void    DWC_GHTTPNewPost( DWCGHTTPPost* post ){

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_GHTTPNewPost\n");
    
    *post = ghttpNewPost();

    if (post == NULL){
        DWCi_HandleGHTTPError((DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY);
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_Alloc Error\n");
    }
}

//=============================================================================
/*!
 *	@brief	Addpost用
 *
 *	@param	post	  [in] Addされるオブジェクト
 *	@param	key 	  [in] postするデータのkey名
 *	@param	buf 	  [in] postするデータの内容
 *
 *	@retval	TRUE		成功
 *	@retval	FALSE   	失敗
 */
//=============================================================================
BOOL    DWC_GHTTPPostAddString( DWCGHTTPPost* post, const char* key, const char* value ){
    
    if(!ghttpPostAddString(*post, key, value)){
        DWC_Printf(DWC_REPORTFLAG_WARNING, "DWC_GHTTPPostAddString FALSE\n");
        return FALSE;
    }
    else {
        return TRUE;
    }
}

//=============================================================================
/*!
 *	@brief	データの送信
 *
 *	@param	url		[in] リスト取得先のＵＲＬ
 *	@param	post	[in] postするオブジェクト
 *	@param	proc	[in] レスポンスを処理するコールバック
 *	@param	param	[in] コールバック用パラメータ
 *
 *	@retval	req		 0以上 リクエスト識別子（成功）
 *                  -1以下 リクエストエラー値
 */
//=============================================================================
int 	DWC_PostGHTTPData( const char* url, GHTTPPost* post, DWCGHTTPCompletedCallback completedCallback, void* param){

   	GHTTPRequest req;
    DWCGHTTPParam* parameter = NULL;

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_PostGHTTPData\n");

    if (DWCi_IsError()) return DWC_GHTTP_IN_ERROR;
    
    parameter = (DWCGHTTPParam*)DWC_Alloc(DWC_ALLOCTYPE_BASE, sizeof(DWCGHTTPParam));
    
    if(!parameter){
        DWCi_HandleGHTTPError((DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY);
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_Alloc Error\n");
        completedCallback( NULL, 0, (DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY, parameter->param );
        return (int)DWC_GHTTP_INSUFFICIENT_MEMORY;
    }
    
    parameter->param = param;
    parameter->completedCallback = completedCallback;
    parameter->progressCallback = NULL;
    parameter->buffer_clear = TRUE;

	req = ghttpPost( url, *post, GHTTPFalse, 
                      GHTTPCompletedCallback, (void *)parameter );
   
    if(req < 0){
        DWCi_HandleGHTTPError((DWCGHTTPResult)req);
        completedCallback( NULL, 0, (DWCGHTTPResult)req, parameter->param );
        DWC_Free(DWC_ALLOCTYPE_BASE, (void*)parameter, 0);
    }

	return (int)req;

}

//=============================================================================
/*!
 *	@brief	データの受信
 *
 *	@param	url		[in] リスト取得先のＵＲＬ
 *	@param	proc	[in] レスポンスを処理するコールバック
 *	@param	param	[in] コールバック用パラメータ
 *
 *	@retval	req		 0以上 リクエスト識別子（成功）
 *                  -1以下 リクエストエラー値
 */
//=============================================================================
int 	DWC_GetGHTTPData( const char* url, DWCGHTTPCompletedCallback completedCallback, void* param ){
    
   	GHTTPRequest req;
    DWCGHTTPParam* parameter = NULL;

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_GetGHTTPData\n");

    if (DWCi_IsError()) return DWC_GHTTP_IN_ERROR;
    
    parameter = (DWCGHTTPParam*)DWC_Alloc(DWC_ALLOCTYPE_BASE, sizeof(DWCGHTTPParam));
    
    if(!parameter){
        DWCi_HandleGHTTPError((DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY);
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_Alloc Error\n");
        completedCallback( NULL, 0, (DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY, param );
        return (int)DWC_GHTTP_INSUFFICIENT_MEMORY;
    }

    parameter->param = param;
    parameter->completedCallback = completedCallback;
    parameter->progressCallback  = NULL;
    parameter->buffer_clear = TRUE;

    req = ghttpGet( url, GHTTPFalse, GHTTPCompletedCallback, (void *)parameter );

    if(req < 0){
        DWCi_HandleGHTTPError((DWCGHTTPResult)req);
        completedCallback( NULL, 0, (DWCGHTTPResult)req, parameter->param );
        DWC_Free(DWC_ALLOCTYPE_BASE, (void*)parameter, 0);
    }
    
	return (int)req;

}

//=============================================================================
/*!
 *	@brief	データの受信(バッファサイズ指定、ProgressCallback指定)
 *
 *	@param	url		            [in] リスト取得先のＵＲＬ
 *	@param	bufferlen           [in] 受信バッファのサイズ
 *	@param	progressCallback	[in] 処理中に呼ばれるコールバック
 *  @param  completedCallback   [in] レスポンスを処理するコールバック
 *	@param	param	            [in] コールバック用パラメータ
 *
 *	@retval	req		 0以上 リクエスト識別子（成功）
 *                  -1以下 リクエストエラー値
 */
//=============================================================================
int 	DWC_GetGHTTPDataEx( const char* url, int bufferlen, BOOL buffer_clear, DWCGHTTPProgressCallback progressCallback, DWCGHTTPCompletedCallback completedCallback, void* param ){

    char* buffer = NULL;
   	GHTTPRequest req;
    DWCGHTTPParam* parameter = NULL;

    DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_GetGHTTPDataEx\n");

    if (DWCi_IsError()) return DWC_GHTTP_IN_ERROR;
    
    parameter = (DWCGHTTPParam*)DWC_Alloc(DWC_ALLOCTYPE_BASE, sizeof(DWCGHTTPParam));
    
    if(!parameter){
        DWCi_HandleGHTTPError((DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY);
        DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_Alloc Error\n");
        completedCallback( NULL, 0, (DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY, param );
        return (int)DWC_GHTTP_INSUFFICIENT_MEMORY;
    }

    if(bufferlen > 0){
        buffer = (char*)DWC_Alloc(DWC_ALLOCTYPE_BASE, (u32)bufferlen);

        if(!buffer){
            DWCi_HandleGHTTPError((DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY);
            DWC_Printf(DWC_REPORTFLAG_DEBUG, "DWC_Alloc Error\n");
            completedCallback( NULL, 0, (DWCGHTTPResult)DWC_GHTTP_INSUFFICIENT_MEMORY, param );
            DWC_Free(DWC_ALLOCTYPE_BASE, (void*)parameter, 0);
            return (int)DWC_GHTTP_INSUFFICIENT_MEMORY;
        }
    }

    parameter->param = param;
    parameter->completedCallback = completedCallback;
    parameter->progressCallback  = progressCallback;
    parameter->buffer_clear = buffer_clear;

    if(parameter->progressCallback)
        req = ghttpGetEx( url, NULL, buffer, bufferlen, NULL, GHTTPFalse, GHTTPFalse,
                          GHTTPProgressCallback, GHTTPCompletedCallback, (void *)parameter );
    else
        req = ghttpGetEx( url, NULL, buffer, bufferlen, NULL, GHTTPFalse,
                          GHTTPFalse, NULL, GHTTPCompletedCallback, (void *)parameter );
        
    if(req < 0){
        DWCi_HandleGHTTPError((DWCGHTTPResult)req);
        completedCallback( NULL, 0, (DWCGHTTPResult)req, parameter->param );
        DWC_Free(DWC_ALLOCTYPE_BASE, (void*)parameter, 0);
        if(bufferlen > 0)
            DWC_Free(DWC_ALLOCTYPE_BASE, (void*)buffer, 0);
    }
    
	return (int)req;

}

//=============================================================================
/*!
 *	@brief	HTTPリクエストのキャンセル
 *
 *	@param	req		[in] 停止するHTTPリクエストのID(DWC_GetGHTTPDataExの返り値)
 *
 *	@retval	なし
 */
//=============================================================================
void DWC_CancelGHTTPRequest(int reqid)
{
	GHTTPRequest req;
	
	req = (GHTTPRequest)reqid;
	ghttpCancelRequest(req); 
}

//=============================================================================
/*!
 *	@brief	通信状態の確認
 *
 *	@param	req		[in] リクエスト識別子
 *
 *	@retval	state	通信状態
 *  @retval False   失敗
 */
//=============================================================================
DWCGHTTPState DWC_GetGHTTPState( int req )
{
    if(req < 0){
        return DWCGHTTPFalse;
    }
    else{
        return (DWCGHTTPState)(ghttpGetState(req));
    }
}

//=============================================================================
/*!
 *	@brief	GHTTP対応エラー処理関数
 *
 *	@param	result	[in] GHTTPの処理結果
 *
 *	@retval	state	GHTTPの処理結果型（引数をそのまま返す）
 */
//=============================================================================
static DWCGHTTPResult DWCi_HandleGHTTPError(DWCGHTTPResult result)
{
    int errorCode = DWC_ECODE_SEQ_ETC + DWC_ECODE_GS_HTTP;
    DWCError dwcError = DWC_ERROR_NETWORK;

    if (result == DWC_GHTTP_SUCCESS) return DWC_GHTTP_SUCCESS;

    DWC_Printf(DWC_REPORTFLAG_ERROR, "Main, DWCGHTTP error %d\n", result);

    // Possible Error values returned from GHTTP functions.
    switch (result){
	case DWC_GHTTP_FAILED_TO_OPEN_FILE:
        errorCode += DWC_ECODE_TYPE_OPEN_FILE;
        break;
	case DWC_GHTTP_INVALID_POST:
        errorCode += DWC_ECODE_TYPE_INVALID_POST;
        break;
	case DWC_GHTTP_INSUFFICIENT_MEMORY:
        errorCode += DWC_ECODE_TYPE_BUFF_OVER;
        break;
	case DWC_GHTTP_INVALID_FILE_NAME:
	case DWC_GHTTP_INVALID_BUFFER_SIZE:
	case DWC_GHTTP_INVALID_URL:
        errorCode += DWC_ECODE_TYPE_REQ_INVALID;
        break;
	case DWC_GHTTP_UNSPECIFIED_ERROR:
        errorCode += DWC_ECODE_TYPE_UNSPECIFIED;
        break;
        
    // The result of an HTTP request.
	case DWC_GHTTP_OUT_OF_MEMORY:
    case DWC_GHTTP_MEMORY_ERROR:
        dwcError = DWC_ERROR_FATAL;
        errorCode += DWC_ECODE_TYPE_ALLOC;
        break;
	case DWC_GHTTP_BUFFER_OVERFLOW:
        errorCode += DWC_ECODE_TYPE_BUFF_OVER;
        break;
	case DWC_GHTTP_PARSE_URL_FAILED:
        errorCode += DWC_ECODE_TYPE_PARSE_URL;
        break;
	case DWC_GHTTP_HOST_LOOKUP_FAILED:
        errorCode += DWC_ECODE_TYPE_DNS;
        break;
    case DWC_GHTTP_SOCKET_FAILED:
        errorCode += DWC_ECODE_TYPE_SOCKET;
        break;
	case DWC_GHTTP_CONNECT_FAILED:
	case DWC_GHTTP_FILE_NOT_FOUND:
	case DWC_GHTTP_SERVER_ERROR:
        errorCode += DWC_ECODE_TYPE_SERVER;
        break;
	case DWC_GHTTP_BAD_RESPONSE:
        errorCode += DWC_ECODE_TYPE_BAD_RESPONSE;
        break;
	case DWC_GHTTP_REQUEST_REJECTED:
	case DWC_GHTTP_UNAUTHORIZED:
	case DWC_GHTTP_FORBIDDEN:
        errorCode += DWC_ECODE_TYPE_REJECTED;
        break;
	case DWC_GHTTP_FILE_WRITE_FAILED:
	case DWC_GHTTP_FILE_READ_FAILED:
        errorCode += DWC_ECODE_TYPE_FILE_RW;
        break;
	case DWC_GHTTP_FILE_INCOMPLETE:
        errorCode += DWC_ECODE_TYPE_INCOMPLETE;
        break;
	case DWC_GHTTP_FILE_TOO_BIG:
        errorCode += DWC_ECODE_TYPE_TO_BIG;
        break;
	case DWC_GHTTP_ENCRYPTION_ERROR:
        errorCode += DWC_ECODE_TYPE_ENCRYPTION;
        break;

    default:
        break;
    }

    DWCi_SetError(dwcError, errorCode);
    return result;
}
