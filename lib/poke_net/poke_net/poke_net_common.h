/*===========================================================================*
  Project:  Syachi PokemonGDS Network Library
  File:     poke_net_common.h

  $NoKeywords: $
 *===========================================================================*/
/*! @file
	@brief	Syachi Pokemon GDS WiFi ライブラリ

*/
//===========================================================================
#ifndef ___POKE_NET_GDS_COMMON___
#define ___POKE_NET_GDS_COMMON___


#include "poke_net_dscomp.h"				// LINUX/WINDOWS/DSのtypedef吸収
#include "gds_header/gds_define.h"
#include "gds_header/gds_profile_local.h"	// プロフィール関係
#include "gds_header/gds_battle_rec_sub.h"	// バトルビデオデータ関係
#include "gds_header/gds_battle_rec.h"		// バトルビデオデータ関係
#include "gds_header/gds_musical_sub.h"		// ミュージカルショット関係
#include "gds_header/gds_musical.h"			// ミュージカルショット関係
#include "gds_header/gds_min_max.h"


//===============================================
//                時間パック
//===============================================
#define POKE_NET_TIMETOU64(Y,M,D,h,m,s)	(u64)((((u64)Y) << 40) | (((u64)M) << 32) | (((u64)D) << 24) | (((u64)h) << 16) | (((u64)m) << 8) | (((u64)s) << 0))


//===============================================
//!          ＧＤＳリクエストコード
//===============================================
enum POKE_NET_GDS_REQCODE
{
	// ------------------------- 以下デバッグリクエスト ----------------------
	POKE_NET_GDS_REQCODE_DEBUG_START							= 0,	// ※以下デバッグで使用するリクエスト
	POKE_NET_GDS_REQCODE_DEBUG_MESSAGE							= 0,	//!< デバッグメッセージ

	POKE_NET_GDS_REQCODE_DEBUG_END								= 9999,	// ※9999まではデバッグで仕様するリクエスト

#ifndef ___POKE_NET_BUILD_DS___
	// -------------------------- 以下管理者リクエスト -----------------------
	POKE_NET_GDS_REQCODE_ADMIN_START							= 10000,// ※10000以降は管理者が使用できるリクエスト
	POKE_NET_GDS_REQCODE_ADMIN_SVR_STATUS						= 10000,// サーバーステータス取得
	POKE_NET_GDS_REQCODE_ADMIN_SVR_COMMAND,								// サーバーへの命令

	POKE_NET_GDS_REQCODE_ADMIN_MUSICALSHOT_STATUS				= 11000,// ミュージカルショット状態取得
	POKE_NET_GDS_REQCODE_ADMIN_MUSICALSHOT_DELETE,						// ミュージカルショット削除

	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_STATUS				= 12000,// バトルデータ状態取得
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_DELETE,						// バトルデータ削除
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_RANKHISTORY,					// バトルデータランキング履歴取得
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_EVENTFLAG,					// バトルデータイベント登録要求
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_EVENTFREEWORD,				// イベントデータの改変
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_GETPARAM,						// パラメータ取得
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_SETPARAM,						// パラメータ設定
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_EXECDECISION,					// 確定処理実行(強制)

	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_START				= 13000,
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_ROMCODELIST_GET,			// 不正チェック:許可ロムコード取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_ROMCODELIST_SET,			// 不正チェック:許可ロムコードセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_NGWORD_GET,					// 不正チェック:ＮＧワード取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_NGWORD_SET,					// 不正チェック:ＮＧワードセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_GET,		// 不正チェック:ミュージカルショットの許可タイトルリストの取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_SET,		// 不正チェック:ミュージカルショットの許可タイトルリストのセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_GET,	// 不正チェック:ミュージカルショットの不許可ポケモン番号リストの取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_SET,	// 不正チェック:ミュージカルショットの不許可ポケモン番号リストのセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_MAXLIST_GET,				// 不正チェック:各種最大値パラメータの取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_MAXLIST_SET,				// 不正チェック:各種最大値パラメータのセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_GET,	// 不正チェック:許可されたバトルモード値リストの取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_SET,	// 不正チェック:許可されたバトルモード値リストのセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_GET,		// 不正チェック:許可されたバトルバージョンリストの取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_SET,		// 不正チェック:許可されたバトルバージョンリストのセット
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_DFTRAINERNAME_GET,			// 不正チェック:デフォルトトレイナー名取得
	POKE_NET_GDS_REQCODE_ADMIN_ILLEGALCHECK_DFTRAINERNAME_SET,			// 不正チェック:デフォルトトレイナー名設定

	POKE_NET_GDS_REQCODE_ADMIN_DHOCHECKLOG_GET					= 14000,// 不正チェックログ:取得

	POKE_NET_GDS_REQCODE_ADMIN_END								= 19999,// ※19999までは管理者が使用できるリクエスト
#endif

	// ------------------------- 以下ユーザーリクエスト ----------------------
	POKE_NET_GDS_REQCODE_USER_START								= 20000,// ※20000以降は一般ユーザーが使えるリクエスト
	POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST						= 21000,//!< ミュージカルショットアップロード
	POKE_NET_GDS_REQCODE_MUSICALSHOT_GET,								//!< ミュージカルショット取得

	POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST						= 22000,//!< バトルデータアップロード
	POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH,								//!< バトルデータ検索
	POKE_NET_GDS_REQCODE_BATTLEDATA_GET,								//!< バトルデータ取得
	POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE,							//!< バトルデータお気に入り登録

	POKE_NET_GDS_REQCODE_LAST
};


// 認証トークン長
#define	POKE_NET_GDS_REQUESTCOMMON_AUTH_SVLTOKEN_LENGTH (304)

//===============================================
//! ポケモンWiFiライブラリ リクエスト認証情報
//===============================================
typedef struct 
{
	s32		PID;								//!< プロファイルＩＤ
	u8		ROMCode;							//!< バージョンコード
	u8		LangCode;							//!< 言語コード
	char	SvlToken[POKE_NET_GDS_REQUESTCOMMON_AUTH_SVLTOKEN_LENGTH];	//!< サービスロケータトークン
	u16		Dummy;
} 
POKE_NET_REQUESTCOMMON_AUTH;


//===============================================
//!  ポケモンWiFiライブラリ リクエストヘッダ
//===============================================
typedef struct 
{
	unsigned short ReqCode;						//!< リクエストコード(POKE_NET_GDS_REQUEST_REQCODE_xxxx)
	unsigned short Option;						//!< リクエストオプション
	POKE_NET_REQUESTCOMMON_AUTH Auth;			//!< ユーザー認証情報
	unsigned char Param[0];						//!< パラメータ(各リクエストの構造体)
}
POKE_NET_REQUEST;


//===============================================
//!  ポケモンWiFiライブラリ レスポンスヘッダ
//===============================================
typedef struct
{
	unsigned short ReqCode;						//!< リクエストコード(POKE_NET_GDS_REQUEST_REQCODE_xxxx)
	unsigned short Result;						//!< レスポンス結果(POKE_NET_GDS_RESPONSE_RESULT_xxxx)
	unsigned char Param[0];						//!< パラメータ(各レスポンスの構造体)
}
POKE_NET_RESPONSE;



//===============================================
//       リクエスト/レスポンス構造体
//===============================================

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//      デバッグリクエスト/レスポンス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//------------------------------------
//         デバッグメッセージ
//------------------------------------
//! ＧＤＳデバッグメッセージ結果
enum POKE_NET_GDS_RESPONSE_RESULT_DEBUG_MESSAGE 
{
	POKE_NET_GDS_RESPONSE_RESULT_DEBUG_MESSAGE_SUCCESS ,				//!< 登録成功
	POKE_NET_GDS_RESPONSE_RESULT_DEBUG_MESSAGE_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳデバッグメッセージリクエスト
typedef struct
{
	char Message[1024];						//!< デバッグメッセージ
}
POKE_NET_GDS_REQUEST_DEBUG_MESSAGE;

//! ＧＤＳデバッグメッセージレスポンス
typedef struct
{
	char ResultMessage[1024];				//!< デバッグメッセージ
}
POKE_NET_GDS_RESPONSE_DEBUG_MESSAGE;


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//        ユーザーリクエスト/レスポンス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//------------------------------------
//     ミュージカルショット登録
//------------------------------------

//! ＧＤＳミュージカルショット登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST 
{
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_SUCCESS,				//!< 登録成功
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_AUTH,				//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ALREADY,			//!< すでに登録されている
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ILLEGAL,			//!< 不正なデータ
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ILLEGALPOKEMON,	//!< 不正なポケモン番号
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ILLEGALPROFILE,	//!< 不正なユーザープロフィール
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳミュージカルショット登録リクエスト
typedef struct 
{
	MUSICAL_SHOT_SEND Data;							//!< ミュージカルショット送信データ
}
POKE_NET_GDS_REQUEST_MUSICALSHOT_REGIST;

//! ＧＤＳミュージカルショット登録レスポンス
typedef struct 
{
	u64 Code;										//!< 登録されたミュージカルショットのコード
} 
POKE_NET_GDS_RESPONSE_MUSICALSHOT_REGIST;


//------------------------------------
//     ミュージカルショット取得
//------------------------------------

// クライアントが受け取るミュージカルショット最大数
#define	POKE_NET_GDS_RESPONSE_MUSICALSHOT_GET_MAX	(5)				

//! ＧＤＳミュージカルショット取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET 
{
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_SUCCESS,				//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_ERROR_AUTH,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_ERROR_ILLEGALPOKEMON,	//!< ポケモン番号エラー
	POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳミュージカルショット取得タイプ
enum POKE_NET_GDS_REQUEST_MUSICALSHOT_GETTYPE 
{
	POKE_NET_GDS_REQUEST_MUSICALSHOT_GETTYPE_POKEMON,				//!< ポケモン番号で最新取得(SearchParamにポケモン番号)
	POKE_NET_GDS_REQUEST_MUSICALSHOT_GETTYPE_CODE,					//!< コード直接指定で取得
	POKE_NET_GDS_REQUEST_MUSICALSHOT_GETTYPE_PID					//!< PIDによる取得(管理用)
};

//! ＧＤＳミュージカルショット取得リクエスト
typedef struct
{
	unsigned short SearchType;										//!< 検索タイプ(POKE_NET_GDS_REQUEST_MUSICALSHOT_GETTYPE_xxxx)
	unsigned short SearchOpt;										//!< 検索オプション(現在未使用)
	u64 SearchParam;												//!< 検索パラメータ
} 
POKE_NET_GDS_REQUEST_MUSICALSHOT_GET;

//! ＧＤＳミュージカルショット取得レスポンス内容
typedef struct 
{
	long PID;														//!< アップロード者のプロファイルＩＤ
	u64 Code;														//!< ミュージカルショットの管理コード
	MUSICAL_SHOT_RECV Data;											//!< ミュージカルショット受信データ
} 
POKE_NET_GDS_RESPONSE_MUSICALSHOT_RECVDATA;

//! ＧＤＳミュージカルショット取得レスポンス
typedef struct 
{
	long HitCount;													//!< ヒット件数
	POKE_NET_GDS_RESPONSE_MUSICALSHOT_RECVDATA Data[0];				//!< ミュージカルショット受信データ
} 
POKE_NET_GDS_RESPONSE_MUSICALSHOT_GET;


//------------------------------------
//       バトルデータ登録
//------------------------------------

//! ＧＤＳバトルデータ登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST 
{
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_SUCCESS ,					//!< 登録成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_AUTH ,					//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ALREADY ,				//!< すでに登録されている
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL ,				//!< 不正なデータ
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL_RANKINGTYPE,	//!< 不正なランキング種別
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPROFILE ,		//!< 不正なユーザープロフィール
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPOKEMON_VERIFY, //!< ポケモン署名でエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_UNKNOWN				//!< その他エラー
};

// 署名データ用バッファの最大サイズ
#define	POKE_NET_GDS_REQUEST_BATTLEDATA_SIGN_MAXSIZE	(256)


//! ＧＤＳバトルデータ登録リクエスト
typedef struct
{
	BATTLE_REC_SEND Data;														//!< バトルデータ送信データ
	long			SignSize;													//!< ポケモンチェック署名データサイズ
	u8				SignModule[POKE_NET_GDS_REQUEST_BATTLEDATA_SIGN_MAXSIZE];	//!< ポケモンチェック署名データ
} 
POKE_NET_GDS_REQUEST_BATTLEDATA_REGIST;

//! ＧＤＳバトルデータ登録レスポンス
typedef struct
{
	u64 Code;																//!< 登録されたバトルデータのコード
} 
POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST;


//------------------------------------
//      バトルデータリスト取得
//------------------------------------

//! ＧＤＳバトルデータリスト取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH 
{
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESS ,			//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_ILLEGALPARAM ,	//!< 検索パラメータエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_ILLEGALRANKINGTYPE ,//!< 検索パラメータエラー(ランキング種別が不正)
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_UNKNOWN		//!< その他エラー
};

//! ＧＤＳバトルデータリスト取得タイプ
enum POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_CONDITION,				//!< 条件指定で取得
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_LATEST,			//!< 最新30件ランキング
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_SUBWAY,			//!< バトルサブウェイランキング
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_COMM,			//!< 通信対戦ランキング
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_PID,						// PIDによる取得(管理用)
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_EVENT					// イベント領域
};

//! ＧＤＳバトルデータリスト取得リクエスト
typedef struct
{
	unsigned short SearchType;											//!< 検索タイプ(POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_xxxx)
	unsigned short SearchOpt;											//!< 検索オプション(現在未使用)
	BATTLE_REC_SEARCH_SEND ConditionParam;								//!< 複合検索パラメータ
	u64 SearchParam;													//!< 検索パラメータ
}
POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH;

// クライアントが受け取るバトルデータヘッダ最大数
#define	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_MAXNUM	(30)


//! ＧＤＳバトルデータリスト取得レスポンス内容
typedef struct 
{
	long PID;															//!< アップロード者のプロファイルＩＤ
	u64 Code;															//!< バトルデータコード
	BATTLE_REC_OUTLINE_RECV Data;										//!< バトルデータヘッダ
}
POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA;

//! ＧＤＳバトルデータリスト取得レスポンス
typedef struct 
{
	long HitCount;														//!< ヒット件数
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA Data[0];				//!< バトルデータリスト
}
POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH;


//------------------------------------
//      バトルデータ取得
//------------------------------------

//! ＧＤＳバトルデータ取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET 
{
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_SUCCESS,				//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_AUTH,				//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_ILLEGALCODE,		//!< コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_ILLEGALVERSION,	//!< バージョンエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳバトルデータ取得リクエスト
typedef struct 
{
	u64 Code;															//!< バトルデータコード
	u32 ServerVersion;													//!< 取得対象バトルデータのサーバーバージョン
}
POKE_NET_GDS_REQUEST_BATTLEDATA_GET;

//! ＧＤＳバトルデータ取得レスポンス
typedef struct
{
	long PID;															//!< アップロード者のプロファイルＩＤ
	u64 Code;															//!< バトルデータコード
	BATTLE_REC_RECV Data;												//!< バトルデータ
} 
POKE_NET_GDS_RESPONSE_BATTLEDATA_GET;


//------------------------------------
//      バトルデータお気に入り登録
//------------------------------------

//! ＧＤＳバトルデータお気に入り登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE 
{
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_SUCCESS ,				//!< お気に入り登録成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_ILLEGALCODE ,	//!< コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳバトルデータお気に入り登録リクエスト
typedef struct 
{
	u64 Code;																//!< バトルデータコード
}
POKE_NET_GDS_REQUEST_BATTLEDATA_FAVORITE;

// レスポンスはコードのみなので、構造体の定義無し


#ifndef ___POKE_NET_BUILD_DS___
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//        管理者リクエスト/レスポンス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//------------------------------------
//        サーバー状態取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS_SUCCESS,		// 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS_ERROR_UNKNOWN	// その他エラー
};

enum 
{
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_STOPPED,	// 停止状態
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_ACTIVE,	// 稼動状態
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_SHUTDOWN,	// シャットダウン
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_UNKNOWN	// 不明状態(応答なし)
};


// 最大サーバー数
#define POKE_NET_GDS_RESPONSE_ADMIN_SVR_MAXNUM	(256)		


enum 
{
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_SVRTYPE_ADMIN,	// 管理サーバー
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_SVRTYPE_SLAVE	// 通常サーバー
};

// -- リクエスト構造体 --
// なし

// -- レスポンス構造体 --
typedef struct 
{
	long			Type;				// サーバー種(POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_SVRTYPE_xxxx)
	long			Status;				// ステータス(POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_xxxxx)
	long			MaxConnectCount;	// 最大コネクション可能数
	long			ConnectCount;		// コネクション数
	u64				LastUpdateTime;		// 最終ステータス更新時間(0xYYYYMMDDHHMMSS)
	unsigned long	IP;					// サーバーIPアドレス
	long			Request;			// サーバーへ送っているリクエストコード
}
POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA;


typedef struct 
{
	long ServerCount;										// サーバー数
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA Status[0];	// 各サーバーのステータス
}
POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUS;


//------------------------------------
//        サーバーへの命令
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_SUCCESS,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_AUTH,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ILLEGALIP,		// 不正なサーバーＩＰアドレス
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ILLEGALTYPE,	// 通常サーバー以外には命令できない
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ILLEGALSTATUS,	// 命令できないステータス状態
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ALREADYREQUEST,// すでに要求が送られているサーバー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_UNKNOWN		// その他エラー
};

enum POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_REQCODE 
{
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_NOREQ,						// 要求なし
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_START,						// 開始要求
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_STOP,						// 停止要求
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_SHUTDOWN						// シャットダウン要求
};

// -- リクエスト構造体 --
typedef struct
{
	short Command;														// コマンド番号(POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_xxx)
	short Option;														// オプション(現在未使用)
	unsigned long IP;													// 開始するサーバーのIPアドレス
} 
POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND;


// -- レスポンス構造体 --
// なし


//------------------------------------
//  ミュージカルショット：全体情報
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_STATUS 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_STATUS_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_STATUS_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_STATUS_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

typedef struct 
{
	long TotalCount;													// 登録総数
	// 今後追加予定
} 
POKE_NET_GDS_RESPONSE_ADMIN_MUSICALSHOT_STATUS;


//------------------------------------
//  ミュージカルショット：削除
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_DELETE 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_DELETE_SUCCESS,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_DELETE_ERROR_ILLEGALCODE,	// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_DELETE_ERROR_AUTH,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_MUSICALSHOT_DELETE_ERROR_UNKNOWN			// その他エラー
};

typedef struct
{
	u64 Code;																	// 削除したいコード
} 
POKE_NET_GDS_REQUEST_ADMIN_MUSICALSHOT_DELETE;

// - レスポンス -
// 定義無し


//------------------------------------
//  バトルデータ：全体情報
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS_SUCCESS,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS_ERROR_AUTH,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS_ERROR_UNKNOWN			// その他エラー
};

// - リクエスト -
// 定義無し

typedef struct 
{
//	long TotalRankingCount;									// 現在までのランキング確定回数
	long TotalRegistCount;									// 登録総数
	long TotalEventCount;									// イベント登録総数
	// 今後追加予定
} 
POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_STATUS;


//------------------------------------
//  バトルデータ：削除
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_SUCCESS,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_ERROR_ILLEGALCODE,		// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_ERROR_AUTH,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_ERROR_UNKNOWN			// その他エラー
};

enum POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA
{
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_ALL						= 0xFF,	// 全体
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_REGIST					= 0x01,	// 登録エリア
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_EVENT						= 0x02,	// イベントエリア

	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_TODAYRANKING_COMM			= 0x04,	// 今日の集計中通信対戦ランキング
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_YESTERDAYRANKING_COMM		= 0x08,	// 確定された先日の通信対戦ランキングエリア

	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_TODAYRANKING_SUBWAY		= 0x10,	// 今日の集計中バトルサブウェイランキング
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_YESTERDAYRANKING_SUBWAY	= 0x20	// 確定された先日のサブウェイランキングエリア
};

typedef struct 
{
	long TargetArea;										// ターゲットエリア(POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_xxxx)
	u64 Code;												// 削除したいコード
} 
POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE;


// - レスポンス -
// 定義無し


//------------------------------------
//  バトルデータ：ランキング履歴取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY_ERROR_UNKNOWN	// その他エラー
};

enum POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE 
{
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_RANKING_SUBWAY,	// バトルサブウェイランキング
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_RANKING_COMM		// 通信対戦ランキング
};

// - ランキング履歴リクエスト -
typedef struct 
{															
	long Type;												// タイプ(POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_xxxx)
	u32 ServerVersion;										// 取得対象バトルデータのサーバーバージョン
} 
POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY;

// - レスポンス -

// - ランキング内データ情報 -
typedef struct 
{											
	s32 PID;												// プロファイルＩＤ
	u64 Code;												// バトルデータコード
	u64 Point;												// 獲得ポイント
	u64 RegistTime;											// 登録日時(0xYYYYMMDDHHMMSS)
	u32 FavoriteCount;										// お気に入り登録数
	BATTLE_REC_OUTLINE_RECV Data;							// バトルデータヘッダ
}
POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_PARAM;

// - ランキング情報 -
typedef struct
{																		
	long Count;															// ヒット数
	POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_PARAM List[0];	// 各データ
} 
POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_RANKING;

// - ランキング履歴レスポンス -
typedef struct
{														
	long Type;											// タイプ(POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_xxxx)
	long HitCount;										// ヒットしたランキング数
	char RankingList[0];								// ランキングリスト
														// (sizeof(POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_RANKING) +
														//	sizeof(POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_PARAM) * それぞれのランキング内のCount) * HitCount
}
POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY;


//------------------------------------
//  バトルデータ：イベント登録要求
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_SUCCESS,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_ERROR_ILLEGALCODE,		// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_ERROR_AUTH,				// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_ERROR_UNKNOWN			// その他エラー
};

typedef struct
{
	u64 Code;																		// イベント領域へ移動させたいデータのコード
}
POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_EVENTFLAG;

// - レスポンス -
// 定義無し


//------------------------------------
// バトルデータ：イベントデータの改変
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFREEWORD
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFREEWORD_SUCCESS,
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFREEWORD_NOTFOUND,
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFREEWORD_ERROR_UNKNOWN
};

typedef struct
{
	u64	Code;
	u8	Message_flag;
	STRCODE Event_self_introduction[EVENT_SELF_INTRO];
	u8	Secure;
}
POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_EVENTFREEWORD;

// - レスポンス -
// 定義無し


//------------------------------------
//  バトルデータ：管理パラメータ取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_GETPARAM 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_GETPARAM_SUCCESS,			// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_GETPARAM_ERROR_AUTH,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_GETPARAM_ERROR_UNKNOWN	// エラー
};

// 順位によるパラメータ数(順位数+その他/最新)
#define	POKE_NET_GDS_BATTLEDATA_GETPARAM_WEIGHT_MAXNUM	(30+2)

// 順位によるパラメータ数
#define	POKE_NET_GDS_BATTLEDATA_GETPARAM_SCALING_MAXNUM	(10+1)


// - リクエスト -
// 定義無し


typedef struct 
{
	long Weight[POKE_NET_GDS_BATTLEDATA_GETPARAM_WEIGHT_MAXNUM];			// ポイント重みづけ
	long Scaling[POKE_NET_GDS_BATTLEDATA_GETPARAM_SCALING_MAXNUM];			// 順位によるポイント倍率
	s64 InitPoint;															// 初期ポイント差分値
	s64 Bias;																// 平坦化閾値
} 
POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_GETPARAM;


//------------------------------------
//  バトルデータ：管理パラメータ設定
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_SETPARAM 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_SETPARAM_SUCCESS,			// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_SETPARAM_ERROR_AUTH,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_SETPARAM_ERROR_PARAMS,	// パラメータエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_SETPARAM_ERROR_UNKNOWN	// エラー
};

// 順位によるパラメータ数(順位数+その他/最新)
#define	POKE_NET_GDS_BATTLEDATA_SETPARAM_WEIGHT_MAXNUM	(30+2)

// 順位によるパラメータ数
#define	POKE_NET_GDS_BATTLEDATA_SETPARAM_SCALING_MAXNUM	(10+1)

// - リクエスト -
typedef struct
{
	long Weight[POKE_NET_GDS_BATTLEDATA_SETPARAM_WEIGHT_MAXNUM];			// ポイント重みづけ
	long Scaling[POKE_NET_GDS_BATTLEDATA_SETPARAM_SCALING_MAXNUM];			// 順位によるポイント倍率
	s64 InitPoint;															// 初期ポイント差分値
	s64 Bias;																// 平坦化閾値
}
POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_SETPARAM;

// - レスポンス -
// 定義無し


//------------------------------------
//  バトルデータ：強制確定処理
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EXECDECISION 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EXECDECISION_SUCCESS,			// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EXECDECISION_ERROR_AUTH,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EXECDECISION_ERROR_UNKNOWN	// エラー
};

// - リクエスト -
// 定義無し

// - レスポンス -
// 定義無し


// ***************************************************
// *                                                 *
// **            不正チェック関連                   **
// **                ここから                       **


//------------------------------------
// 不正チェック:許可ロムコードを取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_GET 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_GET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_GET_ERROR_AUTH ,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_ROMCODELIST_MAXNUM (10)
#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_MEMOLENGTH_MAX (0x40)	// 64バイト

// - レスポンス -
typedef struct
{
	u16 m_RomCode;
	STRCODE m_Memo[POKE_NET_GDS_ADMIN_ILLEGALCHECK_MEMOLENGTH_MAX];
}
POKE_NET_GDS_ADMIN_ILLEGAL_ROMCODE_LIST_ITEM;

typedef struct
{								
	long m_Count;
	POKE_NET_GDS_ADMIN_ILLEGAL_ROMCODE_LIST_ITEM m_List[0];
} 
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_ROMCODELIST_GET;


//------------------------------------
// 不正チェック:許可ロムコードをセット
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_SET 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_SET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_SET_ERROR_AUTH ,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_ROMCODELIST_SET_ERROR_UNKNOWN	// その他エラー
};
	
// - リクエスト -
typedef struct
{								
	long m_Count;
	POKE_NET_GDS_ADMIN_ILLEGAL_ROMCODE_LIST_ITEM m_List[POKE_NET_GDS_ADMIN_ILLEGALCHECK_ROMCODELIST_MAXNUM];
} 
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_ROMCODELIST_SET;

// - レスポンス -
// 定義無し


//------------------------------------
// 不正チェック:ＮＧワードの設定取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_GET 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_GET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_GET_ERROR_AUTH ,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_NGWORD_MAXNUM (100)

// - レスポンス -
typedef struct
{								
	long Count;			// ＮＧワードコード数
	long Codes[0];		// ＮＧワードコード
} 
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_NGWORD_GET;


//------------------------------------
// 不正チェック:ＮＧワードの設定セット
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_SET 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_SET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_SET_ERROR_AUTH ,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_NGWORD_SET_ERROR_UNKNOWN	// その他エラー
};
	
// - リクエスト -
typedef struct 
{													
	long Count;														// ＮＧワードコード数
	long Codes[POKE_NET_GDS_ADMIN_ILLEGALCHECK_NGWORD_MAXNUM];		// ＮＧワードコード
} 
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_NGWORD_SET;

// - レスポンス -
// 定義無し


//------------------------------------------
// 不正チェック:ミュージカルショットの許可タイトルリストの取得
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_GET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_GET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_GET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_MAXNUM (1000)

// - レスポンス -
typedef struct
{
	int m_Count;
	STRCODE m_TitleList[0][MUSICAL_PROGRAM_NAME_MAX];  // ミュージカルタイトルのリスト((日本18・海外36＋EOM) * m_Count)
}
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_GET;


//------------------------------------------
// 不正チェック:ミュージカルショットの許可タイトルリストの設定
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_SET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_SET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_SET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_SET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
typedef struct
{
	int m_Count;
	STRCODE m_TitleList[POKE_NET_GDS_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_MAXNUM][MUSICAL_PROGRAM_NAME_MAX];  // ミュージカルタイトルのリスト((日本18・海外36＋EOM) * m_Count)
}
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_MUSICALTITLELIST_SET;

// - レスポンス -
// 定義無し


//------------------------------------------
// 不正チェック:ミュージカルショットの不許可ポケモン番号リストの取得
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_GET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_GET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_GET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_MAXNUM (1000)

// - レスポンス -
typedef struct
{
	int m_Count;
	u16 m_DisablePokeNoList[0];  // 不許可ポケモン番号リスト(m_Count * 2)
}
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_GET;


//------------------------------------------
// 不正チェック:ミュージカルショットの不許可ポケモン番号リストの設定
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_SET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_SET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_SET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_SET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
typedef struct
{
	int m_Count;
	u16 m_DisablePokeNoList[POKE_NET_GDS_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_MAXNUM];  // 不許可ポケモン番号リスト(m_Count * 2)
}
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_MUSICALDISABLEPOKENO_SET;

// - レスポンス -
// 定義無し


//------------------------------------------
// 不正チェック:各種最大値パラメータの取得
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_GET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_GET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_GET_ERROR_AUTH,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

// - レスポンス -
typedef struct
{
	u16 m_PokeNoMax;
	u32 m_TRAINER_STATUS_ai_bit_MAX;
	u16 m_TRAINER_STATUS_tr_id_MAX;
	u16 m_TRAINER_STATUS_tr_type_MAX;
}
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_MAXLIST_GET;


//------------------------------------------
// 不正チェック:各種最大値パラメータの設定
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_SET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_SET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_SET_ERROR_AUTH,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_MAXLIST_SET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
typedef struct
{
	u16 m_PokeNoMax;
	u32 m_TRAINER_STATUS_ai_bit_MAX;
	u16 m_TRAINER_STATUS_tr_id_MAX;
	u16 m_TRAINER_STATUS_tr_type_MAX;
}
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_MAXLIST_SET;

// - レスポンス -
// 定義無し


//------------------------------------------
// 不正チェック:許可されたバトルモード値リストの取得
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_GET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_GET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_GET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_MAXNUM (100)

// - レスポンス -
typedef struct
{
	int m_Count;
	u16 m_BattleModeValList[0];  // 許可されたバトルモード値リスト(m_Count * 2)
}
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_GET;


//------------------------------------------
// 不正チェック:許可されたバトルモード値リストの設定
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_SET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_SET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_SET_ERROR_AUTH,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_SET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
typedef struct
{
	int m_Count;
	u16 m_BattleModeValList[POKE_NET_GDS_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_MAXNUM];  // 許可されたバトルモード値リスト(m_Count * 2)
}
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_BATTLEMODEVALUELIST_SET;

// - レスポンス -
// 定義無し


//------------------------------------------
// 不正チェック:許可されたバトルバージョンリストの取得
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_GET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_GET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_GET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

#define POKE_NET_GDS_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_MAXNUM (100)

// - レスポンス -
typedef struct
{
	int m_Count;
	u8 m_BattleVersionList[0];  // 許可されたバトルバージョンリスト
}
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_GET;


//------------------------------------------
// 不正チェック:許可されたバトルバージョンリストの設定
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_SET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_SET_SUCCESS,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_SET_ERROR_AUTH,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_SET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
typedef struct
{
	int m_Count;
	u8 m_BattleVersionList[POKE_NET_GDS_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_MAXNUM];  // 許可されたバトルバージョンリスト
}
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_BATTLEVERSIONLIST_SET;

// - レスポンス -
// 定義無し


//------------------------------------------
// 不正チェック:デフォルトトレイナー名の取得
//------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_GET 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_GET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_GET_ERROR_AUTH ,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_GET_ERROR_UNKNOWN	// その他エラー
};

// - リクエスト -
// 定義無し

// - レスポンス -
typedef struct 
{
	STRCODE Name[7][16][PLW_TRAINER_NAME_SIZE];
}
POKE_NET_GDS_RESPONSE_ADMIN_ILLEGALCHECK_DFTRAINERNAME_GET;


//---------------------------------------------
// 不正チェック:デフォルトトレイナー名のセット
//---------------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_SET
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_SET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_SET_ERROR_AUTH ,	// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_ILLEGALCHECK_DFTRAINERNAME_SET_ERROR_UNKNOWN	// その他エラー
};


// - リクエスト -
typedef struct 
{
	STRCODE Name[7][16][PLW_TRAINER_NAME_SIZE];
}
POKE_NET_GDS_REQUEST_ADMIN_ILLEGALCHECK_DFTRAINERNAME_SET;


// - レスポンス -
// 定義無し



//------------------------------------
// 不正チェックログ:取得
//------------------------------------
#define POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_TIMECAPA	(20)	// 時刻文字列の大きさ
#define POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_CONTENTCAPA	(256)	// 内容文字列の大きさ
#define POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_MAXRESPONSE	(30)	// 最大何件までのデータを返答するか

enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DHOCHECKLOG_GET 
{
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DHOCHECKLOG_GET_SUCCESS ,
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DHOCHECKLOG_GET_ERROR_UNKNOWN
};

// - リクエスト -
typedef struct 
{
	long	PageNo;		// ページ番号
} 
POKE_NET_GDS_REQUEST_ADMIN_DHOCHECKLOG_GET;

// - レスポンス -
typedef struct
{
	char Time[POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_TIMECAPA];
	char Content[POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_CONTENTCAPA];
}
POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_PARAM;

typedef struct
{
	long	HitCount;
	POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET_PARAM	List[0];
} 
POKE_NET_GDS_RESPONSE_ADMIN_DHOCHECKLOG_GET;


// **                ここまで                       **
// **            不正チェック関連                   **
// *                                                 *
// ***************************************************




#endif // #ifndef ___POKE_NET_BUILD_DS___

#endif // #ifndef ___POKE_NET_GDS_COMMON___
