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
#include "ui.h"
#include "test/ohno/debug_ohno.h"
#include "system/main.h"

//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------

extern void	AriFieldBoot( HEAPID heapID );
extern void	AriFieldEnd( void );
extern BOOL	AriFieldMain( void );

static GFL_PROC_RESULT DebugAriizumiMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WATANABE_DEBUG, 0x200000 );
	AriFieldBoot( HEAPID_WATANABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */  
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugAriizumiMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	if( AriFieldMain() == TRUE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugAriizumiMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	AriFieldEnd();
	GFL_HEAP_DeleteHeap( HEAPID_WATANABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugAriizumiMainProcData = {
	DebugAriizumiMainProcInit,
	DebugAriizumiMainProcMain,
	DebugAriizumiMainProcEnd,
};
