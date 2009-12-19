//======================================================================
/**
 * @file	fldeff_fishing.h
 * @brief	釣り用エフェクト
 * @author	iwasawa
 * @date	09.12.18
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

//======================================================================
//  define
//======================================================================

typedef enum{
 FISHING_LURE_ANM_WAIT,
 FISHING_LURE_ANM_HIT,
}FISHING_LURE_ANM_ID;

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_FISHING_LURE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_FISHING_LURE_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK* FLDEFF_FISHING_LURE_Set( FLDEFF_CTRL *fectrl, VecFx32* tpos );
extern void FLDEFF_FISHING_LURE_ChangeAnime( FLDEFF_TASK* task, FISHING_LURE_ANM_ID anm_id );
