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
extern void ENCEFF_FreeUserWork(ENCEFF_CNT_PTR ptr);
extern void *ENCEFF_GetUserWorkPtr(ENCEFF_CNT_PTR ptr);

extern GMEVENT *ENCEFF_CI_CreateRival(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateSupport(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym01A(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym01B(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym01C(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym02(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym03(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym04(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym05(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym06(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym07(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym08A(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateGym08B(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateBigFour1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateBigFour2(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateBigFour3(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateBigFour4(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateChamp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateBoss(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreateSage(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_CI_CreatePlasma(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);

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

