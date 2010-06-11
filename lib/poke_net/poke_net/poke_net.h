/*===========================================================================*
  Project:  Syachi PokemonGDS Network Library
  File:     poke_net.h

  $NoKeywords: $
 *===========================================================================*/
//===========================================================================

#ifndef ___POKE_NET___
#define ___POKE_NET___

//==============================================
//                SSL 定義
//==============================================
#define ___COMMUNICATES_BY_USING_SSL___	 // SSL通信を利用しない場合は、コメントアウトしてください。

// アドレスによらず SSL を使用する
// コメントアウトした場合、アドレス先が <https> で始まる
// 場合に SSL を使用する
// ___COMMUNICATES_BY_USING_SSL___ が未定義の場合は無効
#define ___ALWAYS_ENABLE_SSL___


//==============================================
//                インクルード
//==============================================
//------------------------------------
//               共通
//------------------------------------
#include "poke_net_dscomp.h"					// LINUX/WINDOWS/DSのtypedef吸収

//------------------------------------
//             WINDOW特有
//------------------------------------
#ifdef ___POKE_NET_BUILD_WINDOWS___
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <locale.h>
#include "../openssl\openssl\ssl.h"
#include "../openssl\openssl\err.h"
#include "../openssl\openssl\x509.h"

#pragma comment(lib, "../openssl/lib/libeay32MTd.lib")
#pragma comment(lib, "../openssl/lib/ssleay32MTd.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

//------------------------------------
//               DS特有
//------------------------------------
#ifdef ___POKE_NET_BUILD_DS___
#include <nitro.h>
#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/thread.h>
//#include <ninet/ip.h>
#include <twlWiFi.h>
#include <dwc.h>
// 以下は NITRO ではライブラリ内に定義されているが、
// TWL では定義されていないので、ここに定義しておく
// 定義方法は NITRO と同じ
// NitroDWC/include/gs/common/gsPlatformSocket.h を参考
typedef int SOCKET;
#ifndef INVALID_SOCKET 
	#define INVALID_SOCKET (-1)
#endif
// 有効にすると受信部分をスレッドで分離して処理します
#define RECEIVE_THREAD
#endif

//------------------------------------
//            LINUX特有
//------------------------------------
#ifdef ___POKE_NET_BUILD_LINUX___
/*
#include "gds_header/attr_def.h"				// バトルデータ定義
#include "gds_header/battle_type.h"				// バトルデータ戦闘タイプ
#include "gds_header/trtype_def.h"				// トレーナータイプ定義
#include "gds_header/trtype_sex.h"				// トレーナー性別定義
*/
#include <openssl/ssl.h>
#include <openssl/err.h>

#endif

//------------------------------------
//               共通
//------------------------------------
#include "poke_net_common.h"					// Windows/Linux/DS共通ヘッダ
#include "poke_net_opt.h"						// データの暗号化、複合化


//==============================================
//                 置き換え文字
//==============================================
//------------------------------------
//             WINDOW特有
//------------------------------------
#ifdef ___POKE_NET_BUILD_WINDOWS___
#define POKE_NET_CONNECT_TIMEOUT	15			// コネクトタイムアウト(sec)
#define POKE_NET_SEND_TIMEOUT		30			// 送信タイムアウト(sec)
#define POKE_NET_RECV_TIMEOUT		30			// 受信タイムアウト(sec)
#endif

//------------------------------------
//               DS特有
//------------------------------------
#ifdef ___POKE_NET_BUILD_DS___
#define POKE_NET_CONNECT_TIMEOUT	15			// コネクトタイムアウト(sec)
#define POKE_NET_SEND_TIMEOUT		30			// 送信タイムアウト(sec)
#define POKE_NET_RECV_TIMEOUT		30			// 受信タイムアウト(sec)
#endif
//------------------------------------
//             LINUX特有
//------------------------------------
#ifdef ___POKE_NET_BUILD_LINUX___
#define POKE_NET_ACCEPT_TIMEOUT		15			// SSL_acceptタイムアウト(sec)
#define POKE_NET_RECV_TIMEOUT		40			// 受信タイムアウト(sec)
#define POKE_NET_SEND_TIMEOUT		40			// 送信タイムアウト(sec)
#endif

//------------------------------------
//               共通
//------------------------------------
#ifndef ___POKE_NET_BUILD_LINUX___

//! ポケモン WiFi ライブラリの現在の状態
typedef enum {
	POKE_NET_STATUS_INACTIVE ,					//!< 非稼動中
	POKE_NET_STATUS_INITIALIZED ,				//!< 初期化済み
	POKE_NET_STATUS_REQUEST ,					//!< リクエスト発行
	POKE_NET_STATUS_NETSETTING ,				//!< ネットワーク設定中
	POKE_NET_STATUS_CONNECTING ,				//!< 接続中
	POKE_NET_STATUS_SENDING ,					//!< 送信中
	POKE_NET_STATUS_RECEIVING ,					//!< 受信中
	POKE_NET_STATUS_ABORTED ,					//!< 中断
	POKE_NET_STATUS_FINISHED ,					//!< 正常終了
	POKE_NET_STATUS_ERROR ,						//!< エラー終了
	POKE_NET_STATUS_COUNT
} POKE_NET_STATUS;

//! ポケモン WiFi ライブラリの最終エラーコード
typedef enum {
	POKE_NET_LASTERROR_NONE ,					//!< 特になし
	POKE_NET_LASTERROR_NOTINITIALIZED ,			//!< 初期化されていない
	POKE_NET_LASTERROR_ILLEGALREQUEST ,			//!< リクエストが異常
	POKE_NET_LASTERROR_CREATESOCKET ,			//!< ソケットが作れなかった
	POKE_NET_LASTERROR_IOCTRLSOCKET ,			//!< ソケットを非同期にできなかった
	POKE_NET_LASTERROR_NETWORKSETTING ,			//!< 接続先など取得エラー
	POKE_NET_LASTERROR_CREATETHREAD ,			//!< スレッドを作れなかった
	POKE_NET_LASTERROR_CONNECT ,				//!< 接続エラー
	POKE_NET_LASTERROR_SENDREQUEST ,			//!< リクエストエラー
	POKE_NET_LASTERROR_RECVRESPONSE ,			//!< レスポンスエラー
	POKE_NET_LASTERROR_CONNECTTIMEOUT ,			//!< 接続タイムアウト
	POKE_NET_LASTERROR_SENDTIMEOUT ,			//!< 送信タイムアウト
	POKE_NET_LASTERROR_RECVTIMEOUT ,			//!< 受信タイムアウト
	POKE_NET_LASTERROR_ABORTED ,				//!< 中断終了
	POKE_NET_LASTERROR_GETSVL ,					//!< SVL取得エラー
	POKE_NET_LASTERROR_SSL_SETTING ,			//!< SSL セットアップ関連のエラー
	POKE_NET_LASTERROR_COUNT
} POKE_NET_LASTERROR;

//! ポケモン WiFi ライブラリのデバッグメッセージモード
typedef enum 
{
	POKE_NET_DEBUGLEVEL_NOT,					//!< 非デバッグモード
	POKE_NET_DEBUGLEVEL_NORMAL,					//!< 通常デバッグモード
	POKE_NET_DEBUGLEVEL_LEVEL1,					//!< 詳細デバッグモード
	POKE_NET_DEBUGLEVEL_LEVEL2					//!< 超詳細デバッグモード
}
POKE_NET_DEBUGLEVEL;

#define POKE_NET_CURRENT_DEBUGLEVEL	POKE_NET_DEBUGLEVEL_LEVEL1		// デバッグレベル設定

#endif // ___POKE_NET_BUILD_LINUX___

//==============================================
//              クラス/構造体定義
//==============================================
#ifndef ___POKE_NET_BUILD_LINUX___
typedef struct poke_net_control {
	POKE_NET_REQUESTCOMMON_AUTH Auth;		// クライアント情報
	BOOL SvlToken;							// サービスロケーショントークンがセットされているか
	POKE_NET_STATUS Status;					// ステータス(POKE_NET_STATUS_xxxx)
	char URL[256];							// 接続先サーバーＵＲＬ
	unsigned short PortNo;					// ポート番号
	long ConnectTimeOut;					// コネクトタイムアウト時間(sec)
	long SendTimeOut;						// 送信タイムアウト時間(sec)
	long RecvTimeOut;						// 受信タイムアウト時間(sec)
	POKE_NET_LASTERROR LastErrorCode;		// 最終エラーコード(POKE_NET_LASTERROR_xxxx)
	long SendSize;							// 送信サイズ
	char *SendBuffer;						// 送信バッファ(ライブラリで用意)
	long RecvSize;							// 受信サイズ
	char *RecvBuffer;						// 受信バッファ(ユーザーで用意)
	unsigned long IP;						// 接続先ＩＰ
	BOOL AbortFlag;							// アボート要請
	SOCKET Socket;							// ソケット
	unsigned long ThreadLevel;				// スレッドレベル
#ifdef ___POKE_NET_BUILD_WINDOWS___			// == WINDOWS特有 ==
	int ThreadID;							// スレッドＩＤ
	HANDLE MutexHandle;						// ミューテクスハンドル
		// SSL関係
#ifdef ___COMMUNICATES_BY_USING_SSL___
	// 確立した接続毎にサーバかクライアントによって作られる主な SSL/TLS 構成です。
	// これは実際には SSL API では中核となる構成です。
	SSL* m_pSSL;
#endif	// ___COMMUNICATES_BY_USING_SSL___
#endif  // ___POKE_NET_BUILD_WINDOWS___
#ifdef ___POKE_NET_BUILD_DS___				// == DS特有 ==
	OSThread ThreadID;						// スレッドＩＤ
	OSMutex MutexHandle;					// ミューテクスハンドル
#ifdef RECEIVE_THREAD
    OSThread RecvThread;                    // 受信処理用スレッド
    int      RecvResult;                    // 受信処理用戻り値
    int      RecvOffset;                    // 受信処理用オフセット値
    BOOL     RecvFlag;                      // 受信フラグ
#endif  // RECEIVE_THREAD
#ifdef ___COMMUNICATES_BY_USING_SSL___
    SOCSslConnection sslConnection;         // SSL コネクション
#endif // ___COMMUNICATES_BY_USING_SSL___
#endif // ___POKE_NET_BUILD_DS___
} POKE_NET_CONTROL;
#endif // ___POKE_NET_BUILD_LINUX___

#ifndef	___POKE_NET_BUILD_LINUX___
	
#endif //___POKE_NET_BUILD_LINUX___

//==============================================
//              関数定義
//==============================================
#ifndef ___POKE_NET_BUILD_LINUX___
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// -- メモリ確保 --
void *POKE_NET_AllocFunc( long _size );										// メモリ確保
void POKE_NET_FreeFunc( void *_ptr  );										// 解放

// -- ミューテクス操作 --
BOOL POKE_NET_CreateMutex();												// ミューテクス作成
BOOL POKE_NET_ReleaseMutex();												// ミューテクス解放
void POKE_NET_LockMutex();													// ミューテクスロック
void POKE_NET_UnlockMutex();												// ミューテクスアンロック

// -- スレッドハンドラ --
BOOL POKE_NET_CreateThread();												// スレッド作成
void POKE_NET_ThreadFunction();												// スレッド処理


// -- ネットワーク関係 --
void POKE_NET_ControlInitialize();											// 制御情報初期化
void POKE_NET_ControlRelease();												// 制御情報開放

BOOL POKE_NET_Initialize(const char *_url ,unsigned short _portno ,POKE_NET_REQUESTCOMMON_AUTH *_auth);	// 初期化
void POKE_NET_SetThreadLevel(unsigned long _level);							// スレッドレベル設定
void POKE_NET_Clear();														// 再度リクエストが出せるようクリア
void POKE_NET_Release();													// 解放
BOOL POKE_NET_ExInitialize();												// 機種特有初期化
void POKE_NET_ExRelease();													// 機種特有解放

BOOL POKE_NET_Request(unsigned short _reqno ,void *_param ,void *_response ,long _size);// リクエスト発行
BOOL POKE_NET_Abort();														// 通信中断

BOOL POKE_NET_GetSvl();														// SVL取得
BOOL POKE_NET_Setting();													// セッティング中
BOOL POKE_NET_Connecting();													// 接続中
BOOL POKE_NET_Sending();													// 送信中
BOOL POKE_NET_Receiving();													// 受信中
void POKE_NET_CloseSocket();												// ソケット解放

void *POKE_NET_GetResponse();												// 返答取得
long POKE_NET_GetResponseSize();											// 返答サイズ取得
POKE_NET_STATUS POKE_NET_GetStatus();										// 現在のステータス取得
POKE_NET_LASTERROR POKE_NET_GetLastErrorCode();								// 最終エラーコード取得

// -- SSL 関係 --
void POKE_NET_SSL_Initialize();
void POKE_NET_SSL_Clear();

void POKE_NET_Sleep(long _ms);												// スリープ

#ifdef SDK_FINALROM
#define POKE_NET_DebugPrintf(_level , ...)		(void)(0)
#define POKE_NET_DebugPrintfW(_level , ...)		(void)(0)
#else
void POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL _level ,char *_format ,...);	// デバッグプリント
void POKE_NET_DebugPrintfW(POKE_NET_DEBUGLEVEL _level ,wchar_t *_format ,...);	// デバッグプリント(ワイド文字対応)
#endif

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ___POKE_NET_BUILD_LINUX___

#endif // ___POKE_NET___
