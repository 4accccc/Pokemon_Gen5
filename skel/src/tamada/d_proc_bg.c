//============================================================================================
/**
 * @file	d_proc_bg.c
 * @author	tamada GAME FREAK inc.
 * @date	06.11.30
 */
//============================================================================================
#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"
#include "procsys.h"
#include "tcbl.h"

#include "tamada/tamada_internal.h"
#include "tamada/bg/debug_graphic.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcInit1(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	Debug_GraphicInit();
	Debug_GraphicPut(1);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcMain1(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_TAMADA_CONTROL * ctrl = pwk;
	if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_A) {
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcEnd1(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_TAMADA_CONTROL * ctrl = pwk;
	GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &DebugTamadaSubProcData2, ctrl);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugTamadaSubProcData1 = {
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
static GFL_PROC_RESULT DebugTamadaSubProcInit2(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	Debug_GraphicInit();
	Debug_GraphicPut(2);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcMain2(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_TAMADA_CONTROL * ctrl = pwk;
	if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_A) {
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcEnd2(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugTamadaSubProcData2 = {
	DebugTamadaSubProcInit2,
	DebugTamadaSubProcMain2,
	DebugTamadaSubProcEnd2,
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GFL_TCBLSYS * tcbsys;
}MYWORK3;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int counter;
}TWK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static void TaskFunc(GFL_TCBL * tcb, void * work)
{
	TWK * wk = work;
	(wk->counter) --;
	if (wk->counter == 0) {
		OS_Printf("removed\n");
		GFL_TCBL_Delete(tcb);
	}
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcInit3(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	MYWORK3 * m3;
	DEBUG_TAMADA_CONTROL * ctrl = pwk;
	GFL_STD_MtRandInit(0);
	m3 = GFL_PROC_AllocWork(proc, sizeof(MYWORK3), ctrl->debug_heap_id);
	m3->tcbsys = GFL_TCBL_Init(ctrl->debug_heap_id, ctrl->debug_heap_id, 32, sizeof(TWK));
	Debug_GraphicInit();
	Debug_GraphicPut(2);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcMain3(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_TAMADA_CONTROL * ctrl = pwk;
	MYWORK3 * m3 = mywk;

	GFL_TCBL_Main(m3->tcbsys);
	if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_A) {
		return GFL_PROC_RES_FINISH;
	}
	if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B) {
		GFL_TCBL * tcb;
		TWK * twk;
		tcb = GFL_TCBL_Create(m3->tcbsys, TaskFunc, sizeof(TWK), GFL_STD_MtRand(100) + 1);
		twk = GFL_TCBL_GetWork(tcb);
		twk->counter = GFL_STD_MtRand(20) * 10 + 20;
		
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcEnd3(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_TAMADA_CONTROL * ctrl = pwk;
	MYWORK3 * m3 = mywk;

	GFL_TCBL_Exit(m3->tcbsys);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugTamadaSubProcData3 = {
	DebugTamadaSubProcInit3,
	DebugTamadaSubProcMain3,
	DebugTamadaSubProcEnd3,
};

