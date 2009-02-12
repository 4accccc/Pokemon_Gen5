//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカルのドレスアップProc
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "musical/musical_system.h"
#include "musical/dressup_system.h"
#include "dup_fitting.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
enum	//メインシーケンス
{
	DUP_SEQ_INIT_FITTING,	//ドレスアップ試着部分
	DUP_SEQ_LOOP_FITTING,
	DUP_SEQ_TERM_FITTING,
};

//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
	FITTING_INIT_WORK *fitInitWork;
	FITTING_WORK *fitWork;
	FITTING_RETURN	fitRet;
}DRESSUP_LOCAL_WORK;

//======================================================================
//	proto
//======================================================================

static GFL_PROC_RESULT DressUpProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT DressUpProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT DressUpProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA DressUp_ProcData =
{
	DressUpProc_Init,
	DressUpProc_Main,
	DressUpProc_Term
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

static GFL_PROC_RESULT DressUpProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	DRESSUP_LOCAL_WORK *work;
	DRESSUP_INIT_WORK *initWork = pwk;
	*seq = DUP_SEQ_INIT_FITTING;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_DRESSUP, 0x40000 );

	work = GFL_PROC_AllocWork( proc, sizeof(DRESSUP_LOCAL_WORK), HEAPID_MUSICAL_DRESSUP );
	work->fitInitWork = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_DRESSUP , sizeof(FITTING_INIT_WORK) );
	
	work->fitInitWork->heapId = HEAPID_MUSICAL_DRESSUP;
	work->fitInitWork->musPoke = MUSICAL_SYSTEM_InitMusPoke( initWork->pokePara , HEAPID_MUSICAL_DRESSUP );

	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT DressUpProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	DRESSUP_LOCAL_WORK *work = mywk;
	GFL_HEAP_FreeMemory( work->fitInitWork->musPoke );
	GFL_HEAP_FreeMemory( work->fitInitWork );
	GFL_PROC_FreeWork( proc );

	GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_DRESSUP );

	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT DressUpProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	DRESSUP_LOCAL_WORK *work = mywk;
	switch( *seq )
	{
	case DUP_SEQ_INIT_FITTING:
		work->fitWork = DUP_FIT_InitFitting(work->fitInitWork);
		*seq = DUP_SEQ_LOOP_FITTING;
		break;
		
	case DUP_SEQ_LOOP_FITTING:
		{
			FITTING_RETURN ret = DUP_FIT_LoopFitting(work->fitWork);
			if( ret != FIT_RET_CONTINUE )
			{
				work->fitRet = ret;
				*seq = DUP_SEQ_TERM_FITTING;
			}
		}
		break;

	case DUP_SEQ_TERM_FITTING:
		DUP_FIT_TermFitting(work->fitWork);
		return GFL_PROC_RES_FINISH;
		break;
	}
	return GFL_PROC_RES_CONTINUE;
}

