/*===========================================================================*
  Project:  Syachi PokemonGDS Network Library
  File:     poke_net.c

  $NoKeywords: $
 *===========================================================================*/
//===========================================================================

#include "poke_net.h"
#include <string.h>

extern POKE_NET_CONTROL POKE_NET_Control;

//====================================
/*!
	ポケモンWiFiライブラリ　解放

	ポケモンWiFiライブラリの使用後に実行されなければなりません。<br>
	ただし、リクエスト処理後に再度リクエストを発行したい場合は<br>
	POKE_NET_ReleaseではなくPOKE_NET_Clearを実行します。
*/
//====================================
void POKE_NET_Release()
{
	// データクリア
	POKE_NET_Clear();

	// クリア対象外のデータ初期化
	memset(&POKE_NET_Control.Auth ,0x00 ,sizeof(POKE_NET_REQUESTCOMMON_AUTH));
    POKE_NET_Control.SvlToken = FALSE;
	POKE_NET_Control.URL[0] = (char)NULL;
	POKE_NET_Control.PortNo = 0;

	// ミューテクス解放
	POKE_NET_ReleaseMutex();

	// 機種特有解放
	POKE_NET_ExRelease();

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET released\n");
}

//====================================
/*!
	ポケモンWiFiライブラリ　初期化

	ポケモンWiFiライブラリ使用前に呼び出す必要があります。

	@param[in]	_url	接続先サーバーＵＲＬ<br>
						ライブラリ内部に保存されるので初期化後解放できます。
	@param[in]	_portno	接続先サーバーポート番号
	@param[in]	_auth	クライアント情報<br>
						NULLの場合は前にセットされたクライアント情報が使用されます

	@retval TRUE	初期化に成功しました
	@retval FALSE	初期化に失敗しました
*/
//====================================
BOOL POKE_NET_Initialize(const char *_url ,unsigned short _portno ,POKE_NET_REQUESTCOMMON_AUTH *_auth)
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET initialize\n");

	// パラメータ
	if( _auth ){
        POKE_NET_Control.Auth = *_auth;
        POKE_NET_Control.SvlToken = FALSE;
	}

#ifdef ___COMMUNICATES_BY_USING_SSL___
    // SSL 初期化
    POKE_NET_SSL_Initialize();
#endif

    // 機種特有初期化
	if( !POKE_NET_ExInitialize() )return(FALSE);

	// ミューテクス作成
	POKE_NET_CreateMutex();

	// 接続先登録
	strcpy(POKE_NET_Control.URL ,_url);
	POKE_NET_Control.PortNo = _portno;

	// 各パラメータクリア
	POKE_NET_Control.SendBuffer = NULL;				// 送信バッファ(ライブラリで用意)
	POKE_NET_Control.Socket = INVALID_SOCKET;		// ソケット
	POKE_NET_Clear();

	return(TRUE);
}

//====================================
/*!
	ポケモンWiFiライブラリ　クリア

	リクエスト終了後、再度リクエストを<br>
	呼び出す前に実行されなければなりません。
*/
//====================================
void POKE_NET_Clear()
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET clear\n");

	// ソケットクローズ
	POKE_NET_CloseSocket();

	// 送信バッファ解放
	if( POKE_NET_Control.SendBuffer ){
		POKE_NET_FreeFunc(POKE_NET_Control.SendBuffer);
		POKE_NET_Control.SendBuffer=NULL;
	}

	// 各パラメータ初期化
	POKE_NET_Control.ConnectTimeOut = POKE_NET_CONNECT_TIMEOUT;	// コネクトタイムアウト時間(sec)
	POKE_NET_Control.SendTimeOut = POKE_NET_SEND_TIMEOUT;		// 送信タイムアウト時間(sec)
	POKE_NET_Control.RecvTimeOut = POKE_NET_RECV_TIMEOUT;		// 受信タイムアウト時間(sec)
	POKE_NET_Control.SendSize = 0;								// 送信サイズ
	POKE_NET_Control.RecvSize = 0;								// 受信サイズ
	POKE_NET_Control.RecvBuffer = NULL;							// 受信バッファ(ユーザーで用意)
	POKE_NET_Control.IP = 0;									// 接続先ＩＰ
	POKE_NET_Control.AbortFlag = FALSE;							// アボート要請

	// ステータスを初期化済へ移行
	POKE_NET_Control.Status = POKE_NET_STATUS_INITIALIZED;
	POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_NONE;

	// SSL関係
#ifdef ___COMMUNICATES_BY_USING_SSL___
    POKE_NET_SSL_Clear();
#endif
}

//====================================
//           スレッド関数
//====================================
void POKE_NET_ThreadFunction()
{
	int flag;
	POKE_NET_STATUS chstat;
		
	// スレッド
	for(;;){
		// スレッド終了要請チェック
		flag = FALSE;
		switch(POKE_NET_Control.Status){
			case POKE_NET_STATUS_INACTIVE:					// 非稼動中
			case POKE_NET_STATUS_INITIALIZED:				// 初期化済み
			case POKE_NET_STATUS_ABORTED:					// 中断
			case POKE_NET_STATUS_FINISHED:					// 正常終了
			case POKE_NET_STATUS_ERROR:						// エラー終了
				flag = TRUE;
				break;
			case POKE_NET_STATUS_REQUEST:					// リクエスト発行
				// スレッド起動完了と同時にこの状態は解除される
            	chstat = POKE_NET_STATUS_REQUEST;
	            if( POKE_NET_GetSvl() == FALSE ){
                    if( POKE_NET_Control.LastErrorCode == POKE_NET_LASTERROR_ABORTED ){
                        chstat = POKE_NET_STATUS_ABORTED;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET request aborted\n");
                    } else {
                        chstat = POKE_NET_STATUS_ERROR;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET request error\n");
                    }
                    flag = TRUE;
                } else {
                    chstat = POKE_NET_STATUS_NETSETTING;
                }
				POKE_NET_LockMutex();
				POKE_NET_Control.Status = chstat;
				break;
			case POKE_NET_STATUS_NETSETTING:				// ネットワーク設定中
				if( POKE_NET_Setting() == FALSE ){
					if( POKE_NET_Control.LastErrorCode == POKE_NET_LASTERROR_ABORTED ){
						chstat = POKE_NET_STATUS_ABORTED;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET setting aborted\n");
					}else{
						chstat = POKE_NET_STATUS_ERROR;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET setting error\n");
					}
					flag = TRUE;
				}else{
					chstat = POKE_NET_STATUS_CONNECTING;
				}
				POKE_NET_LockMutex();
				POKE_NET_Control.Status = chstat;
				break;
			case POKE_NET_STATUS_CONNECTING:				// 接続中
				if( POKE_NET_Connecting() == FALSE ){
					if( POKE_NET_Control.LastErrorCode == POKE_NET_LASTERROR_ABORTED ){
						chstat = POKE_NET_STATUS_ABORTED;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET connecting aborted\n");
					}else{
						chstat = POKE_NET_STATUS_ERROR;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET connecting error\n");
					}
					flag = TRUE;
				}else{
					chstat = POKE_NET_STATUS_SENDING;
				}
				POKE_NET_LockMutex();
				POKE_NET_Control.Status = chstat;
				break;
			case POKE_NET_STATUS_SENDING:					// 送信中
				if( POKE_NET_Sending() == FALSE ){
					if( POKE_NET_Control.LastErrorCode == POKE_NET_LASTERROR_ABORTED ){
						chstat = POKE_NET_STATUS_ABORTED;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET sending aborted\n");
					}else{
						chstat = POKE_NET_STATUS_ERROR;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET sending error\n");
					}
					flag = TRUE;
				}else{
					chstat = POKE_NET_STATUS_RECEIVING;
				}

				// 送信バッファはここで不要になるので解放
				if( POKE_NET_Control.SendBuffer ){
					POKE_NET_FreeFunc(POKE_NET_Control.SendBuffer);
					POKE_NET_Control.SendBuffer=NULL;
				}

				POKE_NET_LockMutex();
				POKE_NET_Control.Status = chstat;
				break;
			case POKE_NET_STATUS_RECEIVING:					// 受信中
				if( POKE_NET_Receiving() == FALSE ){
					if( POKE_NET_Control.LastErrorCode == POKE_NET_LASTERROR_ABORTED ){
						chstat = POKE_NET_STATUS_ABORTED;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET receiving aborted\n");
					}else{
						chstat = POKE_NET_STATUS_ERROR;
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET receiving error\n");
					}
				}else{
					chstat = POKE_NET_STATUS_FINISHED;
					POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET receive finished\n");
				}
				flag = TRUE;
				POKE_NET_LockMutex();
				POKE_NET_Control.Status = chstat;
				break;
		}

		if( flag ){
			// 処理が完了orエラー終了した
			break;
		}else{
			// ここでアボート処理
			if( POKE_NET_Control.AbortFlag == TRUE ){
				// アボート要請が来ている：ここで処理を中断する
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET thread aborted\n");
				POKE_NET_Control.Status = POKE_NET_STATUS_ABORTED;
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_ABORTED;
				break;
			}
			POKE_NET_UnlockMutex();
		}
		POKE_NET_Sleep(1);
	}

	POKE_NET_UnlockMutex();
}

//====================================
/*!
	ポケモンWiFiライブラリ　リクエスト発行

	サーバーに対してリクエストを発行します。

	@param[in]	_reqno		リクエスト番号
	@param[in]	_param		リクエスト内容<br>
							サイズはリクエスト番号から自動計算されます。<br>
							リクエスト内容はライブラリ内部に保存されるので<br>
							初期化後解放できます。
	@param[in]	_response	レスポンス内容を受け取るメモリの先頭アドレス<br>
							POKE_NET_RESPONSE構造体が返ってきます。<br>
							ResultメンバにはReqCodeメンバに対応したPOKE_NET_GDS_RESPONSE_RESULT_xxxxが<br>
							返ってきており、ParamメンバにはReqCodeメンバに対応したレスポンス構造体が<br>
							返ってきます。<br>
							エラーの場合にはParamメンバにはデータが入っておりませんので注意してください。<br>
							<br>
							cf. ReqCodeメンバの値がPOKE_NET_GDS_REQCODE_MUSICALSHOT_REGISTの場合には<br>
							　ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_xxxxの結果コードが返り<br>
							　SUCCESSの場合にはParamメンバにはPOKE_NET_GDS_RESPONSE_MUSICALSHOT_REGIST構造体が返ってきます。


	@retval TRUE	リクエスト発行に成功しました
	@retval FALSE	リクエスト発行に失敗しました
*/
//====================================
BOOL POKE_NET_Request(unsigned short _reqno ,void *_param ,void *_response ,long _size)
{
	long size;
	POKE_NET_REQUEST *reqhead;

	POKE_NET_LockMutex();
	switch(POKE_NET_Control.Status){
		// 状態をクリアする
		case POKE_NET_STATUS_INACTIVE:					// 非稼動中
		case POKE_NET_STATUS_REQUEST:					// リクエスト発行
		case POKE_NET_STATUS_NETSETTING:				// ネットワーク設定中
		case POKE_NET_STATUS_CONNECTING:				// 接続中
		case POKE_NET_STATUS_SENDING:					// 送信中
		case POKE_NET_STATUS_RECEIVING:					// 受信中
			// 処理中、初期化していない状態はエラー
			POKE_NET_UnlockMutex();
			POKE_NET_Control.Status = POKE_NET_STATUS_ERROR;
			POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_NOTINITIALIZED;
			return(FALSE);
		case POKE_NET_STATUS_INITIALIZED:				// 初期化済み
			// 初期化直後はスルー
			break;
		case POKE_NET_STATUS_ABORTED:					// 中断
		case POKE_NET_STATUS_FINISHED:					// 正常終了
		case POKE_NET_STATUS_ERROR:						// エラー終了
			// 初期化後で、処理が完了している
			POKE_NET_Clear();
			break;
	}
	POKE_NET_UnlockMutex();

	// 受信バッファの登録
	POKE_NET_Control.RecvSize   = POKE_NET_GetResponseMaxSize(_reqno);
	if( POKE_NET_Control.RecvSize == -1 ){
		POKE_NET_Control.Status = POKE_NET_STATUS_ERROR;
		POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_ILLEGALREQUEST;
		return(FALSE);
	}
	POKE_NET_Control.RecvBuffer = (char *)_response;

	// バッファ最大サイズとリクエスト番号チェック
	if( _size != -1 ){
		size = (long)(_size + sizeof(long) + sizeof(POKE_NET_REQUEST));
	}else{
		size = POKE_NET_GetRequestSize(_reqno);
		if( size == -1 ){
			POKE_NET_Control.Status = POKE_NET_STATUS_ERROR;
			POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_ILLEGALREQUEST;
			return(FALSE);
		}
	}

	// 送信バッファ確保
	POKE_NET_Control.SendBuffer = (char*)POKE_NET_AllocFunc(size);
	if( POKE_NET_Control.SendBuffer == NULL )return(FALSE);

	// 送信サイズ登録
	POKE_NET_Control.SendSize = size;

	// 送信パケット登録
	memcpy(&POKE_NET_Control.SendBuffer[0] ,&size ,sizeof(long));
	reqhead = (POKE_NET_REQUEST *)&POKE_NET_Control.SendBuffer[sizeof(long)];
	reqhead->ReqCode  = _reqno;
	reqhead->Auth     = POKE_NET_Control.Auth;
	reqhead->Option   = 0;
	memcpy(reqhead->Param ,_param ,size - sizeof(long)- sizeof(POKE_NET_REQUEST));

	// *******************************************************************
	// [10/03/05]
	// SSL通信を利用するため、暗号化の必要が無くなった。
	// <該当ソース>
	// poke_net_svr_conn.cc line:322前後	サーバ側受信
	// poke_net_svr_conn.cc line:617前後	サーバ側送信
	// poke_net.c			line:359前後	クライアント共通送信
	// poke_net_win.cpp		line:668前後	Win側受信
	// poke_net_ds.cpp		line:469前後	DS側受信
	// *******************************************************************
	//// 暗号化
	//POKE_NET_OptEncode(
	//	(unsigned char *)&POKE_NET_Control.SendBuffer[sizeof(long)] ,
	//	(long)(POKE_NET_Control.SendSize - sizeof(long)) ,
	//	(unsigned char *)&POKE_NET_Control.SendBuffer[sizeof(long)]);

	// 送信へ移行
	POKE_NET_Control.Status = POKE_NET_STATUS_REQUEST;

	// スレッド開始
	POKE_NET_LockMutex();
	if( POKE_NET_CreateThread() == FALSE ){
		POKE_NET_Control.Status = POKE_NET_STATUS_ERROR;
		POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_CREATETHREAD;
		POKE_NET_UnlockMutex();
		return(FALSE);
	}
	POKE_NET_UnlockMutex();

	return(TRUE);
}

//====================================
/*!
	ポケモンWiFiライブラリ　ステータス取得

	現在のライブラリの状態を取得します。<br>
	POKE_NET_LockMutex<br>
	POKE_NET_UnlockMutex<br>
	を用いてミューテクスロック状態で取得する必要があります。

	@retval	現在の状態
*/
//====================================
POKE_NET_STATUS POKE_NET_GetStatus()
{
	return(POKE_NET_Control.Status);
}

//====================================
/*!
	ポケモンWiFiライブラリ　中断要求

	リクエスト処理を中断する要求を出します。<br>
	中断要求後、ポケモンWiFiライブラリの状態が<br>
	POKE_NET_GDS_STATUS_ABORTEDになるまで待たねばなりません。<br>
	また、スレッド処理で中断要求を発行したにも関わらず<br>
	POKE_NET_GDS_STATUS_FINISHED<br>
	POKE_NET_GDS_STATUS_ERROR<br>
	などの状態へ遷移することがあります。

	@retval TRUE	要求を出すことに成功しました
	@retval FALSE	要求を出すことに失敗しました
*/
//====================================
BOOL POKE_NET_Abort()
{
	POKE_NET_LockMutex();
	if( POKE_NET_Control.Status >= POKE_NET_STATUS_NETSETTING ){
		if( POKE_NET_Control.Status <= POKE_NET_STATUS_RECEIVING ){
			POKE_NET_Control.AbortFlag = TRUE;
		}
	}
	POKE_NET_UnlockMutex();
	return(TRUE);
}

//====================================
/*!
	ポケモンWiFiライブラリ　返答取得

	リクエストに対する返答を取得します。<br>
	状態がPOKE_NET_STATUS_FINISHEDのときのみ使用できます。<br>
	戻り値のアドレスはPOKE_NET_Request関数で<br>
	指定した_responseと同じものです。<br>

	@retval NULL以外	取得に成功しました
	@retval NULL		取得に失敗しました
*/
//====================================
void *POKE_NET_GetResponse()
{
	POKE_NET_LockMutex();
	if( POKE_NET_GetStatus() != POKE_NET_STATUS_FINISHED ){
		POKE_NET_UnlockMutex();
		return(NULL);
	}
	POKE_NET_UnlockMutex();

	return(POKE_NET_Control.RecvBuffer+sizeof(long));
}

//====================================
/*!
	ポケモンWiFiライブラリ　返答サイズ取得

	リクエストに対する返答のサイズを取得します。<br>
	状態がPOKE_NET_STATUS_FINISHEDのときのみ使用できます。

	@retval NULL以外	取得に成功しました
	@retval NULL		取得に失敗しました
*/
//====================================
long POKE_NET_GetResponseSize()
{
	POKE_NET_LockMutex();
	if( POKE_NET_GetStatus() != POKE_NET_STATUS_FINISHED ){
		POKE_NET_UnlockMutex();
		return(-1);
	}
	POKE_NET_UnlockMutex();

	return(POKE_NET_Control.RecvSize);
}

//====================================
/*!
	ポケモンWiFiライブラリ　最終エラーコード取得

	エラー終了時の最終エラーコードを取得します。<br>
	状態がPOKE_NET_STATUS_ERRORのときのみ使用できます。

	@retval POKE_NET_LASTERROR_NONE	取得に失敗しました
	@retval POKE_NET_LASTERROR_NONE以外	取得に成功しました
*/
//====================================
POKE_NET_LASTERROR POKE_NET_GetLastErrorCode()
{
	POKE_NET_LockMutex();
	if( POKE_NET_GetStatus() != POKE_NET_STATUS_ERROR ){
		POKE_NET_UnlockMutex();
		return(POKE_NET_LASTERROR_NONE);
	}
	POKE_NET_UnlockMutex();

	return(POKE_NET_Control.LastErrorCode);
}

//====================================
/*!
	ポケモンWiFiライブラリ　スレッドレベル設定

	内部で起動する通信スレッドのレベルを指定します<br>
	POKE_NET_Initializeの後に呼ばれる必要があります。<br>
	POKE_NET_Clearでは初期化されません。
*/
//====================================
void POKE_NET_SetThreadLevel(unsigned long _level)
{
	POKE_NET_Control.ThreadLevel = _level;
}
