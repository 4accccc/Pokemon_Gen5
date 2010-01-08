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

typedef struct ENCEFF_CNT_tag * ENCEFF_CNT_PTR;

typedef enum {
  ENCEFFID_1 = 0,
  ENCEFFID_2,
  ENCEFFID_3,
  ENCEFFID_4,
  ENCEFFID_MAX,
}ENCEFF_ID;

extern ENCEFF_CNT_PTR ENCEFF_CreateCntPtr(const HEAPID inHeapID, FIELDMAP_WORK * fieldmap);
extern void ENCEFF_DeleteCntPtr(ENCEFF_CNT_PTR ptr);
extern void ENCEFF_SetEncEff(ENCEFF_CNT_PTR ptr, GMEVENT * event, const ENCEFF_ID inID);
extern void *ENCEFF_GetWork(ENCEFF_CNT_PTR ptr);
extern void ENCEFF_Draw(ENCEFF_CNT_PTR ptr);


extern GMEVENT *ENCEFF_CreateEff1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern void ENCEFF_DrawEff1(ENCEFF_CNT_PTR ptr);
extern GMEVENT *ENCEFF_CreateEff2(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern void ENCEFF_DrawEff2(ENCEFF_CNT_PTR ptr);

extern GMEVENT *ENCEFF_PNL1_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_PNL2_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern GMEVENT *ENCEFF_PNL3_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork);
extern void ENCEFF_PNL_Draw(ENCEFF_CNT_PTR ptr);
