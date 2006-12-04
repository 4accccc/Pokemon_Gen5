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
	if (GFL_UI_KeyGetTrg(ctrl->uisys) & PAD_BUTTON_A) {
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
	if (GFL_UI_KeyGetTrg(ctrl->uisys) & PAD_BUTTON_A) {
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

