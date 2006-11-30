//============================================================================================
/**
 * @file	debug_graphic.c
 * @author	tamada GAME FREAK inc.
 * @date	06.11.27
 */
//============================================================================================
#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"
#include "procsys.h"

#include "tamada/tamada_internal.h"
#include "tamada/bg/debug_graphic.h"
#include "tamada/bg/loader.h"

#include "tamada/bg/base_color.dat"
#include "tamada/bg/base_chr.dat"
#include "tamada/bg/bg00.dat"
#include "tamada/bg/bg01.dat"
#include "tamada/bg/bg02.dat"
#include "tamada/bg/bg03.dat"
#include "tamada/bg/bg04.dat"
#include "tamada/bg/bg05.dat"
#include "tamada/bg/bg06.dat"
#include "tamada/bg/bg07.dat"
#include "tamada/bg/bg08.dat"


typedef struct {
	const u16 * screen_data;
	u32 size;
}DEBUG_CG_DATA;

static const DEBUG_CG_DATA DebugCgData[] = {
	{ bg00_Screen, sizeof(bg00_Screen), },
	{ bg01_Screen, sizeof(bg01_Screen), },
	{ bg02_Screen, sizeof(bg02_Screen), },
	{ bg03_Screen, sizeof(bg03_Screen), },
	{ bg04_Screen, sizeof(bg04_Screen), },
	{ bg05_Screen, sizeof(bg05_Screen), },
	{ bg06_Screen, sizeof(bg06_Screen), },
	{ bg07_Screen, sizeof(bg07_Screen), },
	{ bg08_Screen, sizeof(bg08_Screen), },
};

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void Debug_GraphicInit(void)
{
  LDR_Init();
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void Debug_GraphicPut(int id)
{
	const DEBUG_CG_DATA * cg;
	if (id >= NELEMS(DebugCgData)) {
		id = 0;
	}
	cg = &DebugCgData[id];

  LDR_Trans(
		  base_chr_Character, sizeof(base_chr_Character),
		  cg->screen_data, cg->size,
		  base_color_Palette, sizeof(base_color_Palette), TRUE
		  );
}

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
	Debug_GraphicPut(0);
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
	Debug_GraphicPut(0);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugTamadaSubProcData2 = {
	DebugTamadaSubProcInit2,
	DebugTamadaSubProcMain2,
	DebugTamadaSubProcEnd2,
};

