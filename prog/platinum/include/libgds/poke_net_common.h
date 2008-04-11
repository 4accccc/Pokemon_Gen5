/*===========================================================================*
  Project:  Pokemon Global Data Station Network Library
  File:     poke_net_common.h

  $NoKeywords: $
 *===========================================================================*/
/*! @file
	@brief	Pokemon GDS WiFi ライブラリ

*/
//===========================================================================
#ifndef ___POKE_NET_GDS_COMMON___
#define ___POKE_NET_GDS_COMMON___

//===============================================
//                時間パック
//===============================================
#define POKE_NET_TIMETOU64(Y,M,D,h,m,s)	(u64)((((u64)Y) << 40) | (((u64)M) << 32) | (((u64)D) << 24) | (((u64)h) << 16) | (((u64)m) << 8) | (((u64)s) << 0))

//===============================================
//!          ＧＤＳリクエストコード
//===============================================
enum POKE_NET_GDS_REQCODE {
	POKE_NET_GDS_REQCODE_DEBUG_START = 0 ,				// ※以下デバッグで使用するリクエスト

	// ------------------------- 以下デバッグリクエスト ----------------------
	POKE_NET_GDS_REQCODE_DEBUG_MESSAGE = 0 ,			//!< デバッグメッセージ

	POKE_NET_GDS_REQCODE_DEBUG_END   = 9999 ,			// ※9999まではデバッグで仕様するリクエスト
	POKE_NET_GDS_REQCODE_ADMIN_START = 10000 ,			// ※10000以降は管理者が使用できるリクエスト

#ifndef ___POKE_NET_BUILD_DS___
	// -------------------------- 以下管理者リクエスト -----------------------
	POKE_NET_GDS_REQCODE_ADMIN_SVR_STATUS = 10000 ,			// サーバーステータス取得
	POKE_NET_GDS_REQCODE_ADMIN_SVR_CLEARSTATUS ,			// サーバー情報クリア
	POKE_NET_GDS_REQCODE_ADMIN_SVR_COMMAND ,				// サーバーへの命令

	POKE_NET_GDS_REQCODE_ADMIN_DRESSUPSHOT_STATUS = 11000 ,	// ドレスアップショット状態取得
	POKE_NET_GDS_REQCODE_ADMIN_DRESSUPSHOT_DELETE ,			// ドレスアップショット削除
	POKE_NET_GDS_REQCODE_ADMIN_DRESSUPSHOT_CLEAR ,			// ドレスアップショットクリア

	POKE_NET_GDS_REQCODE_ADMIN_BOXSHOT_STATUS = 12000 ,		// ボックスショット状態取得
	POKE_NET_GDS_REQCODE_ADMIN_BOXSHOT_DELETE ,				// ボックスショット削除
	POKE_NET_GDS_REQCODE_ADMIN_BOXSHOT_CLEAR ,				// ボックスショットクリア

	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_STATUS = 13000 ,	// バトルデータ状態取得
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_DELETE ,			// バトルデータ削除
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_RANKHISTORY ,		// バトルデータランキング履歴取得
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_EVENTFLAG ,		// バトルデータイベント登録要求
	POKE_NET_GDS_REQCODE_ADMIN_BATTLEDATA_CLEAR ,			// バトルデータクリア

	POKE_NET_GDS_REQCODE_ADMIN_RANKING_STATUS = 14000 ,		// ランキング状態取得
	POKE_NET_GDS_REQCODE_ADMIN_RANKING_HISTORY ,			// ランキング履歴取得
	POKE_NET_GDS_REQCODE_ADMIN_RANKING_GET ,				// ユーザーのランキング情報取得
	POKE_NET_GDS_REQCODE_ADMIN_RANKING_SET ,				// ユーザーのランキング情報設定
	POKE_NET_GDS_REQCODE_ADMIN_RANKING_CLEAR ,				// ランキング情報クリア

#endif
	POKE_NET_GDS_REQCODE_ADMIN_END  = 19999 ,			// ※19999までは管理者が使用できるリクエスト
	POKE_NET_GDS_REQCODE_USER_START = 20000 ,			// ※20000以降は一般ユーザーが使えるリクエスト

	// ------------------------- 以下ユーザーリクエスト ----------------------
	POKE_NET_GDS_REQCODE_DRESSUPSHOT_REGIST = 20000 ,	//!< ドレスアップショット登録
	POKE_NET_GDS_REQCODE_DRESSUPSHOT_GET ,				//!< ドレスアップショット取得
	POKE_NET_GDS_REQCODE_BOXSHOT_REGIST = 21000,		//!< ボックスショット登録
	POKE_NET_GDS_REQCODE_BOXSHOT_GET ,					//!< ボックスショット取得
	POKE_NET_GDS_REQCODE_RANKING_GETTYPE = 22000,		//!< 現在のランキングタイプ取得
	POKE_NET_GDS_REQCODE_RANKING_UPDATE ,				//!< 現在のランキング情報の更新と取得
	POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST = 23000,		//!< バトルデータ登録
	POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH ,			//!< バトルデータ検索
	POKE_NET_GDS_REQCODE_BATTLEDATA_GET ,				//!< バトルデータ取得
	POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE ,			//!< バトルデータお気に入り登録

	POKE_NET_GDS_REQCODE_LAST
};

//===============================================
//! ポケモンWiFiライブラリ リクエスト認証情報
//===============================================
typedef struct {
	long PID;									//!< プロファイルＩＤ
} POKE_NET_REQUESTCOMMON_AUTH;

//===============================================
//!  ポケモンWiFiライブラリ リクエストヘッダ
//===============================================
typedef struct {
	unsigned short ReqCode;						//!< リクエストコード(POKE_NET_GDS_REQUEST_REQCODE_xxxx)
	unsigned short Option;						//!< リクエストオプション
	POKE_NET_REQUESTCOMMON_AUTH Auth;			//!< ユーザー認証情報
	unsigned char Param[0];						//!< パラメータ(各リクエストの構造体)
} POKE_NET_REQUEST;

//===============================================
//!  ポケモンWiFiライブラリ レスポンスヘッダ
//===============================================
typedef struct {
	unsigned short ReqCode;						//!< リクエストコード(POKE_NET_GDS_REQUEST_REQCODE_xxxx)
	unsigned short Result;						//!< レスポンス結果(POKE_NET_GDS_RESPONSE_RESULT_xxxx)
	unsigned char Param[0];						//!< パラメータ(各レスポンスの構造体)
} POKE_NET_RESPONSE;

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
enum POKE_NET_GDS_RESPONSE_RESULT_DEBUG_MESSAGE {
	POKE_NET_GDS_RESPONSE_RESULT_DEBUG_MESSAGE_SUCCESS ,				//!< 登録成功
};

//! ＧＤＳデバッグメッセージリクエスト
typedef struct {
	char Message[1024];						//!< デバッグメッセージ
} POKE_NET_GDS_REQUEST_DEBUG_MESSAGE;

//! ＧＤＳデバッグメッセージレスポンス
typedef struct {
	char ResultMessage[1024];				//!< デバッグメッセージ
} POKE_NET_GDS_RESPONSE_DEBUG_MESSAGE;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//        ユーザーリクエスト/レスポンス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//------------------------------------
//     ドレスアップショット登録
//------------------------------------
//! ＧＤＳドレスアップショット登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST {
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST_SUCCESS ,				//!< 登録成功
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST_ERROR_ALREADY ,			//!< すでに登録されている
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST_ERROR_ILLEGAL ,			//!< 不正なデータ
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST_ERROR_ILLEGALPROFILE ,	//!< 不正なユーザープロフィール
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_REGIST_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳドレスアップショット登録リクエスト
typedef struct {
	GT_GDS_DRESS_SEND Data;							//!< ドレスアップショット送信データ
} POKE_NET_GDS_REQUEST_DRESSUPSHOT_REGIST;

//! ＧＤＳドレスアップショット登録レスポンス
typedef struct {
	u64 Code;										//!< 登録されたドレスアップショットのコード
} POKE_NET_GDS_RESPONSE_DRESSUPSHOT_REGIST;

//------------------------------------
//     ドレスアップショット取得
//------------------------------------
//! ＧＤＳドレスアップショット取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_GET {
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_GET_SUCCESS ,				//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_GET_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_GET_ERROR_ILLEGALPOKEMON ,	//!< ポケモン番号エラー
	POKE_NET_GDS_RESPONSE_RESULT_DRESSUPSHOT_GET_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳドレスアップショット取得タイプ
enum POKE_NET_GDS_REQUEST_DRESSUPSHOT_GETTYPE {
	POKE_NET_GDS_REQUEST_DRESSUPSHOT_GETTYPE_POKEMON ,				//!< ポケモン番号で最新取得(SearchParamにポケモン番号)
	POKE_NET_GDS_REQUEST_DRESSUPSHOT_GETTYPE_CODE					//!< コード直接指定で取得
};

//! ＧＤＳドレスアップショット取得リクエスト
typedef struct {
	unsigned short SearchType;										//!< 検索タイプ(POKE_NET_GDS_REQUEST_DRESSUPSHOT_GETTYPE_xxxx)
	unsigned short SearchOpt;										//!< 検索オプション(現在未使用)
	u64 SearchParam;												//!< 検索パラメータ
} POKE_NET_GDS_REQUEST_DRESSUPSHOT_GET;

//! ＧＤＳドレスアップショット取得レスポンス内容
typedef struct {
	u64 Code;														//!< ドレスアップショットコード
	GT_GDS_DRESS_RECV Data;											//!< ドレスアップショット受信データ
} POKE_NET_GDS_RESPONSE_DRESSUPSHOT_RECVDATA;

#define	POKE_NET_GDS_RESPONSE_DRESSUPSHOT_GET_MAX	(10)			// クライアントが受け取るドレスアップショット最大数

//! ＧＤＳドレスアップショット取得レスポンス
typedef struct {
	long HitCount;													//!< ヒット件数
	POKE_NET_GDS_RESPONSE_DRESSUPSHOT_RECVDATA Data[0];				//!< ドレスアップショット受信データ
} POKE_NET_GDS_RESPONSE_DRESSUPSHOT_GET;

//------------------------------------
//        ボックスショット登録
//------------------------------------
//! ＧＤＳボックスショット登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST {
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_SUCCESS ,				//!< 登録成功
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_ERROR_ALREADY ,			//!< すでに登録されている
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_ERROR_ILLEGAL ,			//!< 不正なデータ
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_ERROR_ILLEGALPROFILE ,	//!< 不正なユーザープロフィール
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_ERROR_ILLEGALGROUP ,	//!< 不正なグループ番号
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_REGIST_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳボックスショット登録リクエスト
typedef struct {
	long GroupNo;													//!< 登録先グループ番号
	GT_BOX_SHOT_SEND Data;											//!< ボックスショット送信データ
} POKE_NET_GDS_REQUEST_BOXSHOT_REGIST;

//! ＧＤＳボックスショット登録レスポンス
typedef struct {
	u64 Code;														//!< 登録されたボックスショットのコード
} POKE_NET_GDS_RESPONSE_BOXSHOT_REGIST;

//------------------------------------
//        ボックスショット取得
//------------------------------------
//! ＧＤＳボックスショット取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_GET {
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_GET_SUCCESS ,				//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_GET_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_GET_ERROR_ILLEGALGROUP ,	//!< グループ番号エラー
	POKE_NET_GDS_RESPONSE_RESULT_BOXSHOT_GET_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳボックスショット取得タイプ
enum POKE_NET_GDS_REQUEST_BOXSHOT_GETTYPE {
	POKE_NET_GDS_REQUEST_BOXSHOT_GETTYPE_GROUP ,					//!< グループ番号で最新取得(SearchParamにグループ番号)
	POKE_NET_GDS_REQUEST_BOXSHOT_GETTYPE_CODE						//!< ＩＤ指定で取得(SearchParamにコード番号)
};

//! ＧＤＳボックスショット取得リクエスト
typedef struct {
	unsigned short SearchType;										//!< 検索タイプ(POKE_NET_GDS_REQUEST_BOXSHOT_GETTYPE_xxxx)
	unsigned short SearchOpt;										//!< 検索オプション(現在未使用)
	u64 SearchParam;												//!< 検索パラメータ
} POKE_NET_GDS_REQUEST_BOXSHOT_GET;

//! ＧＤＳボックスショット取得レスポンス内容
typedef struct {
	u64 Code;														//!< ボックスショットコード
	GT_BOX_SHOT_RECV Data;											//!< ボックスショット受信データ
} POKE_NET_GDS_RESPONSE_BOXSHOT_RECVDATA;

#define	POKE_NET_GDS_RESPONSE_BOXSHOT_GET_MAXNUM	(20)			// クライアントが受け取るボックスショット最大数

//! ＧＤＳボックスショット取得レスポンス
typedef struct {
	long HitCount;													//!< ヒット件数
	POKE_NET_GDS_RESPONSE_BOXSHOT_RECVDATA Data[0];					//!< ボックスショット受信データ
} POKE_NET_GDS_RESPONSE_BOXSHOT_GET;


//------------------------------------
//      ランキングタイプ取得
//------------------------------------
/*
//! ＧＤＳランキングタイプ取得リクエスト
typedef struct {
} POKE_NET_GDS_REQUEST_RANKING_GETTYPE;
*/

//! ＧＤＳランキングタイプ取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_RANKING_GETTYPE {
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_GETTYPE_SUCCESS ,				//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_GETTYPE_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_GETTYPE_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳランキング取得レスポンス内容
typedef struct {
	GT_RANKING_TYPE_RECV Data;					//!< ランキングタイプデータ
} POKE_NET_GDS_RESPONSE_RANKING_GETTYPE;

//------------------------------------
//        ランキング更新
//------------------------------------
//! ＧＤＳランキング更新結果
enum POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE {
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE_SUCCESS ,				//!< 更新成功
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE_ERROR_ILLEGALTYPE	 ,	//!< ランキングタイプエラー
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE_ERROR_ILLEGALDATA	 ,	//!< 不正なデータエラー
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE_ERROR_ILLEGALPROFILE ,	//!< 不正なプロフィール
	POKE_NET_GDS_RESPONSE_RESULT_RANKING_UPDATE_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳランキング更新リクエスト
typedef struct {
	GT_RANKING_MYDATA_SEND Data;					//!< ランキング更新データ
} POKE_NET_GDS_REQUEST_RANKING_UPDATE;

//! ＧＤＳランキング更新レスポンス
typedef struct {
	GT_LAST_WEEK_RANKING_ALL_RECV LastWeek;			//!< 先週のランキングデータ
	GT_THIS_WEEK_RANKING_DATA_ALL_RECV ThisWeek;	//!< 今週のランキングデータ
} POKE_NET_GDS_RESPONSE_RANKING_UPDATE;

//------------------------------------
//       バトルデータ登録
//------------------------------------
//! ＧＤＳバトルデータ登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST {
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_SUCCESS ,				//!< 登録成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_AUTH ,				//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ALREADY ,			//!< すでに登録されている
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL ,			//!< 不正なデータ
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPROFILE ,	//!< 不正なユーザープロフィール
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳバトルデータ登録リクエスト
typedef struct {
	GT_BATTLE_REC_SEND Data;												//!< バトルデータ送信データ
} POKE_NET_GDS_REQUEST_BATTLEDATA_REGIST;

//! ＧＤＳバトルデータ登録レスポンス
typedef struct {
	u64 Code;																//!< 登録されたバトルデータのコード
} POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST;


//------------------------------------
//      バトルデータリスト取得
//------------------------------------
//! ＧＤＳバトルデータリスト取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH {
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESS ,			//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_ILLEGALPARAM ,	//!< 検索パラメータエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_UNKNOWN		//!< その他エラー
};

//! ＧＤＳバトルデータリスト取得タイプ
enum POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE {
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_CONDITION ,				//!< 条件指定で取得
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING ,				//!< 今週の通常ランキング
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_EXRANKING ,				//!< 今週の拡張ランキング
};

//! ＧＤＳバトルデータリスト取得リクエスト
typedef struct {
	unsigned short SearchType;											//!< 検索タイプ(POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_xxxx)
	unsigned short SearchOpt;											//!< 検索オプション(現在未使用)
	GT_BATTLE_REC_SEARCH_SEND ConditionParam;							//!< 複合検索パラメータ
} POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH;

#define	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_MAXNUM	(30)			// クライアントが受け取るバトルデータヘッダ最大数

//! ＧＤＳバトルデータリスト取得レスポンス内容
typedef struct {
	u64 Code;															//!< バトルデータコード
	GT_BATTLE_REC_OUTLINE_RECV Data;									//!< バトルデータヘッダ
} POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA;

//! ＧＤＳバトルデータリスト取得レスポンス
typedef struct {
	long HitCount;														//!< ヒット件数
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA Data[0];			//!< バトルデータリスト
} POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH;

//------------------------------------
//      バトルデータ取得
//------------------------------------
//! ＧＤＳバトルデータ取得結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET {
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_SUCCESS ,				//!< 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_ILLEGALCODE ,		//!< コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳバトルデータ取得リクエスト
typedef struct {
	u64 Code;															//!< バトルデータコード
} POKE_NET_GDS_REQUEST_BATTLEDATA_GET;

//! ＧＤＳバトルデータ取得レスポンス
typedef struct {
	u64 Code;															//!< バトルデータコード
	GT_BATTLE_REC_RECV Data;											//!< バトルデータ
} POKE_NET_GDS_RESPONSE_BATTLEDATA_GET;

//------------------------------------
//      バトルデータお気に入り登録
//------------------------------------
//! ＧＤＳバトルデータお気に入り登録結果
enum POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE {
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_SUCCESS ,				//!< お気に入り登録成功
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_AUTH ,			//!< ユーザー認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_ILLEGALCODE ,	//!< コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_UNKNOWN			//!< その他エラー
};

//! ＧＤＳバトルデータお気に入り登録リクエスト
typedef struct {
	u64 Code;															//!< バトルデータコード
} POKE_NET_GDS_REQUEST_BATTLEDATA_FAVORITE;

/*
//! ＧＤＳバトルデータお気に入り登録レスポンス
typedef struct {
} POKE_NET_GDS_RESPONSE_BATTLEDATA_FAVORITE;
*/

#ifndef ___POKE_NET_BUILD_DS___
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//        管理者リクエスト/レスポンス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//------------------------------------
//        サーバー状態取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS_SUCCESS ,				// 取得成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_STATUS_ERROR_UNKNOWN			// その他エラー
};

enum {
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_STOPPED ,			// 停止状態
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_ACTIVE ,			// 稼動状態
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_UNKNOWN ,			// 不明状態(応答なし)
};

#define POKE_NET_GDS_RESPONSE_ADMIN_SVR_MAXNUM	(256)			// 最大サーバー数

enum {
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_SVRTYPE_ADMIN ,	// 管理サーバー
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_SVRTYPE_SLAVE ,	// 通常サーバー
};

typedef struct {
	long Type;												// サーバー種(POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_SVRTYPE_xxxx)
	long Status;											// ステータス(POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA_STATUS_xxxxx)
	long MaxConnectCount;									// 最大コネクション可能数
	long ConnectCount;										// コネクション数
	u64 LastUpdateTime;										// 最終ステータス更新時間(0xYYYYMMDDHHMMSS)
	unsigned long IP;										// サーバーIPアドレス
	long Request;											// サーバーへ送っているリクエストコード
} POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA;

typedef struct {
	long ServerCount;										// サーバー数
	POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUSDATA Status[0];	// 各サーバーのステータス
} POKE_NET_GDS_RESPONSE_ADMIN_SVR_STATUS;

//------------------------------------
//        サーバー情報クリア
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_CLEARSTATUS {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_CLEARSTATUS_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_CLEARSTATUS_ERROR_AUTH ,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_CLEARSTATUS_ERROR_UNKNOWN	// その他エラー
};

//------------------------------------
//        サーバーへの命令
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_AUTH ,				// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ILLEGALIP ,		// 不正なサーバーＩＰアドレス
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ILLEGALTYPE ,		// 通常サーバー以外には命令できない
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ILLEGALSTATUS ,	// 命令できないステータス状態
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_ALREADYREQUEST ,	// すでに要求が送られているサーバー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_SVR_COMMAND_ERROR_UNKNOWN			// その他エラー
};

enum POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_REQCODE {
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_NOREQ ,						// 要求なし
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_START ,						// 開始要求
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_STOP ,						// 停止要求
	POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_SHUTDOWN ,					// シャットダウン要求
};

typedef struct {
	short Command;														// コマンド番号(POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND_xxx)
	short Option;														// オプション(現在未使用)
	unsigned long IP;													// 開始するサーバーのIPアドレス
} POKE_NET_GDS_REQUEST_ADMIN_SVR_COMMAND;


//------------------------------------
//  ドレスアップショット：全体情報
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_STATUS {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_STATUS_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_STATUS_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_STATUS_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_DRESSUPSHOT_STATUS;
*/

typedef struct {
	long TotalCount;										// 登録総数
	// 今後追加予定
} POKE_NET_GDS_RESPONSE_ADMIN_DRESSUPSHOT_STATUS;

//------------------------------------
//  ドレスアップショット：削除
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_DELETE {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_DELETE_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_DELETE_ERROR_ILLEGALCODE ,	// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_DELETE_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_DELETE_ERROR_UNKNOWN			// その他エラー
};

typedef struct {
	u64 Code;												// 削除したいコード
} POKE_NET_GDS_REQUEST_ADMIN_DRESSUPSHOT_DELETE;

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_DRESSUPSHOT_DELETE;
*/

//------------------------------------
//  ドレスアップショット：クリア
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_CLEAR {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_CLEAR_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_CLEAR_ERROR_PERMISSION ,		// 認められていない
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_CLEAR_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_DRESSUPSHOT_CLEAR_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_DRESSUPSHOT_CLEAR;
*/

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_DRESSUPSHOT_CLEAR;
*/

//------------------------------------
//  ボックスショット：全体情報
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_STATUS {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_STATUS_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_STATUS_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_STATUS_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_BOXSHOT_STATUS;
*/

typedef struct {
	long TotalCount;										// 登録総数
	// 今後追加予定
} POKE_NET_GDS_RESPONSE_ADMIN_BOXSHOT_STATUS;

//------------------------------------
//  ボックスショット：削除
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_DELETE {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_DELETE_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_DELETE_ERROR_ILLEGALCODE ,	// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_DELETE_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_DELETE_ERROR_UNKNOWN			// その他エラー
};

typedef struct {
	u64 Code;												// 削除したいコード
} POKE_NET_GDS_REQUEST_ADMIN_BOXSHOT_DELETE;

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_BOXSHOT_DELETE;
*/

//------------------------------------
//  ボックスショット：クリア
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_CLEAR {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_CLEAR_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_CLEAR_ERROR_PERMISSION ,		// 認められていない
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_CLEAR_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BOXSHOT_CLEAR_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_BOXSHOT_CLEAR;
*/

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_BOXSHOT_CLEAR;
*/

//------------------------------------
//  バトルデータ：全体情報
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_STATUS_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_STATUS;
*/

typedef struct {
	long TotalRankingCount;									// 現在までのランキング確定回数
	long TotalRegistCount;									// 登録総数
	long TotalEventCount;									// イベント登録総数
	// 今後追加予定
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_STATUS;

//------------------------------------
//  バトルデータ：削除
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_ERROR_ILLEGALCODE ,	// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_DELETE_ERROR_UNKNOWN			// その他エラー
};

enum POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA {
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_ALL ,					// 全体
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_REGIST ,				// 登録エリア
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_TODAYRANKING ,		// 今日のランキング
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_YESTERDAYRANKING ,	// 確定された先日ランキングエリア
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_TODAYEXRANKING ,		// 今日の拡張ランキング
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_YESTERDAYEXRANKING ,	// 確定された先日拡張ランキングエリア
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_EVENT ,				// イベントエリア
};

typedef struct {
	long TargetArea;										// ターゲットエリア(POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE_AREA_xxxx)
	u64 Code;												// 削除したいコード
} POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_DELETE;

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_DELETE;
*/

//------------------------------------
//  バトルデータ：クリア
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_CLEAR {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_CLEAR_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_CLEAR_ERROR_PERMISSION ,		// 認められていない
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_CLEAR_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_CLEAR_ERROR_UNKNOWN			// その他エラー
};

/*リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_CLEAR;
*/

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_CLEAR;
*/


//------------------------------------
//  バトルデータ：ランキング履歴取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_RANKHISTORY_ERROR_UNKNOWN			// その他エラー
};

enum POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE {
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_RANKING ,			// 通常ランキング
	POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_EXRANKING ,			// 拡張ランキング
};

typedef struct {											// - ランキング履歴リクエスト -
	long Type;												// タイプ(POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY_TYPE_xxxx)
	long StartBefore;										// 取得開始過去日数(現在からStartBefore日前のランキングから取得)
	long Count;												// 取得するランキング日数
} POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_RANKHISTORY;

typedef struct {											// - ランキング内データ情報 -
	u64 Code;												// バトルデータコード
	u64 Point;												// 獲得ポイント
	u64 RegistTime;											// 登録日時(0xYYYYMMDDHHMMSS)
	GT_BATTLE_REC_OUTLINE_RECV Data;						// バトルデータヘッダ
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_PARAM;

typedef struct {														// - ランキング情報 -
	u64 RegistTime;														// ランキング確定日時(0xYYYYMMDDHHMMSS)
	long Count;															// ヒット数
	POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_PARAM List[0];	// 各データ
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_RANKING;

typedef struct {											// - ランキング履歴レスポンス -
	long HitCount;											// ヒットしたランキング数
	char RankingList[0];									// ランキングリスト
															// (sizeof(POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_RANKING) +
															//	sizeof(POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY_PARAM) * それぞれのランキング内のCount) * HitCount
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_RANKHISTORY;

//------------------------------------
//  バトルデータ：イベント登録要求
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_ERROR_ILLEGALCODE ,		// コードエラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_BATTLEDATA_EVENTFLAG_ERROR_UNKNOWN			// その他エラー
};

typedef struct {
	u64 Code;												// イベント領域へ移動させたいデータのコード
} POKE_NET_GDS_REQUEST_ADMIN_BATTLEDATA_EVENTFLAG;

/*
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_BATTLEDATA_EVENTFLAG;
*/

//------------------------------------
//  ランキング：全体情報
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_STATUS {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_STATUS_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_STATUS_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_STATUS_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_RANKING_STATUS;
*/

typedef struct {
	long TotalRankingCount;									// 現在までのランキング回数
	long ThisWeekRankingNo;									// 現在のランキングの開催番号
	// 今後追加予定
} POKE_NET_GDS_RESPONSE_ADMIN_RANKING_STATUS;


//------------------------------------
//  ランキング：ランキング履歴取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_RANKHISTORY {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_RANKHISTORY_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_RANKHISTORY_ERROR_AUTH ,				// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_RANKHISTORY_ERROR_UNKNOWN			// その他エラー
};

typedef struct {											// - ランキング履歴リクエスト -
	long Type;												// ランキングタイプ(-1:指定なし)
	long StartBefore;										// 取得開始過去開催回数(現在からStartBefore回前のランキングから取得)
	long Count;												// 取得するランキング回数
} POKE_NET_GDS_REQUEST_ADMIN_RANKING_RANKHISTORY;


typedef struct {											// - ランキング履歴レスポンス -
	long Type;												// 取得したランキングタイプ
	long HitCount;											// ヒットしたランキング数
	GT_LAST_WEEK_RANKING_DATA Ranking[0];					// ランキングデータ(Type:-1の時はGT_RANKING_WEEK_NUM個で１ランキング)
} POKE_NET_GDS_RESPONSE_ADMIN_RANKING_RANKHISTORY;


//------------------------------------
//  ランキング：ユーザーパラメータ取得
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_GET {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_GET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_GET_ILLEGALPID ,		// 不正なPID
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_GET_ERROR_AUTH ,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_GET_ERROR_UNKNOWN	// その他エラー
};

typedef struct {													// - ランキング履歴リクエスト -
	long PID;														// ユーザーのプロファイルＩＤ
} POKE_NET_GDS_REQUEST_ADMIN_RANKING_GET;

typedef struct {													// - ランキング履歴レスポンス -
	long Count;														// パラメータ数(一応)
	u64 Params[0];													// 各パラメータ(サーバーに保持しているユーザー毎の得点)
} POKE_NET_GDS_RESPONSE_ADMIN_RANKING_GET;


//------------------------------------
//  ランキング：ユーザーパラメータ設定
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_SET {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_SET_SUCCESS ,		// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_SET_ILLEGALPID ,		// 不正なPID
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_SET_ERROR_AUTH ,		// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_SET_ERROR_UNKNOWN	// その他エラー
};

typedef struct {													// - ランキング履歴リクエスト -
	long PID;														// ユーザーのプロファイルＩＤ
	long Count;														// パラメータ数(一応)
	u64 Params[0];													// 各パラメータ(サーバーに保持しているユーザー毎の得点)
} POKE_NET_GDS_REQUEST_ADMIN_RANKING_SET;

/* レスポンスはコードのみ
typedef struct {													// - ランキング履歴レスポンス -
} POKE_NET_GDS_RESPONSE_ADMIN_RANKING_SET;
*/

//------------------------------------
//  ランキング：クリア
//------------------------------------
enum POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_CLEAR {
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_CLEAR_SUCCESS ,				// 成功
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_CLEAR_ERROR_PERMISSION ,		// 認められていない
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_CLEAR_ERROR_AUTH ,			// 認証エラー
	POKE_NET_GDS_RESPONSE_RESULT_ADMIN_RANKING_CLEAR_ERROR_UNKNOWN			// その他エラー
};

/* リクエストはコードのみ
typedef struct {
} POKE_NET_GDS_REQUEST_ADMIN_RANKING_CLEAR;
*/

/* レスポンスはコードのみ
typedef struct {
} POKE_NET_GDS_RESPONSE_ADMIN_RANKING_CLEAR;
*/

#endif

#endif //___POKE_NET_GDS_COMMON___
