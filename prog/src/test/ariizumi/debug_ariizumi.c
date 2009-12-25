//============================================================================================
/**
 * @file	debug_ariizumi.c
 * @brief	デバッグ 有泉
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================
#include "gflib.h"
#include "procsys.h"
#include "system/main.h"

#include "gamesystem/game_init.h"

typedef struct
{
	u8 dummy;
}DEBUG_ARIIZUMI_WORK;

DEBUG_ARIIZUMI_WORK *debWork = NULL;



//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugAriizumiMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ARIIZUMI_DEBUG, 0x80000 );

	debWork = GFL_HEAP_AllocMemory( HEAPID_ARIIZUMI_DEBUG , sizeof(DEBUG_ARIIZUMI_WORK));
	
	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugAriizumiMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_FreeMemory( debWork );
	debWork = NULL;

	GFL_HEAP_DeleteHeap( HEAPID_ARIIZUMI_DEBUG );
	
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */  
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugAriizumiMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	//return GFL_PROC_RES_FINISH;
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
	{
    GF_ASSERT(0);
  }
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
    OS_TPanic( "panic!!" );
  }
  
	return GFL_PROC_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugAriizumiMainProcData = {
	DebugAriizumiMainProcInit,
	DebugAriizumiMainProcMain,
	DebugAriizumiMainProcEnd,
};

