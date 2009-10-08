//======================================================================
/**
 * @file	fldeff_ripple.h
 * @brief	水波紋エフェクト
 * @author	kagaya
 * @date	05.07.13
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
extern void * FLDEFF_RIPPLE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_RIPPLE_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_RIPPLE_Set( FLDEFF_CTRL *fectrl, s16 gx, s16 gz, fx32 y );
