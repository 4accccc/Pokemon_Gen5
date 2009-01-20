#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================
/**
 * @file	wifi_p2pmatch_se.h
 * @brief	サウンド定義置き換えファイル
 * @author	k.ohno
 * @date    2006.6.7
 */
//=============================================================================

#ifndef __WIFI_P2PMATCH_SE_H__
#define __WIFI_P2PMATCH_SE_H__


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#if 0 //SEはまだ無いので封印 2009.01.19
#define _SE_CURSOR			(SEQ_SE_DP_SELECT)		// カーソル
#define _SE_DESIDE   		(SEQ_SE_DP_DECIDE)		// 決定&キャンセル
#define _SE_OFFER       	(SEQ_SE_DP_UG_020)		// もうしこみ

//#define _SE_TBLCHANGE    	(SEQ_SE_DP_UG_020)		// 床の変更
#define _SE_TBLCHANGE    	(SEQ_SE_DP_SELECT5)		// 床の変更

#define _SE_INOUT			(SEQ_SE_DP_TELE2)		// 入退室

#define _BGM_MAIN           (SEQ_WIFILOBBY)			// BGM
#endif //if 0


#endif  //__WIFI_P2PMATCH_SE_H__

#ifdef __cplusplus
} /* extern "C" */
#endif
