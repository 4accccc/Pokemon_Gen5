//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		worldtimer.h
 *	@brief		世界時計
 *	@author		tomoya takahashi
 *	@data		2007.09.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WORLDTIMER_H__
#define __WORLDTIMER_H__

#include "savedata/savedata_def.h"
#include "application/wifi_lobby/wflby_def.h"
#include "system/procsys.h"
#include "application/wifi_lobby/wflby_system_def.h"

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
///	世界時計データ
//=====================================
typedef struct {
	const WFLBY_WLDTIMER*	cp_data;	// 表示データ
	SAVEDATA*				p_save;		// セーブデータ
	WFLBY_TIME				worldtime;	// 世界時間
	WFLBY_SYSTEM*			p_system;
} WLDTIMER_PARAM;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
extern GFL_PROC_RESULT WLDTIMER_Init(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk);
extern GFL_PROC_RESULT WLDTIMER_Main(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk);
extern GFL_PROC_RESULT WLDTIMER_Exit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk);

#ifdef  PM_DEBUG
extern GFL_PROC_RESULT WLDTIMER_DebugInit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk);
extern GFL_PROC_RESULT WLDTIMER_DebugExit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk);
#endif


#endif		// __WORLDTIMER_H__

