//============================================================================================
/**
 * @file	debug_tamada.c
 * @brief	デバッグ＆テスト用アプリ（玉田専用）
 * @author	tamada
 * @date	2006.11.28
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "tamada/debug_tamada.h"
#include "tamada/bg/debug_graphic.h"

#include "tamada_internal.h"

//============================================================================================
//============================================================================================

static DEBUG_TAMADA_CONTROL * DebugTamadaControl;






//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void GFL_INTR_SetVblankFunc(GFL_INTR_FUNC func, void * work)
{
	DEBUG_TAMADA_CONTROL * ctrl = DebugTamadaControl;
	ctrl->vblank_func = func;
	ctrl->vblank_work = work;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void VBlankIntr(void)
{
	DEBUG_TAMADA_CONTROL * ctrl = DebugTamadaControl;
	if (ctrl->vblank_func) {
		ctrl->vblank_func(ctrl->vblank_work);
	}

    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugTamadaInit(u32 heap_id)
{
	DEBUG_TAMADA_CONTROL * ctrl;
	ctrl = GFL_HEAP_AllocMemory(heap_id, sizeof(DEBUG_TAMADA_CONTROL));
	DebugTamadaControl = ctrl;
	ctrl->debug_heap_id = heap_id;

	//GFL_PROC_SysInit(ctrl->debug_heap_id);
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugTamadaMainProcData, ctrl);
	//ctrl->uisys = GFL_UI_sysInit(ctrl->debug_heap_id);

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
void DebugTamadaMain(void)
{
	//GFL_UI_sysMain();
	//GFL_PROC_SysMain();
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugTamadaExit(void)
{
	//GFL_UI_sysEnd();
	//GFL_PROC_SysExit();
	GFL_HEAP_FreeMemory(DebugTamadaControl);
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	//DEBUG_TAMADA_CONTROL * ctrl = pwk;
	DEBUG_TAMADA_CONTROL * ctrl;

	GFL_HEAP_CreateHeap(GFL_HEAPID_APP, HEAPID_TAMADA_DEBUG, 0x40000);
	//ctrl = GFL_HEAP_AllocMemory(heap_id, sizeof(DEBUG_TAMADA_CONTROL));
	ctrl = GFL_PROC_AllocWork(proc, sizeof(DEBUG_TAMADA_CONTROL), HEAPID_TAMADA_DEBUG);
	DebugTamadaControl = ctrl;
	ctrl->debug_heap_id = HEAPID_TAMADA_DEBUG;

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_PROC * subproc;
	DEBUG_TAMADA_CONTROL * ctrl = mywk;
	//DEBUG_TAMADA_CONTROL * ctrl = pwk;
	int key = GFL_UI_KeyGetTrg();

	switch (*seq) {
	case 0:
		Debug_GraphicInit();
		Debug_GraphicPut(0);
		(*seq) ++;
		break;
	case 1:
		if ((GFL_UI_KeyGetCont() & (PAD_BUTTON_L|PAD_BUTTON_R)) == (PAD_BUTTON_L|PAD_BUTTON_R)) {
			return GFL_PROC_RES_FINISH;

		} else if (key & PAD_BUTTON_A) {
			GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugTamadaSubProcData1, ctrl);
			*seq = 0;
			return GFL_PROC_RES_CONTINUE;
		} else if (key & PAD_BUTTON_B){
			GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugTamadaSubProcData2, ctrl);
			*seq = 0;
			return GFL_PROC_RES_CONTINUE;
		} else if (key & PAD_BUTTON_X) {
			GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DebugTamadaSubProcData3, ctrl);
			*seq = 0;
			return GFL_PROC_RES_CONTINUE;
		} else if (key & PAD_BUTTON_START) {
			GFL_PROC_SysCallProc(NO_OVERLAY_ID, &OamKeyDemoProcData, ctrl);
			*seq = 0;
			return GFL_PROC_RES_CONTINUE;
		}
		break;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_DeleteHeap(HEAPID_TAMADA_DEBUG);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugTamadaMainProcData = {
	DebugTamadaMainProcInit,
	DebugTamadaMainProcMain,
	DebugTamadaMainProcEnd,
};

