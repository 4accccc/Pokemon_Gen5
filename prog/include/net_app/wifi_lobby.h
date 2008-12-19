//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wifi_lobby.h
 *	@brief		WiFiロビーメインシステム
 *	@author		tomoya takahashi
 *	@data		2007.09.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WIFI_LOBBY_H__
#define __WIFI_LOBBY_H__

#include <procsys.h>
#include "savedata/save_control.h"
#include "field/wflby_counter.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	プロックパラメータ
//=====================================
typedef struct {
	WFLBY_COUNTER_TIME* p_wflby_counter;	// WiFiひろば入室時間格納先
	SAVE_CONTROL_WORK*			p_save;				// セーブデータ
	BOOL				check_skip;			// 接続確認をスキップする　TRUE：スキップ	FALSE：通常
} WFLBY_PROC_PARAM;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
extern GFL_PROC_RESULT WFLBYProc_Init( PROC* p_proc, int* p_seq );
extern GFL_PROC_RESULT WFLBYProc_Main( PROC* p_proc, int* p_seq );
extern GFL_PROC_RESULT WFLBYProc_Exit( PROC* p_proc, int* p_seq );



#endif		// __WIFI_LOBBY_H__

