//============================================================================================
/**
 * @file	wifi_p2pmatch_def.h
 * @bfief	WIFI P2P接続のマッチングに関する定義  スクリプトで使用
 * @author	k.ohno
 * @date	06.04.07
 */
//============================================================================================
#ifndef __WIFI_P2PMATCH_DEF_H__
#define __WIFI_P2PMATCH_DEF_H__


// 各デバック用定義
#ifdef PM_DEBUG
#define WFP2P_DEBUG	// デバック機能をON

#ifdef WFP2P_DEBUG
//#define WFP2P_DEBUG_EXON	// ほぼ必要ないリストも表示する
//#define WFP2P_DEBUG_PLON_PC	// プレイヤーの初期位置パソコン前
#endif

#endif

// P2PMATCH への指定
#define WIFI_P2PMATCH_DPW     (1)   // この後DPWへ移行する
#define WIFI_P2PMATCH_P2P     (2)   // このままP2P処理に移行



// P2PMATCH からの戻り値
typedef enum {
  WIFI_P2PMATCH_NONE,
  WIFI_P2PMATCH_SBATTLE50,   // 通信対戦呼び出し
  WIFI_P2PMATCH_SBATTLE100,   // 通信対戦呼び出し
  WIFI_P2PMATCH_SBATTLE_FREE,      // 通信対戦呼び出し
  WIFI_P2PMATCH_DBATTLE50,        // 通信対戦呼び出し
  WIFI_P2PMATCH_DBATTLE100,        // 通信対戦呼び出し
  WIFI_P2PMATCH_DBATTLE_FREE,      // 通信対戦呼び出し
  WIFI_P2PMATCH_TRADE,         // ポケモントレード呼び出し
  WIFI_P2PMATCH_TVT,      // トランシーバ
  WIFI_P2PMATCH_END,           // 通信切断してます。終了します。
  WIFI_P2PMATCH_UTIL,          // 接続設定を呼び出します。戻れません
  WIFI_P2PMATCH_DPW_END,       // 世界対戦用設定終了
} WIFI_P2PMATCH_KINDENUM;

  
typedef enum{
	WIFI_BATTLEFLAG_SINGLE,	// シングルバトル
	WIFI_BATTLEFLAG_DOUBLE,	// ダブルバトル
	WIFI_BATTLEFLAG_MULTI,	// マルチバトル
} WIFI_BATTLEFLAG;

#endif //__WIFI_P2PMATCH_DEF_H__

