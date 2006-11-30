//============================================================================================
/**
 * @file	debug_ohno.c
 * @brief	デバッグ用関数
 * @author	ohno
 * @date	2006.11.29
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "debug_ohno.h"
#include "test_ui.h"


//------------------------------------------------------------------
typedef struct {
	u32 debug_heap_id;
	UISYS * uisys;
	GFL_PROCSYS * psys;
}DEBUG_OHNO_CONTROL;

static DEBUG_OHNO_CONTROL * DebugOhnoControl;



static GFL_PROC_RESULT _debugUIProcInit(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	DEBUG_OHNO_CONTROL * ctrl = p_work;
	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT _debugUIProcMain(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	GFL_PROC * subproc;
	DEBUG_OHNO_CONTROL * ctrl = p_work;

    TEST_UI_Main(ctrl->uisys);  //UI TEST
    

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT _debugUIProcEnd(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
// UIテスト用プロセス定義テーブル
//------------------------------------------------------------------
static const GFL_PROC_DATA UITestProcTbl = {
	_debugUIProcInit,
	_debugUIProcMain,
	_debugUIProcEnd,
};

static void VBlankIntr(void)
{
    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//------------------------------------------------------------------
//  デバッグ用メイン関数
//------------------------------------------------------------------
void DebugOhnoInit(u32 heap_id)
{
	DEBUG_OHNO_CONTROL * ctrl;
	ctrl = GFL_HEAP_AllocMemory(heap_id, sizeof(DEBUG_OHNO_CONTROL));
	DebugOhnoControl = ctrl;
	ctrl->debug_heap_id = heap_id;

	ctrl->psys = GFL_PROC_SysInit(ctrl->debug_heap_id);
	GFL_PROC_SysCallProc(ctrl->psys, NO_OVERLAY_ID, &UITestProcTbl, ctrl);
	ctrl->uisys = GFL_UI_sysInit(ctrl->debug_heap_id);

    /* Vブランク割込設定 */
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

}


//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugOhnoMain(void)
{
	DEBUG_OHNO_CONTROL * ctrl = DebugOhnoControl;
	GFL_UI_sysMain(ctrl->uisys);
	GFL_PROC_SysMain(ctrl->psys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugOhnoExit(void)
{
	DEBUG_OHNO_CONTROL * ctrl = DebugOhnoControl;
	GFL_UI_sysEnd(ctrl->uisys);
	GFL_PROC_SysExit(ctrl->psys);
	GFL_HEAP_FreeMemory(ctrl);
    DebugOhnoControl = NULL;
}

