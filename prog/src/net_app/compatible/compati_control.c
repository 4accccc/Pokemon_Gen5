//==============================================================================
/**
 * @file	compati_control.c
 * @brief	相性チェックゲームのPROC制御
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include "system\main.h"

#include "compati_check.h"
#include "compati_types.h"
#include "compati_tool.h"


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	COMPATI_PARENTWORK cppw;		///<子PROCへ渡すパレントワーク
	union{
		CCNET_FIRST_PARAM first_param[2];	///< [0]:自分のデータ [1]:受信した相手のデータ
		CCNET_RESULT_PARAM result_param[2];	///< [0]:自分のデータ [1]:受信した相手のデータ
	};
}COMPATI_CONTROL_SYS;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT CompatiControlProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CompatiControlProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CompatiControlProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

//==============================================================================
//	データ
//==============================================================================
const GFL_PROC_DATA CompatiControlProcData = {
	CompatiControlProcInit,
	CompatiControlProcMain,
	CompatiControlProcEnd,
};


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CompatiControlProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	COMPATI_CONTROL_SYS *ccs;
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_COMPATI_CONTROL, 0x1000 );
	ccs = GFL_PROC_AllocWork( proc, sizeof(COMPATI_CONTROL_SYS), HEAPID_COMPATI_CONTROL );
	GFL_STD_MemClear(ccs, sizeof(COMPATI_CONTROL_SYS));

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT CompatiControlProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	COMPATI_CONTROL_SYS *ccs = mywk;

	switch(*seq){
	case 0:
		CCTOOL_CircleDataGet(&ccs->cppw.circle_package);
		(*seq)++;
		break;
	case 1:
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &CompatiResultProcData, &ccs->cppw);
		(*seq)++;
		break;
	case 2:
		GFL_PROC_SysCallProc(NO_OVERLAY_ID, &CompatiCheckProcData, &ccs->cppw);
		(*seq)++;
		break;
	default:
		return GFL_PROC_RES_FINISH;
	}
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT CompatiControlProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_COMPATI_CONTROL);

	return GFL_PROC_RES_FINISH;
}
