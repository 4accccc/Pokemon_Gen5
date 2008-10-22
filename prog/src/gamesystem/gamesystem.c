//============================================================================================
/**
 * @file	gamemsystem.c
 * @brief	ポケモンゲームシステム（ここが通常ゲームのトップレベル）
 * @author	tamada GAME FREAK Inc.
 * @date	08.10.20
 *
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"		//HEAPIDの参照用

#include "gamesystem/gamesystem.h"

enum {
	HEAPID_GAMESYS = HEAPID_TAMADA_DEBUG,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
struct _GAMESYS_WORK {
	HEAPID heapID;
	void * parent_work;

	GFL_PROCSYS * procsys;
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static GAMESYS_WORK * GameSysWork;

extern const GFL_PROC_DATA DebugFieldProcData;

//============================================================================================
//============================================================================================
static void GameSystem_Init(GAMESYS_WORK * gsys);
static BOOL GameSystem_Main(GAMESYS_WORK * gsys);
static void GameSystem_End(GAMESYS_WORK * gsys);
static GFL_PROC_RESULT GameMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

//------------------------------------------------------------------
/**
 * @brief		プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA GameMainProcData = {
	GameMainProcInit,
	GameMainProcMain,
	GameMainProcEnd,
};


//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYS, 0x2000 );
	//GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYS, 0x200000 );
	GameSysWork = GFL_PROC_AllocWork(proc, sizeof(GAMESYS_WORK), HEAPID_GAMESYS);
	GameSysWork->heapID = HEAPID_GAMESYS;
	GameSysWork->parent_work = pwk;
	GameSystem_Init(GameSysWork);
	return GFL_PROC_RES_FINISH;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys = mywk;
	if (GameSystem_Main(gsys)) {
		return GFL_PROC_RES_FINISH;
	} else {
		return GFL_PROC_RES_CONTINUE;
	}
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GameSystem_End(GameSysWork);
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap( HEAPID_GAMESYS );
	return GFL_PROC_RES_FINISH;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_Init(GAMESYS_WORK * gsys)
{
	gsys->procsys = GFL_PROC_LOCAL_boot(gsys->heapID);
	GameSystem_CallProc(gsys, NO_OVERLAY_ID, &DebugFieldProcData, gsys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL GameSystem_Main(GAMESYS_WORK * gsys)
{
	GFL_PROC_LOCAL_Main(gsys->procsys);
	return FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_End(GAMESYS_WORK * gsys)
{
	GFL_PROC_LOCAL_Exit(gsys->procsys);
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void GameSystem_SetNextProc(GAMESYS_WORK * gsys,
		FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void * pwk)
{
	GFL_PROC_LOCAL_SetNextProc(GameSysWork->procsys, ov_id, procdata, pwk);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GameSystem_CallProc(GAMESYS_WORK * gsys,
		FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void * pwk)
{
	GFL_PROC_LOCAL_CallProc(GameSysWork->procsys, ov_id, procdata, pwk);
}





