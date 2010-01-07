//======================================================================
/*
 * @file	enceff_prg.h
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"

typedef void (*DRAW_FUNC)(void*);
typedef GMEVENT* (*CREATE_FUNC)(GAMESYS_WORK *);

typedef struct ENCEFF_PRG_WORK_tag * ENCEFF_PRG_PTR;

extern GMEVENT *ENCEFF_PRG_Create(
    GAMESYS_WORK *gsys,
    const VecFx32 *inCamPos,
    CREATE_FUNC createFunc,
    DRAW_FUNC drawFunc);

extern void ENCEFF_PRG_Draw(ENCEFF_PRG_PTR ptr);


