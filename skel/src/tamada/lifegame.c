//============================================================================================
/**
 * @file	lifegame.c
 * @author	tamada GAME FREAK inc.
 * @date	2007.02.13
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"

#include "lifegame.h"


//============================================================================================
//
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
struct _LG_CELL {
	int lifetime;
	BOOL life;
	int count;
};

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
struct _LIFEGAME_WORK {
	int sizeX;
	int sizeY;
	LG_CELL * cell;
};


//============================================================================================
//
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
LIFEGAME_WORK * LifeGame_Init(HEAPID heap_id, int sizeX, int sizeY)
{
	LIFEGAME_WORK * lfwk = GFL_HEAP_AllocMemory(heap_id, sizeof(LIFEGAME_WORK));
	lfwk->sizeX = sizeX;
	lfwk->sizeY = sizeY;
	lfwk->cell = GFL_HEAP_AllocMemory(heap_id, sizeof(u8) * sizeX * sizeY);
	GFL_STD_MemFill(lfwk->cell, 0, sizeof(u8) * sizeX * sizeY);
	return lfwk;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void SetCell(LG_CELL * cell)
{
	cell->lifetime = 0;
	cell->life = TRUE;
	cell->count = 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ResetCell(LG_CELL * cell)
{
	cell->lifetime = 0;
	cell->life = FALSE;
	cell->count = 0;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void LifeGame_SetStartPattern(LIFEGAME_WORK * lfwk, const u8 * pattern,
		u32 sizeX, u32 sizeY, u32 posX, u32 posY)
{
	int x,y;
	GF_ASSERT(posX + sizeX < lfwk->sizeX);
	GF_ASSERT(posY + sizeY < lfwk->sizeY);

	for (x = posX; x < posX + sizeX; x ++) {
		for (y = posY; y < posY + sizeY; y ++) {
			if (pattern[x + y * sizeX]) {
				SetCell(lfwk->cell);
			} else {
				ResetCell(lfwk->cell);
			}
		}
	}
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void LifeGame_Main(LIFEGAME_WORK * lfwk)
{
	int x, y;
	int sx, sy;
	int count;

	for (x = 0; x < lfwk->sizeX; x ++) {
		for (y = 0; y < lfwk->sizeY; y ++) {
			count = 0;
			for (sx = -1; sx <= 1; sx ++) {
				for (sy = -1; sy <= 1; sy ++) {
					if (sx + x >= 0
							&& sx + x < lfwk->sizeX
							&& sy + y >= 0
							&& sy + y < lfwk->sizeY
							&& lfwk->cell[sx + x + (sy + y) * lfwk->sizeX].life
					   ) {
						count ++;
					}
				}
			}
			lfwk->cell[x + y * lfwk->sizeX].count = count;
		}
	}

	for (x = 0; x < lfwk->sizeX; x ++) {
		for (y = 0; y < lfwk->sizeY; y ++) {
			LG_CELL * cell = &lfwk->cell[x + y * lfwk->sizeX];
			if (cell->life) {
				if (cell->count == 2 || cell->count == 3) {
					cell->life = TRUE;
					cell->lifetime ++;
				} else {
					cell->life = FALSE;
					cell->lifetime = 0;
				}
			} else {
				if (cell->count == 3) {
					cell->life = TRUE;
					cell->lifetime ++;
				} else {
					cell->life = FALSE;
					cell->lifetime = 0;
				}
			}
		}
	}
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void LifeGame_Exit(LIFEGAME_WORK * lfwk)
{
	GFL_HEAP_FreeMemory(lfwk->cell);
	GFL_HEAP_FreeMemory(lfwk);
}

//============================================================================================
//============================================================================================





