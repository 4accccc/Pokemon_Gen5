/*===========================================================================*
  Project:  Syachi PokemonGDS Network Library
  File:     poke_net_gds.c

  $NoKeywords: $
 *===========================================================================*/
/*! @file
	@brief	Syachi Pokemon GDS WiFi ライブラリ

*/
//===========================================================================

#include "poke_net_gds.h"
#include "poke_net_common.h"	// Windows/Linux/DS共通ヘッダ


//====================================
/*!
	ポケモン WiFi GDS ライブラリを初期化します

	@param[in]	_auth	クライアント情報<br>
						NULLの場合は前にセットされたクライアント情報が使用されます。
	@param[in]	_Url	URL<br>
	@param[in]	_PortNo	ポート番号<br>

	@retval		TRUE	初期化に成功しました
	@retval		FALSE	初期化に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_Initialize(POKE_NET_REQUESTCOMMON_AUTH* _auth, const char* _Url, unsigned short _PortNo)
{
	// 初期化
	return(POKE_NET_Initialize(_Url ,_PortNo ,_auth));
}


//====================================
/*!
	ポケモン WiFi GDS ライブラリを解放します

	１回の POKE_NET_GDS_Initialize() 関数に対して１回だけ<br>
	実行されなければなりません。
*/
//====================================
void POKE_NET_GDS_Release()
{
	// POKE_NET通信解放
	POKE_NET_Release();
}


//====================================
/*!
	ポケモン WiFi GDS ライブラリの現在の状態を取得します

	@retval	現在の状態
*/
//====================================
POKE_NET_GDS_STATUS POKE_NET_GDS_GetStatus()
{
	POKE_NET_STATUS stat;

	POKE_NET_LockMutex();					// ミューテクスロック
	stat = POKE_NET_GetStatus();			// 現在のステータス取得
	POKE_NET_UnlockMutex();					// ミューテクスアンロック

	return((POKE_NET_GDS_STATUS)stat);
}


//====================================
/*!
	ポケモン WiFi GDS ライブラリの最終エラーコードを取得します

	@retval	最終エラーコード
*/
//====================================
POKE_NET_GDS_LASTERROR POKE_NET_GDS_GetLastErrorCode()
{
	POKE_NET_LASTERROR code;

	code = POKE_NET_GetLastErrorCode();		// 現在のステータス取得

	return((POKE_NET_GDS_LASTERROR)code);
}


//====================================
/*!
	ポケモン WiFi GDS ライブラリのリクエスト処理を中断します

	この関数を実行したのち POKE_NET_GDS_GetStatus() 関数の戻り値が<br>
	POKE_NET_GDS_STATUS_ABORTEDとなれば中断が完了しています。<br>

	@retval TRUE	中断要求に成功しました
	@retval FALSE	中断要求に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_Abort()
{
	return(POKE_NET_Abort());
}


//====================================
/*!
	レスポンス内容が格納されたメモリの先頭アドレスを返します

	POKE_NET_GDS_GetStatus() 関数の戻り値(ステータス)が<br>
	POKE_NET_GDS_STATUS_FINISHEDのときのみ有効です。<br>
	リクエスト発行時に指定した_response引数とは違うアドレスが<br>
	返ってきますので注意してください。

	@retval レスポンス格納先のアドレス
*/
//====================================
void* POKE_NET_GDS_GetResponse()
{
	return(POKE_NET_GetResponse());
}


//====================================
/*!
	各リクエストのレスポンスサイズを取得します

	POKE_NET_GDS_GetStatus() 関数の戻り値(ステータス)が<br>
	POKE_NET_GDS_STATUS_FINISHEDのときのみ有効です。

	@retval レスポンス容量
*/
//====================================
long POKE_NET_GDS_GetResponseSize()
{
	return(POKE_NET_GetResponseSize());
}


//====================================
/*!
	各リクエストのレスポンス最大サイズを取得します

	各リクエストのレスポンスを格納するのに必要な最大サイズを返します。

	@param[in]	_reqno	リクエスト番号<br>
						-1を指定した場合もっともレスポンスサイズの大きいリクエストの<br>
						レスポンスサイズを返します。<br>

	@retval		0以上	レスポンス容量
	@retval		-1		エラー
*/
//====================================
long POKE_NET_GDS_GetResponseMaxSize(long _reqno)
{
	return(POKE_NET_GetResponseMaxSize(_reqno));
}


//====================================
/*!
	通信スレッドの実行レベルを設定します。

	通信スレッドの実行レベルを変更したい場合、<br>
	POKE_NET_GDS_Initialize() 関数を実行した後、<br>
	各リクエストを実行する前に呼び出す必要があります。<br>

	@param[in]	_level	スレッドレベル<br>
						デフォルト：16
*/
//====================================
void POKE_NET_GDS_SetThreadLevel(unsigned long _level)
{
	POKE_NET_SetThreadLevel(_level);
}


//====================================
/*!
	ミュージカルショットのアップロードを行います<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST

	@param[in]		_data		アップロードするミュージカルショットデータ<br>
								データはライブラリ内に確保されますので<br>
								この関数終了後に解放できます。
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_MUSICALSHOT_REGIST構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。


	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_MusicalShotRegist(MUSICAL_SHOT_SEND* _data, void* _response)
{
	POKE_NET_GDS_REQUEST_MUSICALSHOT_REGIST req;

	req.Data = *_data;

	// リクエスト発行
	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST, &req, _response, -1));
}


//====================================
/*!
	ミュージカルショットのダウンロードを行います<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_MUSICALSHOT_GET

	@param[in]		_pokemonno	ダウンロードするミュージカルショットに写っているポケモン番号
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_MUSICALSHOT_GET構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_MusicalShotGet(long _pokemonno, void* _response)
{
	POKE_NET_GDS_REQUEST_MUSICALSHOT_GET req;

	req.SearchType  = POKE_NET_GDS_REQUEST_MUSICALSHOT_GETTYPE_POKEMON;
	req.SearchOpt   = 0;
	req.SearchParam = _pokemonno;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_MUSICALSHOT_GET, &req, _response, -1));
}


//====================================
/*!
	バトルデータのアップロードを行います<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST

	@param[in]		_data		アップロードしたいバトルデータ
	@param[in]		_signModule	署名データ
	@param[in]		_signSize	署名データサイズ<br>
								指定する値が256を超えると関数は何も行わずにFALSEを返します。
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataRegist(BATTLE_REC_SEND* _data, u8* _signModule, long _signSize, void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_REGIST req = {0};

	if(_signSize > POKE_NET_GDS_REQUEST_BATTLEDATA_SIGN_MAXSIZE)
	{
		POKE_NET_DebugPrintf(POKE_NET_DEBUGLEVEL_NORMAL,
			"#POKE_NET Error. The maximum size of the prepared SignModuleBuffer is (%d). \n",
			POKE_NET_GDS_REQUEST_BATTLEDATA_SIGN_MAXSIZE
		);
		return FALSE;
	}

	req.Data = *_data;
	req.SignSize = _signSize;
	memcpy(req.SignModule, _signModule, (u32)_signSize);	// 署名データをコピー

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST, &req, _response, -1));
}


//====================================
/*!
	バトルデータをくわしく検索<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH

	@param[in]		_condition	検索条件<br>
								※検索結果は新しいもの順で返ってきます。<br>
								※検索条件のBATTLE_REC_SEARCH_SEND構造体の中の条件すべてを<br>
								　「指定なし」とした場合全データが検索対象となります。<br>
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataSearchCondition(BATTLE_REC_SEARCH_SEND* _condition, void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH req;

	req.SearchType     = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_CONDITION;
	req.ConditionParam = *_condition;
	req.SearchType     = 0;
	req.SearchOpt      = 0;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH, &req, _response, -1));
}


//====================================
/*!
	バトルデータの最新30件取得<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH

	@param[in]		_svrversion	取得するランキング内バトルデータの有効サーバーバージョン
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataSearchRankingLatest(u32 _svrversion, void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH req;

	memset(&req ,0x00 ,sizeof(POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH));

	req.SearchType = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_LATEST;
	req.ConditionParam.server_version = _svrversion;
	req.SearchParam = 0;
	req.SearchOpt = 0;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH, &req, _response, -1));
}


//====================================
/*!
	バトルデータの通信対戦ランキング取得<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH

	@param[in]		_svrversion	取得するランキング内バトルデータの有効サーバーバージョン
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataSearchRankingComm(u32 _svrversion ,void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH req;

	memset(&req ,0x00 ,sizeof(POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH));

	req.SearchType = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_COMM;
	req.ConditionParam.server_version = _svrversion;
	req.SearchParam = 0;
	req.SearchOpt = 0;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH, &req, _response, -1));
}


//====================================
/*!
	バトルデータのバトルサブウェイランキング取得<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH

	@param[in]		_svrversion	取得するランキング内バトルデータの有効サーバーバージョン
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataSearchRankingSubway(u32 _svrversion, void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH req;

	memset(&req ,0x00 ,sizeof(POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCH));

	req.SearchType = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_SUBWAY;
	req.ConditionParam.server_version = _svrversion;
	req.SearchParam = 0;
	req.SearchOpt = 0;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH, &req, _response, -1));
}


//====================================
/*!
	バトルデータの取得<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_GET

	@param[in]		_code		バトルデータコード
	@param[in]		_svrversion	取得対象バトルデータのサーバーバージョン
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_GET構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataGet(u64 _code, u32 _svrversion, void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_GET req;

	req.Code = _code;
	req.ServerVersion = _svrversion;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_GET, &req, _response, -1));
}


//====================================
/*!
	バトルデータのお気に入り登録<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE

	@param[in]		_code		バトルデータコード
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ResultメンバにはPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_xxxxの値が返ってきます。<br>
								これがPOKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_SUCCESSの場合、<br>
								ParamメンバにはPOKE_NET_GDS_RESPONSE_BATTLEDATA_FAVORITE構造体が返ってきます。<br>
								それ以外の場合にはParamメンバにはデータは返ってきませんので注意してください。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_BattleDataFavorite(u64 _code, void* _response)
{
	POKE_NET_GDS_REQUEST_BATTLEDATA_FAVORITE req;

	req.Code = _code;

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE, &req, _response, -1));
}


//====================================
/*!
	デバッグメッセージリクエスト<br>
	<br>
	<b>発行リクエスト:</b>POKE_NET_GDS_REQCODE_DEBUG_MESSAGE

	@param[in]		_message	メッセージの先頭アドレス<br>
								データはライブラリ内に確保されますので<br>
								この関数終了後に解放できます。
	@param[in,out]	_response	レスポンスを受け取るメモリの先頭アドレス<br>
								POKE_NET_RESPONSE構造体が返ってきます。<br>
								ParamメンバにPOKE_NET_GDS_RESPONSE_DEBUG_MESSAGE構造体が返ってきます。

	@retval			TRUE		リクエストの登録に成功しました
	@retval			FALSE		リクエストの登録に失敗しました
*/
//====================================
BOOL POKE_NET_GDS_DebugMessageRequest(char* _message, void* _response)
{	
	POKE_NET_GDS_REQUEST_DEBUG_MESSAGE req;

	strcpy(req.Message ,_message);

	return(POKE_NET_Request(POKE_NET_GDS_REQCODE_DEBUG_MESSAGE ,&req ,_response ,-1));
}









