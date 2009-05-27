//==============================================================================
/**
 * @file	balloon.h
 * @brief	風船割りのヘッダ
 * @author	matsuda
 * @date	2007.11.06(火)
 */
//==============================================================================
#ifndef __BALLOON_H__
#define __BALLOON_H__

#include <procsys.h>
#include "net_app/wifi_lobby/minigame_tool.h"
#include "gamesystem/game_comm.h"

//==============================================================================
//	構造体定義
//==============================================================================
//--------------------------------------------------------------
/**
 *	風船割りゲームプロセスワーク(parent_work)
 */
//--------------------------------------------------------------
typedef struct {
	WFLBY_MINIGAME_WK lobby_wk;
	SAVE_CONTROL_WORK* p_save;
	GAME_COMM_SYS_PTR game_comm;
	u8 vchat;
	u8 wifi_lobby;
	u8 pad[2];

#ifdef PM_DEBUG
	int debug_offline;		///<TRUE:デバッグ用のオフラインモード
#endif
}BALLOON_PROC_WORK;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern GFL_PROC_RESULT BalloonProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT BalloonProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT BalloonProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_PROC_DATA BalloonProcData;


#endif		// __BALLOON_H__

