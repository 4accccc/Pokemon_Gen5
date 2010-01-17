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


#endif //__WIFI_P2PMATCH_DEF_H__

