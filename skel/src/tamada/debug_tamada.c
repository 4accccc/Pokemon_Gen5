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

#include "main.h"
#include "tamada/debug_graphic.h"
#include "tamada/debug_tamada.h"
#include "ui.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	u32 debug_heap_id;
	UISYS * uisys;
	GFL_PROCSYS * psys;
}DEBUG_TAMADA_CONTROL;


static DEBUG_TAMADA_CONTROL * DebugTamadaControl;




static const GFL_PROC_DATA DebugTamadaMainProcData1;
static const GFL_PROC_DATA DebugTamadaSubProcData1;
static const GFL_PROC_DATA DebugTamadaSubProcData2;


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void VBlankIntr(void)
{
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

	ctrl->psys = GFL_PROC_SysInit(ctrl->debug_heap_id);
	GFL_PROC_SysCallProc(ctrl->psys, &DebugTamadaMainProcData1, ctrl);
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
void DebugTamadaMain(void)
{
	GFL_UI_sysMain(DebugTamadaControl->uisys);
	GFL_PROC_SysMain(DebugTamadaControl->psys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void DebugTamadaExit(void)
{
	GFL_UI_sysEnd(DebugTamadaControl->uisys);
	GFL_PROC_SysExit(DebugTamadaControl->psys);
	GFL_HEAP_FreeMemory(DebugTamadaControl);
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaMainProcInit1(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	DEBUG_TAMADA_CONTROL * ctrl = p_work;
	Debug_GraphicInit();
	Debug_GraphicPut(0);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaMainProcMain1(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	GFL_PROC * subproc;
	DEBUG_TAMADA_CONTROL * ctrl = p_work;
	if (GFL_UI_KeyGetTrg(ctrl->uisys) & PAD_BUTTON_A) {
		GFL_PROC_SysCallProc(ctrl->psys, &DebugTamadaSubProcData1, ctrl);
		return GFL_PROC_RES_CONTINUE;
	} else if (GFL_UI_KeyGetTrg(ctrl->uisys) & PAD_BUTTON_B){
		GFL_PROC_SysCallProc(ctrl->psys, &DebugTamadaSubProcData2, ctrl);
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaMainProcEnd1(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const GFL_PROC_DATA DebugTamadaMainProcData1 = {
	DebugTamadaMainProcInit1,
	DebugTamadaMainProcMain1,
	DebugTamadaMainProcEnd1,
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcInit1(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	Debug_GraphicInit();
	Debug_GraphicPut(1);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcMain1(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	DEBUG_TAMADA_CONTROL * ctrl = p_work;
	if (GFL_UI_KeyGetTrg(ctrl->uisys) & PAD_BUTTON_A) {
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcEnd1(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	Debug_GraphicPut(0);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const GFL_PROC_DATA DebugTamadaSubProcData1 = {
	DebugTamadaSubProcInit1,
	DebugTamadaSubProcMain1,
	DebugTamadaSubProcEnd1,
};


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcInit2(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	Debug_GraphicInit();
	Debug_GraphicPut(2);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcMain2(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	DEBUG_TAMADA_CONTROL * ctrl = p_work;
	if (GFL_UI_KeyGetTrg(ctrl->uisys) & PAD_BUTTON_A) {
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcEnd2(GFL_PROC * proc, int * seq, void * p_work, void * my_work)
{
	Debug_GraphicPut(0);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const GFL_PROC_DATA DebugTamadaSubProcData2 = {
	DebugTamadaSubProcInit2,
	DebugTamadaSubProcMain2,
	DebugTamadaSubProcEnd2,
};

