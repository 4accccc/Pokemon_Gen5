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
#include "net_app/wflby_counter.h"
#include "gamesystem/game_comm.h"

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
	GAME_COMM_SYS_PTR p_game_comm;
} WFLBY_PROC_PARAM;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
extern GFL_PROC_RESULT WFLBYProc_Init( GFL_PROC* p_proc, int* p_seq , void * pwk, void * mywk);
extern GFL_PROC_RESULT WFLBYProc_Main( GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT WFLBYProc_Exit( GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk );

//==============================================================================
//	データ
//==============================================================================
extern const GFL_PROC_DATA WFLBY_PROC;


#endif		// __WIFI_LOBBY_H__

