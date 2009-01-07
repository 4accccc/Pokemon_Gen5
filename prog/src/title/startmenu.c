//======================================================================
/**
 * @file	startmenu.c
 * @brief	最初から・続きからを行うトップメニュー
 * @author	ariizumi
 * @data	09/01/07
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "title/startmenu.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================

//Procデータ
static GFL_PROC_RESULT StartMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT StartMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT StartMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

const GFL_PROC_DATA StartMenuProcData = {
	StartMenuProcInit,
	StartMenuProcMain,
	StartMenuProcEnd,
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------




//--------------------------------------------------------------
static GFL_PROC_RESULT StartMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
static GFL_PROC_RESULT StartMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
static GFL_PROC_RESULT StartMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}


