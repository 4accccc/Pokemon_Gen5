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

#include "tamada/debug_graphic.h"
#include "tamada/loader.h"

#include "tamada/base_color.dat"
#include "tamada/base_chr.dat"
#include "tamada/bg00.dat"
#include "tamada/bg01.dat"
#include "tamada/bg02.dat"
#include "tamada/bg03.dat"
#include "tamada/bg04.dat"
#include "tamada/bg05.dat"
#include "tamada/bg06.dat"
#include "tamada/bg07.dat"
#include "tamada/bg08.dat"


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

