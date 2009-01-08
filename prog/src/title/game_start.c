//==============================================================================
/**
 * @file	game_start.c
 * @brief	「さいしょから」or「つづきから」を選択後の処理
 * @author	matsuda
 * @date	2009.01.07(水)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include "system\main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "title/game_start.h"


//--------------------------------------------------------------
/**
 * @brief   「最初から始める」を選択
 */
//--------------------------------------------------------------
void GameStart_Beginning(void)
{
	GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 0);
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);
}

//--------------------------------------------------------------
/**
 * @brief   「続きから始める」を選択
 */
//--------------------------------------------------------------
void GameStart_Continue(void)
{
	GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 0);
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);
}

//--------------------------------------------------------------
/**
 * @brief   「デバッグ開始」を選択
 */
//--------------------------------------------------------------
void GameStart_Debug(void)
{
#ifdef PM_DEBUG
	GAME_INIT_WORK * init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 0);
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);
#endif
}

