//======================================================================
/**
 * @file	 fldeff_iaigiri.h
 * @brief	 居合い切りエフェクト
 * @author obata
 * @date	 09.12.04
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

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_IAIGIRI_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_IAIGIRI_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_IAIGIRI_SetMMdl( MMDL *fmmdl, FLDEFF_CTRL *fectrl );

