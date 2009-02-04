//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifi_p2pmatchfour.h
 *	@brief		wifi４人マッチング専用画面
 *	@author		tomoya takahashi
 *	@data		2007.05.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WIFI_P2PMATCHFOUR_H__
#define __WIFI_P2PMATCHFOUR_H__

#include <gflib.h>
#include "net_app/wificlub/wifi_p2pmatch.h"

#undef GLOBAL
#ifdef	__WIFI_P2PMATCHFOUR_H_GLOBAL
#define	GLOBAL	/* */
#else
#define	GLOBAL	extern
#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	マッチングタイプ
//=====================================
enum {
	WFP2PMF_TYPE_POFIN,				// ポフィン
	WFP2PMF_TYPE_BUCKET,			// たまいれ
	WFP2PMF_TYPE_BALANCE_BALL,		// たまのり	
	WFP2PMF_TYPE_BALLOON,			// ふうせんわり
	WFP2PMF_TYPE_NUM,
} ;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	マッチング内容　構造体
//=====================================
typedef struct {
	u8 type;		// マッチングタイプ
	u8 comm_min;	// 通信最低人数
	u8 comm_max;	// 通信最大人数

	u8 result;		// 正常に終了したのか、マッチング画面に戻るのか
	u8 vchat;		// VCHAT ON/OFF
	u8 pad[3];
	
//	GFL_SAVEDATA*  p_savedata;
//	FNOTE_DATA* p_fnote;
//  CONFIG* p_config;
    int wintype;

} WFP2PMF_INIT;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
extern const GFL_PROC_DATA WifiP2PMatchFourProcData;


#undef	GLOBAL
#endif		// __WIFI_P2PMATCHFOUR_H__

