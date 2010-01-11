#pragma once
//============================================================================================
/**
 * @file	  wifi_status.h
 * @brief	  WIFIに流れる情報の管理
 * @author	k.ohno
 * @date	  2009.6.6
 */
//============================================================================================
//#include "savedata/mystatus.h"
//#include "savedata/mystatus_local.h"

typedef struct _WIFI_STATUS WIFI_STATUS;


//---------------WIFISTATUSデータ
typedef enum{
  WIFI_GAME_NONE,   // 何も無い	NONEのときは出現もしません
  WIFI_GAME_LOGIN_WAIT,    // 待機中 何も選んで無い
  WIFI_GAME_UNIONMATCH,   // 繋ぐだけ。繋いだ後に各ゲームに進む
  WIFI_GAME_VCT,      // VCT
  WIFI_GAME_TVT,          // TVトランシーバ
  WIFI_GAME_TRADE,          // 交換
  WIFI_GAME_BATTLE_SINGLE_ALL,      // シングルバトル
  WIFI_GAME_BATTLE_SINGLE_FLAT,      // シングルバトル
  WIFI_GAME_BATTLE_DOUBLE_ALL,      // ダブル
  WIFI_GAME_BATTLE_DOUBLE_FLAT,      // ダブル
  WIFI_GAME_BATTLE_TRIPLE_ALL,      // トリプル
  WIFI_GAME_BATTLE_TRIPLE_FLAT,      // トリプル
  WIFI_GAME_BATTLE_ROTATION_ALL,      // ローテーション
  WIFI_GAME_BATTLE_ROTATION_FLAT,      // ローテーション
  WIFI_GAME_UNKNOWN,   // 新たに作ったらこの番号以上になる

} WIFI_GAME_e;



typedef enum{
  WIFI_STATUS_NONE,   // 何も無い	NONEのときは出現もしません
  WIFI_STATUS_WAIT,   // 待機中
  WIFI_STATUS_RECRUIT,    // 募集中  = 誰でも繋ぐ事が可能
  WIFI_STATUS_CALL,   //他の人に接続しようとしている
  WIFI_STATUS_PLAYING,      // 他の人と接続中
  WIFI_STATUS_PLAY_AND_RECRUIT,      // 他の人と接続中でさらに募集中
  WIFI_STATUS_UNKNOWN,   // 新たに作ったらこの番号以上になる

} WIFI_STATUS_e;




// dwc_rap.hにある定義分増やす事が可能
// #define MYDWC_STATUS_DATA_SIZE_MAX (128)
// WIFIフレンド情報サイズ DWCに189バイト可能とあるが、定義が無いので控えめサイズで決めうち
// 増やしすぎると全体に重くなる

//#define _POKEMON_NUM   (6)

struct _WIFI_STATUS{
  u32 profileID;                                 //プロファイルID
//	MYSTATUS aMyStatus;   // MYSTATUS                           ROM LANGはこの中にある
	u8 VChatMac[6];       // ゲームを呼びかける人のMacアドレス 自分のMACだった場合開始      
	u8 MyMac[6];       // 自分のMacアドレス
  u8 trainer_view;	// ユニオンルーム内での見た目
  u8 pm_version;	// バージョン
	u8 pm_lang;		  // 言語
	u8 GameMode;         // WIFIでのゲーム等の状態         WIFI_GAME_e
  u8 status;    // 自分の状態                           WIFI_STATUS_e
  u8 nation;           //  国
  u8 area;             //  地域
  u8 shooter:1;    //シューター有り無し
  u8 VChatStatus:1;       // VChatの状態
  u8 sex:2;
  u8 dummybit:4;
  u8 dummy2;   //上位バージョン用
  u8 dummy3;   //上位バージョン用
  u8 dummy4;   //上位バージョン用
};


//extern MYSTATUS* WIFI_STATUS_GetMyStatus(WIFI_STATUS* pStatus);
//extern void WIFI_STATUS_SetMyStatus(WIFI_STATUS* pStatus, const MYSTATUS* pMy);

extern u8 WIFI_STATUS_GetWifiStatus(const WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetWifiStatus( WIFI_STATUS* pStatus, u8 mode);

extern u8 WIFI_STATUS_GetGameMode(const WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetGameMode( WIFI_STATUS* pStatus, u8 mode);



extern u8 WIFI_STATUS_GetVChatStatus(const WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetVChatStatus(WIFI_STATUS* pStatus,u8 vct);

extern void WIFI_STATUS_SetMyNation(WIFI_STATUS* pStatus,u8 no);
extern void WIFI_STATUS_SetMyArea(WIFI_STATUS* pStatus,u8 no);

extern u8 WIFI_STATUS_GetActive(const WIFI_STATUS* pStatus);
extern BOOL WIFI_STATUS_IsVChatMac(const WIFI_STATUS* pMyStatus, const WIFI_STATUS* pFriendStatus);
extern void WIFI_STATUS_SetVChatMac(WIFI_STATUS* pStatus, const WIFI_STATUS* pFriendStatus);
extern void WIFI_STATUS_ResetVChatMac(WIFI_STATUS* pStatus);
extern void WIFI_STATUS_SetMyMac(WIFI_STATUS* pStatus);


static inline void WIFI_STATUS_SetTrainerView(WIFI_STATUS* pStatus,u8 trainer_view){ pStatus->trainer_view = trainer_view; }
static inline u8 WIFI_STATUS_GetTrainerView(WIFI_STATUS* pStatus){ return pStatus->trainer_view; }

static inline void WIFI_STATUS_SetPMVersion(WIFI_STATUS* pStatus,u8 pm_version){ pStatus->pm_version = pm_version; }
static inline void WIFI_STATUS_SetPMLang(WIFI_STATUS* pStatus,u8 pm_lang){ pStatus->pm_lang = pm_lang; }
static inline void WIFI_STATUS_SetSex(WIFI_STATUS* pStatus,u8 sex){ pStatus->sex = sex; }


//extern void WIFI_STATUS_SetTrainerView(WIFI_STATUS* pStatus,u8 trainer_view);
