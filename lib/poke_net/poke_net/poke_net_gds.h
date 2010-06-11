/*===========================================================================*
  Project:  Syachi PokemonGDS Network Library
  File:     poke_net_gds.h

  $NoKeywords: $
 *===========================================================================*/
/*! @file
	@brief	Syachi Pokemon GDS WiFi ライブラリ
	@date	2010/06/11
*/
//===========================================================================
/**
		@mainpage	Syachi Pokemon GDS WiFi ライブラリ
			<b>・概要</b><br>
			　本ライブラリは、NintendoDS・DSi上でポケモンＧＤＳ関連の通信を行う為のものです。<br>
			<br>
			<b>・説明</b><br>
			　本ヘルプファイルはリファレンスマニュアルです。<br>
			　セットアップ方法は別途 syachi_poke_net_inst.pdf を参照してください。<br>
			<br>
			<b>・バージョン履歴</b><br>
			　0.25　[10/06/11]　・同期モードで受信中に線を抜くとブロックされて制御が返らない問題の対策に、<br>
              　　　　　　　　　　　　　　　　受信側でスレッドを分けて待ち受け側でタイムアウト処理を行うようにした。<br>
              　　　　　　　　　　　　　　　　poke_net.h 内の RECEIVE_THREAD 定義をオン・オフすることで上記処理にあり/なしを切り替えます。<br>
			　0.24　[10/06/08]　・BTLREC_SETUP_SUBSET構造体へのメンバ追加に対応<br>
			　0.23a [10/04/27]　・MUSICAL_SHOT_POKE構造体へのメンバ追加に対応<br>
			　0.23　[10/03/29]　・バトルビデオ登録機能のインタフェースを、署名データを送信出来るように修正<br>
			　0.22　[10/03/25]　・GDS構造体の一部変更への対応<br>
			　　　　　　　　　　　　　　　・OS_Alloc, OS_Freeを使用しないように修正<br>
			　0.21　[10/03/17]　・GDS構造体の一部変更への対応<br>
			　0.20　[10/03/05]　・SSL通信を実装<br>
			　　　　　　　　　　　　　　　・SSL通信の実装に伴い、前バージョンまで通信時に行っていた暗号化／復号化を廃止<br>
			　　　　　　　　　　　　　　　・POKE_NET_GDS_Initialize関数の引数を変更<br>
			　0.10　[10/02/15]　・初回リリース<br>
			<br>
			<b>・謝辞</b><br>
			　この製品には、OpenSSL Toolkit で使用するためにOpenSSL Project によって開発されたソフトウェアが組み込まれています。 (http://www.openssl.org/)<br>
			　この製品には、Eric Young 氏 (eay@cryptsoft.com) によって作成された暗号ソフトウェアが含まれています。<br>
			　この製品には、Tim Hudson 氏 (tjh@cryptsoft.com) によって作成されたソフトウェアが含まれています。<br>
			<br>
			　<b>※ 上記の記載はWindows版通信ライブラリでのOpenSSLの利用に伴う記載です。</b><br>
			　　　<b>Nitro/TWL版通信ライブラリ、及びそのサンプルプログラムには一切関係はありません。</b><br>
			<br>
			<br>
		@version	0.25<br>
*/




#ifndef ___POKE_NET_GDS___
#define ___POKE_NET_GDS___

#include "poke_net.h"


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/

//! ポケモン WiFi GDS ライブラリの現在の状態
typedef enum
{
	POKE_NET_GDS_STATUS_INACTIVE,			//!< 非稼動中
	POKE_NET_GDS_STATUS_INITIALIZED,		//!< 初期化済
	POKE_NET_GDS_STATUS_REQUEST,			//!< リクエスト発行済
	POKE_NET_GDS_STATUS_NETSETTING,			//!< ネットワーク設定中
	POKE_NET_GDS_STATUS_CONNECTING,			//!< 接続中
	POKE_NET_GDS_STATUS_SENDING,			//!< 送信中
	POKE_NET_GDS_STATUS_RECEIVING,			//!< 受信中
	POKE_NET_GDS_STATUS_ABORTED,			//!< 中断終了
	POKE_NET_GDS_STATUS_FINISHED,			//!< 正常終了
	POKE_NET_GDS_STATUS_ERROR,				//!< エラー終了
	POKE_NET_GDS_STATUS_COUNT
}
POKE_NET_GDS_STATUS;


//! ポケモン WiFi GDS ライブラリがエラー終了した際の詳細
typedef enum
{
	POKE_NET_GDS_LASTERROR_NONE,			//!< 特になし
	POKE_NET_GDS_LASTERROR_NOTINITIALIZED,	//!< 初期化されていない
	POKE_NET_GDS_LASTERROR_ILLEGALREQUEST,	//!< リクエストが異常
	POKE_NET_GDS_LASTERROR_CREATESOCKET,	//!< ソケット生成に失敗
	POKE_NET_GDS_LASTERROR_IOCTRLSOCKET,	//!< ソケットのノンブロッキング化に失敗
	POKE_NET_GDS_LASTERROR_NETWORKSETTING,	//!< ネットワーク設定失敗
	POKE_NET_GDS_LASTERROR_CREATETHREAD,	//!< スレッド生成失敗
	POKE_NET_GDS_LASTERROR_CONNECT,			//!< 接続失敗
	POKE_NET_GDS_LASTERROR_SENDREQUEST,		//!< リクエストエラー
	POKE_NET_GDS_LASTERROR_RECVRESPONSE,	//!< レスポンスエラー
	POKE_NET_GDS_LASTERROR_CONNECTTIMEOUT,	//!< 接続タイムアウト
	POKE_NET_GDS_LASTERROR_SENDTIMEOUT,		//!< 送信タイムアウト
	POKE_NET_GDS_LASTERROR_RECVTIMEOUT,		//!< 受信タイムアウト
	POKE_NET_GDS_LASTERROR_ABORTED,			//!< 中断終了
	POKE_NET_GDS_LASTERROR_GETSVL,			//!< SVL取得エラー
	POKE_NET_GDS_LASTERROR_COUNT
}
POKE_NET_GDS_LASTERROR;


/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*
					関数外部宣言
 *-----------------------------------------------------------------------*/
extern BOOL POKE_NET_GDS_Initialize(POKE_NET_REQUESTCOMMON_AUTH *_auth, const char* _Url, unsigned short _PortNo);
extern void POKE_NET_GDS_Release();

extern POKE_NET_GDS_STATUS POKE_NET_GDS_GetStatus();
extern POKE_NET_GDS_LASTERROR POKE_NET_GDS_GetLastErrorCode();
extern BOOL POKE_NET_GDS_Abort();

extern void* POKE_NET_GDS_GetResponse();
extern long  POKE_NET_GDS_GetResponseSize();
extern long  POKE_NET_GDS_GetResponseMaxSize(long _reqno);

extern void POKE_NET_GDS_SetThreadLevel(unsigned long _level);


// ==============================================
// 通信インタフェース
// ==============================================

// ミュージカルショット
// POKE_NET_GDS_MusicalShotRegist	: アップロード
// POKE_NET_GDS_MusicalShotGet		: ポケモン選択ダウンロード
// (実装 : poke_net_gds.c)
extern BOOL POKE_NET_GDS_MusicalShotRegist(MUSICAL_SHOT_SEND* _data, void* _response);
extern BOOL POKE_NET_GDS_MusicalShotGet(long _pokemonno, void* _response);

// バトルビデオ
// POKE_NET_GDS_BattleDataRegist				: アップロード
// POKE_NET_GDS_BattleDataSearchCondition		: くわしく検索
// POKE_NET_GDS_BattleDataSearchRankingLatest	: ランキング検索 >> 最新30件
// POKE_NET_GDS_BattleDataSearchRankingComm		: ランキング検索 >> 通信対戦ランキング
// POKE_NET_GDS_BattleDataSearchRankingSubway	: ランキング検索 >> バトルサブウェイランキング
// POKE_NET_GDS_BattleDataGet					: ダウンロード
// POKE_NET_GDS_BattleDataFavorite				: お気に入り登録
// (実装 : poke_net_gds.c)
extern BOOL POKE_NET_GDS_BattleDataRegist(BATTLE_REC_SEND* _data, u8* _signModule, long _signSize, void* _response);
extern BOOL POKE_NET_GDS_BattleDataSearchCondition(BATTLE_REC_SEARCH_SEND* _condition, void* _response);
extern BOOL POKE_NET_GDS_BattleDataSearchRankingLatest(u32 _svrversion, void* _response);
extern BOOL POKE_NET_GDS_BattleDataSearchRankingComm(u32 _svrversion, void* _response);
extern BOOL POKE_NET_GDS_BattleDataSearchRankingSubway(u32 _svrversion, void* _response);
extern BOOL POKE_NET_GDS_BattleDataGet(u64 _code, u32 _serverVersion, void* _response);
extern BOOL POKE_NET_GDS_BattleDataFavorite(u64 _code, void* _response);

// デバッグメッセージ
// (実装 : poke_net_gds.c)
extern BOOL POKE_NET_GDS_DebugMessageRequest(char* _message, void* _response);

// ダンプ
// POKE_NET_GDS_DEBUG_PrintProfile		: プロフィール内容を出力
// POKE_NET_GDS_DEBUG_PrintProfile_Light: プロフィール内容を出力(軽め)
// POKE_NET_GDS_DEBUG_PrintMusicalshot	: ミュージカルショット内容を出力
// POKE_NET_GDS_DEBUG_PrintBattleHeader	: バトルヘッダ内容を出力
// POKE_NET_GDS_DEBUG_PrintBattleData	: バトルデータ内容を出力
// POKE_NET_GDS_DEBUG_PrintResponse		: レスポンス内容を出力
// (実装 : poke_net_gds_debug.c)
extern void POKE_NET_GDS_DEBUG_PrintProfile(GDS_PROFILE* _profile);
extern void POKE_NET_GDS_DEBUG_PrintProfile_Light(GDS_PROFILE* _profile);
extern void POKE_NET_GDS_DEBUG_PrintMusicalshot(MUSICAL_SHOT_DATA* _pMusical);
extern void POKE_NET_GDS_DEBUG_PrintBattleHeader(BATTLE_REC_HEADER* _pBattleHeader);
extern void POKE_NET_GDS_DEBUG_PrintBattleData(BATTLE_REC_WORK* _pBattleData);
extern void POKE_NET_GDS_DEBUG_PrintResponse(POKE_NET_RESPONSE* _response, long _size);


#ifndef SDK_FINALROM

#else

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // ___POKE_NET_GDS___
