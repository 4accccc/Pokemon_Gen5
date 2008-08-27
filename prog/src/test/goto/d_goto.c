//============================================================================================
/**
 * @file	d_goto.c
 * @brief	デバッグ用関数
 * @author	goto
 * @date	2008.08.27
 */
//============================================================================================
#include "gflib.h"
#include "procsys.h"
#include "tcbl.h"
#include "system/main.h"

#include "arc/arc_def.h"


//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugGotoMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	OS_TPrintf( "proc Init\n" );

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugGotoMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	OS_TPrintf( "proc Main\n" );

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugGotoMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	OS_TPrintf( "proc Exit\n" );

	return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugGotoMainProcData = {
	DebugGotoMainProcInit,
	DebugGotoMainProcMain,
	DebugGotoMainProcEnd,
};

 


