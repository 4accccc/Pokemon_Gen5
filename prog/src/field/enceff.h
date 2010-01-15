//======================================================================
/*
 * @file	enceff.h
 * @brief	エンカウントエフェクト
 * @author saito
 */
//======================================================================

#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"

#include "enceffno_def.h"

typedef struct ENCEFF_CNT_tag * ENCEFF_CNT_PTR;

extern ENCEFF_CNT_PTR ENCEFF_CreateCntPtr(const HEAPID inHeapID, FIELDMAP_WORK * fieldmap);
extern void ENCEFF_DeleteCntPtr(ENCEFF_CNT_PTR ptr);
extern void ENCEFF_SetEncEff(ENCEFF_CNT_PTR ptr, GMEVENT * event, const ENCEFF_ID inID);
extern void *ENCEFF_GetWork(ENCEFF_CNT_PTR ptr);
extern void ENCEFF_Draw(ENCEFF_CNT_PTR ptr);

extern void *ENCEFF_AllocUserWork(ENCEFF_CNT_PTR ptr, const int size, const HEAPID inHeapID);
extern void *ENCEFF_GetUserWorkPtr(ENCEFF_CNT_PTR ptr);

extern GMEVENT *ENCEFF_CI_Create1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);

extern GMEVENT *ENCEFF_MDL_Create1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern void ENCEFF_MDL_Draw(ENCEFF_CNT_PTR ptr);

extern GMEVENT *ENCEFF_WAV_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern void ENCEFF_WAV_Draw(ENCEFF_CNT_PTR ptr);

extern GMEVENT *ENCEFF_PNL1_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_PNL2_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_PNL3_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern void ENCEFF_PNL_Draw(ENCEFF_CNT_PTR ptr);

extern void ENCEFF_LoadPanelEffOverlay(ENCEFF_CNT_PTR ptr);
extern void ENCEFF_LoadVertexEffOverlay(ENCEFF_CNT_PTR ptr);
extern void ENCEFF_UnloadEffOverlay(ENCEFF_CNT_PTR ptr);

