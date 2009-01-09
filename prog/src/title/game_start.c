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
#include "savedata/situation.h"
#include "app/name_input.h"


//==============================================================================
//	
//==============================================================================
FS_EXTERN_OVERLAY(title);

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT GameStart_FirstProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_FirstProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameStart_FirstProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_ContinueProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_ContinueProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_ContinueProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameStart_DebugProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


//==============================================================================
//	データ
//==============================================================================
///最初から始める
static const GFL_PROC_DATA GameStart_FirstProcData = {
	GameStart_FirstProcInit,
	GameStart_FirstProcMain,
	GameStart_FirstProcEnd,
};

///続きから始める
static const GFL_PROC_DATA GameStart_ContinueProcData = {
	GameStart_ContinueProcInit,
	GameStart_ContinueProcMain,
	GameStart_ContinueProcEnd,
};

///デバッグ開始
static const GFL_PROC_DATA GameStart_DebugProcData = {
	GameStart_DebugProcInit,
	GameStart_DebugProcMain,
	GameStart_DebugProcEnd,
};


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   「最初から始める」を選択
 */
//--------------------------------------------------------------
void GameStart_Beginning(void)
{
	GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_FirstProcData, NULL);
}

//--------------------------------------------------------------
/**
 * @brief   「続きから始める」を選択
 */
//--------------------------------------------------------------
void GameStart_Continue(void)
{
	GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_ContinueProcData, NULL);
}

//--------------------------------------------------------------
/**
 * @brief   「デバッグ開始」を選択
 */
//--------------------------------------------------------------
void GameStart_Debug(void)
{
#ifdef PM_DEBUG
	GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &GameStart_DebugProcData, NULL);
#endif
}



//==============================================================================
//
//	最初から始める
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_FirstProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SaveControl_ClearData(SaveControl_GetPointer());	//セーブデータクリア
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_FirstProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	switch(*seq){
	case 0:
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &NameInputProcData,(void*)NAMEIN_MYNAME);
		(*seq)++;
		break;
	case 1:
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_FirstProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GAME_INIT_WORK * init_param;
	VecFx32 pos = {0,0,0};
	
	init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_FIRST, 0, &pos, 0);
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);

	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//
//	続きから始める
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_ContinueProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_ContinueProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_ContinueProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GAME_INIT_WORK * init_param;
	PLAYERWORK_SAVE plsv;
	
	SaveControl_Load(SaveControl_GetPointer());
	SaveData_SituationLoad_PlayerWorkSave(SaveControl_GetPointer(), &plsv);

	init_param = DEBUG_GetGameInitWork(
		GAMEINIT_MODE_CONTINUE, plsv.zoneID, &plsv.position, plsv.direction);
	
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);

	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//
//	デバッグスタート
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT GameStart_DebugProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#ifdef PM_DEBUG
	GAME_INIT_WORK * init_param;
	VecFx32 pos = {0,0,0};

	SaveControl_ClearData(SaveControl_GetPointer());
	init_param = DEBUG_GetGameInitWork(GAMEINIT_MODE_DEBUG, 0, &pos, 0);
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &GameMainProcData, init_param);
#endif

	return GFL_PROC_RES_FINISH;
}
