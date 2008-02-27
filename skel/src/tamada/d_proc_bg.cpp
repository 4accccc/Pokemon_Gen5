//============================================================================================
//
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
	DEBUG_TAMADA_CONTROL * ctrl = (DEBUG_TAMADA_CONTROL*)pwk;
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
	DEBUG_TAMADA_CONTROL * ctrl = (DEBUG_TAMADA_CONTROL*)pwk;
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
	DEBUG_TAMADA_CONTROL * ctrl = (DEBUG_TAMADA_CONTROL*)pwk;
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
typedef struct {
	u8 code;
	u8 pri;
}INIT_DATA;
static const INIT_DATA init_code[] = {
	{'A',	0 },
	{'B',	1 },
	{'C',	2 },
	{'D',	10},
	{'E',	11},
	{'F',	12},
	{'G',	20},
	{'H',	23},
};

#define	USE_TCB_SIZE	NELEMS(init_code)

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GFL_TCBLSYS * tcbsys;
	GFL_TCBL * target;
	BOOL sucide_flag;
	u8 array[USE_TCB_SIZE];
	u8 old_array[USE_TCB_SIZE];
}MYWORK3;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	u8 code;
	MYWORK3 * oya;
}TWK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static void FillArray(MYWORK3 * m3, const u8 code)
{
	int count = 0;
	u8 * array = m3->array;
	while (*array != '0') {
		array ++;
		count ++;
		GF_ASSERT(count < USE_TCB_SIZE);
	}
	*array = code;
}
static void TaskFunc(GFL_TCBL * tcb, void * work);

static void Init(MYWORK3 * m3)
{
	int i;
	GFL_TCBL * tcb;
	TWK * twk;
	for (i = 0; i < USE_TCB_SIZE; i++) {
		tcb = GFL_TCBL_Create(m3->tcbsys, TaskFunc, sizeof(TWK), init_code[i].pri);
		twk = (TWK*)GFL_TCBL_GetWork(tcb);
		twk->code = init_code[i].code;
		twk->oya = m3;
		OS_TPrintf("ADDRESS TCB:%08x WORK:%08x\n", tcb, twk);
	}
	m3->target = tcb;
	OS_TPrintf("ADDRESS TCB:%08x\n", tcb);

	GFL_STD_MemFill(m3->array, USE_TCB_SIZE, '0');
	GFL_STD_MemFill(m3->old_array, USE_TCB_SIZE, '0');
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void TaskFunc(GFL_TCBL * tcb, void * work)
{
	TWK * wk = (TWK*)work;
	FillArray(wk->oya, wk->code);
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcInit3(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	MYWORK3 * m3;
	DEBUG_TAMADA_CONTROL * ctrl = (DEBUG_TAMADA_CONTROL*)pwk;
	GFL_STD_MtRandInit(0);
	m3 = (MYWORK3*)GFL_PROC_AllocWork(proc, sizeof(MYWORK3), ctrl->debug_heap_id);
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
	DEBUG_TAMADA_CONTROL * ctrl = (DEBUG_TAMADA_CONTROL*)pwk;
	MYWORK3 * m3 = (MYWORK3*)mywk;

	switch (*seq) {
	case 0:
		Init(m3);
		(*seq) ++;
		break;
	case 1:
		if (GFL_STD_MemComp(m3->array, m3->old_array, USE_TCB_SIZE) != 0) {
			int i;
			OS_PutString("Array[");
			for (i = 0; i < USE_TCB_SIZE; i++) {
				OS_TPrintf("%c", m3->array[i]);
			}
			OS_PutString("]\n");
		}
		GFL_STD_MemCopy(m3->array, m3->old_array, USE_TCB_SIZE);
		GFL_STD_MemFill(m3->array, '0', USE_TCB_SIZE);

		GFL_TCBL_Main(m3->tcbsys);

		if (GFL_UI_KEY_GetTrg() & PAD_KEY_UP) {
			u32 pri = GFL_TCBL_GetPriority(m3->target);
			OS_TPrintf("ADDRESS TCB:%08x\n", m3->target);
			if (pri > 0) {
				GFL_TCBL_ChangePriority(m3->target, pri - 1);
				OS_Printf("pri = %d\n", pri - 1);
			}
		} else if (GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN) {
			u32 pri = GFL_TCBL_GetPriority(m3->target);
			OS_TPrintf("ADDRESS TCB:%08x\n", m3->target);
			if (pri < 30) {
				GFL_TCBL_ChangePriority(m3->target, pri + 1);
				OS_Printf("pri = %d\n", pri + 1);
			}
		}
	}
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugTamadaSubProcEnd3(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_TAMADA_CONTROL * ctrl = (DEBUG_TAMADA_CONTROL*)pwk;
	MYWORK3 * m3 = (MYWORK3*)mywk;

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

