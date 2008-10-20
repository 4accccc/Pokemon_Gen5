//============================================================================================
/**
 * @file	lifegame.h
 * @author	tamada GAME FREAK inc.
 * @date	2007.02.13
 */
//============================================================================================

#ifndef	__LIFEGAME_H__
#define	__LIFEGAME_H__

#include <nitro.h>
#include <nnsys.h>

#include "gflib.h"

typedef struct _LG_CELL LG_CELL;
typedef struct _LIFEGAME_WORK LIFEGAME_WORK;
//============================================================================================
//============================================================================================
extern LIFEGAME_WORK * LifeGame_Init(HEAPID heap_id, int sizeX, int sizeY);
extern void LifeGame_Main(LIFEGAME_WORK * lfwk);
extern void LifeGame_Exit(LIFEGAME_WORK * lfwk);
extern void LifeGame_SetStartPattern(LIFEGAME_WORK * lfwk, const u8 * pattern,
		u32 sizeX, u32 sizeY, u32 posX, u32 posY);


#endif	/* __LIFEGAME_H__ */
