
//=============================================================================
/**
 * @file  network_define.h
 * @brief WBのネットワーク定義を書くヘッダーファイル
 * @author  Katsumi Ohno
 * @date    2008.07.29
 */
//=============================================================================

#pragma once

#define NET_WORK_ON (1)   // サンプルプログラム上で通信を動かす場合定義　動かさない場合未定義にする


#define GAME_NAME					"syachi2ds"		// 使用するゲーム名
#define GAME_SECRET_KEY		"tXH2sN"					// 使用するシークレットキー
#define GAME_PRODUCTID		12230							// 使用するプロダクトID
#define GAME_ID           2911              // ゲームID



#if 0

●GameSpy設定情報（IRAJ/IRBJ 両バージョン共通です）
Game Name     syachi2ds 
Secret Key    tXH2sN 
Product ID    12230 
Game ID       2911 
Master Index  10

●ダウンロードサーバ（両バージョン共通です）
ダウンロードサーバ接続用ゲームコード・
ダウンロードサーバ管理画面ユーザー名 
IRAJ 

－開発用ダウンロードサーバ
管理画面URL   https://dls1ctl.test.nintendowifi.net/control
管理画面パスワード  wG7Q2xL4
ゲーム用パスワード  JDXNcp79mr54RUga 

－製品用ダウンロードサーバ
管理画面：（製品用サーバへアップする際は吉原までご相談下さい）
ゲーム用パスワード  fZrUY6SVMLp4tdmB 


●エラーシミュレーションサーバ
・IRAJ
エラーシミュレーションサーバーのID TWL-IRA 
エラーシミュレーションサーバーのパスワード cmMWFX 

・IRBJ
エラーシミュレーションサーバーのID TWL-IRB 
エラーシミュレーションサーバーのパスワード NvT4rI 

//また、GGIDにつきましても発行いたしました。
(1)0x00001380　通常通信用
(2)0x00001381　データビーコン配信用
(3)0x00001382　WCSレギュレーション配信用


#endif

/// 分けたかったのですが、CGEARの仕様によりGGIDを統一します
/// 2010.01.08 k.ohno
#define SYASHI_NETWORK_GGID  (0x00001380)    //通常はこれ
#define SYASHI_DOWNLOAD_GGID  (0x00001380)   //映画館配信専用
#define SYACHI_LEG_GGID (0x00001380)         //大会レギュレーション配信専用

#define NET_DREAMWORLD_VER (1)  //ドリームワールド通信バーション（かならず整数）

//==============================================================================
//  サーバー接続先定義
//==============================================================================
///認証サーバーの開発用サーバー、製品版サーバーの切り替え
#ifdef PM_DEBUG //--
///有効にしているとデバッグサーバーへ接続
#define DEBUG_SERVER
#endif  //-- PM_DEBUG

//DWC_Init～で仕様する、GameCode
#ifdef VERSION_BLACK
#define GF_DWC_GAMECODE		('IRAJ')
#define GF_DWC_ND_LOGIN		"IRAJ"
#elif defined(VERSION_WHITE)
#define GF_DWC_GAMECODE		('IRBJ')
#endif

//DWC_Init～で仕様する、GameName
#define GF_DWC_GAMENAME		("syachi2ds")

#ifdef DEBUG_SERVER
//開発用サーバー
#define GF_DWC_CONNECTINET_AUTH_TYPE  (DWC_CONNECTINET_AUTH_TEST)
//不思議な贈り物　定義を有効にすると本番サーバへ接続
//#define USE_AUTHSERVER_RELEASE      // 本番サーバへ接続
// dpw_common.h のDPW_SERVER_PUBLICの定義は直接ファイル内で変更する必要があります。
//DPW_SERVER_PUBLIC

//Wi-Fiロビーサーバ
//#define GF_DWC_LOBBY_CHANNEL_PREFIX   (PPW_LOBBY_CHANNEL_PREFIX_DEBUG)
#define GF_DWC_LOBBY_CHANNEL_PREFIX   (PPW_LOBBY_CHANNEL_PREFIX_DEBUG5) //金銀デバッグ中の為、WBは9

#define MATCHINGKEY (0)

#else //---------- DEBUG_SERVER

//製品版用サーバー
#define GF_DWC_CONNECTINET_AUTH_TYPE  (DWC_CONNECTINET_AUTH_RELEASE)
//不思議な贈り物　定義を有効にすると本番サーバへ接続
#define USE_AUTHSERVER_RELEASE      // 本番サーバへ接続
// dpw_common.h のDPW_SERVER_PUBLICの定義は直接ファイル内で変更する必要があります。
//DPW_SERVER_PUBLIC
//Wi-Fiロビーサーバ
#define GF_DWC_LOBBY_CHANNEL_PREFIX   (DWC_LOBBY_CHANNEL_PREFIX_RELEASE)

#define MATCHINGKEY (1)

#endif  //---------- DEBUG_SERVER

//不思議な贈り物でしようするパスワード
#ifdef USE_AUTHSERVER_RELEASE
#define WIFI_ND_LOGIN_PASSWD		"fZrUY6SVMLp4tdmB"		// パスワード(本番サーバ)
#else
#define WIFI_ND_LOGIN_PASSWD		"JDXNcp79mr54RUga"    // パスワード(テストサーバ)
#endif


#ifdef DEBUG_SERVER
#define LIBDPW_SERVER_TYPE ( DPW_SERVER_DEBUG )   ///< PDW接続先 現在手違いがあってもデバッグにしかいかない
#else
#define LIBDPW_SERVER_TYPE ( DPW_SERVER_DEBUG )   ///< PDW接続先  DPW_SERVER_RELEASE
#endif


//==============================================================================
//  定義
//==============================================================================

extern void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo,void* pWork);
extern u32 GFLR_NET_GetGGID(void);

//==============================================================================
//  型宣言
//==============================================================================

/// ゲームの種類を区別する為の定義   初期化構造体のgsidとして渡してください
enum NetworkServiceID_e {
  WB_NET_NOP_SERVICEID = 0,
  WB_NET_BATTLE_SERVICEID,       ///< バトル用ID
  WB_NET_PALACE_SERVICEID,       ///< パレス
  WB_NET_FIELDMOVE_SERVICEID,  ///< サンプルのフィールド移動
  WB_NET_MULTIBOOT_SERVICEID,  ///< ダウンロード通信のボックス取得
  WB_NET_DEBUG_OHNO_SERVICEID,
  WB_NET_SERVICEID_DEBUG_BATTLE,
  WB_NET_DEBUG_MATSUDA_SERVICEID,
  WB_NET_TRADE_SERVICEID,     ///<ポケモン交換
  WB_NET_MINIGAME_TOOL,       ///<WIFI広場ミニゲームツール
  WB_NET_MYSTERY,         ///<不思議な贈り物
  WB_NET_BALLOON,         ///<風船割り(ソーナンス)
  WB_NET_BUCKET,          ///<球入れ(マルノーム)
  WB_NET_BALANCEBALL,       ///<バランスボール(マネネ)
  WB_NET_WIFICLUB,                 ///<WIFICLUB
  WB_NET_COMPATI_CHECK,       ///<相性チェック（テスト版）
  WB_NET_IRCBATTLE,               ///< IRC>WIRELESS>接続
  WB_NET_WIFILOBBY,                 ///<WIFI広場
  WB_NET_IRCCOMPATIBLE,     ///相性チェック
  WB_NET_IRCAURA,         ///相性チェック オーラチェック
  WB_NET_IRCRHYTHM,       ///相性チェック　RHYTHMチェック
  WB_NET_IRCMENU,       ///相性チェック　RHYTHMチェック
  WB_NET_MUSICAL,       ///ミュージカル
  WB_NET_UNION,           ///<ユニオンルーム
  WB_NET_COLOSSEUM,       ///<コロシアム
  WB_NET_IRCTRADE,       ///<赤外線ぽけ交換
  WB_NET_IRCFRIEND,       ///<赤外線ともだち交換
  WB_NET_WIFIGTS,         ///<GTS
  WB_NET_IRC_BATTLE,      ///<赤外線バトル・(大会用)
  WB_NET_COMM_TVT,         ///<通信TVT
  WB_NET_GDS,              ///<GDS
  WB_NET_GTSNEGO,          ///< GTSネゴシエーション
	WB_NET_DEBUG_BEACON_FRIENDCODE,	///<デバッグ　ビーコンでの友達コード交換
  WB_NET_WIFIMATCH,         ///<WIFI世界対戦
  WB_NET_IRCBATTLE_MULTI,  ///< 赤外線バトルマルチ
  WB_NET_COMM_ENTRY_MENU,   ///<通信エントリーメニュー
  WB_NET_GURUGURU,        ///<ぐるぐる交換
  WB_NET_PICTURE,         ///<お絵かき
  WB_NET_BATTLE_ADD_CMD,  ///< 通信バトル コマンド送信
  WB_NET_BTL_REC_SEL,      ///<通信対戦後の録画選択画面

  WB_NET_SERVICEID_MAX   // 終端
};

///通信コマンドを区別する為の定義  コールバックテーブルを渡すときにIDとして定義してください
/// 上の定義と同じ並びで書いてください
enum NetworkCommandHeaderNo_e {
  GFL_NET_CMD_BASE = (WB_NET_NOP_SERVICEID<<8),  ///< ベースコマンド開始番号
  GFL_NET_CMD_BATTLE = (WB_NET_BATTLE_SERVICEID<<8),   ///< バトル開始番号
  GFL_NET_CMD_PALACE = (WB_NET_PALACE_SERVICEID<<8),   ///< パレスのベース開始番号
  GFL_NET_CMD_FIELD = (WB_NET_FIELDMOVE_SERVICEID<<8),   ///< フィールドのベース開始番号
  GFL_NET_CMD_MULTIBOOT = (WB_NET_MULTIBOOT_SERVICEID<<8),   ///< フィールドのベース開始番号
  GFL_NET_CMD_DEBUG_OHNO = (WB_NET_DEBUG_OHNO_SERVICEID<<8),
  GFL_NET_CMD_DEBUG_BATTLE = (WB_NET_SERVICEID_DEBUG_BATTLE<<8),
  GFL_NET_CMD_DEBUG_MATSUDA = (WB_NET_DEBUG_MATSUDA_SERVICEID<<8),
  GFL_NET_CMD_TRADE = (WB_NET_TRADE_SERVICEID<<8),      ///<ポケモン交換の開始番号
  GFL_NET_CMD_MINIGAME_TOOL = (WB_NET_MINIGAME_TOOL<<8),  ///<WIFI広場ミニゲームツール
  GFL_NET_CMD_MYSTERY = (WB_NET_MYSTERY<<8),        ///<不思議な贈り物
  GFL_NET_CMD_BALLOON = (WB_NET_BALLOON<<8),        ///<風船割り
  GFL_NET_CMD_BUCKET = (WB_NET_BUCKET<<8),          ///<球入れ
  GFL_NET_CMD_BALANCEBALL = (WB_NET_BALANCEBALL<<8),    ///<バランスボール
  GFL_NET_CMD_WIFICLUB = (WB_NET_WIFICLUB<<8),        ///<WIFICLUB
  GFL_NET_CMD_COMPATI_CHECK = (WB_NET_COMPATI_CHECK<<8),  ///<相性チェック（テスト版）
  GFL_NET_CMD_IRCBATTLE = (WB_NET_IRCBATTLE<<8),  ///< IRC>WIRELESS>接続
  GFL_NET_CMD_WIFILOBBY = (WB_NET_WIFILOBBY<<8),      ///<WIFI広場
  GFL_NET_CMD_IRCCOMPATIBLE = (WB_NET_IRCCOMPATIBLE<<8),  ///<相性チェック
  GFL_NET_CMD_IRCAURA = (WB_NET_IRCAURA<<8),  ///<相性チェック　オーラチェック
  GFL_NET_CMD_IRCRHYTHM = (WB_NET_IRCRHYTHM<<8),  ///<相性チェック　リズムチェック
  GFL_NET_CMD_IRCMENU = (WB_NET_IRCMENU<<8),  ///<相性チェック　メニュー
  GFL_NET_CMD_MUSICAL = (WB_NET_MUSICAL<<8),  ///ミュージカル
  GFL_NET_CMD_UNION = (WB_NET_UNION<<8),      ///<ユニオンルーム
  GFL_NET_CMD_COLOSSEUM = (WB_NET_COLOSSEUM<<8),      ///<コロシアム
  GFL_NET_CMD_IRCTRADE = (WB_NET_IRCBATTLE<<8),  ///< IRCぽけこうかん
  GFL_NET_CMD_IRCFRIEND = (WB_NET_IRCFRIEND<<8),  ///< IRCともだちこうかん
  GFL_NET_CMD_WIFIGTS   = (WB_NET_WIFIGTS<<8),    ///< GTS
  GFL_NET_CMD_IRC_BATTLE    = (WB_NET_IRC_BATTLE<<8),   ///< IRC
  GFL_NET_CMD_COMM_TVT   = (WB_NET_COMM_TVT<<8),    ///< 通信TVT
  GFL_NET_CMD_GDS   = (WB_NET_GDS<<8),    ///< GDS
  GFL_NET_CMD_GTSNEGO   = (WB_NET_GTSNEGO<<8),    ///< GTSネゴシエーション
	GFL_NET_CMD_DEBUG_BEACON_FRIENDCODE	= (WB_NET_DEBUG_BEACON_FRIENDCODE<<8),	///<デバッグ　ビーコンでの友達コード交換
  GFL_NET_CMD_WIFIMATCH = (WB_NET_WIFIMATCH<<8),         ///<WIFI世界対戦
  GFL_NET_CMD_IRCBATTLE_MULTI = (WB_NET_IRCBATTLE_MULTI<<8),         ///<赤外線バトルマルチ
  GFL_NET_CMD_COMM_ENTRY_MENU = (WB_NET_COMM_ENTRY_MENU<<8),  ///<通信エントリーメニュー
  GFL_NET_CMD_GURUGURU = (WB_NET_GURUGURU << 8),      ///<ぐるぐる交換
  GFL_NET_CMD_PICTURE = (WB_NET_PICTURE << 8),        ///<お絵かき


  
};


/// 通信送信基本サイズ 同じゲームが複数ある場合に統一した速度になるように指定してある
/// ４人でのゲームの想定
#define NET_SEND_SIZE_STANDARD (96)


typedef enum{
  NHTTP_POKECHK_RANDOMMATCH = 0,//ランダムマッチ      │POKEMON_PARAM         │6                 │220     │
  NHTTP_POKECHK_GTS,   //GTS                 │POKEMON_PASO_PARAM    │1                 │136     │
  NHTTP_POKECHK_GTSNEGO,       // GTSネゴシエーション │POKEMON_PASO_PARAM    │3                 │136     │
  NHTTP_POKECHK_VIDIO,   //     │バトルビデオ        │POKE_BATTLEVIDEO_PARAM│12                │108     │
  NHTTP_POKECHK_SUBWAY,  // │バトルサブウェイ    │_B_TOWER_POKEMON      │3                 │56      │
} NHTTP_POKECHK_ENUM;

#include "net_irc.h"



