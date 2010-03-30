//============================================================================
//                         Pokemon Network Library
//============================================================================
#include "poke_net.h"


#ifdef ___POKE_NET_BUILD_WINDOWS___


extern "C" {
POKE_NET_CONTROL POKE_NET_Control;
}

//====================================
//         デバッグプリント
//====================================
void POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL _level ,char *_format ,...)
{
	va_list ap;

	if( _level > POKE_NET_CURRENT_DEBUGLEVEL )return;
	
	va_start(ap, _format);
	vprintf(_format, ap);
	va_end(ap);
}


//====================================
//         デバッグプリント
// [10/03/16]追加。unicode版。
//====================================
void POKE_NET_DebugPrintfW(POKE_NET_DEBUGLEVEL _level ,wchar_t *_format ,...)
{
	va_list ap;

	if( _level > POKE_NET_CURRENT_DEBUGLEVEL )return;
	
	va_start(ap, _format);
	vwprintf(_format, ap);
	va_end(ap);
}

//====================================
//           メモリ確保
//====================================
void *POKE_NET_AllocFunc( long _size )
{
	char *addr;

	addr = new char [_size];
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET memalloc %d addr:%08X\n" ,_size ,addr);

	return(addr);
}

//====================================
//              解放
//====================================
void POKE_NET_FreeFunc( void *_ptr  )
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET memfree %08X\n" ,_ptr);
	delete [] ((char *)_ptr);
}

//====================================
//      ミューテクスロック
//====================================
void POKE_NET_LockMutex()
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL2 ,"#POKE_NET lock mutex %08X\n" ,POKE_NET_Control.MutexHandle);
	WaitForSingleObject(POKE_NET_Control.MutexHandle ,INFINITE);
}

//====================================
//       ミューテクスアンロック
//====================================
void POKE_NET_UnlockMutex()
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL2 ,"#POKE_NET unlock %08X\n" ,POKE_NET_Control.MutexHandle);
	ReleaseMutex(POKE_NET_Control.MutexHandle);
}

//====================================
//      ミューテクス作成
//====================================
BOOL POKE_NET_CreateMutex()
{
	POKE_NET_Control.MutexHandle = CreateMutex(0 ,FALSE ,0);
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET create mutex : %08X\n" ,POKE_NET_Control.MutexHandle);
	return(TRUE);
}

//====================================
//      ミューテクス解放
//====================================
BOOL POKE_NET_ReleaseMutex()
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET close mutex : %08X\n" ,POKE_NET_Control.MutexHandle);
	CloseHandle(POKE_NET_Control.MutexHandle);
	POKE_NET_Control.MutexHandle = NULL;
	return(TRUE);
}

//====================================
//        機種特有初期化
//====================================
BOOL POKE_NET_ExInitialize()
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET exInitialize\n");
	POKE_NET_Control.ThreadLevel = THREAD_PRIORITY_NORMAL;

	// ロケール設定
	setlocale(LC_ALL, "");

	return(TRUE);
}

//====================================
//         機種特有解放
//====================================
void POKE_NET_ExRelease()
{
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET exreleased\n");
}

//====================================
//        スレッド処理ラップ
//====================================
void POKE_NET_ThreadFunctionWrap(void *_Param)
{
	POKE_NET_ThreadFunction();
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET exit thread %08X\n" ,POKE_NET_Control.ThreadID);
	_endthread();
}

//====================================
//        スレッド作成
//====================================
BOOL POKE_NET_CreateThread()
{
	POKE_NET_Control.ThreadID = _beginthread(POKE_NET_ThreadFunctionWrap ,NULL ,NULL);
	SetThreadPriority((HANDLE)POKE_NET_Control.ThreadID ,POKE_NET_Control.ThreadLevel);

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET create thread %08X\n" ,POKE_NET_Control.ThreadID);
	if( POKE_NET_Control.ThreadID == -1 ){
		return(FALSE);
	}
	return(TRUE);
}

//=====================================
// IPアドレス文字列が正しいかチェック
//=====================================
BOOL CheckCorrectIP(char *_Data)
{
	long NCount = 0;
	long Mode = 0;

	while(*_Data){
		if( (*_Data >= '0') && (*_Data <= '9') ){
			NCount++;
			if( NCount >= 4 )return(FALSE);
		}else if( *_Data == '.' ){
			if( NCount == 0 )return(FALSE);
			NCount = 0;
			Mode++;
			if( Mode >= 4 )return(FALSE);
		}else return(FALSE);
	
		_Data++;
	}
	if( (Mode == 3) && (NCount != 0) )return(TRUE);

	return(FALSE);
}

//====================================
//           URLからIP取得
//====================================
BOOL GetIPfromURL(char *_URL ,char *_IP)
{
	struct hostent *HostTmp;
	struct in_addr *IAddress;
	long IP[4];

	if( CheckCorrectIP(_URL) == FALSE ){
		if( HostTmp = gethostbyname(_URL) ){
			IAddress=(LPIN_ADDR)HostTmp->h_addr; //h_addr_list[0];
			strcpy(_IP ,inet_ntoa(*IAddress));
		}else{
			return(FALSE);
		}
	}else{
		sscanf(_URL ,"%d.%d.%d.%d" ,&IP[0] ,&IP[1] ,&IP[2] ,&IP[3]);
		sprintf(_IP ,"%d.%d.%d.%d" ,IP[0] ,IP[1] ,IP[2] ,IP[3]);
	}

	return(TRUE);
}


// SSL関係
#ifdef ___COMMUNICATES_BY_USING_SSL___

//====================================
//    SSLハンドシェイク検証コールバック
//====================================
static int Verify_Callback(int _ok, X509_STORE_CTX* _pStore)
{
	char issuer_name[255] = {0};
	char subject_name[255] = {0}; 

	if(!_ok)
	{
		X509* pCert = X509_STORE_CTX_get_current_cert(_pStore);
		int depth	= X509_STORE_CTX_get_error_depth(_pStore);
		int err		= X509_STORE_CTX_get_error(_pStore);

		X509_NAME_oneline(X509_get_issuer_name(pCert), issuer_name, sizeof(issuer_name));
		X509_NAME_oneline(X509_get_subject_name(pCert), subject_name, sizeof(subject_name));

		POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
			"POKE_NET_SVR::Verify_Callback >> Failed. >>\n issuer:%s\n subject:%s\n error:%s\n depth:%d\n",
			issuer_name,
			subject_name,
			X509_verify_cert_error_string(err),
			depth
		);
	}

	return _ok;
}

#endif
// ___COMMUNICATES_BY_USING_SSL___


//====================================
//       セッティング中
//====================================
BOOL POKE_NET_Setting()
{
	unsigned long argp = 1;
	char iptmp[16];

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL ,"#POKE_NET network setting\n");

	// ソケット作成
	POKE_NET_Control.Socket = socket(AF_INET, SOCK_STREAM, 0);
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET create socket %08X\n" ,POKE_NET_Control.Socket);
	if( POKE_NET_Control.Socket == INVALID_SOCKET ){
		POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_NETWORKSETTING;
		return(FALSE);
	}

	// ソケットを非同期に設定
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET socket mode to nonblocking %08X\n" ,POKE_NET_Control.Socket);
	if( ioctlsocket( POKE_NET_Control.Socket , FIONBIO , &argp ) == SOCKET_ERROR ){
		POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_IOCTRLSOCKET;
		return(FALSE);
	}

	// URLからIPアドレス取得
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET gethostbyname %s" ,POKE_NET_Control.URL);
	if( GetIPfromURL(POKE_NET_Control.URL ,iptmp) == FALSE ){
		POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_NETWORKSETTING;
		POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ," ... unknown\n");
		return(FALSE);
	}
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ," (%s):%d\n" ,iptmp ,POKE_NET_Control.PortNo);

	// IPアドレス登録
	POKE_NET_Control.IP = inet_addr(iptmp);

	// SSL関係
#ifdef ___COMMUNICATES_BY_USING_SSL___
	const char SSL_SERVER_CERT_FILENAME[] = "pkgdstest_cert.pem";
	//const char SSL_SERVER_CERT_FILENAME[] = "root.pem";

	// SSLの初期化
	SSL_library_init(); 
	POKE_NET_Control.m_pSSL_CTX = SSL_CTX_new(SSLv23_client_method());

	// 設定
	SSL_CTX_set_mode(POKE_NET_Control.m_pSSL_CTX, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	//// 証明書チェインの情報を読み込み
	//// 多分要らない
	////if(SSL_CTX_use_certificate_file(m_pSSL_CTX, SSL_SERVER_CERT_FILENAME, SSL_FILETYPE_PEM) != 1)
	//if(SSL_CTX_use_certificate_chain_file(POKE_NET_Control.m_pSSL_CTX, SSL_SERVER_CERT_FILENAME) != 1)
	//{
	//	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
	//		"POKE_NET_SVR SSL_CTX_use_certificate_chain_file failed %s.\n", 
	//		SSL_SERVER_CERT_FILENAME
	//	);
	//}
	//else
	//{
	//	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
	//		"POKE_NET_SVR SSL_CTX_use_certificate_chain_file success %s.\n",
	//		SSL_SERVER_CERT_FILENAME
	//	);
	//}

	// 信頼出来る証明書の組み込み
	if(SSL_CTX_load_verify_locations(POKE_NET_Control.m_pSSL_CTX, SSL_SERVER_CERT_FILENAME, NULL) != 1)
	{
		POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
			"POKE_NET_SVR load failed %s.\n", 
			SSL_SERVER_CERT_FILENAME
		);
	}
	else
	{
		POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
			"POKE_NET_SVR load success %s.\n",
			SSL_SERVER_CERT_FILENAME
		);
	}

	// 検証種類の割り当て
	// 検証をフィルタするコールバックの割り当て
	// ■SSL_VERIFY_NONE
	// <server>
	// 　証明書の要求がクライアントに一切送信されず、クライアントは証明書を送信しません。
	// <client>
	// 　サーバから受け取った証明書は全て検証されますが、検証に失敗してもハンドシェイクを停止しません。
	// ※このフラグは他のフラグと組み合わせては駄目。
	// ■SSL_VERIFY_PEER
	// <server>
	//   証明書の要求がクライアントに送信されます。クライアントは無視可能。
	//   証明書が送り返された場合にはその証明書は検証されます。検証に失敗したらハンドシェイクは停止されます。
	// <client>
	//   サーバが証明書を送信すると、その証明書が検証されます。検証に失敗したらハンドシェイクは停止されます。
	//   サーバが証明書を送信しない場合は匿名モードが使用されている場合のみ。これはデフォルトでは無効。
	// ■SSL_VERIFY_FAIL_IF_NO_PEER_CERT
	// <server>
	//   SSL_VERIFY_PEERと合わせて使用する必要があります。
	//   クライアントが証明書を提供しない場合、ハンドシェイクは停止されます。
	// <client>
	//   サーバモード以外では無視されます。
	// ■SSL_VERIFY_CLIENT_ONCE
	// <server>
	//   SSL_VERIFY_PEERと合わせて使用する必要があります。
	//   ネゴシエーションが再度行われている場合には、クライアント証明書を要求しません。最初のみ要求。
	// <client>
	//   サーバモード以外では無視されます。
	SSL_CTX_set_verify(POKE_NET_Control.m_pSSL_CTX, SSL_VERIFY_NONE, Verify_Callback);
//	SSL_CTX_set_verify(POKE_NET_Control.m_pSSL_CTX, SSL_VERIFY_PEER, Verify_Callback);
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
		"POKE_NET_SVR SSL_CTX_set_verify success.\n"
	);

	// 証明書チェインの検証数の設定(デフォルト=9)
	SSL_CTX_set_verify_depth(POKE_NET_Control.m_pSSL_CTX, 9);
	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
		"POKE_NET_SVR SSL_CTX_set_verify_depth success.\n"
	);

#endif

	return(TRUE);
}


//====================================
//          ソケット解放
//====================================
void POKE_NET_CloseSocket()
{
	if( POKE_NET_Control.Socket != INVALID_SOCKET ){
		POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET close socket %08X\n" ,POKE_NET_Control.Socket);
		shutdown(POKE_NET_Control.Socket ,2);
		closesocket(POKE_NET_Control.Socket);
		POKE_NET_Control.Socket = INVALID_SOCKET;
	}

	// SSL関係
#ifdef ___COMMUNICATES_BY_USING_SSL___
	// SSLの終了
	if(	POKE_NET_Control.m_pSSL != NULL)
	{
		SSL_shutdown(POKE_NET_Control.m_pSSL); 
		SSL_free(POKE_NET_Control.m_pSSL); 
	}

	if(	POKE_NET_Control.m_pSSL != NULL)
	{
		SSL_CTX_free(POKE_NET_Control.m_pSSL_CTX);
	}
#endif

}

//====================================
//          接続中
//====================================
BOOL POKE_NET_Connecting()
{
//	unsigned long address;
	SOCKADDR_IN sockaddrtmp;
	long errcode;
#ifdef ___COMMUNICATES_BY_USING_SSL___
	int ret;
#endif
	unsigned long timetmp;
	unsigned long starttime;

	memcpy(&sockaddrtmp.sin_addr ,&POKE_NET_Control.IP ,sizeof(unsigned long));
	sockaddrtmp.sin_port = htons(POKE_NET_Control.PortNo);
	sockaddrtmp.sin_family = AF_INET;

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET connect to %s socket:%08X\n" ,POKE_NET_Control.URL ,POKE_NET_Control.Socket);
	starttime = GetTickCount();
	for(;;){
		if( connect(POKE_NET_Control.Socket ,(LPSOCKADDR)&sockaddrtmp ,sizeof(SOCKADDR_IN)) == SOCKET_ERROR ){
			errcode = WSAGetLastError();
			if( (errcode == WSAEWOULDBLOCK) || (errcode == WSAEALREADY)  || (errcode == WSAEINVAL) ){
				// ブロッキングよるエラー
				if( POKE_NET_Control.ConnectTimeOut != 0 ){
					// タイムアウト設定あり
					timetmp = GetTickCount();
					if( (signed)(timetmp - starttime) >= (POKE_NET_Control.ConnectTimeOut*1000) ){
						// タイムアウト
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET connect timeout socket:%08X\n" ,POKE_NET_Control.Socket);
						POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_CONNECTTIMEOUT;
						return(FALSE);
					}
				}
			}else if( errcode == WSAEISCONN ){
				// 接続されている

				// SSL関係
#ifdef ___COMMUNICATES_BY_USING_SSL___
				POKE_NET_Control.m_pSSL = SSL_new(POKE_NET_Control.m_pSSL_CTX);
				errcode = SSL_set_fd(POKE_NET_Control.m_pSSL, POKE_NET_Control.Socket);
				if(errcode <= 0)
				{
					POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
						"#POKE_NET POKE_NET_Control() failed. errcode = %d, get_error() = %d\n",
						errcode,
						SSL_get_error(POKE_NET_Control.m_pSSL, errcode)
					);
					ERR_print_errors_fp(stderr);
				}
				else
				{
					POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, "#POKE_NET SSL_set_fd() Succeeded.\n");
				}

				// SSL通信の開始
				while((ret = SSL_connect(POKE_NET_Control.m_pSSL)) <= 0)
				{
					// タイムアウト処理
					if( POKE_NET_Control.ConnectTimeOut != 0 )
					{
						timetmp = GetTickCount();
						if((signed)(timetmp - starttime) >= (POKE_NET_Control.ConnectTimeOut * 1000))
						{
							// タイムアウト
							POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET SSL_connect timeout socket:%08X\n" ,POKE_NET_Control.Socket);
							POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_CONNECTTIMEOUT;
							return FALSE;
						}
					}

					errcode = SSL_get_error(POKE_NET_Control.m_pSSL, ret);

					if(errcode == SSL_ERROR_WANT_READ || errcode == SSL_ERROR_WANT_WRITE)
					{
						continue;
					}
					else
					{
						POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
							"#POKE_NET SSL_connect() failed. errcode = %d\n",
							errcode
						);
						ERR_print_errors_fp(stderr);

						return FALSE;
					}
				}
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, "#POKE_NET SSL_connect() Succeeded.\n");

				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, "#POKE_NET ---------- SSL通信情報 ----------\n");
				// session
				//SSL_SESSION* pSSI = SSL_get_session(POKE_NET_Control.m_pSSL);

				// 使用する暗号化方式を取得
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, 
					"  暗号化方式=%s\n",
					SSL_get_cipher(POKE_NET_Control.m_pSSL)
				);

				// 受信したサーバ証明書情報を取得
				X509* pServerCert = SSL_get_peer_certificate(POKE_NET_Control.m_pSSL);

				// サーバ証明書の所有者情報の取得
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, 
					"  Server certificate::Subject=%s\n",
					X509_NAME_oneline(X509_get_subject_name(pServerCert), 0, 0)
				);

				// サーバ証明書の発行者情報の取得
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, 
					"  Server certificate::Issuer=%s\n",
					X509_NAME_oneline(X509_get_issuer_name(pServerCert), 0, 0)
				);

				// 認証結果のステータスを取得
				// X509_V_OK = 0
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, 
					"#POKE_NET verify_result=%d\n",
					SSL_get_verify_result(POKE_NET_Control.m_pSSL)
				);

				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1, "#POKE_NET ---------- SSL通信情報 ここまで--\n");

				// X509解放
				X509_free(pServerCert);
#endif
				break;
			}else{
				// コネクト失敗
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_CONNECT;
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET connect error socket:%08X\n" ,POKE_NET_Control.Socket);
				return(FALSE);
			}
		}else{
			// 接続されている

			break;
		}

		POKE_NET_LockMutex();
		if( POKE_NET_Control.AbortFlag == TRUE ){
			POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET connect abort:%08X\n" ,POKE_NET_Control.Socket);
			POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_ABORTED;
			POKE_NET_UnlockMutex();
			return(FALSE);
		}
		POKE_NET_UnlockMutex();

		POKE_NET_Sleep(1);
	}

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET connected socket:%08X\n" ,POKE_NET_Control.Socket);
	return(TRUE);
}

//====================================
//          送信中
//====================================
BOOL POKE_NET_Sending()
{
	int ret;
	fd_set fdset; 
	struct timeval timeout;
	int sret;
	long sizetmp;
	long offset;
	unsigned long starttime;
	unsigned long timetmp;

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET send to %s socket:%08X size:%d\n" ,POKE_NET_Control.URL ,POKE_NET_Control.Socket ,POKE_NET_Control.SendSize);
	starttime = GetTickCount();
	offset = 0;
	for(;;){
		sizetmp = POKE_NET_Control.SendSize - offset;									// 残りサイズ計算
		if( sizetmp > POKE_NET_Control.SendSize )sizetmp = POKE_NET_Control.SendSize;	// 一度に送る量をこえたら補正

#ifdef ___COMMUNICATES_BY_USING_SSL___
		// ブロッキングしないで書き込めるデータがあるか調べる
		FD_ZERO( &fdset ); 
		FD_SET( POKE_NET_Control.Socket , &fdset ); 
		timeout.tv_sec = 0; 
		timeout.tv_usec = 0; 
		sret = select( POKE_NET_Control.Socket+1 ,NULL ,&fdset ,NULL ,&timeout ); 
		if( sret && (sret != -1) ){
			// SSL関係
			ret = SSL_write(POKE_NET_Control.m_pSSL, (char *)&POKE_NET_Control.SendBuffer[offset], sizetmp);	// 送信
			if(ret <= 0)
			{
				long errcode = SSL_get_error(POKE_NET_Control.m_pSSL, ret);
				if(errcode != SSL_ERROR_WANT_READ && errcode != SSL_ERROR_WANT_WRITE)
				{
					// < 0:送信エラー
					// = 0:切断された

					// 受信失敗を起こした場合はDisconnectしてTRUE
					// -----------------------------------------
					// = ログ
					// -----------------------------------------
					POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
						"#POKE_NET send error socket:0x%08X, ret = %d, SSL_ERROR = %d\n" ,
						POKE_NET_Control.Socket,
						ret,
						errcode
					);

					ERR_print_errors_fp(stderr);

					POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_SENDTIMEOUT;
					return(FALSE);
				}
			}
#else
		// ブロッキングしないで書き込めるデータがあるか調べる
		FD_ZERO( &fdset ); 
		FD_SET( POKE_NET_Control.Socket , &fdset ); 
		timeout.tv_sec = 0; 
		timeout.tv_usec = 0; 
		sret = select( POKE_NET_Control.Socket+1 ,NULL ,&fdset ,NULL ,&timeout ); 
		if( sret && (sret != -1) ){
			ret = send(POKE_NET_Control.Socket ,(char *)&POKE_NET_Control.SendBuffer[offset] ,sizetmp ,0 );	// 送信
			if( (ret == SOCKET_ERROR) || (ret == 0) ){
				// 送信エラー、もしくは切断された
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET send error socket:%08X\n" ,POKE_NET_Control.Socket);
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_SENDTIMEOUT;
				return(FALSE);
			}
#endif

			// 送った用量補正
			offset += ret;
			if( offset == POKE_NET_Control.SendSize ){
				// すべてのデータを送り終えた
				break;
			}
		}

		if( POKE_NET_Control.SendTimeOut != 0 ){
			// タイムアウト設定あり
			timetmp = GetTickCount();
			if( (signed)(timetmp - starttime) >= (POKE_NET_Control.SendTimeOut*1000) ){
				// タイムアウト
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET send timeout socket:%08X\n" ,POKE_NET_Control.Socket);
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_SENDTIMEOUT;
				return(FALSE);
			}
		}

		POKE_NET_LockMutex();
		if( POKE_NET_Control.AbortFlag == TRUE ){
			POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET send abort socket:%08X\n" ,POKE_NET_Control.Socket);
			POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_ABORTED;
			POKE_NET_UnlockMutex();
			return(FALSE);
		}
		POKE_NET_UnlockMutex();

		POKE_NET_Sleep(1);
	}

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET sent socket:%08X\n" ,POKE_NET_Control.Socket);
	return(TRUE);
}

//====================================
//          受信中
//====================================
BOOL POKE_NET_Receiving()
{
	int ret;
	fd_set fdset; 
	struct timeval timeout;
	int sret;
	long offset;
	long size;
	unsigned long starttime;
	unsigned long timetmp;

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET recv from %s socket:%08X\n" ,POKE_NET_Control.URL ,POKE_NET_Control.Socket);
	starttime = GetTickCount();
	offset = 0;
	size = -1;
	for(;;){
		// ブロッキングしないで読み込めるデータがあるか調べる
		FD_ZERO( &fdset ); 
		FD_SET( POKE_NET_Control.Socket , &fdset ); 
		timeout.tv_sec = 0; 
		timeout.tv_usec = 0; 
		sret = select( POKE_NET_Control.Socket+1 ,&fdset ,NULL ,NULL ,&timeout ); 
		if( sret && (sret != -1) ){
			// 読むべきデータがある(もしくはクローズされている)
			// 受信

#ifdef ___COMMUNICATES_BY_USING_SSL___
			// SSL関係
			ret = SSL_read(POKE_NET_Control.m_pSSL, (char *)&POKE_NET_Control.RecvBuffer[offset], POKE_NET_Control.RecvSize-offset);

			if(ret <= 0)
			{
				long errcode = SSL_get_error(POKE_NET_Control.m_pSSL, ret);
				if(errcode != SSL_ERROR_WANT_READ && errcode != SSL_ERROR_WANT_WRITE)
				{
					// 受信失敗を起こした場合はDisconnectしてTRUE

					// -----------------------------------------
					// = ログ
					// -----------------------------------------
					POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,
						"#POKE_NET recv error socket:0x%08X, ret = %d, SSL_ERROR = %d\n" ,
						POKE_NET_Control.Socket,
						ret,
						errcode
					);

					fprintf(stderr, "err:");
					ERR_print_errors_fp(stderr);
					fprintf(stderr, "\n");

					POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_RECVRESPONSE;
					return(FALSE);
				}
			}
#else
			ret = recv(POKE_NET_Control.Socket ,(char *)&POKE_NET_Control.RecvBuffer[offset] ,POKE_NET_Control.RecvSize-offset ,0 );

			if( ret == SOCKET_ERROR ){
				// エラー：受信エラー
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET recv error socket:%08X\n" ,POKE_NET_Control.Socket);
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_RECVRESPONSE;
				return(FALSE);
			}else if( ret == 0 ){
				// 切断された：受信エラー
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET recv disconnected socket:%08X\n" ,POKE_NET_Control.Socket);
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_RECVRESPONSE;
				return(FALSE);
			}
#endif
			else{
				// 何バイトか受け取った
				offset += ret;
				if( (offset >= sizeof(long)) && (size == -1) ){
					// 容量を受け取った：初回のみ用量セット
					memcpy(&size ,POKE_NET_Control.RecvBuffer ,sizeof(long));
				}
				if( (offset == size) && (size != -1) ){
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
					//// 指定容量受け取った：複合化
					//POKE_NET_OptDecode(
					//	(unsigned char *)&POKE_NET_Control.RecvBuffer[sizeof(long)] ,
					//	offset - sizeof(long) ,
					//	(unsigned char *)&POKE_NET_Control.RecvBuffer[sizeof(long)]);

					// サイズ補正
					POKE_NET_Control.RecvSize = size - sizeof(long);
					break;
				}
			}
		}

		if( POKE_NET_Control.RecvTimeOut != 0 ){
			// タイムアウト設定あり
			timetmp = GetTickCount();
			if( (signed)(timetmp - starttime) >= (POKE_NET_Control.RecvTimeOut*1000) ){
				// タイムアウト
				POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET recv timeout socket:%08X\n" ,POKE_NET_Control.Socket);
				POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_RECVTIMEOUT;
				return(FALSE);
			}
		}

		POKE_NET_LockMutex();
		if( POKE_NET_Control.AbortFlag == TRUE ){
			POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET recv abort socket:%08X\n" ,POKE_NET_Control.Socket);
			POKE_NET_Control.LastErrorCode = POKE_NET_LASTERROR_ABORTED;
			POKE_NET_UnlockMutex();
			return(FALSE);
		}
		POKE_NET_UnlockMutex();

		POKE_NET_Sleep(1);
	}

	POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_LEVEL1 ,"#POKE_NET recved socket:%08X size:%d\n" ,POKE_NET_Control.Socket ,POKE_NET_Control.RecvSize);
	return(TRUE);
}


//====================================
//          受信中
//====================================
void POKE_NET_Sleep(long _ms)
{
	Sleep(_ms);
}
//=====================================
//		SVL取得メインルーチン
//=====================================
BOOL POKE_NET_GetSvl()
{
	return(TRUE);
}

//=====================================
//		SSL 初期化
//=====================================
void POKE_NET_SSL_Initialize()
{
}

//=====================================
//		SSL クリア処理
//=====================================
void POKE_NET_SSL_Clear()
{
#ifdef ___COMMUNICATES_BY_USING_SSL___
	POKE_NET_Control.m_pSSL = NULL;
	POKE_NET_Control.m_pSSL_CTX = NULL;
#endif
}

#endif	// ___POKE_NET_BUILD_WINDOWS___
